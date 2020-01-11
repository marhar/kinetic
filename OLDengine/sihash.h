//----------------------------------------------------------------------
// sihash.h -- static string-int hash table
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

#include <stdlib.h>
extern "C" {
#include "tcl.h"
}

class SIhash
{
private:
    Tcl_HashTable tbl;
    Tcl_HashSearch srec;

public:
    SIhash() {
        Tcl_InitHashTable(&tbl, TCL_STRING_KEYS);
    }
    ~SIhash() {
        Tcl_DeleteHashTable(&tbl);
    }
    void ins(char* key, int value) {
        int isnew;
        Tcl_HashEntry* ent;
        ent = Tcl_CreateHashEntry(&tbl, key, &isnew);
        Tcl_SetHashValue(ent, value);
    }
    int get(char* key) {
        Tcl_HashEntry* ent;
        ent = Tcl_FindHashEntry(&tbl, key);
        return (ent) ? (int)Tcl_GetHashValue(ent) : 0;
    }
    void del(char* key) {
        Tcl_HashEntry* ent;
        ent = Tcl_FindHashEntry(&tbl, key);
        if (ent != 0)
            Tcl_DeleteHashEntry(ent);
    }
    char* firstkey() {
        Tcl_HashEntry* ent;
        ent = Tcl_FirstHashEntry(&tbl, &srec);
        return Tcl_GetHashKey(&tbl, ent);
    }
    char* nextkey() {
        Tcl_HashEntry* ent;
        ent = Tcl_NextHashEntry(&srec);
        return (ent) ? Tcl_GetHashKey(&tbl, ent) : 0;
    }
    void printall(void) {
        char* p;
        if ((p = firstkey()) != 0) {
            printf("%s = %d\n", p, get(p));
            while ((p = nextkey()) != 0)
                printf("%s = %d\n", p, get(p));
        }
    }
    void stats(void) {
        char* s = Tcl_HashStats(&tbl);
        printf("%s\n", s);
        free(s);

    }
};
