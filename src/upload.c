//
// Created by ivo on 5/18/21.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>

#include "upload.h"

#define VIDEO_DIR "/mnt/sdcard/video/"

#define EVENT_SIZE    ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

void filewatch_init(char *dir, int *fd, int *wd) {
    *fd = inotify_init();
    *wd = inotify_add_watch( *fd, dir, IN_CREATE | IN_OPEN | IN_CLOSE | IN_DELETE );
    printf("filewatch_init %s, fw %d, wd %d\n", dir, *fd, *wd);
}

void * upload_thread(void *ptr) {
    (void) ptr;
    int fd, wd;
    int numRead;
    char buf[EVENT_BUF_LEN];
    struct inotify_event * p_event;

    printf("upload_thread\n");
    filewatch_init(VIDEO_DIR, &fd, &wd);
    if (fd == -1 || wd == -1) {
        printf("inotify error dir %s\n", VIDEO_DIR);
    }
    for (;;) {
        usleep(1000);
        numRead = read(fd, buf, EVENT_BUF_LEN);
        for (char *p = buf; p < buf + numRead; ) {
            p_event = ( struct inotify_event * ) p;
            if ( p_event->mask & IN_OPEN ) {
                printf( "=== File %s opened.\n", p_event->name );
            } else if ( p_event->mask & IN_CREATE ) {
                printf( "=== New file %s created.\n", p_event->name );
            } else if ( p_event->mask & IN_CLOSE ) {
                printf( "=== File %s closed.\n", p_event->name );
            } else if ( p_event->mask & IN_DELETE ) {
                printf( "=== File %s deleted.\n", p_event->name );
            }
            p += sizeof(struct inotify_event) + p_event->len;
        }
    }
    inotify_rm_watch( fd, wd );
}
