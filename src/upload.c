/**
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/inotify.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>

#include "upload.h"

#define VIDEO_DIR "/mnt/sdcard/video/"

#define MAX_CONTEXT 16
#define MAX_FILE_NAME 32
#define MAX_JSON_RESP 2048

#define EVENT_SIZE    ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t trigger_lock = PTHREAD_MUTEX_INITIALIZER;;

// todo: token and config
const char * CURL= "curl -X POST 'https://api.cloudflare.com/client/v4/accounts/22b892bb1c1f18e5abf75e8b3cccf34b/stream' -H 'X-Auth-Email: ivostoy@gmail.com' -H 'X-Auth-Key: 5aac03bca5f786b2cac6fd42b58a7603869fe' --form 'file=@/data/video/%s'";

char files[MAX_CONTEXT][MAX_FILE_NAME];
long triggers[MAX_CONTEXT];

char * upload_file(char *p, int ctx, long time);

char * build_file_name(char *p, int ctx, long time);
char * build_command(char *file_name);

void mark_trigger(int context, long time) {
    if (context < 0 || context >= MAX_CONTEXT) {
        printf("error ctx %d\n", context);
        return;
    }
    pthread_mutex_lock(&trigger_lock);
    triggers[context] = time;
    pthread_mutex_unlock(&trigger_lock);
}

long get_trigger(int context) {
    if (context < 0 || context >= MAX_CONTEXT) {
        printf("error ctx %d\n", context);
        return 0;
    }
    long res;
    pthread_mutex_lock(&trigger_lock);
    res = triggers[context];
    pthread_mutex_unlock(&trigger_lock);
    return res;
}

void mark_file(int context, char *name) {
    if (context < 0 || context >= MAX_CONTEXT) {
        //printf("error ctx %d\n", context);
        return;
    }
    if (name == NULL || strlen(name) < 3 || strlen(name) > MAX_FILE_NAME) {
        //printf("error file name %d\n", name);
        return;
    }
    pthread_mutex_lock(&file_lock);
    strcpy(files[context], name);
    pthread_mutex_unlock(&file_lock);
}

void get_file(int context, char * result, int len) {
    if (context < 0 || context >= MAX_CONTEXT) {
        printf("error ctx %d\n", context);
        return;
    }
    if (result == NULL || len < MAX_FILE_NAME) {
        printf("error result len%d\n", len);
        return;
    }
    pthread_mutex_lock(&file_lock);
    strcpy(result, files[context]);
    pthread_mutex_unlock(&file_lock);
}

int is_digit(char c) {
    if (c >= '0' && c <= '9') return 1;
    return 0;
}

int get_context(char *name) {
    int res = -1;
    char ctx[MAX_FILE_NAME+1];
    // A01-003.mp4 -> context id 1
    // B02-002.mp4 -> context id 2
    char *p, *q;
    for (p = name; *p != 0 && !is_digit(*p); p++);
    if (*p == 0) return res;
    // p -> 1st digit
    for (q = p+1; *q != 0 && is_digit(*q); q++);
    // q -> after last digit
    int len = q-p;
    strncpy(ctx, p, len);
    ctx[len] = 0;
    sscanf(ctx, "%d", &res);
    //printf("get_context %s -> %d\n", name, res);
    return res;
}

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
    int ctx;

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
            char *name = p_event->name;
            ctx = get_context(name);
            if (ctx < 0) {
                continue;
            }
            if ( p_event->mask & IN_OPEN ) {
                //printf( "File %s opened.\n",  name);
                mark_file(ctx, name);
            } else if ( p_event->mask & IN_CREATE ) {
                //printf( "New file %s created.\n", name);
                mark_file(ctx, name);
            } else if ( p_event->mask & IN_CLOSE ) {
                printf( "File %s closed.\n", name);
                mark_file(ctx, name);
                long t = get_trigger(ctx);
                if (t) {
                    mark_trigger(ctx, 0);
                    // todo: need some queue struct and another upload thread
                    char * json = upload_file(name, ctx, t);
                    // todo: process result
                    printf("json: %s\n", json);
                    free(json);
                }
            } else if ( p_event->mask & IN_DELETE ) {
                printf( "File %s deleted.\n", p );
                mark_file(ctx, "");
            }
            p += sizeof(struct inotify_event) + p_event->len;
        }
    }
    inotify_rm_watch( fd, wd );
}

char * upload_file(char *name, int ctx, long trig_time) {
    FILE * fp = NULL;
    char * json = NULL;

    printf("====== upload file %s, ctx %d, trigger %ld\n", name, ctx, trig_time);
    char * file_name = build_file_name(name, ctx, trig_time);
    char * cmd = build_command(file_name);
    printf("====== upload command %s\n", cmd);

    do {
        fp = popen(cmd,"r");
        if ( fp == NULL) {
            printf("Unable to open process");
            break;
        }
        json = (char *) malloc(MAX_JSON_RESP);
        size_t ret_code = fread(json, 1, MAX_JSON_RESP, fp);
        if (ret_code <= 0) {
            printf("read error");
            break;
        }
        if (ret_code == MAX_JSON_RESP) {
            printf("need larger buffer");
        }
    } while (0);

    pclose(fp);
    free(cmd);
    free(file_name);
    return json;
}

char * build_file_name(char *name, int ctx, long time) {
    char * fname = malloc(100);
    sprintf(fname, "%s", name);
    return fname;
}

char * build_command(char *file_name) {
    char * cmd = (char *) malloc(strlen(CURL) + MAX_FILE_NAME);
    sprintf(cmd, "%s", file_name);
    return cmd;
}
