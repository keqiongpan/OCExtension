//
//  OCExtension.m
//
//  Created by keqiongpan@163.com on 2015/11/23.
//  Copyright © 2015 SVS. All rights reserved.
//


#import <Foundation/Foundation.h>
#import "OCExtension.h"


#pragma mark - 调试器相关操作


/**
 * 异常捕获状态缓存，用于暂停调试器捕获异常能力时，对原来的设置进行备份。
 */
static exception_catch_state _exception_catch_state = {0};


/**
 *  判断当前程序是否处在被调试状态。
 */
int is_debugging()
{
    kern_return_t retcode = KERN_FAILURE;
    task_t task = mach_task_self();
    exception_catch_state state = {0};

    state.mask = EXC_MASK_BREAKPOINT;
    state.count = sizeof(state.masks) / sizeof(state.masks[0]);
    retcode = task_get_exception_ports(task,
                                       state.mask,
                                       state.masks,
                                       &state.count,
                                       state.ports,
                                       state.behaviors,
                                       state.flavors);

    return ((KERN_SUCCESS == retcode) && (state.count > 0) && state.ports[0]);
}


/**
 *  保存异常捕获状态，成功返回非0，否则返回0。
 */
int store_exception_catch_state(exception_catch_state *state)
{
    kern_return_t retcode = KERN_FAILURE;
    task_t task = mach_task_self();
    exception_mask_t try_masks[] = {
        EXC_MASK_ALL & ~EXC_MASK_RESOURCE,  // 系统定义的全部异常类型。
        EXC_MASK_BAD_ACCESS      | \
        EXC_MASK_BAD_INSTRUCTION | \
        EXC_MASK_ARITHMETIC      | \
        EXC_MASK_EMULATION       | \
        EXC_MASK_SOFTWARE        | \
        EXC_MASK_BREAKPOINT      | \
        EXC_MASK_SYSCALL         | \
        EXC_MASK_MACH_SYSCALL    | \
        EXC_MASK_RPC_ALERT       | \
        EXC_MASK_MACHINE                    // 仅包含感兴趣的异常类型。
    };
    int i = 0;

    if (!state) {
        return FALSE;
    }

    // 根据异常类型的范围从大到小尝试获取其异常端口配置，成功则直接返回。

    for (i = 0; i < sizeof(try_masks) / sizeof(try_masks[0]); ++i) {
        memset(state, 0, sizeof(*state));
        state->mask = try_masks[i];
        state->count = sizeof(state->masks) / sizeof(state->masks[0]);
        retcode = task_get_exception_ports(task,
                                           state->mask,
                                           state->masks,
                                           &state->count,
                                           state->ports,
                                           state->behaviors,
                                           state->flavors);
        if (KERN_SUCCESS == retcode) {
            break;
        }
    }

    return (KERN_SUCCESS == retcode);
}


/**
 *  恢复异常捕获状态，成功返回非0，否则返回0。
 */
int restore_exception_catch_state(const exception_catch_state *state)
{
    kern_return_t retcode = KERN_SUCCESS;
    task_t task = mach_task_self();
    int i = 0;

    // 如果传入参数有误，则认为没有什么需要恢复的，直接返回成功。

    if (!state || state->count > sizeof(state->masks) / sizeof(state->masks[0])) {
        return TRUE;
    }

    // 遍历异常捕获状态中的保存的异常端口配置，并恢复到当前进程。

    for (i = 0; i < state->count; ++i) {
        retcode = task_set_exception_ports(task,
                                           state->masks[i],
                                           state->ports[i],
                                           state->behaviors[i],
                                           state->flavors[i]);
        if (KERN_SUCCESS != retcode) {
            break;
        }
    }

    return (KERN_SUCCESS == retcode);
}


/**
 *  暂停调试器捕获异常的能力，成功返回非0，否则返回0。
 */
int suspend_debuger_catching()
{
    kern_return_t retcode = KERN_FAILURE;
    exception_mask_t suspend_mask;

    // 如果此前已经暂停调试器，则直接返回成功。

    if (_exception_catch_state.mask) {
        return TRUE;
    }

    // 保存当前进程的异常捕获状态。

    if (!store_exception_catch_state(&_exception_catch_state)) {
        return FALSE;
    }

    // 重置调试器捕获异常所用到的异常端口。

    suspend_mask = EXC_MASK_ALL;
    suspend_mask &= ~EXC_MASK_CRASH;
    suspend_mask &= ~EXC_MASK_RESOURCE;
    suspend_mask &= ~EXC_MASK_CORPSE_NOTIFY;
    suspend_mask &= ~EXC_MASK_BREAKPOINT;

    retcode = task_set_exception_ports(mach_task_self(), suspend_mask, 0, 0, 0);

    return (KERN_SUCCESS == retcode);
}


/**
 *  恢复调试器捕获异常的能力，成功返回非0，否则返回0。
 */
int resume_debuger_catching()
{
    int result = FALSE;

    // 恢复调试器捕获异常所用到的异常端口。

    result = restore_exception_catch_state(&_exception_catch_state);

    // 如果恢复成功，则清空缓存以表示状态已被恢复。

    if (result) {
        _exception_catch_state.mask = 0;
    }

    return result;
}


#pragma mark - 系统信号相关操作


/**
 *  替换系统信号处理器，成功返回非0，否则返回0。
 */
int replace_signal_handler(int signal, sig_t handler, signal_handler_state *state)
{
    if (!signal || !handler || !state) {
        return FALSE;
    }

    memset(state, 0, sizeof(*state));
    state->signal = signal;
    state->replacement.sa_handler = handler;
    sigemptyset(&state->replacement.sa_mask);

    return (0 == sigaction(state->signal,
                           &state->replacement,
                           &state->original));
}


/**
 *  恢复系统信号处理器，成功返回非0，否则返回0。
 */
int restore_signal_handler(const signal_handler_state *state)
{
    // 如果输入参数有误，则认为没有什么需要恢复的，直接返回成功。

    if (!state || !state->signal) {
        return TRUE;
    }

    return (0 == sigaction(state->signal,
                           &state->original,
                           NULL));
}


#pragma mark - 内存访问相关操作


/**
 *  无效内存信号跳转状态。
 */
static sigjmp_buf _badmemory_sigjmp_state;


/**
 *  无效内存信号处理器。
 */
static void badmemory_signal_handler(int signal)
{
    siglongjmp(_badmemory_sigjmp_state, 1);
}


/**
 *  测试指定内存块的访问权限，可以访问则返回非0，否则返回0。
 */
static int access_memory(void *address, size_t size, int writing)
{
    int accessable = FALSE;
    signal_handler_state segv;
    signal_handler_state bus;

    // 安装无效内存信号处理器。

    if (!replace_signal_handler(SIGSEGV, &badmemory_signal_handler, &segv)) {
        return FALSE;
    }

    if (!replace_signal_handler(SIGBUS, &badmemory_signal_handler, &bus)) {
        restore_signal_handler(&segv);
        return FALSE;
    }

    // 暂停调试器捕获异常的能力，使得访问无效内存时可以走到信号处理器中。

    // 注意：由于暂停调试器捕获异常的能力后，调试器的部份功能在恢复前不能正常工
    //       作，所以建议不要在恢复调试器捕获异常的能力前进行代码调试，通常这只
    //       在很短的一段代码执行期间，不会影响对程序其它功能的调试。

    if (!suspend_debuger_catching()) {
        restore_signal_handler(&bus);
        restore_signal_handler(&segv);
        return FALSE;
    }

    // 设置无效内存信号返回点，并根据分支判断访问是否成功。

    accessable = (sigsetjmp(_badmemory_sigjmp_state, 1) == 0);

    // 根据待校验的访问权限进行内存测试。

    if (accessable) {
        void *pointer = (void *)address;
        void *chrlimit = (void *)address + size;
        void *intlimit = chrlimit - sizeof(int);
        char chrx;
        int intx;

        if (writing) {
            for (; pointer <= intlimit; pointer += sizeof(int)) {
                intx = *(int *)pointer;
                *(int *)pointer = intx;
            }

            for (; pointer < chrlimit; ++pointer) {
                chrx = *(char *)pointer;
                *(char *)pointer = chrx;
            }
        }
        else {
            for (; pointer <= intlimit; pointer += sizeof(int)) {
                intx = *(int *)pointer;
            }

            for (; pointer < chrlimit; ++pointer) {
                chrx = *(char *)pointer;
            }
        }
    }

    // 恢复调试器捕获异常的能力。

    resume_debuger_catching();

    // 卸载无效内存信号处理器。

    restore_signal_handler(&bus);
    restore_signal_handler(&segv);

    return accessable;
}


/**
 *  校验指定内存块是否可以读取，可以读取则返回非0，否则返回0。
 */
int is_memory_readable(const void *address, size_t size)
{
    return access_memory((void *)address, size, FALSE);
}


/**
 *  校验指定内存块是否可以读写，可以读写则返回非0，否则返回0。
 */
int is_memory_writable(void *address, size_t size)
{
    return access_memory(address, size, TRUE);
}


#pragma mark - 对象原型相关操作


/**
 *  校验指针指向的内容是否是有效对象。
 */
int is_object_pointer(const void *pointer)
{
    void **instance = (void **)pointer;
    void **selfclass = NULL;
    void **metaclass = NULL;
    void **baseclass = NULL;

    // pointer->selfclass <= pointer->instance->isa

    if (instance && is_memory_readable(instance, sizeof(void *))) {
        selfclass = *instance;
    }

    // pointer->metaclass <= pointer->selfclass->isa

    if (selfclass && is_memory_readable(selfclass, sizeof(void *))) {
        metaclass = *selfclass;
    }

    // pointer->baseclass <= pointer->metaclass->isa

    if (metaclass && is_memory_readable(metaclass, sizeof(void *))) {
        baseclass = *metaclass;
    }

    // pointer->baseclass == pointer->baseclass->isa

    if (baseclass && is_memory_readable(baseclass, sizeof(void *))) {
        if (baseclass != *baseclass) {
            return FALSE;
        }
    }

    // pointer->baseclass <= NSObject->baseclass
    //                    <= NSObject->metaclass
    //                    <= NSObject->selfclass->isa

    return (*(void ***)objc_unretainedPointer([NSObject class]) == baseclass);
}


/**
 *  校验指针指向对象是否是有效的类对象。
 */
int is_class_pointer(const void *pointer)
{
    if (!is_object_pointer(pointer)) {
        return FALSE;
    }

    id instance = objc_unretainedObject(pointer);

    return ([instance class] == instance);
}


/**
 *  校验指针指向对象是否是有效的实例对象。
 */
int is_instance_pointer(const void *pointer)
{
    if (!is_object_pointer(pointer)) {
        return FALSE;
    }

    id instance = objc_unretainedObject(pointer);

    return ([instance class] != instance);
}
