#include <stdio.h>
#include <stdlib.h> 
#include "myTimer.h"
#include <QDebug>


int my_timer_create(timer_t *timer_id, int ms, timer_handler_t timer_handler, void *ptr)
{
    struct sigevent sev;
    struct sigaction sa;

    // if(timer_id != NULL)
    // {
    //     perror("timer_id is not NULL when create timer");
    //     return -1;
    //  }


    // 设置定时器信号回调函数
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGRTMIN, &sa, nullptr) < 0)
    {
        perror( "sigaction error" );
        return -1;
    }

    // 创建定时器
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_notify = SIGEV_SIGNAL;
    if(ptr == nullptr)
        sev.sigev_value.sival_ptr = timer_id;
    else
        sev.sigev_value.sival_ptr = ptr;
    if(timer_create(CLOCK_REALTIME, &sev, timer_id) < 0)
    {
        perror( "timer create error" );
        return -1;
    }

    // 设置定时器时间
    if(my_timer_settime(*timer_id, ms) < 0)
        return -1;

    printf("---sig is=%d\n", SIGRTMIN);
    printf("---create timer success!\n");

    return 0;
}

int my_timer_settime(timer_t timer_id, int ms)
{
    struct itimerspec its;

    if(timer_id == nullptr)
    {
        perror("timer_id is NULL");
        return -1;
    }

    its.it_value.tv_sec = ms / 1000;
    its.it_value.tv_nsec = ms % 1000 * 1000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timer_id, 0, &its, nullptr) < 0)
    {
        perror("settime error");
        return -1;
    }

    return 0;
}

int my_timer_gettime(timer_t timer_id, int *ms)
{
    struct itimerspec its;

    if(timer_id == nullptr)
    {
        perror("timer_id is NULL");
        return -1;
    }

    if(timer_gettime(timer_id, &its) < 0)
    {
        perror("get time error");
        return -1;
    }
    *ms = static_cast<int>(its.it_value.tv_sec * 1000 + its.it_value.tv_nsec / 1000000);

    return 0;
}


int my_timer_delete(timer_t timer_id)
{
    if(timer_id == nullptr)
    {
        perror("timer_id is NULL");
        return -1;
    }
        
    if(timer_delete(timer_id) < 0)
    {
        perror("delete timer error");
        return -1;
    }

    return 0;
}

