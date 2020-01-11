/*--------------------------------------------------------------------*/
/* mod_kap.c -- kap apache module                                     */
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
/* This file is part of KAP, the Kinetic Application Processor.       */
/* Copyright (c) 1999, AsiaInfo Technologies, Inc.                    */
/*                                                                    */
/* See the file kap-license.txt for information on usage and          */
/* redistribution of this file, and for a DISCLAIMER OF ALL           */
/* WARRANTIES.  Share and Enjoy!                                      */
/*                                                                    */
/* The KAP home page is: http://www.markharrison.net/kap              */
/*--------------------------------------------------------------------*/

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_core.h"
#include "ap_config.h"
#include "http_conf_globals.h"

/*--------------------------------------------------------------------*/
/* configuration                                                      */
/*                                                                    */
/* this structure contains all parameters set in the http             */
/* configuration file.  It's simpler than the netscape version        */
/* because we depend on apache for things like timeout, etc.          */
/*--------------------------------------------------------------------*/

struct _parms {
    char engine[256];           /* path to engine executable          */
    char config[256];           /* path to engine executable          */
    int timeout;                /* processing timeout (in seconds)    */
} parms;

struct _com {
    FILE* in;                   /* input channel from engine          */
    FILE* out;                  /* output channel from engine         */
    int count;                  /* how many pages have been served    */
    pid_t pid;                  /* pid of engine                      */
} com;

/*--------------------------------------------------------------------*/
/* debug stuff                                                        */
/*--------------------------------------------------------------------*/

#if 0
#define APLOG_EMERG     0       /* system is unusable */
#define APLOG_ALERT     1       /* action must be taken immediately */
#define APLOG_CRIT      2       /* critical conditions */
#define APLOG_ERR       3       /* error conditions */
#define APLOG_WARNING   4       /* warning conditions */
#define APLOG_NOTICE    5       /* normal but significant condition */
#define APLOG_INFO      6       /* informational */
#define APLOG_DEBUG     7       /* debug-level messages */
/* ap_log_error(APLOG_MARK, APLOG_level, s, "xxx");                   */
/* ap_log_rerror(APLOG_MARK, APLOG_level, r, "xxx");                  */
#endif

/*--------------------------------------------------------------------*/
/* support functions                                                  */
/*--------------------------------------------------------------------*/

static int fake(request_rec *r)
{
    time_t now;
    time(&now);
    r->content_type = "text/html";
    ap_send_http_header(r);
    if (!r->header_only) {
        ap_rputs("FAKE MODULE<p>hello from <i>kap_uri()</i><br>\n", r);
        ap_rputs("<b>", r);
        ap_rputs(ctime(&now), r);
        ap_rputs("</b>\n", r);
    }
}

/*--------------------------------------------------------------------*/
/* readconfig -- read configuration options                           */
/*--------------------------------------------------------------------*/

static void readconfig(server_rec *s)
{
    sprintf(parms.engine, "%s/libexec/kap-t", ap_server_root);
    sprintf(parms.config, "%s/libexec/kap-init.tcl", ap_server_root);
}

/*--------------------------------------------------------------------*/
/* kap_child_init -- initialize engine                                */
/*--------------------------------------------------------------------*/

static void kap_child_init(server_rec *s, pool *p)
{
    int pid;       /* for starting engine */
    int pipe1[2];
    int pipe2[2];
    char thepid[100];  /* should go away */

    errno = 0; /* don't let apache log print odd messages */
    readconfig(s);

    /* start engine. use standard pipe magic incantation */

    pipe(pipe1);
    pipe(pipe2);

    pid = fork();
    if (pid == -1) {            /* error */
        ap_log_error(APLOG_MARK, APLOG_EMERG, s,
                     "KAP: problem forking engine, goodbye");
        exit(1);
    }
    else if (pid == 0) {        /* child */
        close(0);
        close(1);
        dup(pipe2[0]);
        dup(pipe1[1]);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        execl(parms.engine, parms.engine, "-i", parms.config, "-", "-", NULL);
        ap_log_error(APLOG_MARK, APLOG_EMERG, s,
                     "KAP: problem execing engine (%d), goodbye", errno);
        exit(1);
    }
    else {                      /* parent */
        close(pipe1[1]);
        close(pipe2[0]);
        com.in = fdopen(pipe1[0],"r");
        com.out = fdopen(pipe2[1],"w");
        if (com.in == NULL || com.out == NULL) {
            ap_log_error(APLOG_MARK, APLOG_EMERG, s,
                         "KAP: problem opening pipes");
            exit(1);
        }
    }

    fgets(thepid, sizeof(thepid), com.in);
    com.pid = (pid_t)atol(thepid);


    /* everything is OK */
    /*
    ap_log_error(APLOG_MARK, APLOG_NOTICE, s,
           "KAP: Application Processor started pid=%s.", thepid);
    */
}

/*--------------------------------------------------------------------*/
/* kap_child_exit -- shut down engine                                 */
/*                                                                    */
/* actually don't need to do much here.  when the http shuts down,    */
/* the kap engine's stdin will receive an EOF and exit normally.      */
/* since apache runs as a process, no thread cleanup is necessary.    */
/*--------------------------------------------------------------------*/

static void kap_child_exit(server_rec *s, pool *p)
{
    /*ap_log_error(APLOG_MARK, APLOG_NOTICE, s, "KAP: Goodbye!");*/
}

int sendhead(void *p, const char *key, const char *value)
{
    request_rec *r = (request_rec *)p;
    fprintf(com.out, "%s=%s\n", key, value);
}

int printhead(void *p, const char *key, const char *value)
{
    FILE* foolog;
    request_rec *r = (request_rec *)p;
    foolog=fopen("/tmp/foolog", "a");
    fprintf(foolog, "%s=%s\n", key, value);
    fclose(foolog);
}

/*--------------------------------------------------------------------*/
/* util_read -- thanks to l. stein                                    */
/*--------------------------------------------------------------------*/

#define HUGE_STRING_LENGTH 5000

static int util_read(request_rec *r, const char **rbuf)
{
    int rc;
    if ((rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)) != OK) {
        return rc;
    }
    if (ap_should_client_block(r)) {
        char argsbuffer[HUGE_STRING_LENGTH];
        int rsize, len_read, rpos = 0;
        long length = r->remaining;
        *rbuf = ap_pcalloc(r->pool, length + 1);

        ap_hard_timeout("util_read", r);
        while ((len_read =
            ap_get_client_block(r, argsbuffer, sizeof(argsbuffer))) > 0) {
            ap_reset_timeout(r);
            if ((rpos + len_read) > length) {
                rsize = length - rpos;
            }
            else {
                rsize = len_read;
            }
            memcpy((char*)*rbuf + rpos, argsbuffer, rsize);
            rpos += rsize;
        }
        ap_kill_timeout(r);
    }
    return rc;
}

/*--------------------------------------------------------------------*/
/* kap_process1 -- process a KAP request                              */
/*--------------------------------------------------------------------*/

static int kap_process1(request_rec *r)
{
    int err;
    char resp[10240];
    const char *postdata;

    const char *docroot = ap_document_root(r);

    /* send http request, and parameters */
    fprintf(com.out, "%s %s %s\n",
                         r->method, r->unparsed_uri, r->protocol);

    if (strcmp(r->method, "POST") == 0) {
        /* send post data */
        int rc;
        if ((rc = util_read(r, &postdata)) != OK) {
            return rc;
        }
        fprintf(com.out, "%s\n", postdata);
    }
    fprintf(com.out, "method=%s\n", r->method);
    fprintf(com.out, "protocol=%s\n", r->protocol);
    fprintf(com.out, "uri=%s\n", r->uri);
    fprintf(com.out, "ntrans-base=%s\n", docroot);
    fprintf(com.out, "path=%s%s\n", docroot, r->uri);
    fprintf(com.out, "clf=%s %s %s\n",
                         r->method, r->unparsed_uri, r->protocol);
    fprintf(com.out, "ip=%s\n", r->connection->remote_ip);
    if (r->args != 0)
        fprintf(com.out, "query=%s\n", r->args);

    ap_table_do(sendhead, r, r->headers_in, NULL);
    /*ap_table_do(printhead, r, r->headers_in, NULL);*/

    /* wrap up with blank line and flush it out */
    fprintf(com.out, "\n");
    err = fflush(com.out);

    /* if flush got an error, the engine is probably dead */
    /* otherwise, process the response                    */
    if (err == EOF) {
        ap_log_rerror(APLOG_MARK, APLOG_ALERT, r, "KAP: eof on flush");
        ap_rputs("\n", r);
        ap_rputs("<h1>Server Error...</h1>\n", r);
        ap_rputs("engine not responding (1).\n", r);
        ap_rputs("<br>(need to add restart here)(1).\n", r);

        ap_log_rerror(APLOG_MARK, APLOG_ALERT, r, "KAP: restarting");
        exit(1);  // is calling exit the best way to restart?
    }
    else {
        char* rc;
        for (;;) {
            rc = fgets(resp, sizeof(resp), com.in);
            if (rc == NULL) {
                ap_log_rerror(APLOG_MARK, APLOG_ALERT, r, "KAP: eof on gets");
                ap_rputs("\n", r);
                ap_rputs("<h1>Server Error...</h1>\n", r);
                ap_rputs("engine not responding (2).\n", r);

                ap_log_rerror(APLOG_MARK, APLOG_ALERT, r, "KAP: restarting");
                exit(1);  // is calling exit the best way to restart?
                break;
            }
            if (resp[0] == '\1' && resp[1] == 'M' &&
                resp[2] == 'e' && resp[3] == 'A' &&
                resp[4] == 'o' && resp[5] == 'R' &&
                resp[6] == 'f' && resp[7] == 'K' &&
                resp[8] == '>' && resp[9] == '\n' &&
                resp[10] == 0) {
                break;
            }
            else {
                /*fprintf(stderr, "resp:%s:\n", resp);*/
                ap_rputs(resp, r);
            }
        }
    }
    return DONE;
}

/*--------------------------------------------------------------------*/
/* kap_process -- process a KAP request                               */
/*--------------------------------------------------------------------*/

static int kap_process(request_rec *r)
{
    int sl;
    sl = strlen(r->uri);

    /* decline request if not *.ktcl.                     */
    /*  better way to do this? if so, nuke kap_process1() */
    if (sl >= 7 && r->uri[sl-5] == '.'
                && r->uri[sl-4] == 'k'
                && r->uri[sl-3] == 't'
                && r->uri[sl-2] == 'c'
                && r->uri[sl-1] == 'l') {
        kap_process1(r);
        return DONE;
    }
    else {
        return DECLINED;
    }
}

/*--------------------------------------------------------------------*/
/* kap_module initialization table                                    */
/*--------------------------------------------------------------------*/

/* Dispatch list for API hooks */
module MODULE_VAR_EXPORT kap_module = {
    STANDARD_MODULE_STUFF, 
    NULL,                  /* module initializer                  */
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    NULL,                  /* [#8] MIME-typed-dispatched handlers */
    kap_process,           /* [#1] URI to filename translation    */
    NULL,                  /* [#4] validate user id from request  */
    NULL,                  /* [#5] check if the user is ok _here_ */
    NULL,                  /* [#3] check access by host address   */
    NULL,                  /* [#6] determine MIME type            */
    NULL,                  /* [#7] pre-run fixups                 */
    NULL,                  /* [#9] log a transaction              */
    NULL,                  /* [#2] header parser                  */
    kap_child_init,        /* child_init                          */
    kap_child_exit,        /* child_exit                          */
    NULL                   /* [#0] post read-request              */
};

/* eof: mod_kap.c */
