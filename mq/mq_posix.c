#include <mqueue.h>
#include <string.h>
#include <stdio.h>

#define QUEUE_NAME "/Qazc"
#define MAX_MSG (8000)
/*
 cat /proc/sys/fs/mqueue/msgsize_max
8192

 */
mqd_t mq;

void send() {
    char msg[MAX_MSG];
    printf("Open %s\n", QUEUE_NAME);
    mq = mq_open(QUEUE_NAME, O_WRONLY);
    for (int i=0; i<10; i++) {
        //sprintf(msg, "Message %d", i);
        //printf("Sending %s\n", msg);
        //if (mq_send(mq, msg, strlen(msg), 0) < 0) {
        if (mq_send(mq, msg, MAX_MSG, 0) < 0) {
            printf("mq_send error\n");
            return;
        }
    }
}

void receive() {
    struct mq_attr ma;      // message queue attributes
    char msg[MAX_MSG];

    // Specify message queue attributes.
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
        return;
    }
    printf("receiving...\n");
    for (int i=0; i<10; i++) {
        int n = mq_receive(mq, msg, MAX_MSG, 0);
        if (n < 0) {
            printf("mq_receive error\n");
            return;
        }
        //msg[n] = 0;
        //printf("got: %s\n", msg);
        printf("got %d bytes\n", n);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        // receive will create the queue
        receive();
        printf("Press key to exit\n");
        getchar();
        printf("Removing queue\n");
        mq_close(mq);
        mq_unlink(QUEUE_NAME);
        return 0;
    }
    send();
    return 0;
}
