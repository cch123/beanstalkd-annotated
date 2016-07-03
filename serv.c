#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include "dat.h"

struct Server srv = {
    Portdef,
    NULL,
    NULL,
    {
        Filesizedef,
    },
};


void
srvserve(Server *s)
{
    int r;
    Socket *sock;
    int64 period;

    if (sockinit() == -1) {
        twarnx("sockinit");
        exit(1);
    }

    s->sock.x = s;
    s->sock.f = (Handle)srvaccept;
    //比较两个连接“ticket”大小的回调函数
    s->conns.less = (Less)connless;
    s->conns.rec = (Record)connrec;

    //这个listen后面的参数是backlog
    //具体的含义比较有意思，并不是说就只能接受这么多连接
    r = listen(s->sock.fd, 1024);
    if (r == -1) {
        twarn("listen");
        return;
    }

    r = sockwant(&s->sock, 'r');
    if (r == -1) {
        twarn("sockwant");
        exit(2);
    }


    for (;;) {
        period = prottick(s);

        //调用epoll_wait，然后根据返回的事件来进行返回
        //h半连接？ => EPOLLHUP|EPOLLRDHUP
        //w事件 => EPOLLIN
        //r事件 => EPOLLOUT

        int rw = socknext(&sock, period);
        printf("%p\n", sock);

        if (rw == -1) {
            twarnx("socknext");
            exit(1);
        }

        if (rw) {
        	//s->sock和这里的临时变量sock竟然是同一个地址
        	//为什么需要这个临时变量呢？
        	//我猜可能是因为在socknext中需要用到Socket结构的双指针
        	//而Socket非指针结构没法进行二次取址

        	//那么两个变量是同一个地址是怎么做到的呢。。
        	printf("%p\n", sock);
        	printf("%p\n", &(s->sock));
        	sock->f(sock->x, rw);

        }
    }
}


void
srvaccept(Server *s, int ev)
{
    h_accept(s->sock.fd, ev, s);
}
