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
#include <stdlib.h>

#include <k_api.h>
#include <yos/kernel.h>

#include <yunit.h>

static yos_timer_t g_timer;
static yos_sem_t sync_sem;
static int timer_trigger_count = 0;

static void TIMER_aosapi_kernel_timer_param()
{

}
static void CASE_aosapi_kernel_timer_param()
{
	int ret;
#if 0
	ret = yos_timer_new(NULL, TIMER_aosapi_kernel_timer_param, NULL, 1000, 0);
	YUNIT_ASSERT(ret==YUNOS_NULL_PTR);
#endif

	ret = yos_timer_new(&g_timer, NULL, NULL, 1000, 0);
	YUNIT_ASSERT(ret==YUNOS_NULL_PTR);

	ret = yos_timer_new(&g_timer, TIMER_aosapi_kernel_timer_param, NULL, 0, 0);
	YUNIT_ASSERT(ret==YUNOS_INV_PARAM);

#if 0
	ret = yos_timer_start(NULL);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif

#if 0
	ret = yos_timer_stop(NULL);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif

#if 0
	yos_timer_free(NULL);
#endif

#if 0
	ret = yos_timer_change(NULL, 0);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////
static void TIMER_aosapi_kernel_timer_norepeat()
{
	timer_trigger_count++;
}
static void CASE_aosapi_kernel_timer_norepeat()
{
	int ret;

	ret = yos_sem_new(&sync_sem, 0);
	YUNIT_ASSERT(ret==YUNOS_SUCCESS);

	timer_trigger_count = 0;
	ret = yos_timer_new(&g_timer, TIMER_aosapi_kernel_timer_norepeat, NULL, 100, 0);
	YUNIT_ASSERT(ret==YUNOS_SUCCESS);

	yos_msleep(1000);
	YUNIT_ASSERT(timer_trigger_count==1);
	yos_timer_free(&g_timer);
}

///////////////////////////////////////////////////////////////////////////////////////////////
static void TIMER_aosapi_kernel_timer_repeat()
{
	timer_trigger_count++;
	if(timer_trigger_count==10) {
		yos_timer_stop(&g_timer);
	}
}
static void CASE_aosapi_kernel_timer_repeat()
{
	int ret;

	ret = yos_sem_new(&sync_sem, 0);
	YUNIT_ASSERT(ret==YUNOS_SUCCESS);

	timer_trigger_count = 0;
	ret = yos_timer_new(&g_timer, TIMER_aosapi_kernel_timer_repeat, NULL, 100, 1);
	YUNIT_ASSERT(ret==YUNOS_SUCCESS);

	yos_msleep(1500);
	YUNIT_ASSERT(timer_trigger_count==10);
	yos_timer_free(&g_timer);
}

///////////////////////////////////////////////////////////////////////////////////////////////
static void TIMER_aosapi_kernel_timer_change()
{
	timer_trigger_count++;
	if(timer_trigger_count==10) {
		yos_timer_change(&g_timer, 50);
	}
	else if(timer_trigger_count==20) {
		yos_timer_stop(&g_timer);
	}
}
static void CASE_aosapi_kernel_timer_change()
{
	int ret;

	ret = yos_sem_new(&sync_sem, 0);
	YUNIT_ASSERT(ret==YUNOS_SUCCESS);

	timer_trigger_count = 0;
	ret = yos_timer_new(&g_timer, TIMER_aosapi_kernel_timer_change, NULL, 100, 1);
	YUNIT_ASSERT(ret==YUNOS_SUCCESS);

	yos_msleep(2000);
	YUNIT_ASSERT(timer_trigger_count==20);
	yos_timer_free(&g_timer);
}


void aosapi_kernel_timer_test_entry(yunit_test_suite_t *suite)
{
	yunit_add_test_case(suite, "kernel.timer.param", CASE_aosapi_kernel_timer_param);
	yunit_add_test_case(suite, "kernel.timer.norepeat", CASE_aosapi_kernel_timer_norepeat);
	yunit_add_test_case(suite, "kernel.timer.repeat", CASE_aosapi_kernel_timer_repeat);
	yunit_add_test_case(suite, "kernel.timer.change", CASE_aosapi_kernel_timer_change);
}

