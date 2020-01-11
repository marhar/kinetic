//----------------------------------------------------------------------
// parsedfile.hh -- parsed aiws file
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

#ifndef _PARSEDFILE_HH
#define _PARSEDFILE_HH

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////
// class ParsedFile -- ProcessedFile
////////////////////////////////////////////////////////////////

class ParsedFile
{
private:
    char* ibuf;
    char* obuf;

    char* in;
    char* out;

    int _status;

    void memcp2(char** dst, char* src);
    int process_code(void);
    int process_html(char* procname);
    int process_file(char* req);

public:
    ParsedFile(char* filename);
    ~ParsedFile();
    int status() { return _status; }
    char* unparsed_body() { return ibuf; }
    char* parsed_body() { return obuf; }
};

#endif
// eof: parsedfile.hh
