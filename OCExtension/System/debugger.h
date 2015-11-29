//
//  debugger.h
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/29.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#ifndef OCExtension_debugger_h
#define OCExtension_debugger_h

#include <OCExtension/PublicHeader.h>


/**
 *  判断调试器是否正在运行。
 */
int
is_debugger_running();


/**
 *  暂停调试器捕获异常的能力，成功返回非0，否则返回0。
 */
int
suspend_debugger_catching();


/**
 *  恢复调试器捕获异常的能力，成功返回非0，否则返回0。
 */
int
resume_debugger_catching();


#endif /* OCExtension_debugger_h */
