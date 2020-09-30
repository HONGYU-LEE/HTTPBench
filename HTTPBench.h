#ifndef __HTTPBENCH_H__
#define __HTTPBENCH_H__

#include<atomic>
#include<thread>
#include<algorithm>
#include<iostream>
#include<string>
#include<vector>
#include<cstdlib>

#include<unistd.h>
#include<stdio.h>
#include<sys/param.h>
//#include"rpc/types.h"
#include<getopt.h>
#include<cstring>
#include<time.h>
#include<signal.h>

#define METHOD_GET 0 //get请求的宏
#define METHOD_HEAD 1 //head请求的宏
#define PROGRAM_VERSION "zzy---1.5" //版本类型
#define MAXHOSTSIZE 256 //host最长256
#define MAXREQUESTSIZE 1024 //请求包最长1024
#define MAXPORT 65535//max port
#define MAXPTHREAD 100 //最多创建100线程
using namespace std;
using std::string;

//线程内部计数器，用来统计当前请求次数
class COUNT
{
    /*本线程运行内部的相关信息*/
public:
    COUNT()
        : _success(0)
        , _fail(0)
        , _bytes(0)
    {}

    int _success;
    int _fail;
    int _bytes;

    void getLocalCount(HTTPBench& bench);
    void atomic_add_suss();
    void atomic_add_fail();
    void atomic_add_byte();
};

class HTTPBench
{
public:
    HTTPBench()
        : time_recpired(0)
        , all_speed(0)
        , all_failed(0)
        , all_bytes(0)
        , method(METHOD_GET)
        , force(0)
        , clients(1)
        , bench_time(30)
        , proxy_port(80)
        , proxy_host("")
        , host("")
        , request("")
    {}

public:
    volatile int time_recpired;
    int all_speed;//可支持的访问数量
    int all_failed;//失败次数
    int all_bytes;//传输比特
    int method ;//请求方法
    int force; 
    int clients ;//线程个数
    int bench_time;//倒计时
    int proxy_port;//代理的端口
    string proxy_host;//代理的host
    string host;//host最大
    string request;//request最大
public:
    bool bench();//压力测试
    void bench_core(COUNT& t);//压力测试核心
    void build_request(string&);//创建http请求包
    void usage(void);//使用说明函数


    int start(int argc,char*argv[]);//启动压力测试
};

#endif // !__HTTPBENCH_H__ 
