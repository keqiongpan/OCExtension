//
//  syssignal.h
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/29.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#ifndef OCExtension_syssignal_h
#define OCExtension_syssignal_h

#include <OCExtension/PublicHeader.h>
#include <signal.h>
#include <setjmp.h>
#include <pthread.h>


/**
 *  系统信号处理器状态。
 */
typedef struct prototype_signal_handler_state {

    /**
     *  系统信号。
     */
    int signal;

    /**
     *  替换行为发生的线程。
     */
    pthread_t thread;

    /**
     *  替换前的处理器。
     */
    struct sigaction original;

    /**
     *  替换后的处理器。
     */
    struct sigaction replacement;

} signal_handler_state;


/**
 *  替换系统信号处理器，成功返回非0，否则返回0。
 */
int
replace_signal_handler(int signal,
                       void(*handler)(int, siginfo_t *, void *),
                       signal_handler_state *state);


/**
 *  恢复系统信号处理器，成功返回非0，否则返回0。
 */
int
restore_signal_handler(const signal_handler_state *state);


/**
 *  signal_try_catch的try动作处理。
 */
int
stc_try_process(int *signals, unsigned int count);


/**
 *  signal_try_catch的catch动作处理。
 */
int
stc_catch_process();


/**
 *  signal_try_catch的finally动作处理。
 */
int
stc_finally_process();


/**
 *  定义signal_try、signal_catch、signal_finally()宏，用于截获一段代码片中抛出的
 *  指定系统信号异常。使用示例如下：
 *      int
 *      read_integer_from_pointer(void *pointer)
 *      {
 *          int number = 0;
 *          signal_try(SIGSEGV, SIGBUS) {
 *              number = *(int *)pointer;
 *          }
 *          signal_catch(signal) {
 *              fprintf(stderr,
 *                      "ERR: Read memory error, address:%p, size:%d, signal:%d.",
 *                      pointer,
 *                      sizeof(number),
 *                      signal);
 *              exit(EXIT_FAILURE);
 *          }
 *          signal_finally() {
 *              // nothing to do.
 *          }
 *          return number;
 *      }
 *
 *  @note 当截获一个信号值为0的异常时，表明signal_try_catch的动作执行不成功。一
 *        般来说可能是出现了嵌套的signal_try_catch动作，需要修改代码避免该问题。
 */
#define signal_try(...) \
    do { \
        extern sigjmp_buf stc_sigjmp_state; \
        int __stc_signals[] = {__VA_ARGS__}; \
        int __stc_throw_signal = 0; \
        int __stc_try_success = stc_try_process(__stc_signals, sizeof(__stc_signals) / sizeof(__stc_signals[0])); \
        if (__stc_try_success) { \
            __stc_throw_signal = sigsetjmp(stc_sigjmp_state, 1); \
            if (!__stc_throw_signal) { \
                do { \

#define signal_catch(signal) \
                } while (0); \
            } \
            stc_catch_process(); \
        } \
        if (!__stc_try_success || __stc_throw_signal) { \
            int signal = __stc_throw_signal; \
            __stc_throw_signal = (int)(signal); \
            do { \

#define signal_finally() \
            } while (0); \
        } \
    } while (0); \
    stc_finally_process();


#endif /* OCExtension_syssignal_h */
