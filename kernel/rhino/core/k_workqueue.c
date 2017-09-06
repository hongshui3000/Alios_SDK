/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>

#if (RHINO_CONFIG_WORKQUEUE > 0)
extern kworkqueue_t g_workqueue_default;
extern cpu_stack_t  g_workqueue_stack[RHINO_CONFIG_WORKQUEUE_STACK_SIZE];

static kstat_t workqueue_is_exist(kworkqueue_t *workqueue)
{
    CPSR_ALLOC();

    kworkqueue_t *current_queue = NULL;

    RHINO_CRITICAL_ENTER();

    yunos_list_for_each_entry(current_queue, &g_workqueue_list_head, workqueue_node) {
        if (current_queue == workqueue) {
            RHINO_CRITICAL_EXIT();
            return RHINO_WORKQUEUE_EXIST;
        }
    }

    RHINO_CRITICAL_EXIT();
    return RHINO_WORKQUEUE_NOT_EXIST;
}

static void worker_task(void *arg)
{
    CPSR_ALLOC();

    kstat_t       ret;
    kwork_t      *work = NULL;
    kworkqueue_t *queue = (kworkqueue_t *)arg;

    while (1) {
        if (is_klist_empty(&(queue->work_list))) {
            ret = yunos_sem_take(&(queue->sem), RHINO_WAIT_FOREVER);
            if (ret != RHINO_SUCCESS) {
                k_err_proc(ret);
            }
        }

        RHINO_CRITICAL_ENTER();
        if (!is_klist_empty(&(queue->work_list))) {
            /* have work to do. */
            work = yunos_list_entry(queue->work_list.next, kwork_t, work_node);
            klist_rm_init(&(work->work_node));
            queue->work_current = work;
            RHINO_CRITICAL_EXIT();

            /* do work */
            work->handle(work->arg);
            RHINO_CRITICAL_ENTER();
            /* clean current work */
            queue->work_current = NULL;
        }
        RHINO_CRITICAL_EXIT();
    }
}

kstat_t yunos_workqueue_create(kworkqueue_t *workqueue, const name_t *name,
                               uint8_t pri, cpu_stack_t *stack_buf, size_t stack_size)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(workqueue);
    NULL_PARA_CHK(name);
    NULL_PARA_CHK(stack_buf);

    if (pri >= RHINO_CONFIG_PRI_MAX) {
        return RHINO_BEYOND_MAX_PRI;
    }

    if (stack_size == 0u) {
        return RHINO_TASK_INV_STACK_SIZE;
    }

    ret = workqueue_is_exist(workqueue);
    if (ret == RHINO_WORKQUEUE_EXIST) {
        return RHINO_WORKQUEUE_EXIST;
    }

    klist_init(&(workqueue->workqueue_node));
    klist_init(&(workqueue->work_list));
    workqueue->work_current = NULL;
    workqueue->name      = name;

    ret = yunos_sem_create(&(workqueue->sem), "WORKQUEUE-SEM", 0);
    if (ret != RHINO_SUCCESS) {
        return ret;
    }

    ret = yunos_task_create(&(workqueue->worker), name, (void *)workqueue, pri,
                            0, stack_buf, stack_size, worker_task, 0);
    if (ret != RHINO_SUCCESS) {
        yunos_sem_del(&(workqueue->sem));
        return ret;
    }

    RHINO_CRITICAL_ENTER();
    klist_insert(&g_workqueue_list_head, &(workqueue->workqueue_node));
    RHINO_CRITICAL_EXIT();

    ret = yunos_task_resume(&(workqueue->worker));
    if (ret != RHINO_SUCCESS) {
        return ret;
    }

    TRACE_WORKQUEUE_CREATE(yunos_cur_task_get(), workqueue);

    return RHINO_SUCCESS;
}

kstat_t yunos_workqueue_del(kworkqueue_t *workqueue)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(workqueue);

    ret = workqueue_is_exist(workqueue);
    if (ret == RHINO_WORKQUEUE_NOT_EXIST) {
        return RHINO_WORKQUEUE_NOT_EXIST;
    }

    RHINO_CRITICAL_ENTER();

    if (!is_klist_empty(&(workqueue->work_list))) {
        RHINO_CRITICAL_EXIT();
        return RHINO_WORKQUEUE_BUSY;
    }

    if (workqueue->work_current != NULL) {
        RHINO_CRITICAL_EXIT();
        return RHINO_WORKQUEUE_BUSY;
    }

    RHINO_CRITICAL_EXIT();

    ret = yunos_task_del(&(workqueue->worker));
    if (ret != RHINO_SUCCESS) {
        return ret;
    }

    ret = yunos_sem_del(&(workqueue->sem));
    if (ret != RHINO_SUCCESS) {
        return ret;
    }

    RHINO_CRITICAL_ENTER();
    klist_rm_init(&(workqueue->workqueue_node));
    TRACE_WORKQUEUE_DEL(g_active_task[cpu_cur_get()], workqueue);
    RHINO_CRITICAL_EXIT();

    return RHINO_SUCCESS;
}

static void work_timer_cb(void *timer, void *arg)
{
    CPSR_ALLOC();

    kstat_t       ret;
    kwork_t      *work = yunos_list_entry(timer, kwork_t, timer);
    kworkqueue_t *wq   = (kworkqueue_t *)arg;

    RHINO_CRITICAL_ENTER();
    if (wq->work_current == work) {
        RHINO_CRITICAL_EXIT();
        return;
    }

    /* NOTE: the work MUST be initialized firstly */
    klist_rm_init(&(work->work_node));
    klist_insert(&(wq->work_list), &(work->work_node));

    work->wq = wq;

    if (wq->work_current == NULL) {
        RHINO_CRITICAL_EXIT();
        ret = yunos_sem_give(&(wq->sem));
        if (ret != RHINO_SUCCESS) {
            return;
        }
    } else {
        RHINO_CRITICAL_EXIT();
    }
}

kstat_t yunos_work_init(kwork_t *work, work_handle_t handle, void *arg,
                        tick_t dly)
{
    kstat_t ret;

    if (work == NULL) {
        return RHINO_NULL_PTR;
    }

    if (handle == NULL) {
        return RHINO_NULL_PTR;
    }

    NULL_PARA_CHK(work);
    NULL_PARA_CHK(handle);

    klist_init(&(work->work_node));
    work->handle  = handle;
    work->arg     = arg;
    work->dly     = dly;
    work->wq      = NULL;

    dly = dly / RHINO_CONFIG_TIMER_RATE;

    if (dly > 0) {
        ret = yunos_timer_create(&(work->timer), "WORK-TIMER", work_timer_cb,
                                 work->dly, 0, (void *)work, 0);
        if (ret != RHINO_SUCCESS) {
            return ret;
        }
    }

    TRACE_WORK_INIT(yunos_cur_task_get(), work);

    return RHINO_SUCCESS;
}

kstat_t yunos_work_run(kworkqueue_t *workqueue, kwork_t *work)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(workqueue);
    NULL_PARA_CHK(work);

    if (work->dly == 0) {
        RHINO_CRITICAL_ENTER();
        if (workqueue->work_current == work) {
            RHINO_CRITICAL_EXIT();
            return RHINO_WORKQUEUE_WORK_RUNNING;
        }

        /* NOTE: the work MUST be initialized firstly */
        klist_rm_init(&(work->work_node));
        klist_insert(&(workqueue->work_list), &(work->work_node));

        work->wq = workqueue;

        if (workqueue->work_current == NULL) {
            RHINO_CRITICAL_EXIT();
            ret = yunos_sem_give(&(workqueue->sem));
            if (ret != RHINO_SUCCESS) {
                return ret;
            }
        } else {
            RHINO_CRITICAL_EXIT();
        }
    } else {
        yunos_timer_stop(&(work->timer));
        work->timer.timer_cb_arg = (void *)workqueue;

        ret = yunos_timer_start(&(work->timer));
        if (ret != RHINO_SUCCESS) {
            return ret;
        }
    }

    return RHINO_SUCCESS;
}

kstat_t yunos_work_sched(kwork_t *work)
{
    return yunos_work_run(&g_workqueue_default, work);
}

kstat_t yunos_work_cancel(kwork_t *work)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(work);

    kworkqueue_t *wq = (kworkqueue_t *)work->wq;

    if (wq == NULL) {
        if (work->dly > 0) {
            yunos_timer_stop(&(work->timer));
            yunos_timer_del(&(work->timer));
        }

        return RHINO_SUCCESS;
    }

    RHINO_CRITICAL_ENTER();
    if (wq->work_current == work) {
        RHINO_CRITICAL_EXIT();
        return RHINO_WORKQUEUE_WORK_RUNNING;
    }
    klist_rm_init(&(work->work_node));
    work->wq      = NULL;
    RHINO_CRITICAL_EXIT();

    return RHINO_SUCCESS;
}

void workqueue_init(void)
{
    klist_init(&g_workqueue_list_head);

    yunos_workqueue_create(&g_workqueue_default, "DEFAULT-WORKQUEUE",
                           RHINO_CONFIG_WORKQUEUE_TASK_PRIO, g_workqueue_stack,
                           RHINO_CONFIG_WORKQUEUE_STACK_SIZE);
}
#endif

