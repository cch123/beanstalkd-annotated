#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include "dat.h"

//struct的一种初始化方式
//会按照定义顺序把struct内的变量进行初始化
/*
struct Server {
    char *port;
    char *addr;
    char *user;

    Wal    wal;
    Socket sock;
    Heap   conns;
};
*/
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
    //这里printf会报error，NULL指针
    //printf("%p\n", sock);
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
    //这里打印也会报NULL指针printf("%p\n", sock);
    //在sockwant函数里把全局的server->sock传进去，并赋值给ev.data.ptr = s->sock;
    r = sockwant(&s->sock, 'r');
    if (r == -1) {
        twarn("sockwant");
        exit(2);
    }

    //这里打印会报空指针printf("%p\n", sock);
    for (;;) {
        period = prottick(s);

        //调用epoll_wait，然后根据返回的事件来进行返回
        //h半连接？ => EPOLLHUP|EPOLLRDHUP
        //w事件 => EPOLLIN
        //r事件 => EPOLLOUT

        //内部的Socket指针是在下面这个socknext函数中初始化的
        //sock = ev.data.ptr;
        //赋值之后就不为空了

        //这个值其实是在上面的sockwant里把全局的server->sock赋值给了ev.data.ptr
        //然后再从里面把ev.data.ptr取出来，赋给sock变量，不太清楚这么做的含义是什么
        int rw = socknext(&sock, period);
        printf("%p\n", sock);

        if (rw == -1) {
            twarnx("socknext");
            exit(1);
        }

        if (rw) {
        	//s->sock和这里的临时变量sock是同一个地址
        	//为什么需要这个临时变量呢？
        	//因为sock的值本身也是从全局的srv->sock得来的
        	//所以其实两者指向的是同一个地址
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
