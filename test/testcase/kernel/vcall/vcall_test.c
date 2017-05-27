#include <stdio.h>
#include <stdlib.h>
#include <yunit.h>
#include <yts.h>
#include "mico_rtos.h"
#include "mico_rtos_common.h"
#include "common.h"
#include <k_api.h>

static mico_thread_t task1;
static mico_semaphore_t sem1;
static mico_mutex_t mutex1;
static mico_queue_t queue1;
static mico_timer_t timer1;

#define MSG_SIZE 50

static void task1_entry(mico_thread_arg_t arg )
{
    char msg[MSG_SIZE];

    mico_rtos_suspend_thread(NULL);
    mico_rtos_suspend_thread(&task1);

    mico_rtos_is_current_thread(&task1);
    mico_rtos_set_semaphore(&sem1);
    msg[0] = 0x11;
    msg[MSG_SIZE - 1] = 0x33;

    mico_rtos_push_to_queue(&queue1, NULL, 0);
    mico_rtos_push_to_queue(&queue1, msg, 0);

    mico_rtos_delete_thread(&task1);
}


static void task2_entry(mico_thread_arg_t arg )
{
    yunos_task_resume((ktask_t *)task1);
    yunos_task_resume((ktask_t *)task1);

    mico_rtos_delete_thread(NULL);
}


static int init(void)
{
    return 0;
}

static int cleanup(void)
{
    return 0;
}

static void setup(void)
{

}

static void teardown(void)
{

}

static void timer_callback(void* arg)
{
    YUNIT_ASSERT((int)arg == 0x11);
}

static void test_vcall_case(void)
{
    static mico_thread_t task_tmp;
    char pcWriteBuffer[20];
    char msg[50];

    printf("vcall test\n");
    mico_rtos_delay_milliseconds(0);
    mico_rtos_init_semaphore(&sem1, 0);
    mico_rtos_init_semaphore(NULL, 0);
    mico_rtos_thread_join(NULL);
    mico_rtos_get_semaphore(&sem1, 0);

    mico_rtos_delete_thread((mico_thread_t*)&g_idle_task);

    mico_rtos_enter_critical();
    mico_rtos_exit_critical();

    mico_rtos_create_thread(&task1, 10, NULL, task1_entry, 1024, 0);
    mico_rtos_create_thread(&task1, 30, "test", task1_entry, 1024, 0);
    mico_rtos_create_thread(NULL, 40, "test2", task2_entry, 1024, 0);

    mico_rtos_suspend_all_thread();
    mico_rtos_resume_all_thread();
    mico_rtos_is_current_thread(&task1);
    mico_rtos_thread_force_awake(&task1);
    
    task_tmp = mico_rtos_get_current_thread();
    (void)task_tmp;

    mico_rtos_get_semaphore(&sem1, MICO_NEVER_TIMEOUT);
    mico_rtos_deinit_semaphore(&sem1);

    mico_rtos_init_mutex(&mutex1);
    mico_rtos_lock_mutex(&mutex1);
    mico_rtos_unlock_mutex(&mutex1);
    mico_rtos_deinit_mutex(&mutex1);

    mico_rtos_init_queue(&queue1, NULL, MSG_SIZE, 2);
    mico_rtos_pop_from_queue(&queue1, msg, 10000);

    YUNIT_ASSERT((msg[0] == 0x11) && (msg[MSG_SIZE -1] == 0x33));

    mico_rtos_is_queue_empty(&queue1);
    mico_rtos_is_queue_full(&queue1);
    mico_rtos_deinit_queue(&queue1);
    mico_rtos_get_time();

    mico_rtos_init_timer(&timer1, 50, timer_callback, (void *)0x11);
    mico_rtos_start_timer(&timer1);
    mico_rtos_is_timer_running(&timer1);

    mico_rtos_delay_milliseconds(200);
    mico_rtos_stop_timer(&timer1);
    mico_rtos_is_timer_running(&timer1);

    mico_rtos_reload_timer(&timer1);
    mico_rtos_delay_milliseconds(200);
    mico_rtos_stop_timer(&timer1);

    mico_rtos_deinit_timer(&timer1);

    mico_rtos_print_thread_status(pcWriteBuffer, 2);
   
}


static yunit_test_case_t yunos_basic_testcases[] = {
    { "vcall", test_vcall_case },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "vcall", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_vcall(void)
{
    yunit_add_test_suites(suites);
}

