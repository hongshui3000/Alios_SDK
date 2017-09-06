/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <test_fw.h>
#include "sem_test.h"

#define MODULE_NAME     "sem_opr"
#define MODULE_NAME_CO1 "sem_coopr1"
#define MODULE_NAME_CO2 "sem_coopr2"

#define LOOP_CNT        8

static uint8_t sem_opr_case1(void)
{
    kstat_t     ret;
    sem_count_t cnt;

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 3);
    MYASSERT(ret == RHINO_SUCCESS);

    ret = yunos_sem_take(test_sem, RHINO_NO_WAIT);
    MYASSERT(ret == RHINO_SUCCESS);

    ret = yunos_sem_take(test_sem, RHINO_NO_WAIT);
    MYASSERT(ret == RHINO_SUCCESS);

    ret = yunos_sem_give(test_sem);
    MYASSERT(ret == RHINO_SUCCESS);

    ret = yunos_sem_give_all(test_sem);
    MYASSERT(ret == RHINO_SUCCESS);

    ret = yunos_sem_count_get(test_sem, &cnt);
    MYASSERT(ret == RHINO_SUCCESS);
    MYASSERT(cnt == 3);

    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == RHINO_SUCCESS);

    return 0;
}

static const test_func_t sem_func_runner[] = {
    sem_opr_case1,
    NULL
};

void sem_opr_test(void)
{
    kstat_t ret;

    task_sem_entry_register(MODULE_NAME, (test_func_t *)sem_func_runner,
                            sizeof(sem_func_runner) / sizeof(test_case_t));

    ret = yunos_task_dyn_create(&task_sem, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sem_entry, 1);

    if ((ret != RHINO_SUCCESS) && (ret != RHINO_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

static void task_sem_coopr1_co1_entry(void *arg)
{
    kstat_t ret;
    uint8_t cnt = 0;

    yunos_sem_take(test_sem, 100);
    yunos_sem_take(test_sem, 100);
    yunos_sem_take(test_sem, 100);

    while (1) {
        ret = yunos_sem_take(test_sem, RHINO_WAIT_FOREVER);

        if (ret == RHINO_SUCCESS) {
            cnt++;

            if (cnt >= LOOP_CNT) {
                yunos_sem_count_set(test_sem, 16);
                break;
            }
        } else {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO1, FAIL);

            next_test_case_notify();
            yunos_task_dyn_del(yunos_cur_task_get());

            return;
        }
    }

    test_case_success++;
    PRINT_RESULT(MODULE_NAME_CO1, PASS);

    next_test_case_notify();
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_sem_coopr1_co2_entry(void *arg)
{
    uint8_t     cnt = 0;
    sem_count_t sem_cnt;

    while (1) {
        if (cnt < LOOP_CNT) {
            cnt++;
            yunos_sem_give(test_sem);
        } else {
            yunos_sem_count_get(test_sem, &sem_cnt);

            if (sem_cnt == 16) {
                break;
            }
        }

    }

    yunos_sem_dyn_del(test_sem);

    yunos_task_dyn_del(yunos_cur_task_get());
}

void sem_coopr1_test(void)
{
    kstat_t ret;

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 3);
    if (ret != RHINO_SUCCESS) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO1, FAIL);
        return;
    }

    ret = yunos_task_dyn_create(&task_sem_co1, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sem_coopr1_co1_entry, 1);

    if ((ret != RHINO_SUCCESS) && (ret != RHINO_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO1, FAIL);
    }

    ret = yunos_task_dyn_create(&task_sem_co2, MODULE_NAME, 0, TASK_SEM_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_sem_coopr1_co2_entry, 1);

    if ((ret != RHINO_SUCCESS) && (ret != RHINO_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO1, FAIL);
    }
}

static void task_sem_coopr2_co1_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_sem_take(test_sem_co1, RHINO_WAIT_FOREVER);

    TEST_FW_VAL_CHK(MODULE_NAME_CO2, ret == RHINO_SUCCESS);

    if (test_case_check_err > 0) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO2, FAIL);
    } else {
        test_case_success++;
        PRINT_RESULT(MODULE_NAME_CO2, PASS);
    }

    yunos_sem_dyn_del(test_sem_co1);
    next_test_case_notify();
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_sem_coopr2_co2_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        /* no task block on the semaphore and wait notification from other task */
        yunos_task_sleep(5);
        ret = yunos_sem_take(test_sem_co2, RHINO_NO_WAIT);

        if (ret == RHINO_SUCCESS) {
            yunos_sem_give(test_sem_co1);
            break;
        }
    }

    yunos_sem_dyn_del(test_sem_co2);
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_sem_coopr2_co3_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        ret = yunos_sem_give(test_sem_co2);

        if (ret == RHINO_SUCCESS) {
            break;
        }
    }

    yunos_task_dyn_del(yunos_cur_task_get());
}

void sem_coopr2_test(void)
{
    kstat_t ret;
    test_case_check_err = 0;

    yunos_sem_dyn_create(&test_sem_co1, MODULE_NAME, 0);
    yunos_sem_dyn_create(&test_sem_co2, MODULE_NAME, 0);

    ret = yunos_task_dyn_create(&task_sem, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sem_coopr2_co1_entry, 1);

    TEST_FW_VAL_CHK(MODULE_NAME_CO2, ret == RHINO_SUCCESS);


    ret = yunos_task_dyn_create(&task_sem_co1, MODULE_NAME, 0, TASK_SEM_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_sem_coopr2_co2_entry, 1);

    TEST_FW_VAL_CHK(MODULE_NAME_CO2, ret == RHINO_SUCCESS);

    ret = yunos_task_dyn_create(&task_sem_co2, MODULE_NAME, 0, TASK_SEM_PRI + 2,
                                0, TASK_TEST_STACK_SIZE, task_sem_coopr2_co3_entry, 1);
    TEST_FW_VAL_CHK(MODULE_NAME_CO2, ret == RHINO_SUCCESS);
}

