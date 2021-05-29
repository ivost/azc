#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iothub.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "iothubtransporthttp.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/tickcounter.h"
//#include "iothubtransportmqtt.h"
#include "iothubtransportamqp.h"
#include "parson.h"
#include "certs.h"
#include "azc.h"
#include "math.h"

static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void *user_context);

static int device_method_callback(const char *method_name, const unsigned char *payload, size_t size,
                                  unsigned char **response, size_t *resp_size, void *userContextCallback);

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result,
                                       IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context);

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback);

static const char *connectionString = "HostName=ivohub2.azure-devices.net;DeviceId=C610;SharedAccessKey=KkuvGKU0am1wL6+fgI9Xp/JUZr5ataYhQcfQ2Csd9XA=";
//static const char *connectionString = "HostName=ivohub2.azure-devices.net;DeviceId=C610-2;SharedAccessKey=6/4didae/YnSXbkuZOCRr+kNikypGV6Jd7B6hm8cNQU=";

static size_t g_message_count_send_confirmations = 0;
static IOTHUB_MESSAGE_HANDLE message_handle;

static IOTHUB_DEVICE_CLIENT_HANDLE device_handle;
static IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

static int messagecount = 0;

// init - returns 0 on success, else error
int azc_init() {

    if (device_ll_handle && device_handle) {
        // already initialized
        return 0;
    }
    int rc = IoTHub_Init();
    if (rc) {
        printf("IoTHub_Init error\r\n");
        return rc;
    }

    device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, HTTP_Protocol);
    if (device_ll_handle == NULL) {
        printf("LL connection error\r\n");
        return 1;
    }
    IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);

    device_handle = IoTHubDeviceClient_CreateFromConnectionString(connectionString, AMQP_Protocol);
    if (device_handle == NULL) {
        printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
        return 2;
    }

    // Setting message callback to get C2D messages
    (void) IoTHubDeviceClient_SetMessageCallback(device_handle, receive_msg_callback, NULL);
    // Setting method callback to handle a SetTelemetryInterval method to control
    //   how often telemetry messages are sent from the simulated device.
    (void) IoTHubDeviceClient_SetDeviceMethodCallback(device_handle, device_method_callback, NULL);
    // Setting connection status callback to get indication of connection to iothub
    (void) IoTHubDeviceClient_SetConnectionStatusCallback(device_handle, connection_status_callback, NULL);

    // Set any option that are necessary.
    // For available options please see the iothub_sdk_options.md documentation

    // Setting Log Tracing.
    // Log tracing is supported in MQTT and AMQP. Not HTTP.

    bool traceOn = false;
    //bool traceOn = true;
    IoTHubDeviceClient_SetOption(device_handle, OPTION_LOG_TRACE, &traceOn);
    // Setting the frequency of DoWork calls by the underlying process thread.
    // The value ms_delay is a delay between DoWork calls, in milliseconds.
    // ms_delay can only be between 1 and 100 milliseconds.
    // Without the SetOption, the delay defaults to 1 ms.
    tickcounter_ms_t ms_delay = 10;
    IoTHubDeviceClient_SetOption(device_handle, OPTION_DO_WORK_FREQUENCY_IN_MS, &ms_delay);
    // Setting the Trusted Certificate. This is only necessary on systems without
    // built in certificate stores.
    IoTHubDeviceClient_SetOption(device_handle, OPTION_TRUSTED_CERT, certificates);

    //Setting the auto URL Encoder (recommended for MQTT). Please use this option unless
    //you are URL Encoding inputs yourself.
    //ONLY valid for use with MQTT
    // bool urlEncodeOn = true;
    // (void) IoTHubDeviceClient_SetOption(device_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);
    return 0;
}

int azc_reset() {
    printf("azc_reset");
    // Clean up the iothub sdk handle
    if (device_handle) {
        IoTHubDeviceClient_Destroy(device_handle);
        device_handle = NULL;
    }
    if (device_ll_handle) {
        IoTHubDeviceClient_LL_Destroy(device_ll_handle);
        device_ll_handle = NULL;
    }
    // Free all the sdk subsystem
    IoTHub_Deinit();
    return 0;
}

static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void *user_context) {
    (void) user_context;
    const char *messageId;
    const char *correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL) {
        messageId = "<unavailable>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL) {
        correlationId = "<unavailable>";
    }

    IOTHUBMESSAGE_CONTENT_TYPE content_type = IoTHubMessage_GetContentType(message);
    if (content_type == IOTHUBMESSAGE_BYTEARRAY) {
        const unsigned char *buff_msg;
        size_t buff_len;

        if (IoTHubMessage_GetByteArray(message, &buff_msg, &buff_len) != IOTHUB_MESSAGE_OK) {
            (void) printf("Failure retrieving byte array message\r\n");
        } else {
            (void) printf(
                    "Received Binary message\r\nMessage ID: %s\r\n Correlation ID: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n",
                    messageId, correlationId, (int) buff_len, buff_msg, (int) buff_len);
        }
    } else {
        const char *string_msg = IoTHubMessage_GetString(message);
        if (string_msg == NULL) {
            (void) printf("Failure retrieving byte array message\r\n");
        } else {
            (void) printf("Received String Message\r\nMessage ID: %s\r\n Correlation ID: %s\r\n Data: <<<%s>>>\r\n",
                          messageId, correlationId, string_msg);
        }
    }
    return IOTHUBMESSAGE_ACCEPTED;
}


static int
device_method_callback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response,
                       size_t *resp_size, void *userContextCallback) {
    const char *SetTelemetryIntervalMethod = "SetTelemetryInterval";
    const char *device_id = (const char *) userContextCallback;
    char *end = NULL;
    int newInterval;

    int status = 501;
    const char *RESPONSE_STRING = "{ \"Response\": \"Unknown method requested.\" }";

    (void) printf("\r\nDevice Method called for device %s\r\n", device_id);
    (void) printf("Device Method name:    %s\r\n", method_name);
    (void) printf("Device Method payload: %.*s\r\n", (int) size, (const char *) payload);

    if (strcmp(method_name, SetTelemetryIntervalMethod) == 0) {
        if (payload) {
            newInterval = (int) strtol((char *) payload, &end, 10);
        }
    }

    (void) printf("\r\nResponse status: %d\r\n", status);
    (void) printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = malloc(*resp_size)) == NULL) {
        status = -1;
    } else {
        memcpy(*response, RESPONSE_STRING, *resp_size);
    }

    return status;
}


static void
connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
                           void *user_context) {
    (void) reason;
    (void) user_context;
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        (void) printf("The device client is connected to iothub\r\n");
    } else {
        (void) printf("The device client has been disconnected\r\n");
    }
}

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback) {
    (void) userContextCallback;
    // When a message is sent this callback will get invoked
    g_message_count_send_confirmations++;
//    printf("Confirmation callback received for message %lu with result %s\r\n",
//                  (unsigned long) g_message_count_send_confirmations,
//                  MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

// 3 decimal points precision - divide by 1000
int32_t float_to_int(float f) {
    return (int32_t) roundf(1000 * f);
}

char * azc_serialize_result(struct objdet_result * res) {
    JSON_Value *rootv = json_value_init_object();
    JSON_Object *root = json_value_get_object(rootv);
    json_object_set_number(root, "time", res->time);
    json_object_set_number(root, "ctx", res->ctx_id);
    json_object_set_number(root, "n", res->numbb);

    JSON_Value *barr = json_value_init_array();
    json_object_set_value(root, "bb", barr);
    JSON_Array * bbs_arr = json_value_get_array(barr);

    struct bbox * pb = res->bb;
    for (int idx = 0; idx < res->numbb; idx++) {
        JSON_Value *arrb = json_value_init_array();
        JSON_Array *arr = json_value_get_array(arrb);
        json_array_append_number(arr, pb->x);
        json_array_append_number(arr, pb->y);
        json_array_append_number(arr, pb->width);
        json_array_append_number(arr, pb->height);
        json_array_append_number(arr, pb->cat);
        json_array_append_number(arr, float_to_int(pb->conf));
        json_array_append_value(bbs_arr, arrb);
        pb ++;
    }
    char * p = json_serialize_to_string(rootv);
    json_value_free(rootv);
    return p;
}

char * azc_serialize_context(struct cam_context * ctx) {
    JSON_Value *rootv = json_value_init_object();
    JSON_Object *root = json_value_get_object(rootv);
    json_object_set_number(root, "cid", ctx->ctx_id);
    json_object_set_number(root, "cam", ctx->cam);
    json_object_set_number(root, "w", ctx->width);
    json_object_set_number(root, "h", ctx->height);
    json_object_set_number(root, "model", ctx->model);
    json_object_set_number(root, "xf", float_to_int(ctx->scale_x));
    json_object_set_number(root, "yf", float_to_int(ctx->scale_y));
    json_object_set_string(root, "fields", ctx->fields);
    char * p = json_serialize_to_string(rootv);
    json_value_free(rootv);
    return p;
}

int azc_send_context(struct cam_context * ctx) {
    int rc = 0;
    printf("azc_send_context - ctx %d\n", ctx->ctx_id);
    char * msg = azc_serialize_context(ctx);
    printf("Sending context message %d to IoTHub\nMessage: %s\n", (int) (messagecount + 1), msg);
    message_handle = IoTHubMessage_CreateFromString(msg);
    IoTHubMessage_SetProperty(message_handle, "T", "C");
    IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application/json");
    rc = IoTHubDeviceClient_SendEventAsync(device_handle, message_handle, send_confirm_callback, NULL);
    // safe to destroy
    json_free_serialized_string(msg);
    IoTHubMessage_Destroy(message_handle);
    return rc;
}

int azc_send_result(struct objdet_result * res) {
    int rc = 0;
    //printf("azc_send_result - ctx %d, %d boxes\n", res->ctx_id, res->numbb);
    char * msg = azc_serialize_result(res);
    //printf("Sending result message %d to IoTHub\nMessage: %s\n", (int) (messagecount + 1), msg);
    message_handle = IoTHubMessage_CreateFromString(msg);
    IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application/json");
    // IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8");
    IoTHubMessage_SetProperty(message_handle, "T", "DR");
    rc = IoTHubDeviceClient_SendEventAsync(device_handle, message_handle, send_confirm_callback, NULL);
    // safe to destroy
    json_free_serialized_string(msg);
    IoTHubMessage_Destroy(message_handle);
    messagecount = messagecount + 1;
    return rc;
}
/*
type Clip struct {
	Id        int64    `db:"id" json:"id"`
	Vid       string   `db:"vid" json:"vid"`
	CtxId     int16    `db:"ctx_id" json:"cid"`
	BeginTime int64    `db:"start_time" json:"s"`
	EndTime   int64    `db:"end_time" json:"t"`
	Width     int16    `db:"width" json:"w"`
	Height    int16    `db:"height" json:"h"`
	Duration  int16    `db:"duration" json:"d"`
	Events    []string `db:"events" json:"events"`
} */
int azc_send_video_id(int ctx_id, const char * uuid, long begin_time, long end_time) {
    int rc = 0;
    printf("azc_send_video_id - ctx %d, begin %ld, end %ld, video uid %s\n",
           ctx_id, begin_time, end_time, uuid);

    JSON_Value *rootv = json_value_init_object();
    JSON_Object *root = json_value_get_object(rootv);
    json_object_set_number(root, "cid", ctx_id);
    json_object_set_number(root, "s", begin_time);
    json_object_set_number(root, "t", end_time);
    json_object_set_number(root, "d", end_time - begin_time);
    json_object_set_string(root, "vid", uuid);
    char * msg = json_serialize_to_string(rootv);
    //printf("=== Sending video_id to IoTHub\nMessage: %s\n",  msg);
    message_handle = IoTHubMessage_CreateFromString(msg);
    IoTHubMessage_SetProperty(message_handle, "T", "V");
    IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application/json");
    rc = IoTHubDeviceClient_SendEventAsync(device_handle, message_handle, send_confirm_callback, NULL);
    // safe to destroy
    json_value_free(rootv);
    json_free_serialized_string(msg);
    IoTHubMessage_Destroy(message_handle);
    return rc;
}

static const int upload_buffer_size = 1024000;
static char *upload_buffer;
static int block_count = 0;
static size_t total_bytes = 0;

static IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_RESULT
getDataCallback(IOTHUB_CLIENT_FILE_UPLOAD_RESULT result, unsigned char const **data, size_t *size, void *context) {
    FILE *fp = (FILE *) context;
    if (result != FILE_UPLOAD_OK) {
        printf("*** Received error %s\n", MU_ENUM_TO_STRING(IOTHUB_CLIENT_FILE_UPLOAD_RESULT, result));
        return IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_ABORT;
    }
    // done?
    if (data == NULL || size == NULL) {
        printf("Uploaded, total_bytes: %ld\n", total_bytes);
        return IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_OK;
    }
    // read block of data
    size_t len = fread(upload_buffer, 1, upload_buffer_size, fp);
    *data = NULL;
    *size = len;
    total_bytes += len;
    if (len > 0) {
        block_count++;
        //printf("Uploading  block %d - %ld bytes\n", block_count, len);
        *data = (const unsigned char *) upload_buffer;
    }
    return IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_OK;
}

// MU_ENUM_TO_STRING(IOTHUB_CLIENT_FILE_UPLOAD_RESULT, result)

int azc_upload(const char *file_name, const char *blob_path) {
    if (device_ll_handle == NULL) {
        printf("azc not initialized\n");
        return 1;
    }
    FILE *fp = fopen(file_name, "rb");
    if (fp == NULL) {
        printf("azc_upload - File %s not found\n", file_name);
        return 2;
    }
    upload_buffer = (char *) malloc(upload_buffer_size);
    if (upload_buffer == NULL) {
        fclose(fp);
        printf("malloc %d failed\n", upload_buffer_size);
        return 3;
    }
    printf("Uploading %s ...\n", file_name);
    total_bytes = 0;
    int rc = IoTHubDeviceClient_LL_UploadMultipleBlocksToBlob(device_ll_handle, blob_path, getDataCallback, fp);
    printf("upload rc %d\n", rc);
    free(upload_buffer);
    fclose(fp);
    return rc;
}

// Set Message properties
//    (void) IoTHubMessage_SetMessageId(message_handle, "MSG_ID");
//    (void) IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID");
//ThreadAPI_Sleep(g_interval);
