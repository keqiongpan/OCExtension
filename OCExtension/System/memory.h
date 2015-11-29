//
//  memory.h
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/30.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#ifndef OCExtension_memory_h
#define OCExtension_memory_h

#include <OCExtension/PublicHeader.h>
#include <mach/mach.h>


/**
 *  校验指定内存块是否可以读取，可以读取则返回非0，否则返回0。
 */
int
is_memory_readable(const void *address, size_t size);


/**
 *  校验指定内存块是否可以读写，可以读写则返回非0，否则返回0。
 */
int
is_memory_writable(void *address, size_t size);


#endif /* OCExtension_memory_h */
