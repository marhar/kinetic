/*
 * This file is part of KAP, the Kinetic Application Processor.
 * Copyright (c) 1999, AsiaInfo Technologies, Inc.
 *
 * See the file kap-license.txt for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.  Share and Enjoy!
 *
 * The KAP home page is: http://www.markharrison.net/kap
 */

////////////////////////////////////////////////////////////////
// parsedfile.cc -- parsed aiws file
// Copyright (c) 1999 AsiaInfo.
////////////////////////////////////////////////////////////////

#include "parsedfile.hh"

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s file\n", argv[0]);
        exit(1);
    }

    ParsedFile* pf;

    for (int i = 0; i < 100; ++i) {
        pf = new ParsedFile(argv[1]);
        //system("ps aux|grep \"[p]arsedfile a1\"");
        delete pf;
    }
    pf = new ParsedFile(argv[1]);
    printf("#status=%d\n", pf->status());
    //printf("unparsed_body=:%s:\n", pf->unparsed_body());
    printf("#parsed_body=\n%s\n", pf->parsed_body());
    delete pf;
    return 0;
}

//eof: parsedfiletest.cc
