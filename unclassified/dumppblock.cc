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

#include <stdio.h>
#include <base/pblock.h>
#include <frame/log.h>
#include <base/file.h>

extern "C" {
int dumpinit(pblock *pBlock, Session *pSession, Request *pRequest);
int dumppb(pblock *pBlock, Session *pSession, Request *pRequest);
}

////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////

static int writeToClient(Session * pSession, char const * cpString) {
    return net_write(pSession->csd, (char*) cpString, strlen(cpString)) != IO_ERROR;
}

////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////

void indent(int lev)
{
    int i;
    for (i = 0; i < lev; ++i)
        printf("    ");
}

void dump_string(char* desc, char* pc, int lev)
{
    int i;
    indent(lev);
    printf("%s = %p", desc, pc);
    if (pc == 0)
        printf("\n");
    else
        printf(" \"%s\"\n", pc);
}

void dump_pb_param(char* desc, pb_param* pp, int lev)
{
    int i;
    indent(lev);
    printf("%s = %p\n", desc, pp);
    if (pp == 0)
        return;
    dump_string("name", pp->name, lev+1);
    dump_string("value", pp->value, lev+1);
}

void dump_pb_entry(char* desc, pb_entry* pe, int lev)
{
    int i;
    indent(lev);
    printf("%s = %p\n", desc, pe);
    if (pe == 0)
        return;
    dump_pb_param("param", pe->param, lev+1);
    dump_pb_entry("next", pe->next, lev+1);
}

void dump_pblock(char* desc, pblock* pb, int lev)
{
    int i;
    indent(lev);
    printf("%s\n", desc);
    if (pb == 0)
        return;
    indent(lev);
    printf("hsize=%d\n", pb->hsize);
    for (i = 0; i < pb->hsize; ++i) {
        dump_pb_entry("pb_entry", pb->ht[i], lev+1);
    }
}

void dump_psession(char* desc, Session* ps, int lev)
{
    int i;
    indent(lev);
    printf("%s\n", desc);
    if (ps == 0)
        return;
    indent(lev+1);
    dump_pblock("pSession->client", ps->client, lev+2);
}

void dump_prequest(char* desc, Request* pr, int lev)
{
    int i;
    indent(lev);
    printf("%s\n", desc);
    if (pr == 0)
        return;
    indent(lev+1);
    dump_pblock("pRequest->vars", pr->vars, lev+2);
    dump_pblock("pRequest->reqpb", pr->reqpb, lev+2);
    indent(lev+2);
    printf("%s->loadhdrs=%d\n", desc, pr->loadhdrs);
    dump_pblock("pRequest->headers", pr->headers, lev+2);
    indent(lev+2);
    printf("%s->senthdrs=%d\n", desc, pr->senthdrs);
    dump_pblock("pRequest->srvhdrs", pr->srvhdrs, lev+2);
}

////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////

int dumpinit(pblock *pBlock, Session *pSession, Request *pRequest)
{
    printf("dumpinit\n");
    //pSession and pRequest are 0 when the Enterprise is initialized
    printf("pSession = %p\tpRequest = %p\n", pSession, pRequest);
    dump_pblock("pBlock", pBlock, 1);
    dump_psession("pSession", pSession, 1);
    dump_prequest("pRequest", pRequest, 1);
    fflush(stdout);
    return REQ_PROCEED;
}

////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////


int dumppb(pblock *pBlock, Session *pSession, Request *pRequest)
{
    printf("dumppb\n");
    dump_pblock("pBlock", pBlock, 1);
    dump_psession("pSession", pSession, 1);
    dump_prequest("pRequest", pRequest, 1);
    fflush(stdout);
    return REQ_PROCEED;
}
