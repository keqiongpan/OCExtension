//
//  OCExtension.h
//
//  Created by keqiongpan@163.com on 2015/11/23.
//  Copyright © 2015 SVS. All rights reserved.
//


#import <mach/mach.h>


#pragma mark - 调试器相关操作


/**
 *  异常捕获状态，调试器通过该设置与被调试程序进行交互。
 */
typedef struct prototype_exception_catch_state {

    /**
     *  此异常捕获状态所关注的异常类型，通常为EXC_MASK_ALL的子集。
     */
    exception_mask_t mask;

    /**
     *  此异常捕获状态所关注的异常类型所涉及的异常端口配置个数。
     */
    mach_msg_type_number_t count;

    /**
     *  异常端口配置中异常类型的列表。
     */
    exception_mask_t masks[EXC_TYPES_COUNT];

    /**
     *  异常端口配置中端口的列表。
     */
    mach_port_t ports[EXC_TYPES_COUNT];

    /**
     *  异常端口配置中行为的列表。
     */
    exception_behavior_t behaviors[EXC_TYPES_COUNT];

    /**
     *  异常端口配置中特性的列表。
     */
    thread_state_flavor_t flavors[EXC_TYPES_COUNT];

} exception_catch_state;


/**
 *  判断当前程序是否处在被调试状态。
 */
int is_debugging();


/**
 *  保存异常捕获状态，成功返回非0，否则返回0。
 */
int store_exception_catch_state(exception_catch_state *state);


/**
 *  恢复异常捕获状态，成功返回非0，否则返回0。
 */
int restore_exception_catch_state(const exception_catch_state *state);


/**
 *  暂停调试器捕获异常的能力，成功返回非0，否则返回0。
 *
 *  @note 在暂停调试器捕获异常能力的期间，程序产生的异常如果没被\@try...\@catch
 *        或singal(...)预设的处理程序捕获的话，程序将会按系统定义的默认行为进行
 *        处理，一般来说是程序崩溃退出。
 */
int suspend_debuger_catching();


/**
 *  恢复调试器捕获异常的能力，成功返回非0，否则返回0。
 */
int resume_debuger_catching();


#pragma mark - 系统信号相关操作


/**
 *  系统信号处理器状态。
 */
typedef struct prototype_signal_handler_state {

    /**
     *  系统信号编码。
     */
    int signal;

    /**
     *  系统信号对应的替换前的行为。
     */
    struct sigaction original;

    /**
     *  系统信号对应的替换后的行为。
     */
    struct sigaction replacement;

} signal_handler_state;


/**
 *  替换系统信号处理器，成功返回非0，否则返回0。
 */
int replace_signal_handler(int signal, sig_t handler, signal_handler_state *state);


/**
 *  恢复系统信号处理器，成功返回非0，否则返回0。
 */
int restore_signal_handler(const signal_handler_state *state);


#pragma mark - 内存访问相关操作


/**
 *  校验指定内存块是否可以读取，可以读取则返回非0，否则返回0。
 */
int is_memory_readable(const void *address, size_t size);


/**
 *  校验指定内存块是否可以读写，可以读写则返回非0，否则返回0。
 */
int is_memory_writable(void *address, size_t size);


#pragma mark - 对象原型相关操作


/**
 *  校验指针指向的内容是否是有效对象。
 */
int is_object_pointer(const void *pointer);


/**
 *  校验指针指向对象是否是有效的类对象。
 */
int is_class_pointer(const void *pointer);


/**
 *  校验指针指向对象是否是有效的实例对象。
 */
int is_instance_pointer(const void *pointer);
