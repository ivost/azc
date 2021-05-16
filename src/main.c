
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mqueue.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azc.h"

static mqd_t mq = -1;

int msg_init() {
    // Specify message queue attributes.
    struct mq_attr ma;      // message queue attributes
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
    return 0;
}

int main(int argc, char *argv[]) {
    int rc;
    struct cam_context ctx;
    struct objdet_result * pr;
    struct bbox * pbt;
    char msg[MAX_MSG];

    rc = msg_init();
    if (rc != 0) {
        printf("Init error");
        return rc;
    }

    rc = azc_init();
    if (rc != 0) {
        printf("Init error");
        return rc;
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
        printf("got %d bytes, num bb %d\n", n, pr->numbb);
        rc = azc_send_result(pr);
        (void) rc;
        //ThreadAPI_Sleep(100);
    }

    ThreadAPI_Sleep(1000);

    azc_reset();
}


