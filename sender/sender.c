#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

const int TYPE = 1;
int msgid;

typedef struct {
    long mtype;
    char data[512];
} msg;

void send() {
    msg mb;
    mb.mtype = TYPE;
    printf("Sending...\n");
    for (int i=0; i<100; i++) {
        sprintf(mb.data, "Message %d", i);
        if (msgsnd(msgid, (void *) &mb, strlen(mb.data), 0) < 0) {
            printf("msgsnd error\n");
            return;
        }
    }
}

void receive() {
    msg mb;
    printf("receiving...\n");
    for (int i=0; i<100; i++) {
        int n = msgrcv(msgid, &mb, sizeof(mb.data), TYPE, 0);
        if (n < 0) {
            printf("msgrcv error\n");
            return;
        }
        mb.data[n] = 0;
        printf("got: %s\n", mb.data);
    }
}

int main() {
    int k = ftok("/tmp", 0x6666);
    msgid = msgget(k, IPC_CREAT | 0644);
    pid_t id = fork();
    if (id < 0) {
        printf("fork error\n");
        return 1;
    }

    if (id == 0) {
        send();
    } else {
        receive();
    }
    printf("Press key to exit\n");
    getchar();
    waitpid(id, NULL, 0);
    printf("Removing queue\n");
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}
