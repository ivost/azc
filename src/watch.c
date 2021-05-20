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
#include <parson.h>

#include "azc.h"
#include "watch.h"

#define VIDEO_DIR "/mnt/sdcard/video/"

#define MAX_CONTEXT 16
#define MAX_FILE_NAME 32
#define MAX_JSON_RESP 2048

#define EVENT_SIZE    ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

pthread_mutex_t trigger_lock = PTHREAD_MUTEX_INITIALIZER;;

// todo: token and config
const char * CURL= "curl --silent -X POST 'https://api.cloudflare.com/client/v4/accounts/22b892bb1c1f18e5abf75e8b3cccf34b/stream' -H 'X-Auth-Email: ivostoy@gmail.com' -H 'X-Auth-Key: 5aac03bca5f786b2cac6fd42b58a7603869fe' --form 'file=@/data/video/%s'";

long triggers[MAX_CONTEXT];

// trigger is 1:max
void set_trigger(int context, long t) {
    pthread_mutex_lock(&trigger_lock);
    if (triggers[context] == 0) {
        triggers[context] = t;
        //printf("set trigger %ld, ctx %d\n", triggers[context], context);
    }
    pthread_mutex_unlock(&trigger_lock);
}

void clear_trigger(int context) {
    pthread_mutex_lock(&trigger_lock);
    triggers[context] = 0;
    pthread_mutex_unlock(&trigger_lock);
}

// trigger is 0:max-1
long get_trigger(int context) {
    pthread_mutex_lock(&trigger_lock);
    long t = triggers[context];
    pthread_mutex_unlock(&trigger_lock);
    return t;
}

int is_digit(char c) {
    if (c >= '0' && c <= '9') return 1;
    return 0;
}

// returns context 1:max
int get_context(const char *name) {
    int res = -1;
    char ctx[MAX_FILE_NAME+1];
    char *p, *q;
    for (p = (char *) name; *p != 0 && !is_digit(*p); p++);
    if (*p == 0) {
        return res;
    }
    // p -> 1st digit
    for (q = p+1; *q != 0 && is_digit(*q); q++);
    // q -> after last digit
    int len = q-p;
    strncpy(ctx, p, len);
    ctx[len] = 0;
    sscanf(ctx, "%d", &res);
    return res;
}

void onFileChange(struct inotify_event *p_event) {
    if ((p_event->mask & IN_CLOSE_WRITE) == 0) {
        return;
    }
    const char *name = (const char *) p_event->name;
    int ctx = get_context(name);
    long t = get_trigger(ctx);
    if (t == 0) {
        return;
    }
    //printf("*** ctx %d File %s closed, trigger %ld\n", ctx, name, t);
    clear_trigger(ctx);
    // todo: need some queue struct and another upload thread
    char *json = upload_file(name, ctx, t);
    JSON_Object *root_object;
    JSON_Value *root_value;
    root_value = json_parse_string(json);
    if (root_value == NULL) {
        printf("JSON PARSE ERROR %s\n", json);
        return;
    }
    root_object = json_value_get_object(root_value);
    const char *vid = json_object_dotget_string(root_object, "result.uid");
    // send video uuid
    if (vid != NULL && strlen(vid) > 0) {
        //printf("VIDEO UUID %s\n", vid);
        azc_send_video_id(ctx, t, vid);
    }
    json_value_free(root_value);
}

_Noreturn
void *watchThread(void *ptr) {
    (void) ptr;
    int fd, wd;
    int numRead;
    char buf[EVENT_BUF_LEN];
    struct inotify_event *p_event;
    fd = inotify_init();
    wd = inotify_add_watch(fd, VIDEO_DIR, IN_CLOSE_WRITE);
    if (fd == -1 || wd == -1) {
        printf("inotify error dir %s\n", VIDEO_DIR);
    }
    for (;;) {
        usleep(1000);
        numRead = read(fd, buf, EVENT_BUF_LEN);
        for (char *p = buf; p < buf + numRead;) {
            p_event = (struct inotify_event *) p;
            onFileChange(p_event);
            p += sizeof(struct inotify_event) + p_event->len;
        }
    }
    inotify_rm_watch( fd, wd );
}

char * upload_file(const char *name, int ctx, long trig_time) {
    FILE * fp = NULL;
    char * json = NULL;

    printf("upload file %s, ctx %d, trigger %ld\n", name, ctx, trig_time);
    char * file_name = build_file_name(name, ctx, trig_time);
    char * cmd = build_command(file_name);
    //printf("====== upload command %s\n", cmd);

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

char * build_file_name(const char *name, int ctx, long time) {
    char * fname = malloc(100);
    sprintf(fname, "%s", name);
    return fname;
}

char * build_command(const char *file_name) {
    char * cmd = (char *) malloc(strlen(CURL) + MAX_FILE_NAME);
    sprintf(cmd, CURL, file_name);
    return cmd;
}
