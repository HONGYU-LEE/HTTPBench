#ifndef __SOCKET_H__
#define __SOCKET_H__

#include<iostream>
#include<string>

#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>

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
        bool Send(const std::string& data)
        {
            int ret = send(_socket_fd, data.c_str(), data.size(), 0); 
            
            if(ret < 0)
            {
                std::cerr << "send error" << std::endl;
            }

            return true;
        }

        //接收数据
        bool Recv(std::string& data)
        {
            char buff[4096] = { 0 };
            
            int ret = recv(_socket_fd, buff, 4096, 0);

            if(ret == 0)
            {
                std::cerr << "connect error" << std::endl;
                return false;
            }
            else if(ret < 0)
            {
                std::cerr << "recv error" << std::endl;
                return false;
            }
            
            data.assign(buff, ret);

            return true;
        }

        void Close()
        {
            if(_socket_fd > 0)
            {
                close(_socket_fd);
                _socket_fd = -1;
            }
        }
               
    private:
        int _socket_fd;
};

#endif // !__SOCKET_H__