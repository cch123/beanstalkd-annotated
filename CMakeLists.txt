cmake_minimum_required(VERSION 3.5)
project(beanstalkd_annotated)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    ct/ct.c
    ct/ct.h
    ct/internal.h
    conn.c
    darwin.c
    dat.h
    file.c
    freebsd.c
    heap.c
    job.c
    linux.c
    main.c
    ms.c
    net.c
    primes.c
    prot.c
    sd-daemon.c
    sd-daemon.h
    serv.c
    testheap.c
    testjobs.c
    testserv.c
    testutil.c
    time.c
    tube.c
    util.c
    walg.c)

add_executable(beanstalkd_annotated ${SOURCE_FILES})