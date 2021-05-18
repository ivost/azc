//
// Created by ivo on 5/18/21.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/inotify.h>
#include "upload.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int filewatch_init(char *dir) {
    int fd;
    fd = inotify_init();
    wd = inotify_add_watch( fd, dir, IN_CREATE | IN_CLOSE_WRITE | IN_DELETE );
    return fd;
}

void * upload_thread(void *ptr) {
    (void) ptr;
    int wd;
    char buffer[EVENT_BUF_LEN];
    printf("upload_thread\n");
    wd = filewatch_init("/home/ivo/Videos");
    while (1) {
        usleep(10000);
        ssize_t length = read( fd, buffer, EVENT_BUF_LEN );
        while ( i > 0 && i < length ) {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
            if ( event->len ) {
                if ( event->mask & IN_CREATE ) {
                    printf( "New file %s created.\n", event->name );
                } else if ( event->mask & IN_CLOSE_WRITE ) {
                    printf( "File %s closed.\n", event->name );
                } else if ( event->mask & IN_DELETE ) {
                    printf( "File %s deleted.\n", event->name );
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
    inotify_rm_watch( fd, wd );
}
