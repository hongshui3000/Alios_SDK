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
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <yos/yos.h>
#include <yos/log.h>
#include <k_err.h>
#include <yunit.h>
#include <yts.h>
#include "iot_import.h"
#include "iot_export.h"


#define IOTX_PRODUCT_KEY         "vtkkbrpmxmF"

#define IOTX_PRE_DTLS_SERVER_URI "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5684"
#define IOTX_PRE_NOSEC_SERVER_URI "coap://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5683"

#define IOTX_ONLINE_DTLS_SERVER_URL "coaps://%s.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

#if defined(COAP_ONLINE)
    #define COAP_URL  IOTX_ONLINE_DTLS_SERVER_URL
#else
    #ifdef COAP_DTLS_SUPPORT
    #define COAP_URL  IOTX_PRE_DTLS_SERVER_URI
    #else
    #define COAP_URL  IOTX_PRE_NOSEC_SERVER_URI
    #endif
#endif

#define TAG "coaptest"

char m_coap_client_running = 0;

static void iotx_response_handler(void * arg, void * p_response)
{
    int            len       = 0;
    unsigned char *p_payload = NULL;
    iotx_coap_resp_code_t resp_code;
    IOT_CoAP_GetMessageCode(p_response, &resp_code);
    IOT_CoAP_GetMessagePayload(p_response, &p_payload, &len);
    printf("[APPL]: Message response code: %d\r\n", resp_code);
    printf("[APPL]: Len: %d, Payload: %s, \r\n", len, p_payload);
}

static void iotx_post_data_to_server(void *param)
{
    int ret;
    char               path[IOTX_URI_MAX_LEN+1] = {0};
    iotx_message_t     message;
    iotx_deviceinfo_t  devinfo;
    message.p_payload = (unsigned char *)"{\"name\":\"hello world\"}";
    message.payload_len = strlen("{\"name\":\"hello world\"}");
    message.resp_callback = iotx_response_handler;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    iotx_coap_context_t *p_ctx = (iotx_coap_context_t *)param;

    iotx_set_devinfo(&devinfo);
    snprintf(path, IOTX_URI_MAX_LEN, "/topic/%s/%s/update/", (char *)devinfo.product_key,
                                            (char *)devinfo.device_name);

    ret = IOT_CoAP_SendMessage(p_ctx, path, &message);
//    YUNIT_ASSERT(0 == ret); 
}

iotx_coap_context_t *p_ctx = NULL;

static void user_code_start()
{
    int ret;

    iotx_post_data_to_server((void *)p_ctx);
    ret = IOT_CoAP_Yield(p_ctx);
    YUNIT_ASSERT(IOTX_SUCCESS == ret); 
    if (m_coap_client_running) {
        yos_post_delayed_action(3000,user_code_start,NULL);
    } else {
        IOT_CoAP_Deinit(&p_ctx);
        YUNIT_ASSERT(NULL == p_ctx); 
    }
}

static void test_coap_start(void)
{
    int ret;
    iotx_coap_config_t config;
    iotx_deviceinfo_t deviceinfo;

    m_coap_client_running = 1;

    #ifdef COAP_ONLINE
        #ifdef COAP_DTLS_SUPPORT
            char url[256] = {0};
            snprintf(url, sizeof(url), IOTX_ONLINE_DTLS_SERVER_URL, IOTX_PRODUCT_KEY);
            config.p_url = url;
        #else
            printf("Online environment must access with DTLS\r\n");
            YUNIT_ASSERT(false); 
        #endif
    #else
        #ifdef COAP_DTLS_SUPPORT
            config.p_url = IOTX_PRE_DTLS_SERVER_URI;
        #else
            config.p_url = IOTX_PRE_NOSEC_SERVER_URI;
        #endif
    #endif
    iotx_set_devinfo(&deviceinfo);
    config.p_devinfo = &deviceinfo;

    p_ctx = IOT_CoAP_Init(&config);
    YUNIT_ASSERT(NULL != p_ctx); 
    if(NULL != p_ctx){
        ret = IOT_CoAP_DeviceNameAuth(p_ctx);
        YUNIT_ASSERT(IOTX_SUCCESS == ret); 
        {
            iotx_post_data_to_server((void *)p_ctx);
            ret = IOT_CoAP_Yield(p_ctx);
            YUNIT_ASSERT(IOTX_SUCCESS == ret); 
        }
        IOT_CoAP_Deinit(&p_ctx);
        YUNIT_ASSERT(NULL == p_ctx); 
    }
    else{
        printf("IoTx CoAP init failed\r\n");
        YUNIT_ASSERT(NULL != p_ctx); 
    }

//    yos_loop_run();
   
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

/* IOT_CoAP_Init: config.url非法coaps域名 */
static void ut_coap_init(void)
{
    iotx_coap_config_t  config;
    iotx_deviceinfo_t devinfo;
    void *p_ctx = NULL;

    iotx_set_devinfo(&devinfo);
    config.p_devinfo = &devinfo;
    config.p_url = "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.cn:5684";
    p_ctx = IOT_CoAP_Init(&config);
    if (p_ctx != NULL) {
        IOT_CoAP_Deinit(&p_ctx);
    }
    YUNIT_ASSERT(NULL == p_ctx); 
}

static void ut_coap_deinit(void)
{
    void *p_ctx;
    iotx_coap_config_t config = {0};
    iotx_deviceinfo_t devinfo;
    iotx_set_devinfo(&devinfo);
    config.p_devinfo = &devinfo;

    config.p_url = COAP_URL;
    p_ctx = IOT_CoAP_Init(&config);
    YUNIT_ASSERT(NULL != p_ctx); 

    IOT_CoAP_Deinit(&p_ctx);
    YUNIT_ASSERT(NULL == p_ctx); 
}

static yunit_test_case_t yunos_basic_testcases[] = {
//    { "init", ut_coap_init},
//    { "deinit", ut_coap_deinit},
    { "coap_start", test_coap_start},
//    { "sandbox",test_alink_sandbox},
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "coap", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_coap(void)
{    
    yunit_add_test_suites(suites);
}

