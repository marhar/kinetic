// tstcfg.cc -- test config file in cfgfile.hh

#include "cfgfile.h"

#if 0
void test1(void)
{
    cfgfile cfg;
    cfg.print();

    cfg.numengines(3);
    cfg.engine("foo");
    cfg.partition("/aaa");
    printf("---\n");
    cfg.print();

    cfg.numengines(4);
    cfg.debug(2);
    cfg.engine("bar");
    cfg.partition("/bbb");
    printf("---\n");
    cfg.print();

    cfg.partition("/ccc");
    printf("---\n");
    cfg.print();
}

void test2()
{
    cfgfile cfg;
    cfg.print();
}
#endif

void test3(char* fname)
{
    int rc;
    cfgparms cfg;
    rc = cfgread(&cfg, fname);
    printf("rc = %d\n", rc);
    cfgprint(&cfg);
    cfgfree(&cfg);
}

int main(int argc, char** argv)
{
#if 0
    printf("==============================\n");
    test1();
    printf("==============================\n");
    test2();
    printf("==============================\n");
#endif
    test3(argv[1]);
    return 0;
}
