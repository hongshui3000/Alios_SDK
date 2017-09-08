/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "umesh_types.h"
#include "umesh_utils.h"

typedef void (*aos_call_t)(void *);
extern int aos_schedule_call(aos_call_t f, void *arg);
ur_error_t umesh_task_schedule_call(umesh_task_t task, void *arg)
{
    int ret;
    ur_error_t error = UR_ERROR_NONE;

    ret = aos_schedule_call(task, arg);
    if (ret < 0) {
        error = UR_ERROR_FAIL;
    }

    return error;
}
