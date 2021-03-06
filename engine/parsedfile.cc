//----------------------------------------------------------------------
// parsedfile.cc -- parsed aiws file
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

#include "parsedfile.hh"

////////////////////////////////////////////////////////////////
// ParsedFile::memcp2 -- memcpy, but modifies dst pointer
////////////////////////////////////////////////////////////////

void ParsedFile::memcp2(char** dst, char* src)
{
    while ((**dst = *src) != 0) {
        ++*dst;
        ++src;
    }
}

////////////////////////////////////////////////////////////////
// ParsedFile::process_code -- process the code portions of an aiws page
////////////////////////////////////////////////////////////////

int ParsedFile::process_code(void)
{
    for (;;) {

        switch (*in) {
        case 0:
            *out++ = 0;
            return 1;

        case '+':
            if (in[1]=='>') {
                *out++ = '\n';
                in += 2; /* 2 = strlen("+>") */
                return 0;
            }
            else {
                *out++ = *in++;
            }
            break;

        case '<':
            if (in[1]=='/' && in[2]=='t' &&
                in[3]=='c' && in[4]=='l' && in[5]=='>') {
                *out++ = '\n';
                in += 6; /* 6 = strlen("</tcl>") */
                return 0;
            }
            else {
                *out++ = *in++;
            }
            break;

        default:
            *out++ = *in++;
            break;
        }
    }
}

////////////////////////////////////////////////////////////////
// ParsedFile::process_html -- process the html portions of an aiws page
////////////////////////////////////////////////////////////////

int ParsedFile::process_html(char* procname)
{
    int rc;
    memcp2(&out, "proc ");
    memcp2(&out, procname);
    memcp2(&out, " {} {\nglobal env CGI QUERY COOKIE\nputs -nonewline \"");

    for (;;) {

        switch (*in) {
        // end of string -- close quote and finish proc
        case 0:
            *out++ = '"';
            *out++ = '\n';
            *out++ = '}';
            *out++ = '\n';
            *out++ = 0;
            return 0;

        // special characters which must be quoted in "" strings
        case '$':
        case '{':
        case '}':
        case '[':
        case ']':
        case '"':
        case '\\':
            *out++ = '\\';
            *out++ = *in++;
            break;

        case '<':
            if (in[1]=='t' && in[2]=='c' && in[3]=='l' && in[4]=='>') {
                *out++ = '"';
                *out++ = '\n';
                in += 5; /* 5 = strlen("<tcl>") */
                rc = process_code();
                if (rc == 0) {
                    memcp2(&out, "\nputs -nonewline \"");
                }
                else {
                    *out = 0;
                    return rc;
                }
            }
            else if (in[1]=='+') {
                *out++ = '"';
                *out++ = '\n';
                in += 2; /* 2 = strlen("<+") */
                rc = process_code();
                if (rc == 0) {
                    memcp2(&out, "\nputs -nonewline \"");
                }
                else {
                    *out = 0;
                    return rc;
                }
            }
            else if (in[1]=='$') {     // <$var> -> ${var}
                in++;
                *out++ = *in++;
                *out++ = '{';
                while ((*out++ = *in++) != '>')
                    ;
                out--;
                *out++ = '}';
            }
            else {
                *out++ = *in++;
            }
            break;

        default:
            *out++ = *in++;
            break;
        }
    }
}

////////////////////////////////////////////////////////////////
// ParsedFile::ParsedFile -- constructor
////////////////////////////////////////////////////////////////

ParsedFile::ParsedFile(char* filename)
{
    int fd;
    struct stat statb;

    _status = 0;

    if ((fd = open(filename, O_RDONLY, 0)) < 0) {
        _status = 1;
        return;
    }
    if (fstat(fd, &statb) < 0) {
        _status = 2;
        return;
    }

    int ibufsz = statb.st_size + 1;
    int obufsz = (ibufsz + 200) * 2;

    if ((ibuf = (char*)malloc(ibufsz)) == NULL) {
        fprintf(stderr, "cannot allocate ibuf, exiting.\n");
        exit(1);
    }
    if ((obuf = (char*)malloc(obufsz)) == NULL) {
        fprintf(stderr, "cannot allocate obuf, exiting.\n");
        exit(1);
    }

    // now read file into input buffer
    read(fd, ibuf, statb.st_size);
    ibuf[statb.st_size] = 0;
    close(fd);

    // initialize output buffer to empty string
    *obuf = 0;

    // initialize current character pointers
    in = ibuf;
    out = obuf;

    int rc;
    if ((rc = process_html(filename)) != 0) {
        _status = 3;
    }
}

////////////////////////////////////////////////////////////////
// ParsedFile::~ParsedFile -- destructor
////////////////////////////////////////////////////////////////

ParsedFile::~ParsedFile()
{
    free(ibuf);
    free(obuf);
}

//eof: parsedfile.cc
