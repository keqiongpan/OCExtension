//
//  exception.c
//  OCExtension
//
//  Created by keqiongpan@163.com on 15/11/29.
//  Copyright © 2015年 Wood Studio. All rights reserved.
//

#include <OCExtension/exception.h>


int
get_exception_catch_state(exception_catch_state *state,
                          exception_mask_t mask)
{
    kern_return_t retcode;

    if (!state) {
        return FALSE;
    }

    memset(state, 0, sizeof(*state));
    state->mask = mask;
    state->count = sizeof(state->ports) / sizeof(state->ports[0]);

    retcode = task_get_exception_ports(mach_task_self(),
                                       state->mask,
                                       state->masks,
                                       &state->count,
                                       state->ports,
                                       state->behaviors,
                                       state->flavors);

    return (KERN_SUCCESS == retcode);
}


int
set_exception_catch_state(const exception_catch_state *state)
{
    int is_setting_success = TRUE;
    task_t task = mach_task_self();
    kern_return_t retcode;
    int i;

    if (!state || state->count > sizeof(state->ports) / sizeof(state->ports[0])) {
        return FALSE;
    }

    for (i = 0; i < state->count; ++i) {
        retcode = task_set_exception_ports(task,
                                           state->masks[i],
                                           state->ports[i],
                                           state->behaviors[i],
                                           state->flavors[i]);
        is_setting_success = is_setting_success && (KERN_SUCCESS == retcode);
    }

    return is_setting_success;
}
