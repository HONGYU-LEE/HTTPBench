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
#include<signal.h>
#include <sys/time.h>

#define METHOD_GET 0           //get请求
#define MAXHOSTSIZE 256        //主机号最大长度
#define MAXREQUESTSIZE 1024    //请求包最大长度
#define MAXPORT 65535          //最大端口号

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

    int _success;   //成功连接数
    int _fail;      //失败连接数
    int _bytes;     //传输字节数
};

class HTTPBench
{
public:
    HTTPBench()
        : time_recpired(0)
        , method(METHOD_GET)
        , reload(0)
        , clients(1)
        , threads(1)
        , total_req(1000000)
        , bench_time(30)
        , proxy_port(80)
        , start_time(0)
        , end_time(0)
        , proxy_host("")
        , host("")
        , request("")
    {}

public:
    volatile int time_recpired; //超时标记
    int reload;                 //是否重新请求加载(无缓存)
    int method;                 //请求方法
    int threads;                //线程数
    int total_req;              //总连接数
    int clients;                //客户端个数, 默认1个
    int bench_time;             //超时时间, 默认30秒
    int proxy_port;             //代理服务器端口号
    int start_time;             //开始时间
    int end_time;               //结束时间
    string proxy_host;          //代理服务器主机号
    string host;                //主机号
    string request;             //请求报文

public:
    bool start(int argc,char*argv[]);       //启动程序
    void build_request(const char* str);    //创建http请求
    bool bench();                           //压力测试
    bool bench_core();                      //压力测试核心
    void create_conn();                     //创建一次连接
    void create_all_conn(int clients);      //创建所有连接
    
};

#endif // !__HTTPBENCH_H__ 
