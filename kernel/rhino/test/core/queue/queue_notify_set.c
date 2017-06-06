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

#include <stdio.h>
#include <k_api.h>
#include <test_fw.h>

#if (YUNOS_CONFIG_KOBJ_SET > 0)
#define QUEUE_SIZE            12
#define QUEUE_MAX_MSG         10
#define BUF_QUEUE_SIZE        32
#define BUF_QUEUE_MAX_MSG     4
#define QUEUE_MAX_MSG         10
#define TASK_TEST_STACK_SIZE  256

#define OBJ_SET_SIZE   10
#define QUEUE_MSG_ADDR 0x8888
#define OBJ_SET_NAME   "YOC_SELECT"
#define MODULE_NAME    "queue select ops"
#define TEST_SEM_COUNT OBJ_SET_SIZE+1

static ktask_t  *task0_test;
static void    *queue0_send_msg0 = (void *)QUEUE_MSG_ADDR;
static void    *g_queue0_msg_pool[QUEUE_SIZE];
static kqueue_t g_queue0;

static char         g_buf_queue0_msg0[BUF_QUEUE_SIZE] = {'Y', 'O', 'C'};
static char         g_buf_queue0_msg_pool[BUF_QUEUE_SIZE];
static kbuf_queue_t g_buf_queue0;

static kobj_set_t *handle;
static blk_obj_t  *blk_obj;
static kmutex_t    *mutex0;
static ksem_t     *sem0;

static void kobj_set_param_test()
{
    kstat_t ret;
    ret = yunos_kobj_set_dyn_create(NULL, OBJ_SET_NAME, OBJ_SET_SIZE);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_NULL_PTR);

    ret = yunos_kobj_set_dyn_create(&handle, NULL, OBJ_SET_SIZE);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_NULL_PTR);

    ret = yunos_kobj_set_insert(NULL, handle);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_NULL_PTR);

    ret = yunos_kobj_set_insert(blk_obj, NULL);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_NULL_PTR);

    ret = yunos_sem_dyn_create(&sem0, "test_sem0", TEST_SEM_COUNT);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_kobj_set_insert((blk_obj_t *)sem0, handle);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_KOBJ_SET_FULL);

    ret = yunos_mutex_dyn_create(&mutex0, "test_mutex0");
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_kobj_set_insert((blk_obj_t *)mutex0, handle);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_KOBJ_TYPE_ERR);
}

static void task_queue0_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_queue_create(&g_queue0, "test_queue0", (void **)&g_queue0_msg_pool,
                             QUEUE_MAX_MSG);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_buf_queue_create(&g_buf_queue0, "test_buf_queue0",
                                 (void *)g_buf_queue0_msg_pool, BUF_QUEUE_SIZE, BUF_QUEUE_MAX_MSG);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_kobj_set_dyn_create(&handle, OBJ_SET_NAME, OBJ_SET_SIZE);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    kobj_set_param_test();

    ret = yunos_queue_back_send(&g_queue0, queue0_send_msg0);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_kobj_set_insert((blk_obj_t *)&g_queue0, handle);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    yunos_kobj_select(handle, &blk_obj, YUNOS_WAIT_FOREVER);
    TEST_FW_VAL_CHK(MODULE_NAME, (size_t)blk_obj == (size_t)&g_queue0);

    ret = yunos_buf_queue_send(&g_buf_queue0, (void *)g_buf_queue0_msg0,
                               strlen(g_buf_queue0_msg0));
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_kobj_set_insert((blk_obj_t *)&g_buf_queue0, handle);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    yunos_kobj_select(handle, &blk_obj, YUNOS_WAIT_FOREVER);
    TEST_FW_VAL_CHK(MODULE_NAME, (size_t)blk_obj == (size_t)&g_buf_queue0);

    ret = yunos_queue_del(&g_queue0);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_buf_queue_del(&g_buf_queue0);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_kobj_set_dyn_del(handle);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    if (test_case_check_err == 0) {
        test_case_success++;
        PRINT_RESULT(MODULE_NAME, PASS);
    } else {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    next_test_case_notify();
    yunos_task_dyn_del(g_active_task);
}

kstat_t task_queue_notify_set_test(void)
{
    kstat_t ret;
    test_case_check_err = 0;

    ret = yunos_task_dyn_create(&task0_test, "task_queue0_test", 0, 10,
                                0, TASK_TEST_STACK_SIZE, task_queue0_entry, 1);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    return 0;
}
#endif

