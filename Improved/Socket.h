#ifndef __SOCKET_H__
#define __SOCKET_H__

#include<iostream>
#include<string>

#include<unistd.h>
#include<fcntl.h>

#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>

#define MAXBUFFSIZE 1024
class Socket
{
    public:
        Socket() : _socket_fd(-1)
        {}
        
        int GetFd() const 
        {
            return _socket_fd;
        }

        void SetFd(int fd)
        {
            _socket_fd = fd;
        }

        //创建套接字
        bool createSocket()
        {
            _socket_fd = socket(AF_INET, SOCK_STREAM, 0);

            if(_socket_fd < 0)
            {
                return false;
            }
            return true;
        }
    
        //发起连接请求
        bool Connect(const std::string& host, uint16_t port)
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);

            unsigned long in_addr = inet_addr(host.c_str());

            //如果IP地址无效，则自动获取当前网卡地址
            if(in_addr == INADDR_NONE)
            {
                struct hostent *hp = gethostbyname(host.c_str()); 
                if(hp == nullptr)
                {
                    return false;
                }
                addr.sin_addr = *((struct in_addr *)hp->h_addr);
            }
            else
            {
                addr.sin_addr.s_addr = in_addr;
            }
            
            int ret = connect(_socket_fd, (sockaddr*)&addr, sizeof(addr));
            if(ret < 0)
            {
                return false;
            }
            return true;
        }

        //发送数据
        bool Send(std::string& data)
        {
            int pos = 0;
            size_t len = data.size();

            if(len != send(_socket_fd, data.c_str(), len, 0))
            {
                return false;
            }
            return true;
        }

        //接收数据
        bool Recv()
        {
            char buff[1024] = { 0 };

            int bytes_read = recv(_socket_fd, buff, 1024, 0);
            if(bytes_read == -1)
            {
                return false;
            }
            else if(bytes_read == 0)
            {
                return false;
            } 

            return true;
        }   

        //关闭套接字
        void Close()
        {
            if(_socket_fd > 0)
            {
                close(_socket_fd);
                _socket_fd = -1;
            }
        }

        void SetNoBlock() 
        {
            int flag = fcntl(_socket_fd, F_GETFL, 0);
            
            flag |= O_NONBLOCK;
            fcntl(_socket_fd, F_SETFL, flag);
        }

    private:
        int _socket_fd;
};

#endif // !__SOCKET_H__