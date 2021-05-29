#include "azblob.h"

#include <stdio.h>
#include <stdlib.h>

#include "azure_c_shared_utility/shared_util_options.h"
#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_message.h"
#include "iothubtransporthttp.h"

#include "certs.h"

static const char *connectionString = "HostName=ivohub2.azure-devices.net;DeviceId=C610;SharedAccessKey=KkuvGKU0am1wL6+fgI9Xp/JUZr5ataYhQcfQ2Csd9XA=";
//static const char *connectionString = "HostName=ivohub2.azure-devices.net;DeviceId=C610-2;SharedAccessKey=6/4didae/YnSXbkuZOCRr+kNikypGV6Jd7B6hm8cNQU=";

static const char *data_to_upload_format = "Hello World from IoTHubDeviceClient_LL_UploadToBlob block: %d\n";
static char data_to_upload[128];
static int block_count = 0;

static IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_RESULT
getDataCallback(IOTHUB_CLIENT_FILE_UPLOAD_RESULT result, unsigned char const **data, size_t *size, void *context) {
    (void) context;
    if (result == FILE_UPLOAD_OK) {
        if (data != NULL && size != NULL) {
            // "block_count" is used to simulate reading chunks from a larger data content, like a large file.
            // Note that the IoT SDK caller does NOT free(*data), as a typical use case the buffer returned
            // to the IoT layer may be part of a larger buffer that this callback is chunking up for network sends.

            if (block_count < 100) {
                int len = snprintf(data_to_upload, sizeof(data_to_upload), data_to_upload_format, block_count);
                if (len < 0 || len >= sizeof(data_to_upload)) {
                    return IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_ABORT;
                }

                *data = (const unsigned char *) data_to_upload;
                *size = strlen(data_to_upload);
                block_count++;
            } else {
                // This simulates reaching the end of the file. At this point all the data content has been uploaded to blob.
                // Setting data to NULL and/or passing size as zero indicates the upload is completed.

                *data = NULL;
                *size = 0;

                (void) printf("Indicating upload is complete (%d blocks uploaded)\r\n", block_count);
            }
        } else {
            // The last call to this callback is to indicate the result of uploading the previous data block provided.
            // Note: In this last call, data and size pointers are NULL.

            (void) printf("Last call to getDataCallback (result for %dth block uploaded: %s)\r\n", block_count,
                          MU_ENUM_TO_STRING(IOTHUB_CLIENT_FILE_UPLOAD_RESULT, result));
        }
    } else {
        (void) printf("Received unexpected result %s\r\n", MU_ENUM_TO_STRING(IOTHUB_CLIENT_FILE_UPLOAD_RESULT, result));
    }

    // This callback returns IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_OK to indicate that the upload shall continue.
    // To abort the upload, it should return IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_ABORT
    return IOTHUB_CLIENT_FILE_UPLOAD_GET_DATA_OK;
}

int upload(void) {
    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

    (void) IoTHub_Init();
    (void) printf("Starting the IoTHub client sample upload to blob with multiple blocks...\r\n");

    device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, HTTP_Protocol);
    if (device_ll_handle == NULL) {
        (void) printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
    } else {
        // Setting the Trusted Certificate. This is only necessary on systems without
        // built in certificate stores.
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);

        if (IoTHubDeviceClient_LL_UploadMultipleBlocksToBlob(device_ll_handle, "subdir/hello_world_mb.txt",
                                                             getDataCallback, NULL) != IOTHUB_CLIENT_OK) {
            (void) printf("hello world failed to upload\n");
        } else {
            (void) printf("hello world has been created\n");
        }
    }

    printf("Press any key to continue");
    (void) getchar();

    // Clean up the iothub sdk handle
    IoTHubDeviceClient_LL_Destroy(device_ll_handle);

    IoTHub_Deinit();
    return 0;
}

