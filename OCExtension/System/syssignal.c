//
//  syssignal.c
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/29.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#include <OCExtension/syssignal.h>
#include <mach/mach.h>


/**
 *  signal_try_catch的错误返回位置。
 */
sigjmp_buf stc_sigjmp_state;


/**
 *  signal_try_catch的整个try-catch过程的互斥锁。
 */
static pthread_mutex_t stc_mutex;


/**
 *  signal_try_catch的系统信号处理器状态列表，内含替换前后的处理器，可在必要的时
 *  候恢复为替换前的处理器。列表元素的下标为对应的信号值。
 */
static signal_handler_state stc_states[NSIG];


/**
 *  当前是否在signal_try_catch的执行过程，以供校验避免同一线程多次执行此过程。
 */
static volatile int is_stc_processing = FALSE;


int
replace_signal_handler(int signal,
                       void(*handler)(int, siginfo_t *, void *),
                       signal_handler_state *state)
{
    if (!signal || !handler || !state) {
        return FALSE;
    }

    memset(state, 0, sizeof(*state));
    state->signal = signal;
    state->thread = pthread_self();
    state->replacement.sa_sigaction = handler;
    sigemptyset(&state->replacement.sa_mask);
    state->replacement.sa_flags = SA_SIGINFO;

    return (0 == sigaction(state->signal,
                           &state->replacement,
                           &state->original));
}


int
restore_signal_handler(const signal_handler_state *state)
{
    if (!state || !state->signal) {
        return FALSE;
    }

    return (0 == sigaction(state->signal,
                           &state->original,
                           NULL));
}


static
pthread_mutex_t *
get_stc_mutex()
{
    static int is_initialized = FALSE;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex);

    // 创建相同线程可以重复锁定的stc_mutex互斥体对象。

    if (!is_initialized) {
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&stc_mutex, &mutexattr);
        pthread_mutexattr_destroy(&mutexattr);
        is_initialized = TRUE;
    }

    pthread_mutex_unlock(&mutex);
    return &stc_mutex;
}


static
void
stc_handler(int signal, siginfo_t *info, void *uap)
{
    pthread_mutex_t *pstc_mutex = get_stc_mutex();
    pthread_t replacing_thread;
    void(*original_handler)(int, siginfo_t *, void *);
    int original_flags;

    // 进入stc_mutex互斥区并取出当前系统信号对应的stc_states信息，用于判别是系统
    // 信号抛出线程或是其它线程的误入者，误入者需要回调到signal_try_catch过程之
    // 前对应的信号处理器。

    // 能够进入stc_mutex互斥区，说明当前与signal_try_catch是同一线程或者
    // signal_try_catch整个过程已经完成，并且stc_catch_process已经对stc_mutex的
    // 锁定状态进行解锁。

    pthread_mutex_lock(pstc_mutex);
    replacing_thread = stc_states[signal].thread;
    original_handler = stc_states[signal].original.sa_sigaction;
    original_flags = stc_states[signal].original.sa_flags;
    pthread_mutex_unlock(pstc_mutex);

    // 如果当前线程与signal_try_catch是同一线程，则说明signal_try_catch的过程中
    // 抛出系统信号异常，需要立即从当前堆栈返回到signal_try预设的位置。

    if (pthread_self() == replacing_thread) {
        siglongjmp(stc_sigjmp_state, signal);
    }

    // 如果当前线程与signal_try_catch不是同一线程，则说明是其它线程的误入者，需
    // 要回调到signal_try_catch过程之前对应的信号处理器。

    if (original_handler) {
        if (original_flags & SA_SIGINFO) {
            original_handler(signal, info, uap);
        }
        else {
            ((sig_t)original_handler)(signal);
        }
    }
}


int
stc_try_process(int *signals, unsigned int count)
{
    unsigned int i;
    int signal;

    if (!signals) {
        return FALSE;
    }

    pthread_mutex_lock(get_stc_mutex());
    if (is_stc_processing) {
        pthread_mutex_unlock(get_stc_mutex());
        return FALSE;
    }

    is_stc_processing = TRUE;
    memset(stc_states, 0, sizeof(stc_states));

    for (i = 0; i < count; ++i) {
        signal = signals[i];

        if (signal < 1 || signal >= sizeof(stc_states) / sizeof(stc_states[0])) {
            continue;
        }

        if (!stc_states[signal].signal) {
            replace_signal_handler(signal,
                                   &stc_handler,
                                   &stc_states[signal]);
        }
    }

    return TRUE;
}


int
stc_catch_process()
{
    int i;

    for (i = 1; i < sizeof(stc_states) / sizeof(stc_states[0]); ++i) {
        if (stc_states[i].signal) {
            restore_signal_handler(&stc_states[i]);
        }
    }

    is_stc_processing = FALSE;
    pthread_mutex_unlock(get_stc_mutex());
    return TRUE;
}


int
stc_finally_process()
{
    return TRUE;
}
