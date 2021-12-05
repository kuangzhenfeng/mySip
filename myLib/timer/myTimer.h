#ifndef MY_TIMER_H
#define MY_TIMER_H

#include <signal.h>
#include <time.h>

/**
 * @brief 定时器回调函数
 * 
 * @param sig 信号的值
 * @param si 包函附加信息的结构siginfo, 其中si->si_value.sival_ptr中为创建定时器传入的参数（当传入参数为NULL时，此值为timer_id）
 * @param uc 为空
 */
typedef void (*timer_handler_t)(int sig, siginfo_t *si, void *uc);



/**
 * @brief 创建定时器（单次定时）
 * 
 * @param timer_id 定时器id
 * @param ms 定时时间，单位为毫秒（ms）
 * @param timer_handler 定时器回调函数，定时时间到后自动执行
 * @param ptr 传给timer_handler的参数，可在回调函数中使用si->si_value.sival_ptr取回（当ptr为NULL时，会传入timer_id）
 * @return int 成功：0 失败：-1
 */
int my_timer_create(timer_t *timer_id, int ms, timer_handler_t timer_handler, void *ptr);


/**
 * @brief 设置定时器时间
 * 
 * @param timer_id 定时器id
 * @param ms 
 * @return int 成功：0 失败：-1
 */
int my_timer_settime(timer_t timer_id, int ms);


/**
 * @brief 获取定时器剩余时间
 * 
 * @param timer_id 定时器id
 * @param ms 
 * @return int 成功：0 失败：-1
 */
int my_timer_gettime(timer_t timer_id, int *ms);


/**
 * @brief 删除定时器
 * 
 * @param timer_id 定时器id
 * @return int 成功：0 失败：-1
 */
int my_timer_delete(timer_t timer_id);

#endif
