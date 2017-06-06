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
#include "time_test.h"

#define MODULE_NAME    "time_opr"
#define MODULE_NAME_CO "time_coopr"

void gettaskinfo(void)
{
    klist_t *taskhead = &g_kobj_list.task_head;
    klist_t *taskend  = taskhead;
    klist_t *tmp;
    ktask_t  *task;

    printf("\n");
    printf("---------------------------------------------\n");
    printf("Name                 State          StackSize\n");
    printf("---------------------------------------------\n");
    for (tmp = taskhead->next; tmp != taskend; tmp = tmp->next) {
        task = yunos_list_entry(tmp, ktask_t, task_stats_item);
        printf("%s\t\t %d\t\t\%d\n", task->task_name, (int)task->task_state,
               (int)task->stack_size);
    }
    printf("---------------------------------------------\n");
}

static uint8_t time_opr_case(void)
{
    tick_t     ticks;
    sys_time_t ms;
    sys_time_t start;
    sys_time_t end;

    ticks = yunos_ms_to_ticks(1000);
    MYASSERT(ticks == YUNOS_CONFIG_TICKS_PER_SECOND);

    ms = yunos_ticks_to_ms(YUNOS_CONFIG_TICKS_PER_SECOND);
    MYASSERT(ms == 1000);

    start = yunos_sys_tick_get();
    yunos_task_sleep(10);
    end   = yunos_sys_tick_get();

    MYASSERT((end - start) < 15);

    start = yunos_sys_time_get();
    yunos_task_sleep(16);
    end   = yunos_sys_time_get();

    MYASSERT((end - start) < (20 * 1000 / YUNOS_CONFIG_TICKS_PER_SECOND));

    return 0;
}

static const test_func_t time_func_runner[] = {
    time_opr_case,
    NULL
};

void time_opr_test(void)
{
    kstat_t ret;

    task_time_entry_register(MODULE_NAME, (test_func_t *)time_func_runner,
                             sizeof(time_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_time, MODULE_NAME, 0, TASK_TIME_PRI,
                                0, TASK_TEST_STACK_SIZE, task_time_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

