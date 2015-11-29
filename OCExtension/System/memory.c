//
//  memory.c
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/30.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#include <OCExtension/memory.h>
#include <OCExtension/debugger.h>
#include <OCExtension/syssignal.h>


int
is_memory_readable(const void *address, size_t size)
{
    int is_readable = TRUE;

    suspend_debugger_catching();
    signal_try(SIGSEGV, SIGBUS) {
        void *pointer = (void *)address;
        void *chrlimit = (void *)address + size;
        void *intlimit = chrlimit - sizeof(int);
        char chrx;
        int intx;

        for (; pointer <= intlimit; pointer += sizeof(int)) {
            intx = *(int *)pointer;
        }

        for (; pointer < chrlimit; ++pointer) {
            chrx = *(char *)pointer;
        }
    }
    signal_catch(signal) {
        is_readable = FALSE;
    }
    signal_finally() {
        // Nothing to do.
    }
    resume_debugger_catching();

    return is_readable;
}


int
is_memory_writable(void *address, size_t size)
{
    int is_writable = TRUE;

    suspend_debugger_catching();
    signal_try(SIGSEGV, SIGBUS) {
        void *pointer = (void *)address;
        void *chrlimit = (void *)address + size;
        void *intlimit = chrlimit - sizeof(int);
        char chrx;
        int intx;

        for (; pointer <= intlimit; pointer += sizeof(int)) {
            intx = *(int *)pointer;
            *(int *)pointer = intx;
        }

        for (; pointer < chrlimit; ++pointer) {
            chrx = *(char *)pointer;
            *(char *)pointer = chrx;
        }
    }
    signal_catch(signal) {
        is_writable = FALSE;
    }
    signal_finally() {
        // Nothing to do.
    }
    resume_debugger_catching();

    return is_writable;
}
