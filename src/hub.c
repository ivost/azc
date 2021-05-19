//
// Created by ivo on 5/18/21.
//

#include <mqueue.h>
#include <string.h>
#include <stdio.h>

#include "azc.h"
#include "azure_c_shared_utility/threadapi.h"
#include "hub.h"

#define QUEUE_NAME "/Qazc"
#define MAX_MSG (8000)

mqd_t msg_init() {
    mqd_t mq = -1;
    // Specify message queue attributes.
    struct mq_attr ma;              // message queue attributes
    ma.mq_flags = 0;                // blocking read/write
    ma.mq_maxmsg = 10;              // maximum number of messages allowed in queue
    ma.mq_msgsize = MAX_MSG;
    ma.mq_curmsgs = 0;              // number of messages currently in queue

    mq_unlink(QUEUE_NAME);
    printf("Creating %s\n", QUEUE_NAME);
    // Create the message queue with some default settings.
    mq = mq_open(QUEUE_NAME, O_RDWR | O_CREAT, 0700, &ma);
    if (mq < 0) {
        printf("error creating queue %d\n", mq);
        return -1;
    }
    printf("mq %d\n", mq);
    return mq;
}

void * hub_thread(void *ptr)
{
    // posix message queue to receive obj.detection events from gst overlay plugin
    mqd_t mq;
    int rc;
    (void) ptr;

    struct cam_context ctx;
    struct objdet_result * pr;
    struct bbox * pbt;
    char msg[MAX_MSG];

    mq = msg_init();
    if (mq == -1) {
        printf("MQ Init error");
        return NULL;
    }

    rc = azc_init();
    if (rc != 0) {
        printf("AZ HUB Init error");
        return NULL;
    }
    // send context - once on start
    //todo: move to plugin
    ctx.ctx_id = 100;
    ctx.cam = 2;
    ctx.model = 3;
    ctx.width = 640;
    ctx.height = 480;
    ctx.scale_x = 0.521;
    ctx.scale_y = 0.4333;
    strcpy(ctx.fields, "x,y,w,h,conf,cat");

    rc = azc_send_context(&ctx);
    (void) rc;

    while (1) {
        int n = mq_receive(mq, msg, MAX_MSG, 0);
        if (n < 0) {
            printf("mq_receive error\n");
            ThreadAPI_Sleep(1000);
            continue;
        }
        pr = (struct objdet_result *) &msg;
        printf("<<<<< ctx %d, num bb %d\n", pr->ctx_id, pr->numbb);
        rc = azc_send_result(pr);
        (void) rc;
        ThreadAPI_Sleep(10);
    }
    azc_reset();
    return NULL;
}

