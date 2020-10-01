#include"HTTPBench.h"

extern HTTPBench http_bench;

int main(int argc,char* argv[])
{
    http_bench.start(argc, argv);
    return 0;
}