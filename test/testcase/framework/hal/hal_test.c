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

#include <yos/list.h>
#include <yos/kernel.h>

#include <hal/soc/soc.h>

#include <yunit.h>
#include <yts.h>
#include <hal/base.h>
#include <hal/wifi.h>


static void test_timer_cb(void *arg)
{
    int *pc = arg;
    (*pc) ++;
}

static void test_timer_case(void)
{
    int counter = 0, old_counter;
    hal_timer_t t;
    hal_timer_init(&t, 50000, 1, 0, test_timer_cb, &counter);
    yos_msleep(1000);
    YUNIT_ASSERT(counter == 0);

    hal_timer_start(&t);
    check_cond_wait(counter > 3, 2);

    hal_timer_stop(&t);
    yos_msleep(1000);

    old_counter = counter;
    yos_msleep(1000);
    YUNIT_ASSERT(counter == old_counter);
    if (counter != old_counter)
        printf("%s %d %d\n", __func__, counter, old_counter);
}


static uint8_t fixmac[6] = {0xd8,0x96,0xe0,0x03,0x04,0x01};

static int wifi_init(hal_wifi_module_t *m)
{
    printf("wifi init success!!\n");
    return 0;
};

static void wifi_get_mac_addr(hal_wifi_module_t *m, uint8_t *mac)
{
    (void)m;
    printf("wifi_get_mac_addr!!\n");
    memcpy(mac, fixmac, 6);
};

static int wifi_start(hal_wifi_module_t *m, hal_wifi_init_type_t *init_para)
{
    (void)init_para;

    hal_wifi_ip_stat_t ip_stat;

    char ip[4]   = {0x7f, 0x00, 0x00, 0x01};
    char mask[4] = {0xFF, 0xFF, 0xFF, 0x00};
    char gate[4] = {0x7f, 0x00, 0x00, 0x01};

    memcpy(ip_stat.ip, ip, 4);
    memcpy(ip_stat.mask, mask, 4);
    memcpy(ip_stat.gate, gate, 4);
    memcpy(ip_stat.mac, fixmac, 6);

    if (m->ev_cb->stat_chg != NULL) {
        m->ev_cb->stat_chg(m, NOTIFY_STATION_UP, NULL);
    }

    if (m->ev_cb->ip_got != NULL) {
        m->ev_cb->ip_got(m, &ip_stat, NULL);
    }

    return 0;
}

static int wifi_start_adv(hal_wifi_module_t *m, hal_wifi_init_type_adv_t *init_para_adv)
{
    (void)init_para_adv;
 
    return 0;
}

static int get_ip_stat(hal_wifi_module_t *m, hal_wifi_ip_stat_t *out_net_para, hal_wifi_type_t wifi_type)
{
    (void)wifi_type;

    char ip[4]   = {0x7f, 0x00, 0x00, 0x01};
    char mask[4] = {0xFF, 0xFF, 0xFF, 0x00};
    char gate[4] = {0x7f, 0x00, 0x00, 0x01};

    memcpy(out_net_para->ip, ip, 4);
    memcpy(out_net_para->mask, mask, 4);
    memcpy(out_net_para->gate, gate, 4);
    memcpy(out_net_para->mac, fixmac, 6);

    return 0;
}

static get_link_stat(hal_wifi_module_t *m, hal_wifi_link_stat_t *out_stat)
{
    out_stat->is_connected = 1;
    return 0;
}

static void start_scan(hal_wifi_module_t *m)
{
    hal_wifi_scan_result_t scan_ret;
    scan_ret.ap_num = 1;
    scan_ret.ap_list->ap_power = 80;
    memcpy(scan_ret.ap_list->ssid, "test", 5);

    if (m->ev_cb->scan_compeleted != NULL) {
        m->ev_cb->scan_compeleted(m, &scan_ret, NULL);
    }
}

static void start_scan_adv(hal_wifi_module_t *m)
{
}

static int power_off(hal_wifi_module_t *m)
{
    return 0;
}

static int power_on(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend_station(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend_soft_ap(hal_wifi_module_t *m)
{

    return 0;
}

static int set_channel(hal_wifi_module_t *m, int ch)
{
    return 0;
}

static void start_monitor(hal_wifi_module_t *m)
{

}

static void stop_monitor(hal_wifi_module_t *m)
{

}

static void register_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn)
{

}


static hal_wifi_module_t sim_yos_wifi_module = {
    .base.name           = "sim_yos_wifi_module",
    .init                =  wifi_init,
    .get_mac_addr        =  wifi_get_mac_addr,
    .start               =  wifi_start,
    .start_adv           =  wifi_start_adv,
    .get_ip_stat         =  get_ip_stat,
    .get_link_stat       =  get_link_stat,
    .start_scan          =  start_scan,
    .start_scan_adv      =  start_scan_adv,
    .power_off           =  power_off,
    .power_on            =  power_on,
    .suspend             =  suspend,
    .suspend_station     =  suspend_station,
    .suspend_soft_ap     =  suspend_soft_ap,
    .set_channel         =  set_channel,
    .start_monitor       =  start_monitor,
    .stop_monitor        =  stop_monitor,
    .register_monitor_cb =  register_monitor_cb,
};

static void test_wifi_case(void)
{
    uint8_t mac[6];

    printf("start wifi test case\n");

    hal_wifi_register_module(&sim_yos_wifi_module);
    hal_wifi_init();
    hal_wifi_get_mac_addr(&sim_yos_wifi_module, mac);

    printf("first mac addr is 0x%x\n", mac[0]);
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

static yunit_test_case_t yunos_basic_testcases[] = {
    { "timer", test_timer_case },
    { "wifi",  test_wifi_case },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "hal", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_hal(void)
{
    yunit_add_test_suites(suites);
}

