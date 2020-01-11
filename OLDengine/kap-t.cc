//----------------------------------------------------------------------
// kap-t.cc -- web server logic processing engine
//----------------------------------------------------------------------
// This file is part of KAP, the Kinetic Application Processor.
// Copyright (c) 1999, AsiaInfo Technologies, Inc.
//
// See the file kap-license.txt for information on usage and
// redistribution of this file, and for a DISCLAIMER OF ALL
// WARRANTIES.  Share and Enjoy!
//
// The KAP home page is: http://www.markharrison.net/kap
//----------------------------------------------------------------------

#include "filemgr.hh"
#include "sshash.h"
#include "string.h"
#include "unistd.h"
#include "cmdlock.h"

#ifndef DEBUG
	#define dbg if(0) printf
#else
	#define dbg printf
#endif

// 8.0 uses internal call with 1 parm
// 8.x uses public   call with 2 parms
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
    // use Tcl*
    #define TCLCHDIR(x) TclChdir(NULL, x)
    #define TCLGETCWD(x) TclGetCwd(x)
    extern "C" {
        int    TclChdir(Tcl_Interp *interp, char *dirName);
        char * TclGetCwd(Tcl_Interp *interp);
    }
#else
    // use Tclp*
    #define TCLCHDIR(x) TclpChdir(x)
    #define TCLGETCWD(x) TclpGetCwd(x, NULL)
    extern "C" {
        int    TclpChdir(char *dirName);
        char * TclpGetCwd(Tcl_Interp *interp,Tcl_DString *bufferPtr);
    }
#endif

#define KINETIC_ID_STRING "AI-Kinetic/1.2"

#define DECLARE_CMD(name) \
    static int name##Cmd(ClientData, Tcl_Interp*, int, char**); \
    int name##Method(Tcl_Interp* interp, int argc, char**argv);

#define DECLARE_OBJ_CMD(name) \
    static int name##ObjCmd(ClientData, Tcl_Interp*, int, char**); \
    int name##ObjMethod(Tcl_Interp* interp, int argc, char**argv);

////////////////////////////////////////////////////////////////
// class Kaptrans -- appserver translator class
////////////////////////////////////////////////////////////////

class Kaptrans {
private:
    FILE* fin;			// input file pointer
    FILE* fout;			// output file pointer
    char  initfile[256];	// initialization file
    FileMgr* fmgr;		// file manager
    Tcl_Interp* interp;		// logic engine interpreter
    Tcl_Channel mychannel;	// the output channel
    SShash cgikeys;		// CGI Keywords

    int haveflushed;		// 1 = headers flushed already
    char statusbuf[1000];	// http status (200,404,...)
    char headerbuf[5000];	// http headers

    void exterr(char* name, int code1, int code2,
                 char* summary, char* detail);
    void httphead();
    void httpredirect(char* url);
    void headerflush();
    void chdir2(char*fname);
    void parsecookies(Tcl_Interp* interp, char* val);
    void parse_vars(char* instr, char* var, char* val);
    int process_file(char* req);
    void parsequery(Tcl_Interp* theinterp, char* val);
    void flushme(void);
    char* mfgets(char*buf, int len, FILE* fp);

public:
    Kaptrans(FILE* _fin, FILE* _fout, int maxcache, char* ifile);
    DECLARE_CMD(loadpage)
    DECLARE_CMD(redirect)
    static int putsObjCmd(ClientData, Tcl_Interp*, int, Tcl_Obj *CONST objv[]);
    int putsObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj *CONST objv[]);
    static int setcookieObjCmd(ClientData, Tcl_Interp*, int,
                               Tcl_Obj *CONST objv[]);
    int setcookieObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj *CONST objv[]);
    static int setheaderObjCmd(ClientData, Tcl_Interp*, int,
                               Tcl_Obj *CONST objv[]);
    int setheaderObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj *CONST objv[]);
    static int setstatusObjCmd(ClientData, Tcl_Interp*, int,
                               Tcl_Obj *CONST objv[]);
    int setstatusObjMethod(Tcl_Interp* interp, int objc, Tcl_Obj *CONST objv[]);
    int process(void);
    int compile(char* path);
};

////////////////////////////////////////////////////////////////
// Kaptrans::loadpageMethod
////////////////////////////////////////////////////////////////

int Kaptrans::loadpageMethod(Tcl_Interp* theinterp, int argc, char**argv)
{
    if (argc != 2) {
        Tcl_SetResult(theinterp, "", TCL_STATIC);
        Tcl_AppendResult(theinterp, "wrong # args: should be \"",
                         argv[0], " filename\"", 0);
        return TCL_ERROR;
    }
    else {
        Tcl_SetResult(theinterp, "", TCL_STATIC);

        // we need to build a path relative to the file being
        // processed.  So if the file /foo/bar/something.ktcl
        // has "loadpage other.ktcl" then the file loaded is
        // /foo/bar/other.ktcl.
        //
        // we also need to set the working directory.

        char olddirpath[1000];
        char* transpath = Tcl_GetVar2(theinterp, "CGI", "PATH_TRANSLATED", 0);
        char* filepath = argv[1];
        char* path;
        char* lastslash;

        if (transpath == NULL)
            transpath = (char *)"./foo";

        path = (char*)malloc(strlen(transpath) + strlen(filepath) + 5);
        strcpy(path, transpath);
        lastslash = strrchr(path, '/');
        strcpy(lastslash + 1, filepath);
    
        strcpy(olddirpath, TCLGETCWD(theinterp));

        chdir2(path);

        //TODO: same validations as mainline
        // maybe make mainline call this function
        int rc;
        char* rcerrmsg;
        char* rcdetail;
        if ((rc = fmgr->load(path, rcerrmsg, rcdetail)) != 0) {
            exterr(argv[1], 1, rc, rcerrmsg, rcdetail);
        }
        else {
            Tcl_Eval(theinterp, path);
        }
        free(path);
      	TCLCHDIR(olddirpath);

        return TCL_OK;
    }
}

////////////////////////////////////////////////////////////////
// Kaptrans::loadpageCmd
////////////////////////////////////////////////////////////////

int Kaptrans::loadpageCmd(ClientData cd, Tcl_Interp* interp,
                                         int argc, char**argv)
{
    Kaptrans* myobj;
    myobj = (Kaptrans*)cd;
    return myobj->loadpageMethod(interp, argc, argv);
}

////////////////////////////////////////////////////////////////
// Kaptrans::redirectMethod
////////////////////////////////////////////////////////////////

int Kaptrans::redirectMethod(Tcl_Interp* theinterp, int argc, char**argv)
{
    if (argc != 2) {
        Tcl_SetResult(theinterp, "", TCL_STATIC);
        Tcl_AppendResult(theinterp, "wrong # args: should be \"",
                         argv[0], " location\"", 0);
        return TCL_ERROR;
    }
    else {
        Tcl_SetResult(theinterp, "", TCL_STATIC);
        httpredirect(argv[1]);
        return TCL_OK;
    }
}

////////////////////////////////////////////////////////////////
// Kaptrans::redirectCmd
////////////////////////////////////////////////////////////////

int Kaptrans::redirectCmd(ClientData cd, Tcl_Interp* interp,
                                         int argc, char**argv)
{
    Kaptrans* myobj;
    myobj = (Kaptrans*)cd;
    return myobj->redirectMethod(interp, argc, argv);
}

////////////////////////////////////////////////////////////////
// Kaptrans::putsObjMethod
////////////////////////////////////////////////////////////////

extern "C" {

int Tcl_PutsObjCmd( ClientData dummy, Tcl_Interp *interp,
                int objc, Tcl_Obj *CONST objv[]);
}

int Kaptrans::putsObjMethod(Tcl_Interp *myinterp,
                int objc, Tcl_Obj *CONST objv[])
{
    char* cp;
    int clen;

    cp = Tcl_GetStringFromObj(objv[objc-1], &clen);

    if (clen > 0) {
        headerflush();
    }
    return Tcl_PutsObjCmd(0, myinterp, objc, objv);
}


////////////////////////////////////////////////////////////////
// Kaptrans::putsObjCmd
////////////////////////////////////////////////////////////////

int Kaptrans::putsObjCmd(ClientData cd, Tcl_Interp* interp,
                           int objc, Tcl_Obj *CONST objv[])
{
    Kaptrans* myobj;
    myobj = (Kaptrans*)cd;
    return myobj->putsObjMethod(interp, objc, objv);
}


////////////////////////////////////////////////////////////////
// Kaptrans::setcookieObjMethod
////////////////////////////////////////////////////////////////

int Kaptrans::setcookieObjMethod(Tcl_Interp *myinterp,
                int objc, Tcl_Obj *CONST objv[])
{
    if (objc != 5) {
        char* cmdname    = Tcl_GetStringFromObj(objv[0], 0);
        Tcl_SetResult(myinterp, "", TCL_STATIC);
        Tcl_AppendResult(myinterp, "wrong # args: should be \"",
                         cmdname, " name value domain expires\"", 0);
        return TCL_ERROR;
    }
    int domainlen;
    int expireslen;
    char* name    = Tcl_GetStringFromObj(objv[1], 0);
    char* value   = Tcl_GetStringFromObj(objv[2], 0);
    char* domain  = Tcl_GetStringFromObj(objv[3], &domainlen);
    char* expires = Tcl_GetStringFromObj(objv[4], &expireslen);

    char tmp[5000];
    sprintf(tmp, "Set-Cookie: %s=%s;", name, value);
    strcat(headerbuf, tmp);
    if (expireslen > 0) {
        sprintf(tmp, " expires=%s;", expires);
        strcat(headerbuf, tmp);
    }
    if (domainlen > 0) {
        sprintf(tmp, " domain=%s;", domain);
        strcat(headerbuf, tmp);
    }
    strcat(headerbuf, "\n");

    return TCL_OK;
}

////////////////////////////////////////////////////////////////
// Kaptrans::setcookieObjCmd
////////////////////////////////////////////////////////////////

int Kaptrans::setcookieObjCmd(ClientData cd, Tcl_Interp* interp,
                           int objc, Tcl_Obj *CONST objv[])
{
    Kaptrans* myobj;
    myobj = (Kaptrans*)cd;
    return myobj->setcookieObjMethod(interp, objc, objv);
}

////////////////////////////////////////////////////////////////
// Kaptrans::setstatusObjMethod
////////////////////////////////////////////////////////////////

int Kaptrans::setstatusObjMethod(Tcl_Interp *myinterp,
                int objc, Tcl_Obj *CONST objv[])
{
    if (objc != 2) {
        char* cmdname    = Tcl_GetStringFromObj(objv[0], 0);
        Tcl_SetResult(myinterp, "", TCL_STATIC);
        Tcl_AppendResult(myinterp, "wrong # args: should be \"",
                         cmdname, " status\"", 0);
        return TCL_ERROR;
    }
    char* status    = Tcl_GetStringFromObj(objv[1], 0);
    sprintf(statusbuf, "HTTP/1.1 %s", status);
    return TCL_OK;
}

////////////////////////////////////////////////////////////////
// Kaptrans::setstatusObjCmd
////////////////////////////////////////////////////////////////

int Kaptrans::setstatusObjCmd(ClientData cd, Tcl_Interp* interp,
                           int objc, Tcl_Obj *CONST objv[])
{
    Kaptrans* myobj;
    myobj = (Kaptrans*)cd;
    return myobj->setstatusObjMethod(interp, objc, objv);
}

////////////////////////////////////////////////////////////////
// Kaptrans::setheaderObjMethod
////////////////////////////////////////////////////////////////

int Kaptrans::setheaderObjMethod(Tcl_Interp *myinterp,
                int objc, Tcl_Obj *CONST objv[])
{
    if (objc != 3) {
        char* cmdname    = Tcl_GetStringFromObj(objv[0], 0);
        Tcl_SetResult(myinterp, "", TCL_STATIC);
        Tcl_AppendResult(myinterp, "wrong # args: should be \"",
                         cmdname, " name value\"", 0);
        return TCL_ERROR;
    }
    char* name    = Tcl_GetStringFromObj(objv[1], 0);
    char* value   = Tcl_GetStringFromObj(objv[2], 0);
    char tmp[5000];
    sprintf(tmp, "%s: %s\n", name, value);
    strcat(headerbuf, tmp);
    return TCL_OK;
}


////////////////////////////////////////////////////////////////
// Kaptrans::setheaderObjCmd
////////////////////////////////////////////////////////////////

int Kaptrans::setheaderObjCmd(ClientData cd, Tcl_Interp* interp,
                           int objc, Tcl_Obj *CONST objv[])
{
    Kaptrans* myobj;
    myobj = (Kaptrans*)cd;
    return myobj->setheaderObjMethod(interp, objc, objv);
}

////////////////////////////////////////////////////////////////
// Kaptrans -- constructor
////////////////////////////////////////////////////////////////

static char Kapinitscript[] = "\
namespace eval ::kap {\n\
\n\
    proc source {fname} {\n\
        uplevel #0 source $fname\n\
    }\n\
\n\
    set alphanumeric    a-zA-Z0-9\n\
\n\
    for {set i 1} {$i <= 256} {incr i} {\n\
        set c [format %c $i]\n\
        if {![string match \\[$alphanumeric\\] $c]} {\n\
            set ::kap::formMap($c) %[format %.2x $i]\n\
        }\n\
    }\n\
    # These are handled specially\n\
    array set ::kap::formMap {\n\
        \" \" +   \\n %0d%0a\n\
    }\n\
\n\
    proc unescapecgi {buf} {\n\
        regsub -all {\\+} $buf { } buf\n\
        regsub -all {([\\\\[\"$])} $buf {\\\\\\1} buf\n\
        regsub -all -nocase \"%0d%0a\" $buf \"\\n\" buf\n\
        regsub -all -nocase {%([a-f0-9][a-f0-9])} $buf {[format %c 0x\\1]} buf\n\
\n\
        eval return \\\"$buf\\\"\n\
    }\n\
\n\
    proc escapecgi {string} {\n\
        set alphanumeric        a-zA-Z0-9\n\
        regsub -all \\[^$alphanumeric\\] $string {$::kap::formMap(&)} string\n\
        regsub -all \\n $string {\\\\n} string\n\
        regsub -all \\t $string {\\\\t} string\n\
        regsub -all {[][{})\\\\]\\)} $string {\\\\&} string\n\
        return [subst $string]\n\
    }\n\
\n\
    proc about {} {\n\
        append s {<h1>About KAP</h1>}\n\
        append s {<h2>Copyright</h2>}\n\
        append s {Copyright 1999, AsiaInfo, Inc.}\n\
        append s {<h2>File Revisions</h2>}\n\
        append s \"kap-t.cc $::kap::revisions(kap-t.cc)<br>\"\n\
        append s {}\n\
        append s {}\n\
        append s {}\n\
        return $s\n\
\n\
    }\n\
    proc credits {} {\n\
        append s {<i><b>KAP</i> Credits:</b><p>}\n\
        append s {<i>Programming:</i> }\n\
        append s {Mark Harrison, Yan Weiliang, Liu Ligong<br>}\n\
        append s {<i>Beta Testing:</i> }\n\
        append s {Bai Xuebin, Zhang Wang, Zhang Yu<br>}\n\
        append s {<i>Ideas and Support:</i> }\n\
        append s {Zhao Yao, Larry Huang, Cindy Zhang, Yi Amin, John Ho<br>}\n\
        append s {<p>Join this list of great people by sending }\n\
        append s {your bug reports, ideas,<br>comments, etc, to }\n\
        append s {<a href=mailto:markh@usai.asiainfo.com>}\n\
        append s {markh@usai.asiainfo.com></a>!}\n\
        append s <p>\n\
        return $s\n\
    }\n\
}\n\
";

////////////////////////////////////////////////////////////////
// Kaptrans -- constructor
////////////////////////////////////////////////////////////////

Kaptrans::Kaptrans(FILE* _fin, FILE* _fout, int maxcache, char* _initfile)
{
    fin = _fin;
    fout = _fout;
    haveflushed = 0;
    *statusbuf = 0;
    *headerbuf = 0;

    strcpy(initfile, _initfile);

    // initialize Tcl interpreter
    interp = Tcl_CreateInterp();
    if (Tcl_Init(interp) == TCL_ERROR) {
        printf("Tcl_Init startup error:\n%s\n", interp->result);
    }
    ClientData cd = (ClientData)this;
    Tcl_Eval(interp, "namespace eval ::kap {}");
    Tcl_SetVar2(interp, "::kap::revisions", "kap-t.cc",
                   "$Revision: 1.6 $", TCL_GLOBAL_ONLY);

    Tcl_CreateCommand(interp, "::kap::loadpage", loadpageCmd, cd, 0);
    Tcl_CreateCommand(interp, "::kap::redirect", redirectCmd, cd, 0);
    Tcl_CreateObjCommand(interp, "::kap::setcookie", setcookieObjCmd, cd, 0);
    Tcl_CreateObjCommand(interp, "::kap::setheader", setheaderObjCmd, cd, 0);
    Tcl_CreateObjCommand(interp, "::kap::setstatus", setstatusObjCmd, cd, 0);
    Tcl_CreateObjCommand(interp, "puts", putsObjCmd, cd, 0);


    if (Tcl_Eval(interp, Kapinitscript) == TCL_ERROR) {
        printf("Initialization script error:\n%s\n", interp->result);
    }
    delete(new CmdLock(interp));

    fmgr = new FileMgr(interp, maxcache);

    // tie interpreter stdout to output fd
    int outfd;
    outfd = fileno(fout);
    mychannel = Tcl_MakeFileChannel((ClientData)outfd, TCL_WRITABLE);
    Tcl_SetStdChannel(mychannel, TCL_STDOUT);

    // source the initialization file
    char srccmd[1000];
    int srcrc;
    sprintf(srccmd, "if {[file isfile %s]} {source %s}", initfile, initfile);
    srcrc = Tcl_Eval(interp, srccmd);
    if (srcrc != TCL_OK) {
        fprintf(stderr, "error in %s: %s\n",
                      initfile, Tcl_GetStringResult(interp));
        fflush(stderr);
    }

    // initialize cgi keyword table.  this maps the netscape name
    // to the official CGI variable name. "??????" means that Mark
    // does not know the corresponding netscape keyword.
    cgikeys.ins("??????", "AUTH_TYPE");
    cgikeys.ins("content-length", "CONTENT_LENGTH");
    cgikeys.ins("content-type", "CONTENT_TYPE");
    cgikeys.ins("ntrans-base", "DOCUMENT_ROOT");
    cgikeys.ins("??????", "GATEWAY_INTERFACE");
    cgikeys.ins("accept", "HTTP_ACCEPT");
    cgikeys.ins("??????", "HTTP_FROM");
    cgikeys.ins("referer", "HTTP_REFERER");
    cgikeys.ins("user-agent", "HTTP_USER_AGENT");
    cgikeys.ins("??????", "PATH_INFO");
    cgikeys.ins("path", "PATH_TRANSLATED");
    cgikeys.ins("clf-request", "QUERY_STRING");
    cgikeys.ins("ip", "REMOTE_ADDR");
    cgikeys.ins("??????", "REMOTE_HOST");
    cgikeys.ins("??????", "REMOTE_IDENT");
    cgikeys.ins("??????", "REMOTE_USER");
    cgikeys.ins("method", "REQUEST_METHOD");
    cgikeys.ins("uri", "SCRIPT_NAME");
    cgikeys.ins("host", "SERVER_NAME");
    cgikeys.ins("host", "SERVER_PORT");
    cgikeys.ins("protocol", "SERVER_PROTOCOL");
    cgikeys.ins("??????", "SERVER_SOFTWARE");

}

////////////////////////////////////////////////////////////////
// chdir2 -- change to directory of specified file
//
// warning: modifies fname in place, but fixes it later.
//
////////////////////////////////////////////////////////////////

void Kaptrans::chdir2(char*fname)
{
    char* lastslash;
    int rc;

    lastslash = strrchr(fname, '/');
    if (lastslash != 0) {
        *lastslash = 0;

        rc =TCLCHDIR(fname);
        if(rc == 0) {
	     rc=TCL_OK;
	} else {
	    rc=TCL_ERROR;
	}
        *lastslash = '/';
    }
}

////////////////////////////////////////////////////////////////
// mfgets -- get string, check for buffer overflow
////////////////////////////////////////////////////////////////

char* Kaptrans::mfgets(char*buf, int len, FILE* fp)
{
    char* rc;
    rc = fgets(buf, len, fp);
    if (rc != 0) {
        int len1 = strlen(buf)-1;
        if (buf[len1] != '\n') { // buffer overflow!!
            dbg("buffer overflow! what to do???\n");
        }
        buf[len1] = 0;  // chop newline
    }

    return rc;
}

////////////////////////////////////////////////////////////////
// flushme -- flush the interpreter's output channel
////////////////////////////////////////////////////////////////

void Kaptrans::flushme(void)
{
    headerflush();
    Tcl_Flush(mychannel);
}

////////////////////////////////////////////////////////////////
// parsecookies -- parse, normalize, and set cookies
////////////////////////////////////////////////////////////////


void Kaptrans::parsecookies(Tcl_Interp* myinterp, char* str)
{
    char* p;
    char* p1;
    char* q;
    char* q1;

    strcat(str, ";"); //TODO -- this is a temporary fix
    p = str;
    while (*p) {
        p1 = strchr(p, '=');
        q = p1 + 1;
        q1 = strchr(q, ';');
        *p1 = *q1 = 0;
        Tcl_SetVar2(interp, "COOKIE", p, q, 0);
        p = q1 + 1;
        while (*p == ' ') // skip extra spaces
            ++p;
    }
}

////////////////////////////////////////////////////////////////
// parse_vars -- parse and normalize HTTP variables
////////////////////////////////////////////////////////////////

void Kaptrans::parse_vars(char* instr, char* var, char* val)
{
    char* p;
    char* q;
    p = instr;
    q = instr;
    for (;;) {
        if (*q == 0)
            break;
        if (*q == '=') {
            *q = 0;
            ++q;
            break;
        }
        ++q;
    }
    strcpy(var, p);
    strcpy(val, q);
}
#ifdef OLD_ONE

////////////////////////////////////////////////////////////////
// asctab -- ascii character translation table
//
// This is used for hex ascii to binary translation.  All
// hex digits in the table ('0' - '9', 'A' - 'F', 'a' - 'f')
// are valued as their numeric values.  So asctab['3'] = 3,
// asctab['a'] = 10, asctab['F'] = 15.
////////////////////////////////////////////////////////////////

static char asctab[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
     0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};  

void Kaptrans::parse_vars(char* instr, char* var, char* val)
{
    char* p;
    char* q = var;

    for (p = instr; *p != 0;/*incr in loop*/) {

        // get "var"
        for (q = var; *p != '=' && *p != 0; *q++ = *p++) {
        }
        if (*p == 0)
            break;
        *q = 0;
        ++p;

        // get "val"
        for (q = val; *p != '&' && *p != 0; q++, p++) {
            // TODO: check if p[1], p[2] is end of string
            if (*p == '%') {
                *q = (asctab[p[1]] * 16) + asctab[p[2]];
                p += 2;
            }
            else {
               *q = *p;
            }
        }
        if (*p != 0)
            ++p;
        *q = 0;

    }
}
#endif

////////////////////////////////////////////////////////////////
// headerflush -- flush the http headers if necessary
////////////////////////////////////////////////////////////////

void Kaptrans::headerflush()
{
    if (!haveflushed) {
        haveflushed = 1;
        if (*statusbuf == 0) {
            strcpy(statusbuf, "HTTP/1.1 200 OK");
        }
        fprintf(fout, "%s\n", statusbuf);

        if (strstr(headerbuf, "Content-type: ") == NULL) {
            strcat(headerbuf, "Content-type: text/html\n");
        }
        fprintf(fout, "%s\n", headerbuf);
        fflush(fout);
    }
}

////////////////////////////////////////////////////////////////
// httphead -- print the HTTP header
////////////////////////////////////////////////////////////////

void Kaptrans::httphead()
{
    //strcat(headerbuf, KINETIC_ID_STRING);
    //strcat(headerbuf, "\n");
    //strcat(headerbuf, "Connection: keep-alive\n");
}


////////////////////////////////////////////////////////////////
// httpredirect -- print an HTTP redirection header
////////////////////////////////////////////////////////////////

void Kaptrans::httpredirect(char* url)
{
    strcpy(statusbuf, "HTTP/1.1 302 Redirect");
    strcat(headerbuf, "Location: ");
    strcat(headerbuf, url);
    strcat(headerbuf, "\n");
}

////////////////////////////////////////////////////////////////
// exterr -- display an extended internal error code
////////////////////////////////////////////////////////////////

void Kaptrans::exterr(char* name, int code1, int code2,
                      char* summary, char* detail)
{
    strcpy(statusbuf, "HTTP/1.1 404 Not Found"); // better add accessor func!!
    flushme();
    fprintf(fout, "<H1>Error 404</H1>\n");
    fprintf(fout, "<HR>\n");
    fprintf(fout, "Error processing %s on this server<P>\n", name);
    fprintf(fout, "%s<P>\n", summary);
    fprintf(fout, "Server extended return code: %d-%d\n",
            code1, code2);

    // log details to file
    FILE* fp;
    fp = fopen("kap-t.log", "a");
    if (fp) {
        fprintf(fp, "=============================================\n");
        fprintf(fp, "%s\n", summary);
        fprintf(fp, "%s\n", detail);
        fclose(fp);
    }
}

////////////////////////////////////////////////////////////////
// arraydump -- print a Tcl array to stdout
////////////////////////////////////////////////////////////////

void arraydump(Tcl_Interp* interp, char* name)
{
    char cmd[1000];
    sprintf(cmd, "puts %s:\nforeach i [lsort [array names %s]] {puts \"    :$i: = :$%s($i):\"};flush stdout", name, name, name);
    Tcl_Eval(interp, cmd);
}

////////////////////////////////////////////////////////////////
// process -- process an HTTP request
////////////////////////////////////////////////////////////////

void Kaptrans::parsequery(Tcl_Interp* theinterp, char* val)
{
    int rc;
    char* p;
    char* p1;
    char* q;
    char* q1;

    strcat(val, "&"); //TODO -- this is a temporary fix
    p = val;
    while (*p) {
        p1 = strchr(p, '=');
        q = p1 + 1;
        q1 = strchr(q, '&');
        *p1 = *q1 = 0;
        // now, p=name, q=value (with % quoting)
        // should we unescape at th C level?  maybe later.
        rc = Tcl_VarEval(interp, "::kap::unescapecgi \"", q, "\"", NULL);
        if (rc == TCL_OK) {
            Tcl_SetVar2(interp, "QUERY", p, interp->result, 0);
        }
        else {
            dbg("setvar error!!!\n");
            dbg("p=:%s:\n", p);
            dbg("q=:%s:\n", q);
            dbg("result=:%s:\n", interp->result);
        }
        p = q1 + 1;
    }
}

////////////////////////////////////////////////////////////////
// process -- process an HTTP request
////////////////////////////////////////////////////////////////

int Kaptrans::process(void)
{
    char req[20000]; // no buffer overflow for us!
    char postdata[20000];
    char line[20000];
    char var[4000], val[4000];
    char path[4000];

    // clean up cgi and cookie variables
    // clean out old request (if it exists)
    Tcl_UnsetVar(interp, "CGI", 0);
    Tcl_UnsetVar(interp, "COOKIE", 0);
    Tcl_UnsetVar(interp, "QUERY", 0);
    Tcl_SetVar2(interp, "CGI", "tmpvar", "", 0);
    Tcl_SetVar2(interp, "COOKIE", "tmpvar", "", 0);
    Tcl_SetVar2(interp, "QUERY", "tmpvar", "", 0);
    Tcl_UnsetVar2(interp, "CGI", "tmpvar", 0);
    Tcl_UnsetVar2(interp, "COOKIE", "tmpvar", 0);
    Tcl_UnsetVar2(interp, "QUERY", "tmpvar", 0);

    // clean up header buffers
    haveflushed = 0;
    *statusbuf = 0;
    *headerbuf = 0;

    // GET/HEAD/POST request
    if (mfgets(req, sizeof(req)-1, fin) == 0) {
        return 1;
    }
    //dbg("req=:%s:\n", req);

    *postdata = 0;
    if (strncmp(req, "POST", 4) == 0) {
        if (mfgets(postdata, sizeof(postdata)-1, fin) == 0) {
            return 1;
        }
        dbg("begin \"parsequrey()\"\n");
        parsequery(interp, postdata);
    }

    // Process HTTP information which has been pre-digested from
    // the http server.  We require these data:
    //        path=
    //        cookie=
    strcpy(path, "/path-not-received-from-server");
    for (;;) {
        if (mfgets(line, sizeof(line)-1, fin) == 0) {
            return 1;
        }
        dbg("line=:%s:\n", line);
		if (0)
			{
			FILE* foolog;
			foolog=fopen("/tmp/foolog", "a");
			fprintf(foolog, "line=:%s:\n", line);
			fflush(foolog);
			fclose(foolog);
			}

        // blank line -- end of http headers
        if (*line == 0) {
            break;
        }

        // loop through the headers, setting the cgi and
        // cookie variables as necessary.
        parse_vars(line, var, val);

        if (strcmp(var, "path") == 0) {
            strcpy(path, val);
        }

        if (strcmp(var, "query") == 0 && strlen(val) > 0) {
            parsequery(interp, val);
        }

        // netscape uses "cookie=", apache uses "Cookie="
        if (strcasecmp(var, "cookie") == 0 && strlen(val) > 0) {
            parsecookies(interp, val);
        }
        else {
            // do CGI variable key substitution before
            // setting into CGI array
            char* subkey = cgikeys.get(var);
            if (subkey == 0)
                subkey = var;
            Tcl_SetVar2(interp, "CGI", subkey, val, 0);
        }
    }

    chdir2(path);
    process_file(path);

    fprintf(fout, "\n\1MeAoRfK>\n");  // unintelligent EOT marker
    fflush(fout);
    return 0;
}

////////////////////////////////////////////////////////////////
// compile -- compile and dump a KAP page
////////////////////////////////////////////////////////////////

int Kaptrans::compile(char* path)
{
    int rc;
    char* rcerrmsg;
    char* rcdetail;

    rc = fmgr->load(path, rcerrmsg, rcdetail);
    if (rc != 0) {
        exterr(path, 2, rc, rcerrmsg, rcdetail);
    }
    else {
        rc = Tcl_VarEval(interp, "info body ", path, (char*)0);
        if (rc == TCL_OK) {
            printf("proc %s {} {\n", path);
            printf("%s\n", Tcl_GetStringResult(interp));
            printf("}\n");
            printf("%s ;# invoke the page\n", path);
        } else {
            fprintf(stderr, "error: %s\n", Tcl_GetStringResult(interp));
        }
    }
    return rc;
}

////////////////////////////////////////////////////////////////
// process_file -- file processing for an HTTP request
////////////////////////////////////////////////////////////////

int Kaptrans::process_file(char* path)
{
    // start out with a nice http header
    httphead();

    int rc;
    char* rcerrmsg;
    char* rcdetail;
    rc = fmgr->load(path, rcerrmsg, rcdetail);

    if (rc != 0) {
        exterr(path, 2, rc, rcerrmsg, rcdetail);
        return 1;
    }
    else {

        // if everything else has gone well, process the
        // request by invoking the interpreter

        fflush(fout);
        rc = Tcl_Eval(interp, path);
        flushme();
        headerflush();
        if (rc != TCL_OK) {
            flushme();
            exterr(path, 3, rc, interp->result, "NEED DETAIL");
            return 1;
        }
    }
    flushme();
    return 0;
}

////////////////////////////////////////////////////////////////
// main -- main program for kap-t
////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    char* initfile;   // initialization file
    char* sif;        // server input pipe
    char* cif;        // client input pipe

    if (0) {
    }
    else if (argc == 3 && strcmp(argv[1], "-c") == 0) {
        char* kfile = argv[2];
        Kaptrans trans(stdin, stdout, 200, "/dev/null");
        trans.compile(kfile);
        exit(0);
    }
    else if (argc == 3) {
        initfile = "";
        sif = argv[1];
        cif = argv[2];
    }
    else if (argc == 5 && strcmp(argv[1], "-i") == 0) {
        initfile = argv[2];
        sif = argv[3];
        cif = argv[4];
    }
    else {
        fprintf(stderr, "usage: %s [-i initfile] output input\n", argv[0]);
        fflush(stderr);
        exit(1);
    }

    fprintf(stderr, "starting kap-t ($Revision: 1.6 $)...\n");
    fflush(stderr);

    FILE* sfp;
    FILE* cfp;

    if (strcmp(sif, "-") == 0)
        sfp = stdout;
    else if ((sfp = fopen(sif, "w")) == NULL) {
        perror(sif);
        exit(1);
    }

    if (strcmp(cif, "-") == 0)
        cfp = stdin;
    else if ((cfp = fopen(cif, "r")) == NULL) {
        perror(cif);
        exit(1);
    }

    //Tell nsapi my pid.
    //fprintf(stderr, "getting pid ...\n");
    long mypid;
    mypid = (long)(getpid());
    //fprintf(stderr, "pid of the engine: %d\n", mypid);
    fprintf(sfp, "%ld\n", mypid);
    fflush(sfp);

    int maxcache = 200;  //TODO: don't hard-code this
    Kaptrans trans(cfp, sfp, maxcache, initfile);

    int rc;
    do {
        rc = trans.process();
    } while (rc == 0);

    printf("kap-t: server closed connection, goodbye\n");

    fclose(sfp);
    fclose(cfp);
}

// eof: kap-t.cc
