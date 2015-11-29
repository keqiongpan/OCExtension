//
//  exception.h
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/29.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#ifndef OCExtension_exception_h
#define OCExtension_exception_h

#include <OCExtension/PublicHeader.h>
#include <mach/mach.h>


/**
 *  异常捕获状态，调试器通过该设置与被调试程序进行交互。
 */
typedef struct prototype_exception_catch_state {

    /**
     *  关注的异常类型，通常为EXC_MASK_ALL的子集。
     */
    exception_mask_t mask;

    /**
     *  涉及的异常端口配置个数。
     */
    mach_msg_type_number_t count;

    /**
     *  异常类型列表。
     */
    exception_mask_t masks[EXC_TYPES_COUNT];

    /**
     *  端口列表。
     */
    mach_port_t ports[EXC_TYPES_COUNT];

    /**
     *  行为列表。
     */
    exception_behavior_t behaviors[EXC_TYPES_COUNT];

    /**
     *  特性列表。
     */
    thread_state_flavor_t flavors[EXC_TYPES_COUNT];

} exception_catch_state;


/**
 *  获取当前进程指定异常类型的异常捕获状态，成功返回非0，否则返回0。
 */
int
get_exception_catch_state(exception_catch_state *state,
                          exception_mask_t mask);


/**
 *  设置当前进程的异常捕获状态，成功返回非0，否则返回0。
 */
int
set_exception_catch_state(const exception_catch_state *state);


#endif /* OCExtension_exception_h */
