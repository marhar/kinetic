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
// sshashtest.cc -- test sshash class
// Copyright (c) 1999 AsiaInfo.
////////////////////////////////////////////////////////////////

#include "sshash.h"

main()
{
    SShash x;
    x.ins("mark", "harrison");
    x.ins("mike", "mclennan");
    x.ins("john", "ousterho");
    x.ins("bren", "welchxxx");

    printf("%s\n", x.get("mark"));
    printf("%s\n", x.get("john"));
    printf("%s\n", x.get("zzzz"));
    printf("%s\n", x.get("bren"));

    printf("---\n");
    x.printall();
}
