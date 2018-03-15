#ifndef LINKKIT_EXPORT_H
#define LINKKIT_EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "dm_export.h"
#include "lite_queue.h"

#ifdef OTA_ENABLED
#include "fota_export.h"
#endif /* OTA_ENABLED */

typedef struct _linkkit_ops {
    int (*on_connect)(void *ctx);
    int (*on_disconnect)(void *ctx);

    int (*raw_data_arrived)(void *thing_id, void *data, int len, void *ctx);

    int (*thing_create)(void *thing_id, void *ctx);

    int (*thing_enable)(void *thing_id, void *ctx);
    int (*thing_disable)(void *thing_id, void *ctx);

    int (*thing_call_service)(void *thing_id, char *service, int request_id, void *ctx);
    int (*thing_prop_changed)(void *thing_id, char *property, void *ctx);
} linkkit_ops_t;

typedef enum _linkkit_loglevel {
    linkkit_loglevel_emerg = 0,
    linkkit_loglevel_crit,
    linkkit_loglevel_error,
    linkkit_loglevel_warning,
    linkkit_loglevel_info,
    linkkit_loglevel_debug,
} linkkit_loglevel_t;

/* domain type */
/* please sync with dm_cloud_domain_type_t */
typedef enum {
    /* iot-as-mqtt.cn-shanghai.aliyuncs.com */
    linkkit_cloud_domain_sh,
    /* USA */
    linkkit_cloud_domain_usa,

    linkkit_cloud_domain_max,
} linkkit_cloud_domain_type_t;

/**
 * @brief dispatch message of queue for further process.
 *
 * @return int, 0 when success, -1 when fail.
 */
int linkkit_dispatch(void);

/**
 * @brief start linkkit routines, and install callback funstions(async type for cloud connecting).
 *
 * @param max_buffered_msg, specify max buffered message size.
 * @param ops, callback function struct to be installed.
 * @param get_tsl_from_cloud, config if device need to get tsl from cloud(!0) or local(0), if local selected, must invoke linkkit_set_tsl to tell tsl to dm after start complete.
 * @param log_level, config log level.
 * @param user_context, user context pointer.
 * @param domain_type, specify the could server domain.
 *
 * @return int, 0 when success, -1 when fail.
 */
int linkkit_start(int max_buffered_msg, int get_tsl_from_cloud, linkkit_loglevel_t log_level, linkkit_ops_t *ops, linkkit_cloud_domain_type_t domain_type, void *user_context);

#ifdef OTA_ENABLED
/**
 * @brief init fota service routines, and install callback funstions.
 *
 * @param callback_fp, callback function to be installed.
 *
 * @return int, 0 when success, -1 when fail.
 */
int linkkit_ota_init(handle_service_fota_callback_fp_t callback_fp);
#endif /* OTA_ENABLED */

/**
 * @brief stop linkkit routines.
 *
 *
 * @return 0 when success, -1 when fail.
 */
int linkkit_end();

/**
 * @brief install user tsl.
 *
 * @param tsl, tsl string that contains json description for thing object.
 * @param tsl_len, tsl string length.
 *
 * @return pointer to thing object, NULL when fails.
 */
extern void* linkkit_set_tsl(const char* tsl, int tsl_len);

/* patterns: */
/* method:
 * set_property_/event_output_/service_output_value:
 * method_set, thing_id, identifier, value */

typedef enum {
    linkkit_method_set_property_value = 0,
    linkkit_method_set_event_output_value,
    linkkit_method_set_service_output_value,

    linkkit_method_set_number,
} linkkit_method_set_t;

/**
 * @brief set value to property, event output, service output items.
 *        if identifier is struct type or service output type or event output type, use '.' as delimeter like "identifier1.ientifier2"
 *        to point to specific item.
 *        value and value_str could not be NULL at the same time;
 *        if value and value_str both as not NULL, value shall be used and value_str will be ignored.
 *        if value is NULL, value_str not NULL, value_str will be used.
 *        in brief, value will be used if not NULL, value_str will be used only if value is NULL.
 *
 * @param method_set, specify set value type.
 * @param thing_id, pointer to thing object, specify which thing to set.
 * @param identifier, property, event output, service output identifier.
 * @param value, value to set.(input int* if target value is int type or enum or bool, float* if float type,
 *        long long* if date type, char* if text type).
 * @param value_str, value to set in string format if value is null.
 *
 * @return 0 when success, -1 when fail.
 */
extern int linkkit_set_value(linkkit_method_set_t method_set, const void* thing_id, const char* identifier,
                             const void* value, const char* value_str);

typedef enum {
    linkkit_method_get_property_value = 0,
    linkkit_method_get_event_output_value,
    linkkit_method_get_service_input_value,
    linkkit_method_get_service_output_value,

    linkkit_method_get_number,
} linkkit_method_get_t;

/**
 * @brief get value from property, event output, service input/output items.
 *        if identifier is struct type or service input/output type or event output type, use '.' as delimeter like "identifier1.ientifier2"
 *        to point to specific item.
 *        value and value_str could not be NULL at the same time;
 *        if value and value_str both as not NULL, value shall be used and value_str will be ignored.
 *        if value is NULL, value_str not NULL, value_str will be used.
 *        in brief, value will be used if not NULL, value_str will be used only if value is NULL.
 * @param method_get, specify get value type.
 * @param thing_id, pointer to thing object, specify which thing to get.
 * @param identifier, property, event output, service input/output identifier.
 * @param value, value to get(input int* if target value is int type or enum or bool, float* if float type,
 *        long long* if date type, char* if text type).
 * @param value_str, value to get in string format. DO NOT modify this when function returns,
 *        user should copy to user's own buffer for further process.
 *        user should NOT free the memory.
 *
 * @return 0 when success, -1 when fail.
 */
extern int linkkit_get_value(linkkit_method_get_t method_get, const void* thing_id, const char* identifier,
                             void* value, char** value_str);

/**
 * @brief answer to a service when a service requested by cloud.
 *
 * @param thing_id, pointer to thing object.
 * @param service_identifier, service identifier to answer, user should get this identifier from handle_dm_callback_fp_t type callback
 *        report that "dm_callback_type_service_requested" happened, use this function to generate response to the service sender.
 * @param response_id, id value in response payload. its value is from "dm_callback_type_service_requested" type callback function.
 *        use the same id as the request to send response as the same communication session.
 * @param code, code value in response payload. for example, 200 when service successfully executed, 400 when not successfully executed.
 *
 * @return 0 when success, -1 when fail.
 */
extern int linkkit_answer_service(const void* thing_id, const char* service_identifier, int response_id, int code);

/**
 * @brief answer a down raw service when a raw service requested by cloud, or invoke a up raw service to cloud.
 *
 * @param thing_id, pointer to thing object.
 * @param is_up_raw, specify up raw(not 0) or down raw reply(0).
 * @param raw_data, raw data that sent to cloud.
 * @param raw_data_length, raw data length that sent to cloud.
 *
 * @return 0 when success, -1 when fail.
 */
extern int linkkit_invoke_raw_service(const void* thing_id, int is_up_raw, void* raw_data, int raw_data_length);

#ifdef OTA_ENABLED
/**
 * @brief perform ota service when "new version detected" reported.
 *
 * @param is_up_raw, specify up raw(not 0) or down raw reply(0).
 * @param data_buf, data buf that used to do ota. ota service will use this buffer to download bin.
 * @param data_buf_length, data buf length that used to do ota.
 *
 * @return 0 when success, -1 when fail.
 */
extern int linkkit_invoke_ota_service(void* data_buf, int data_buf_length);
#endif /* OTA_ENABLED */

/**
 * @brief trigger a event to post to cloud.
 *
 * @param thing_id, pointer to thing object.
 * @param event_identifier, event identifier to trigger.
 * @param property_identifier, used when trigger event with method "event.property.post", if set, post specified property, if NULL, post all.
 *
 * @return 0 when success, -1 when fail.
 */
extern int linkkit_trigger_event(const void* thing_id, const char* event_identifier, const char* property_identifier);

#ifndef CMP_SUPPORT_MULTI_THREAD
/**
 * @brief this function used to yield when want to receive or send data.
 *        if multi-thread enabled, user should NOT call this function.
 *
 * @param timeout_ms, timeout value in ms.
 *
 * @return 0 when success, -1 when fail.
 */
extern int linkkit_yield(int timeout_ms);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LINKKIT_EXPORT_H */
