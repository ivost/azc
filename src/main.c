
#include <time.h>
#include <pthread.h>

#include "hub.h"
#include "upload.h"

int main(int argc, char *argv[]) {
    pthread_t thread1, thread2;
    pthread_create( &thread1, NULL, hub_thread, NULL);
    pthread_create( &thread2, NULL, upload_thread, NULL);
    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);
    return 0;
}


