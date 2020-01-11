//----------------------------------------------------------------------
// sshash.h -- static string hash table
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

class SShash
{
private:
    Tcl_HashTable tbl;
    Tcl_HashSearch srec;

public:
    SShash() {
        Tcl_InitHashTable(&tbl, TCL_STRING_KEYS);
    }
    ~SShash() {
        Tcl_DeleteHashTable(&tbl);
    }
    void ins(char* key, char* value) {
        int isnew;
        Tcl_HashEntry* ent;
        ent = Tcl_CreateHashEntry(&tbl, key, &isnew);
        Tcl_SetHashValue(ent, value);
    }
    char* get(char* key) {
        Tcl_HashEntry* ent;
        ent = Tcl_FindHashEntry(&tbl, key);
        return (ent) ? (char*)Tcl_GetHashValue(ent) : 0;
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
            printf("%s = %s\n", p, get(p));
            while ((p = nextkey()) != 0)
                printf("%s = %s\n", p, get(p));
        }
    }
    void stats(void) {
        char* s = Tcl_HashStats(&tbl);
        printf("%s\n", s);
        free(s);

    }
};
