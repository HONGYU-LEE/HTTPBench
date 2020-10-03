#include"HTTPBench.h"

extern HTTPBench http_bench;

int main(int argc,char* argv[])
{
    if(http_bench.start(argc, argv) == false)
    {
        cout << "启动失败, 请检查输入是否正确." << endl;
    }
    
    return 0;
}