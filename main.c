#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include "dat.h"

static void
su(const char *user) 
{
    int r;
    struct passwd *pwent;

    errno = 0;
    pwent = getpwnam(user);
    if (errno) twarn("getpwnam(\"%s\")", user), exit(32);
    if (!pwent) twarnx("getpwnam(\"%s\"): no such user", user), exit(33);

    r = setgid(pwent->pw_gid);
    if (r == -1) twarn("setgid(%d \"%s\")", pwent->pw_gid, user), exit(34);

    r = setuid(pwent->pw_uid);
    if (r == -1) twarn("setuid(%d \"%s\")", pwent->pw_uid, user), exit(34);
}


static void
set_sig_handlers()
{
    int r;
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    r = sigemptyset(&sa.sa_mask);
    if (r == -1) twarn("sigemptyset()"), exit(111);

    r = sigaction(SIGPIPE, &sa, 0);
    if (r == -1) twarn("sigaction(SIGPIPE)"), exit(111);

    sa.sa_handler = enter_drain_mode;
    r = sigaction(SIGUSR1, &sa, 0);
    if (r == -1) twarn("sigaction(SIGUSR1)"), exit(111);
}

int
main(int argc, char **argv)
{
    int r;
    //全局的job list
    //因为所有的tube共用同一个job list
    struct job list = {};

    //进程名
    progname = argv[0];
    setlinebuf(stdout);

    optparse(&srv, argv+1);
    printf("main函数中的全局sock：%p\n", &(srv.sock));
    //-V的话会输出
    if (verbose) {
        printf("pid %d\n", getpid());
    }

    //addr和port都是在optparse里设置的
    r = make_server_socket(srv.addr, srv.port);
    if (r == -1) twarnx("make_server_socket()"), exit(111);
    //获取到server端的fd
    srv.sock.fd = r;

    prot_init();

    if (srv.user) su(srv.user);

    //设置信号处理
    set_sig_handlers();

    //启动设置-b参数的话，那么会进入这段逻辑
    //也就是操作会写binlog
    if (srv.wal.use) {
        // We want to make sure that only one beanstalkd tries
        // to use the wal directory at a time. So acquire a lock
        // now and never release it.
    	//设置文件锁
        if (!waldirlock(&srv.wal)) {
            twarnx("failed to lock wal dir %s", srv.wal.dir);
            exit(10);
        }

        //文件锁获取成功
        list.prev = list.next = &list;
        //从binglog.X文件中读取内容，并创建一个新的binglog.maxX+1
        //还原上次程序运行现场的tube list和job list
        walinit(&srv.wal, &list);
        //重新扫描一遍job list，将各自对应的job放到适当的数据结构中，参见这个函数的注释
        //因为里面的Ready之类的是一个堆结构，所以只还原job list也是不够的
        r = prot_replay(&srv, &list);
        if (!r) {
            twarnx("failed to replay log");
            return 1;
        }
    }

    //设置listen
    //事件循环等等
    srvserve(&srv);
    return 0;
}
