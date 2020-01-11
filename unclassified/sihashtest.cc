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

#include "sihash.h"

main()
{
    SIhash x;
    printf("%d\n", x.get("aaa"));
    printf("---\n");
    x.ins("aaa", 1);
    x.ins("bbb", 2);
    x.ins("ccc", 3);
    x.ins("ddd", 4);

    printf("%d\n", x.get("aaa"));
    printf("%d\n", x.get("bbb"));
    printf("%d\n", x.get("ccc"));
    printf("%d\n", x.get("ddd"));
    printf("%d\n", x.get("zzz"));

    printf("---\n");
    x.printall();

    x.del("bbb");
    x.del("ccc");

    printf("---\n");
    printf("%d\n", x.get("aaa"));
    printf("%d\n", x.get("bbb"));
    printf("%d\n", x.get("ccc"));
    printf("%d\n", x.get("ddd"));
    printf("%d\n", x.get("zzz"));
    printf("---\n");
    x.printall();
}
