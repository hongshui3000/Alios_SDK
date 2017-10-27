// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stddef.h>
#include <stdint.h>

#include "esp_heap_caps_init.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "espos_scheduler.h"

static espos_size_t s_isr_nested_count[ESPOS_PROCESSORS_NUM];
static espos_size_t s_os_nested_count[ESPOS_PROCESSORS_NUM];
static espos_size_t s_critial_count[ESPOS_PROCESSORS_NUM];

/**
 * @brief initialize ESPOS system
 */
esp_err_t espos_init(void)
{
    heap_caps_init();

    return 0;
}

/**
 * @brief start ESPOS system
 */
esp_err_t espos_start(void)
{
    vTaskStartScheduler();

    return 0;
}

/**
 * @brief start ESPOS system CPU port
 */
esp_err_t espos_start_port(int port)
{
    xPortStartScheduler();

    return 0;
}

/**
 * @brief get ESPOS system state
 */
espos_stat_t espos_sched_state_get(void)
{
    espos_stat_t state;
    BaseType_t os_state = xTaskGetSchedulerState();

    if (os_state == taskSCHEDULER_NOT_STARTED) {
        state = ESPOS_IS_NOT_STARTED;
    } else if (os_state == taskSCHEDULER_RUNNING) {
        state = ESPOS_IS_RUNNING;
    } else {
        state = ESPOS_IS_SUSPENDED;
    }

    return state;
}

/**
 * @brief check if the CPU is at ISR state
 */
bool espos_in_isr(void)
{
    return (s_isr_nested_count[espos_get_core_id()] > 0) ? true : false;
}

/**
 * @brief check if the function is at real OS internal fucntion
 */
bool espos_os_isr(void)
{
    return (s_os_nested_count[espos_get_core_id()] > 0) ? true : false;
}

/**
 * @brief enter ESPOS system critical state
 */
espos_critical_t _espos_enter_critical(espos_spinlock_t *spinlock)
{
    espos_critical_t tmp;

    tmp = (espos_critical_t)XTOS_SET_INTLEVEL(XCHAL_EXCM_LEVEL);

#ifdef CONFIG_ESPOS_SMP
    if (espos_spinlock_get_holder(spinlock) != espos_task_get_current())
        espos_spinlock_lock(spinlock);
#else
    espos_preempt_suspend_local();
#endif

    return tmp;
}

/**
 * @brief exit ESPOS system critical state
 */
void _espos_exit_critical(espos_spinlock_t *spinlock, espos_critical_t tmp)
{
#ifdef CONFIG_ESPOS_SMP
    espos_spinlock_unlock(spinlock);
#else
    espos_preempt_resume_local();
#endif

    XTOS_RESTORE_JUST_INTLEVEL(tmp);
}


/**
* @brief suspend the preempt and the current task will not be preempted
*/
esp_err_t espos_preempt_suspend_local(void)
{
    if (espos_in_isr() == false
        && espos_os_isr() == false
        && espos_sched_state_get() != ESPOS_IS_NOT_STARTED
        && !s_critial_count[espos_get_core_id()]) {
        s_critial_count[espos_get_core_id()]++;
        vTaskSuspendAll();
    }

    return 0;
}

/**
 * @brief resume the preempt
 */
esp_err_t espos_preempt_resume_local(void)
{
    if (espos_in_isr() == false
        && espos_os_isr() == false
        && espos_sched_state_get() != ESPOS_IS_NOT_STARTED
        && s_critial_count[espos_get_core_id()]) {
        s_critial_count[espos_get_core_id()]--;
        xTaskResumeAll();
    }

    return 0;
}

/**
 * @brief enter system interrupt server, the function must be used after entering hardware interrupt
 */
esp_err_t espos_isr_enter (void)
{
    s_isr_nested_count[espos_get_core_id()]++;

    return 0;
}

/**
 * @brief exit system interrupt server, the function must be used before exiting hardware interrupt
 */
void espos_isr_exit(void)
{
    if (!s_isr_nested_count[espos_get_core_id()]) {
        return ;
    }

    s_isr_nested_count[espos_get_core_id()]--;

    return ;
}

/**
 * @brief mark it enters real OS interal function
 */
esp_err_t espos_os_enter (void)
{
    s_os_nested_count[espos_get_core_id()]++;

    return 0;
}

/**
 * @brief remove mark it enters real OS interal function
 */
void espos_os_exit(void)
{
    if (!s_os_nested_count[espos_get_core_id()]) {
        return ;
    }

    s_os_nested_count[espos_get_core_id()]--;

    return ;
}
