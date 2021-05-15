
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mqueue.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azc.h"

#define QUEUE_NAME "/Qazc"
#define MAX_MSG (8000)

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
    struct objdet_result r;
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
    int i = 0;
    while (1) {
        int n = mq_receive(mq, msg, MAX_MSG, 0);
        if (n < 0) {
            printf("mq_receive error\n");
            ThreadAPI_Sleep(1000);
            continue;
        }
        printf("got %d bytes\n", n);
        i++;
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

        r.ctx_id = ctx.ctx_id;

        r.time = time(NULL);
        r.numbb = i+1;
        r.bb = pbt = calloc(r.numbb, sizeof(struct bbox));
        for (int i=0; i<r.numbb; i++) {
            pbt->x = 1 + i;
            pbt->y = 2 + i;
            pbt->width = 10 + 20*i;
            pbt->height = 20 + 40*i;
            pbt->conf = 0.543666666;
            pbt->cat = 1;
            pbt++;
        }
        rc = azc_send_result(&r);
        (void) rc;
        //ThreadAPI_Sleep(100);
        free(r.bb);
    }

    ThreadAPI_Sleep(1000);

    azc_reset();
}


