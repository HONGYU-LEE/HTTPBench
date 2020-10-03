#ifndef __EPOLL_H__
#define __EPOLL_H__

#include<iostream>
#include<vector>
#include<sys/epoll.h>
#include<unistd.h>
#include"Socket.h"

const int EPOLL_SIZE = 1000;

struct Ret_Epoll
{
    Ret_Epoll(const Socket& socket, uint32_t events)
        : _socket(socket)
        , _events(events)
    {}
    Socket _socket;
    uint32_t _events;
};

class Epoll
{
    public:
        Epoll()
        {
            //现版本已经忽略size，随便给一个大于0的数字即可
            _epfd = epoll_create(1);

            if(_epfd < 0)
            {
                exit(0);
            }
        }

        ~Epoll()
        {
            close(_epfd);
        }

        //增加新的监控事件
        bool Add(const Socket& socket, uint32_t events = EPOLLIN | EPOLLOUT | EPOLLERR) const
        {
            int fd = socket.GetFd();

            //组织监控事件结构体
            struct epoll_event ev;
            ev.data.fd = fd;//设置需要监控的描述符
            ev.events = events;
            
            int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
            if(ret < 0)
            {
                return false;
            }

            return true;
        }

        //更改监控事件
        bool Mod(const Socket& socket, uint32_t events = EPOLLIN | EPOLLOUT | EPOLLERR) const
        {
            //组织监控事件结构体
            struct epoll_event ev;
            ev.data.fd = socket.GetFd();//设置需要监控的描述符
            ev.events = events;
            
            int ret = epoll_ctl(_epfd, EPOLL_CTL_MOD, socket.GetFd(), &ev);
            if(ret < 0)
            {
                return false;
            }

            return true;
        }

        //删除监控事件
        bool Del(const Socket& socket) const 
        {
            int fd = socket.GetFd();
            
            int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
            if(ret < 0)
            {
                return false;
            }
            
            return true;
        }

        //开始监控
        bool Wait(std::vector<Ret_Epoll>& vec, int timeout = 1000) const 
        {
            vec.clear();
            struct epoll_event evs[EPOLL_SIZE];
            
            //开始监控，返回值为就绪描述符数量
            int ret = epoll_wait(_epfd, evs, EPOLL_SIZE, timeout);
            
            //当前没有描述符就绪
            if(ret < 0)
            {
                return false;
            }
            //等待超时
            else if(ret == 0)
            {
                return false;
            }
            
            for(int i = 0; i < ret; i++)
            {
                //将所有就绪描述符放进数组中
                Socket new_socket;
                new_socket.SetFd(evs[i].data.fd);
                vec.push_back(Ret_Epoll(new_socket, evs[i].events));
            }

            return true;
        }

    private:
        //epoll的操作句柄
        int _epfd;
};

#endif // !__EPOLL_H__
