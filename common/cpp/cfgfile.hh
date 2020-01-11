//----------------------------------------------------------------------
// cfgfile.hh -- funky config file
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
//
// Any sane person would take the high road to configuration
// files and just drop in a tcl interpreter to handle
// the job.  However, netscape is a bit touchy on how it
// handles plugins, so it's easier all around if we just
// exclude tcl from the list of potential trouble spots
// when loading the shared library.  So, let us write
// yet another quirky, weak configuration processor.
// The only thing that can be said is that this processor
// supports a strict subset of tcl.  Supported syntax is:
//   # for comments
//   white space ignored
//   first word of each line is the command
//   second word of each line is the parameter
// If necessary, this can be improved, but I would suggest
// the effort would be better spent integrating Tcl into
// the application.  For this reason I also lazily put
// everything into the header file, since it is doubtful
// there will be more than one of these things per program
// anyway.

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// every C++ book says this is the bad way to do it, but
// no C++ book gives advice on writing portable programs
// that combines templates and shared libraries... :-(

#define NUMPARM(name) \
    private: \
        int _##name; \
    public: \
        void name(int x) { _##name = x; } \
        int name(void) { return _##name; }

#define STRPARM(name) \
    private: \
        char* _##name; \
    public: \
        void name(char* x) { \
            if (_##name != 0) \
                free(_##name); \
            _##name = (char*)malloc(strlen(x)+1); \
            strcpy(_##name, x); \
        } \
        char* name(void) { return _##name; }

// and no such thing as a standard linked list... :-(

class cfgl {
public:
    char* data;
    struct cfgl* next;
};

#define LSTPARM(name) \
    private: \
        cfgl* _##name; \
    public: \
        void name(char* x) { \
            cfgl* p; \
            p = _##name; \
            while (p->next) \
                p = p->next; \
            p->next = (cfgl*)malloc(sizeof(cfgl)); \
            p->next->next = 0; \
            p->next->data = (char*)malloc(strlen(x)+1); \
            strcpy(p->next->data, x); \
        } \
        cfgl* name(void) { return _##name; }


class cfgfile {
private:
    // declare each parameter here.
    // the macros don't initialize the variable, so
    // you need to construct and deconstruct it.
    NUMPARM(debug)
    NUMPARM(timeout)
    NUMPARM(numengines)
    STRPARM(engine)
    LSTPARM(partition)

public:
    cfgfile() {
        _numengines = 0;
        _debug = 0;
        _timeout = 0;
        _engine = 0;
        engine("undefined");
        _partition = (cfgl*)malloc(sizeof(cfgl));
        _partition->data = 0;
        _partition->next = 0;
    }
    void freelist(cfgl* p) {
        if (p->next)
            freelist(p->next);
        if (p->data)
            free(p->data);
        free(p);
    }
    ~cfgfile() {
        free(_engine);
        freelist(_partition);
    }
    int read(char* fname) {
        int rc;
        FILE* fp;
        char s[1000];
        char* p;
        char* q;
        char* q2;
        int lineno = 0;
        if ((fp = fopen(fname, "r")) == 0) {
            printf("KAP: cannot open config file: %s\n", fname);
            rc = 1;
        } else {
            while (fgets(s, 999, fp) != 0) {
                lineno++;
                p = s;
                // skip any leading whitespace
                while (isspace(*p) && *p != 0)
                    p++;
                // skip blank lines and comments
                if (*p == '#' || *p == 0)
                    continue;
                q = p;
                while (isgraph(*q) && *q != 0)
                    q++;
                *q = 0;
                q++;                    
                // skip to second word
                while (isspace(*q) && *q != 0)
                    ++q;
                q2 = q;
                while (isgraph(*q2) && *q2 != 0)
                    ++q2;
                *q2 = 0;

                if (0)
                    ;
                else if (strcmp("debug", p) == 0)
                    debug(atoi(q));
                else if (strcmp("timeout", p) == 0)
                    timeout(atoi(q));
                else if (strcmp("numengines", p) == 0)
                    numengines(atoi(q));
                else if (strcmp("engine", p) == 0)
                    engine(q);
                else if (strcmp("partition", p) == 0)
                    partition(q);
                else {
                    printf("KAP: %s(%d) unrecognized parm: %s\n",
                                fname, lineno, p);
                }
            }
            fclose(fp);
            rc = 0;
        }
        return rc;


    }
    void print(void) {
        printf("numengines %d\n", _numengines);
        printf("debug %d\n", _debug);
        printf("timeout %d\n", _timeout);
        printf("engine %s\n", _engine);
        cfgl* p = _partition;
        while (p->next) {
            printf("partition %s\n", p->next->data);
            p = p->next;
        }
    }
};

// eof: cfgfile.hh
