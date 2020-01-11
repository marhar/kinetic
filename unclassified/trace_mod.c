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

/* 
**  mod_kap.c -- kap apache module
*/ 

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"

/*----------------------------------------------------------------*
 * support functions
 *----------------------------------------------------------------*/

static const char* s(const char* s)
{
    return s ? s : "NULL";
}

static FILE* kopen()
{
    return fopen("/tmp/kaplog.txt", "a");
}

static void kclose(FILE* fp)
{
    fclose(fp);
}

static void klog(char* msg)
{
    FILE* fp;
    if ((fp = kopen()) != NULL) {
        fprintf(fp, "%5d: %s\n", getpid(), msg);
        kclose(fp);
    }
}

static void dump_request(request_rec *r)
{
    FILE* fp;
    if ((fp = kopen()) != NULL) {
        fprintf(fp, "            request_rec\n");
        fprintf(fp, "            -----------\n");
        fprintf(fp, "                 uri=%s\n", s(r->uri));
        fprintf(fp, "           path_info=%s\n", s(r->path_info));
        fprintf(fp, "            filename=%s\n", s(r->filename));
        fprintf(fp, "                args=%s\n", s(r->args));
        fprintf(fp, "            protocol=%s\n", s(r->protocol));
        fprintf(fp, "              method=%s\n", s(r->method));
        kclose(fp);
    }
}

static void dump_server_rec(server_rec *r)
{
    FILE* fp;
    if ((fp = kopen()) != NULL) {
        fprintf(fp, "            server_rec\n");
        fprintf(fp, "            ----------\n");
        fprintf(fp, "     server_hostname=%s\n", s(r->server_hostname));
        fprintf(fp, "                port=%d\n", r->port);
        fprintf(fp, "            loglevel=%d\n", r->loglevel);
        kclose(fp);
    }

}

static void dump_pool(pool *r)
{
    /* nothing we want to dump now */
}

static int fake(request_rec *r)
{
    time_t now;
    time(&now);
    r->content_type = "text/html";
    ap_send_http_header(r);
    if (!r->header_only) {
        ap_rputs("hello from <i>kap_uri()</i><br>\n", r);
        ap_rputs("<b>", r);
        ap_rputs(ctime(&now), r);
        ap_rputs("</b>\n", r);
    }
}

/*----------------------------------------------------------------*
 * handlers
 *----------------------------------------------------------------*/

static void kap_child_init(server_rec *s, pool *p)
{
    klog("kap_child_init");
    dump_server_rec(s);
    dump_pool(p);
}

static void kap_child_exit(server_rec *s, pool *p)
{
    klog("kap_child_exit");
    dump_server_rec(s);
    dump_pool(p);
}

static int kap_uri(request_rec *r)
{
    klog("kap_uri");
    dump_request(r);
    fake(r);
    return DONE;
}

/*----------------------------------------------------------------*
 * handlers
 *----------------------------------------------------------------*/

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
    kap_uri,               /* [#1] URI to filename translation    */
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
