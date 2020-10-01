#ifndef __HTTPBENCH_H__
#define __HTTPBENCH_H__

#include<atomic>
#include<thread>
#include<iostream>
#include<string>
#include<vector>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<sys/param.h>
#include<getopt.h>
#include<time.h>
#include<signal.h>

#define METHOD_GET 0 //get请求的宏
#define METHOD_HEAD 1 //head请求的宏
#define MAXHOSTSIZE 256 //host最长256
#define MAXREQUESTSIZE 1024 //请求包最长1024
#define MAXPORT 65535//max port

using std::cout;
using std::endl;
using std::atomic;
using std::string;
using std::thread;
using std::vector;

//线程内部计数器，用来统计传输情况
class COUNT
{
public:
    COUNT()
        : _success(0)
        , _fail(0)
        , _bytes(0)
    {}

    int _success;   //成功数
    int _fail;      //失败数
    int _bytes;     //传输字节数
};

class HTTPBench
{
public:
    HTTPBench()
        : time_recpired(0)
        , g_atom_clinet(0)
        , g_atom_sussess(0)
        , g_atom_fail(0)
        , g_atom_byte(0)
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
    atomic<int> g_atom_clinet;  
    atomic<int> g_atom_sussess; //连接成功的数量
    atomic<int> g_atom_fail;    //连接失败的数量
    atomic<int> g_atom_byte;    //总传输量
    volatile int time_recpired; //超时标记
    int force;                  //是否等待服务器响应
    int method;                 //请求方法
    int clients ;               //客户端个数
    int bench_time;             //超时时间
    int proxy_port;             //代理服务器端口号
    string proxy_host;          //代理服务器主机号
    string host;                //主机号
    string request;             //请求报文

public:
    bool start(int argc,char*argv[]);       //启动程序
    void build_request(const char* str);    //创建http请求
    bool bench();                           //压力测试
    void bench_core(COUNT& t);              //压力测试核心
};

#endif // !__HTTPBENCH_H__ 
