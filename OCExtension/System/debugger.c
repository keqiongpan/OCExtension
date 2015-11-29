//
//  debugger.c
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/29.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#include <OCExtension/debugger.h>
#include <OCExtension/exception.h>
#include <pthread.h>


/**
 *  调用suspend_debugger_catching暂停调试器捕获异常能力时，用来备份原来的异常捕
 *  获状态，在调用resume_debugger_catching恢复调试器捕获异常能力时，会将其重新设
 *  置回当前进程。
 */
static exception_catch_state original_state = {0};


/**
 *  暂停调试器捕获异常能力的调用计数，避免重复暂停操作。
 */
static volatile unsigned int suspend_count = 0;


/**
 *  暂停调试器捕获异常能力的互斥锁，用在锁定暂停及恢复的操作。
 */
static pthread_mutex_t suspend_mutex = PTHREAD_MUTEX_INITIALIZER;


int
is_debugger_running()
{
    exception_mask_t debug_mask = EXC_MASK_BREAKPOINT;
    exception_catch_state debug_state = {0};

    if (!get_exception_catch_state(&debug_state, debug_mask)) {
        return FALSE;
    }

    return (debug_state.count > 0 && debug_state.ports[0]);
}


int
suspend_debugger_catching()
{
    int is_suspend_success = TRUE;
    exception_mask_t suspend_mask;
    exception_catch_state suspend_state = {0};

    pthread_mutex_lock(&suspend_mutex);

    if (suspend_count > 0) {
        ++suspend_count;
        is_suspend_success = TRUE;
        goto exit_suspend;
    }

    suspend_mask = EXC_MASK_ALL;
    suspend_mask &= ~EXC_MASK_CRASH;
    suspend_mask &= ~EXC_MASK_RESOURCE;
    suspend_mask &= ~EXC_MASK_CORPSE_NOTIFY;
    suspend_mask &= ~EXC_MASK_BREAKPOINT;

    if (!get_exception_catch_state(&original_state, suspend_mask)) {
        is_suspend_success = FALSE;
        goto exit_suspend;
    }

    suspend_state.count = 1;
    suspend_state.masks[0] = suspend_mask;

    if (!set_exception_catch_state(&suspend_state)) {
        is_suspend_success = FALSE;
        goto exit_suspend;
    }

    ++suspend_count;
    is_suspend_success = TRUE;

exit_suspend:
    pthread_mutex_unlock(&suspend_mutex);
    return is_suspend_success;
}


int
resume_debugger_catching()
{
    int is_resume_success = TRUE;

    pthread_mutex_lock(&suspend_mutex);

    if (suspend_count > 1) {
        --suspend_count;
        is_resume_success = TRUE;
        goto exit_resume;
    }

    if (!set_exception_catch_state(&original_state)) {
        is_resume_success = FALSE;
        goto exit_resume;
    }

    --suspend_count;
    is_resume_success = TRUE;

exit_resume:
    pthread_mutex_unlock(&suspend_mutex);
    return is_resume_success;
}
