#include"HTTPBench.h"
#include"Socket.h"

std::vector<std::thread> task;
HTTPBench http_bench;
atomic<int> g_atom_clinet(0);
atomic<int> g_atom_sussess(0);
atomic<int> g_atom_fail(0);
atomic<int> g_atom_byte(0);

//构建HTTP请求
void HTTPBench::build_request(string& url)
{
    request += "GET ";     //添加请求方法

    //检查URL是否规范
    if(url.size() > MAXHOSTSIZE)
    {
        std::cout << "URL长度超过超出限制." << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t pos = url.find("://");
    if(pos == string::npos)
    {
        std::cout << "找不到'://', URL结构不合法." << std::endl;
        exit(EXIT_FAILURE);
    }

    pos += 3;
    size_t host_end_pos = url.find('/', pos);
    if(host_end_pos != string::npos)
    {
        std::cout << "主机名没有以'/'作为结尾, URL结构不合法." << std::endl;
        exit(EXIT_FAILURE);
    }

    //如果没有代理服务器
    if(proxy_host.empty())
    {
        if(url.compare(0, 7, "http://") == false)
        {
            std::cout << "本工具仅支持HTTP协议" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    //如果没有代理服务器
    size_t port_pos = url.find(':', pos);
    if(proxy_host.empty())
    {
        //如果有端口号存在
        if((port_pos != string::npos) && (port_pos < url.find('/', pos)))
        {
            host = url.substr(pos, port_pos - pos); //设置主机号
            url.assign(url, port_pos + 1, host_end_pos - pos - 1);  //拷贝端口号
            proxy_port = atoi(url.assign(url, port_pos + 1, host_end_pos - pos - 1).c_str());   //设置代理端口号

            //如果没设置代理服务器端口号，则默认为80
            if(proxy_port == 0)
            {
                proxy_port = 80;
            }
        }
        //如果没有则直接拷贝主机名
        else
        {
            host = url.substr(pos, host_end_pos - pos);
        }
    }
    //如果有代理服务器，直接拷贝URL到请求行中
    else
    {
        request += url;
    }
    
    request +=" HTTP/1.1\r\n";      //添加协议版本

    //设置请求报头
    request += "User-Agent: WebBench "PROGRAM_VERSION"\r\n"; 
    
    if(proxy_host.empty())
    {
	    request += "Host: ";
	    request += host;
	    request +="\r\n";
    }
    
    request += "Connection:close\r\n";  //不支持长连接
    request += "\r\n";  //添加空行
}

void HTTPBench::bench_core(COUNT& count)
{
    Socket socket;
    char buff[1024] = {0};

    alarm(bench_time);   //开始计时
    if(!proxy_host.empty())
    {
        host = proxy_host;
    }

nextTry:
    while(1)
    {
        //超时，不再执行任务
        if(time_recpired != 0)
        {
            return;
        }

        //套接字创建失败
        if(socket.createSocket() == false)
        {
            ++count._fail;
            continue;
        }

        size_t req_len = request.size();
        //如果发送的数据的长度和请求长度不同，则说明发送失败
        if(req_len != write(socket.GetFd(), request.c_str(), req_len))
        {
            ++count._fail;
            socket.Close();
            continue;
        }

        //默认阻塞等待服务器回复
        if(force == 0)
        {
            while(1)
            {
                if(time_recpired != 0)
                {
                    break;
                }

                int read_sz = read(socket.GetFd(), buff, 1024);
                //没有接收到任何数据
                if(read_sz < 0)
                {
                    ++count._fail;
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
                    count._bytes += read_sz;
                }
            }
        }

        socket.Close(); //关闭套接字
        ++count._success;   //成功建立一次连接
    }
}

void thr_work(void* ptr)
{
    COUNT count;    //创建计数器

    http_bench.bench_core(count);
    
    g_atom_sussess += count._success;
    g_atom_fail += count._fail;
    g_atom_byte += count._bytes;
}

bool HTTPBench::bench()
{
    //进行短连接，连接一次后直接关闭
    Socket socket;
    //创建套接字
    if(socket.createSocket() == false)
    {
        std::cout << "套接字创建错误." << std::endl;
        return false;
    }
    
    string sock_host = (proxy_host.empty()) ? host : proxy_host;    //如果有代理host则使用代理host
    //发起连接
    if(socket.Connect(sock_host, proxy_port) == false)
    {
        std::cout << "套接字连接失败." << std::endl;
    }

    socket.Close(); //关闭套接字

    //创建线程
    for(int i = 0; i < clients; i++)
    {
        thread t(thr_work);
        task.push_back(t);
    }

    //等待所有线程退出
    for(auto& th : task)
    {
        th.join();
    }
}

//原子计数
void COUNT::atomic_add_suss()
{
    g_atom_sussess += _success;
}

void COUNT::atomic_add_fail()
{
    g_atom_fail += _fail;
}

void COUNT::atomic_add_byte()
{
    g_atom_byte += _bytes;
}
