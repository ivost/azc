#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

const int TYPE = 1;
int qid;

typedef struct {
    long mtype;
    char data[512];
} msg;

void send() {
    msg mb;
    mb.mtype = TYPE;
    printf("Sending...\n");
    for (int i=0; i<10; i++) {
        sprintf(mb.data, "Message %d", i);
        if (msgsnd(qid, (void *) &mb, strlen(mb.data), 0) < 0) {
            printf("msgsnd error\n");
            return;
        }
    }
}

void receive() {
    msg mb;
    printf("receiving...\n");
    for (int i=0; i<10; i++) {
        int n = msgrcv(qid, &mb, sizeof(mb.data), TYPE, 0);
        if (n < 0) {
            printf("msgrcv error\n");
            return;
        }
        mb.data[n] = 0;
        printf("got: %s\n", mb.data);
    }
}

int main() {
    int k = ftok("/tmp", 0x42);
    printf("tok %x\n", k);
    qid = msgget(k, IPC_CREAT | 0644);
    printf("Q id %x\n", qid);

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
    msgctl(qid, IPC_RMID, NULL);
    return 0;
}
