/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <k_api.h>
#include <test_fw.h>
#include "event_test.h"

#define MODULE_NAME     "event_opr"
#define MODULE_NAME_CO  "event_coopr"

#define TEST_FLAG         0x5a5a5a5a
#define CHK_AND_ALL_FLAG  0x5a5a5a5a
#define CHK_AND_ONE_FLAG  0x00000002
#define CHK_AND_ZERO_FLAG 0x00000000
#define CHK_AND_PEND_FLAG 0x5a5a5a55

static uint8_t event_opr_case1(void)
{
    kstat_t  ret;
    uint32_t actl_flags;
    CPSR_ALLOC();

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    /* check event AND FLAG */
    YUNOS_CRITICAL_ENTER();
    test_event.blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    YUNOS_CRITICAL_ENTER();
    test_event.blk_obj.obj_type = YUNOS_EVENT_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();

    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_AND);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    YUNOS_CRITICAL_ENTER();
    test_event.blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_get(&test_event, CHK_AND_ALL_FLAG, YUNOS_AND, &actl_flags,
                          YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    YUNOS_CRITICAL_ENTER();
    test_event.blk_obj.obj_type = YUNOS_EVENT_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_event_get(&test_event, CHK_AND_ALL_FLAG, YUNOS_AND, &actl_flags,
                          YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ONE_FLAG, YUNOS_AND, &actl_flags,
                          YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ZERO_FLAG, YUNOS_AND, &actl_flags,
                          YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t event_opr_case2(void)
{
    kstat_t  ret;
    uint32_t actl_flags;

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    /* check event AND_CLEAR FLAG */
    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ALL_FLAG, YUNOS_AND_CLEAR,
                          &actl_flags, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == (TEST_FLAG & (~CHK_AND_ALL_FLAG)));

    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ONE_FLAG, YUNOS_AND_CLEAR,
                          &actl_flags, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == (TEST_FLAG & (~CHK_AND_ONE_FLAG)));

    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ZERO_FLAG, YUNOS_AND_CLEAR,
                          &actl_flags, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == (TEST_FLAG & (~CHK_AND_ZERO_FLAG)));

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t event_opr_case3(void)
{
    kstat_t  ret;
    uint32_t actl_flags;

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    /* check event OR FLAG */
    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ALL_FLAG, YUNOS_OR, &actl_flags,
                          YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ONE_FLAG, YUNOS_OR, &actl_flags,
                          YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ZERO_FLAG, YUNOS_OR, &actl_flags,
                          YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_NO_PEND_WAIT);

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t event_opr_case4(void)
{
    kstat_t  ret;
    uint32_t actl_flags;

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    /* check event OR_CLEAR FLAG */
    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ALL_FLAG, YUNOS_OR_CLEAR,
                          &actl_flags, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == (TEST_FLAG & (~CHK_AND_ALL_FLAG)));

    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_get(&test_event, CHK_AND_ONE_FLAG, YUNOS_OR_CLEAR,
                          &actl_flags, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(actl_flags == TEST_FLAG);
    MYASSERT(test_event.flags == (TEST_FLAG & (~CHK_AND_ONE_FLAG)));

    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t event_opr_case5(void)
{
    kstat_t  ret;
    uint32_t actl_flags;

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    /* try to get event flag in case of sched disable */
    ret = yunos_event_set(&test_event, TEST_FLAG, YUNOS_OR);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.flags == TEST_FLAG);

    yunos_sched_disable();

    ret = yunos_event_get(&test_event, CHK_AND_PEND_FLAG, YUNOS_AND, &actl_flags,
                          YUNOS_WAIT_FOREVER);
    MYASSERT(ret == YUNOS_SCHED_DISABLE);

    yunos_sched_enable();

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t event_func_runner[] = {
    event_opr_case1,
    event_opr_case2,
    event_opr_case3,
    event_opr_case4,
    event_opr_case5,
    NULL
};

void event_opr_test(void)
{
    kstat_t ret;

    task_event_entry_register(MODULE_NAME, (test_func_t *)event_func_runner,
                              sizeof(event_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_event, MODULE_NAME, 0, TASK_EVENT_PRI,
                                0, TASK_TEST_STACK_SIZE, task_event_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

static void task_event_co1_entry(void *arg)
{
    kstat_t  ret;
    uint32_t actl_flags;

    while (1) {
        ret = yunos_event_get(&test_event, ~CHK_AND_ALL_FLAG, YUNOS_AND, &actl_flags,
                              YUNOS_WAIT_FOREVER);
        if (ret == YUNOS_SUCCESS) {
            break;
        } else {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO, FAIL);

            yunos_event_del(&test_event);

            next_test_case_notify();
            yunos_task_dyn_del(yunos_cur_task_get());

            return;
        }
    }

    test_case_success++;
    PRINT_RESULT(MODULE_NAME_CO, PASS);

    yunos_event_del(&test_event);

    next_test_case_notify();
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_event_co2_entry(void *arg)
{
    while (1) {
        yunos_event_set(&test_event, ~CHK_AND_ALL_FLAG, YUNOS_OR);
        break;
    }

    yunos_task_dyn_del(yunos_cur_task_get());
}

void event_coopr_test(void)
{
    kstat_t ret;

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    if (ret != YUNOS_SUCCESS && test_event.flags != TEST_FLAG) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
        return;
    }

    ret = yunos_task_dyn_create(&task_event_co1, MODULE_NAME, 0, TASK_EVENT_PRI,
                                0, TASK_TEST_STACK_SIZE, task_event_co1_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_event_co2, MODULE_NAME, 0, TASK_EVENT_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_event_co2_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

