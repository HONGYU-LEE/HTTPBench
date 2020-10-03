#include"HTTPBench.h"
#include"Socket.h"

HTTPBench http_bench;

atomic<int> g_atom_sussess; //连接成功的数量
atomic<int> g_atom_fail;    //连接失败的数量
atomic<int> g_atom_byte;    //总传输量

//命令映射表
static const struct option long_options[] = {
    {"force", no_argument, &http_bench.force, 1},
    {"reload", no_argument, &http_bench.reload, 1},
    {"time", required_argument, NULL, 't'},
    {"get", no_argument, &http_bench.method, METHOD_GET},
    {"proxy", required_argument, NULL, 'p'},
    {"total_req", required_argument, NULL, 'n'},
    {"clients", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}
};

//构建HTTP请求
void HTTPBench::build_request(const char* url)
{
    string s_url(url);
    request += "GET ";     //添加请求方法

    //检查URL是否规范
    if(s_url.size() > MAXHOSTSIZE)
    {
        cout << "URL长度超过超出限制." << endl;
        exit(EXIT_FAILURE);
    }

    size_t pos = s_url.find("://", 0);
    if(pos == string::npos)
    {
        cout << "找不到'://', URL结构不合法." << endl;
        exit(EXIT_FAILURE);
    }

    pos += 3;
    size_t host_end_pos = s_url.find('/', pos);
    if(host_end_pos <= pos)
    {
        cout << "主机名没有以'/'作为结尾, URL结构不合法." << endl;
        exit(EXIT_FAILURE);
    }

    //如果没有代理服务器
    if(proxy_host.empty())
    {
        if(s_url.compare(0, 7, "http://") != 0)
        {
            cout << "本工具仅支持HTTP1.1" << endl;
            exit(EXIT_FAILURE);
        }
    }

    //如果没有代理服务器
    size_t port_pos = s_url.find(':', pos);
    if(proxy_host.empty())
    {
        //如果有端口号存在
        if((port_pos != string::npos) && (port_pos < s_url.find('/', pos)))
        {
            host = host.assign(url + pos, port_pos - pos); //设置主机号
            s_url.assign(s_url, port_pos + 1, host_end_pos - pos - 1);  //拷贝端口号
            proxy_port = atoi(s_url.assign(s_url, s_url.find(':',pos) + 1, s_url.find('/', pos) - pos - 1).c_str());   //设置代理端口号

            //如果没设置代理服务器端口号，则默认为80
            if(proxy_port == 0)
            {
                proxy_port = 80;
            }
        }
        //如果没有则直接拷贝主机名
        else
        {
            host = host.assign(url + pos, s_url.rfind('/', s_url.size() - 1) - pos);
        }
        request += url + pos + strcspn(url + pos,"/");
    }
    //如果有代理服务器，直接拷贝URL到请求行中
    else
    {
        request += url;
    }
    
    request +=" HTTP/1.1\r\n";          //添加协议版本

    //设置请求报头
    if(proxy_host.empty())
    {
	    request += "Host: ";
	    request += host;
	    request += "\r\n";
    }
    
    if(reload && !proxy_host.empty())
    {
	    request += "Pragma: no-cache\r\n";
    }

    request += "Connection:close\r\n";  //不支持长连接
    request += "\r\n";                  //添加空行
}

//信号处理事件
static void handle_sigalrm(int sig)
{
    http_bench.time_recpired = 1;
}

//测试核心
void HTTPBench::bench_core(COUNT& count)
{
    Socket socket;
    host = (proxy_host.empty()) ? host : proxy_host;    //如果有代理host则使用代理host
    char buff[1024] = {0};

    struct timeval s_cur_time;
    gettimeofday(&s_cur_time, NULL);
    start_time = s_cur_time.tv_sec;

    signal(SIGALRM, handle_sigalrm);    //注册信号处理事件
    alarm(bench_time);                  //开始计时

nextTry:
    while(1)
    {
        //超时则不再执行任务
        if(time_recpired == 1 || g_atom_sussess >= total_req)
        {
            gettimeofday(&s_cur_time, NULL);
            end_time = s_cur_time.tv_sec;

            return;
        }

        //套接字创建失败
        if(socket.createSocket() == false)
        {
            ++count._fail;
            continue;
        }

        //连接建立失败
        if(socket.Connect(host, proxy_port) == false)
        {
            //++count._fail;
            ++g_atom_fail;
            socket.Close();
            continue;
        }

        //如果发送的数据的长度和请求长度不同，则说明发送失败
        size_t req_len = request.size();
        if(req_len != write(socket.GetFd(), request.c_str(), req_len))
        {
            //++count._fail;
            ++g_atom_fail;
            socket.Close();
            continue;
        }

        //默认阻塞等待服务器回复
        if(force == 0)
        {
            while(1)
            {
                if(time_recpired == 1 || (g_atom_fail + g_atom_sussess) >= total_req)
                {
                    break;
                }

                int read_sz = read(socket.GetFd(), buff, 1024);

                //没有接收到任何数据
                if(read_sz < 0)
                {
                    //++count._fail;
                    ++g_atom_fail;
                    socket.Close();

                    goto nextTry;   //继续下一轮压力发送
                }
                //连接已断开或者传输已完成
                else if(read_sz == 0)
                {
                    break;
                }
                //有数据到来, 记录传输量
                else
                {
                    g_atom_byte += read_sz;
                }
            }
        }

        socket.Close();     //关闭套接字
        //++count._success;   //成功建立一次连接
        ++g_atom_sussess;
    }
}

//子线程工作函数
static void thr_work()
{
    COUNT count;    //创建计数器

    http_bench.bench_core(count);   //线程开始工作
    
    /*
    //原子计数
    g_atom_sussess += count._success;
    g_atom_fail    += count._fail;
    g_atom_byte    += count._bytes;
    */
}

//压力测试
bool HTTPBench::bench()
{
    //首先发起一次连接，确保目标服务器具备通信的能力

    //创建套接字
    Socket socket;
    if(socket.createSocket() == false)
    {
        cout << "套接字创建错误." << endl;
        return false;
    }

    //发起连接
    string sock_host = (proxy_host.empty()) ? host : proxy_host;    //如果有代理host则使用代理host
    if(socket.Connect(sock_host, proxy_port) == false)
    {
        cout << "套接字连接失败." << endl;
        socket.Close();
        return false;
    }
    socket.Close(); //关闭套接字

    vector<thread> task;  //线程数组

    //创建线程
    for(int i = 0; i < clients; i++)
    {
        task.push_back(thread(thr_work));
    }

    //等待所有线程退出
    for(auto& th : task)
    {
        th.join();
    }

    cout << "\n\n" << endl;
    cout << "-----------压力测试结束-----------" << endl;
    cout << "连接速率   : "<< (g_atom_sussess  + g_atom_fail) / (end_time - start_time) << " pages/sec" << endl;
    cout << "传输速率   : "<< g_atom_byte / (end_time - start_time) << " bytes/sec"<< endl;
    cout << "成功数     : "<< g_atom_sussess << endl;
    cout << "失败数     : "<< g_atom_fail << endl;
    cout << "----------------------------" << endl;
   
    return true;
}

//启动函数
bool HTTPBench::start(int argc, char* argv[])
{
    int option = 0;
    int options_index = 0;
    int index = 0;

    //没给参数, 直接返回
    if(argc == 1)
    {
        return false;
    }

    while((option = getopt_long(argc, argv, "frt:p:c:n:", long_options, &options_index)) != EOF)
    {
        switch (option)
        {
        case 'f' : force = 1;   //不等待服务器响应
            break;
        case 'r' : reload = 1;  //无缓存, 重新加载
        case 't' : bench_time = atoi(optarg);   //设置超时时间
            break;
        case 'p' : 
            proxy_host = optarg;    //设置代理服务器 
            index = proxy_host.rfind(':', proxy_host.size() - 1 );  //找到端口号的位置

            if(index ==  string::npos)
            {
                cout << "':'不存在，缺少端口号." << endl;
                return false;
            }
            
            if(index == 0)
            {
                cout << "':'在首位置, 缺少主机号." << endl;
                return false;
            }

            if(index == proxy_host.size() - 1)
            {
                cout << "':'在末尾，缺少端口号." << endl;
                return false;
            }

            proxy_port = atoi(proxy_host.substr(index + 1, proxy_host.size() - index - 1).c_str());   //设置代理端口号
            proxy_host = proxy_host.substr(0, index);   //设置代理主机号

            if(proxy_host.size() > MAXHOSTSIZE)
            {
                cout << "host超出最大范围." << endl;
                return false;
            }

            if(proxy_port > MAXPORT)
            {
                cout << "端口号超出范围." << endl;
                return false;
            }

            break;
        case 'n' : total_req = atoi(optarg);    //设置总连接数
            break;
        case 'c' : clients = atoi(optarg);  //设置并发的线程数
            break;
        default  : cout << "请确认参数是否正确." << endl;  
            break;
        
        }
    }

    build_request(argv[optind]);    //构建HTTP请求

    cout << "-----------开始压力测试-----------" << endl;
    cout << "并发数    : " << clients << endl;
    cout << "总连接数  : " << total_req << endl;
    cout << "测试时间  : " << bench_time << endl;
    cout << "---------------------------------" << endl;

    if(force != 0)
    {
        cout << "当前处于强制返回模式，不会等待服务器回复!" << endl;
    }

    if(reload != 0)
    {
        cout << "当前处于强制重新加载模式，不会缓存!" << endl;
    }

    if(!proxy_host.empty())
    {
        cout << "代理服务器 : " << proxy_host << ':' << proxy_port << endl;
    }
    
    return bench();     //开始压力测试
}