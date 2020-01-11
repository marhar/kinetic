// tstcfg.cc -- test config file in cfgfile.hh

#include "cfgfile.hh"

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

void test3(char* fname)
{
    int rc;
    cfgfile cfg;
    rc = cfg.read(fname);
    printf("rc = %d\n", rc);
    cfg.print();
}

main(int argc, char** argv)
{
    printf("==============================\n");
    test1();
    printf("==============================\n");
    test2();
    printf("==============================\n");
    test3(argv[1]);
}
