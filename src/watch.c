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
pthread_mutex_t upload_lock = PTHREAD_MUTEX_INITIALIZER;;

// todo: token and config
const char *CURL = "curl --silent -X POST 'https://api.cloudflare.com/client/v4/accounts/22b892bb1c1f18e5abf75e8b3cccf34b/stream' -H 'X-Auth-Email: ivostoy@gmail.com' -H 'X-Auth-Key: 5aac03bca5f786b2cac6fd42b58a7603869fe' --form 'file=@/data/video/%s'";

long triggers[MAX_CONTEXT];

// trigger is 1:max
void set_trigger(int context, long t) {
    pthread_mutex_lock(&trigger_lock);
    if (triggers[context] == 0) {
        triggers[context] = t;
        printf("set trigger %ld, ctx %d\n", triggers[context], context);
    }
    pthread_mutex_unlock(&trigger_lock);
}

void clear_trigger(int context) {
    pthread_mutex_lock(&trigger_lock);
    triggers[context] = 0;
    printf("clear trigger %ld, ctx %d\n", triggers[context], context);
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

char * get_number(char *p, int *pn) {
    char *q;
    char tmp[10];
    if (p == NULL || pn == NULL) {
        return NULL;
    }
    *pn = 0;
    // q scans for the first digit
    for (q = p; *q != 0 && !is_digit(*q); q++);
    if (*q == 0) {
        // not found
        return NULL;
    }
    p = q;
    // scan all digits
    for (q = p; *q != 0 && is_digit(*q); q++);
    int len = q-p;
    if (len > 9) {
        return NULL;
    }
    strncpy(tmp, p, len);
    tmp[len] = 0;
    sscanf(tmp, "%d", pn);
    // q points to 1st non-digit or 0
    return q;
}
// file name is CCC_DDD-WWWxHHH-NNN.mp4
// e.g. 001_060-1920x1056-000.mp4
// where
// 001 is the context/camera idx,
// DDD - file duration in sec,
// WWW - width in pixels
// HHH - height in pixels
// NNN - sequential number 000-MMM
// return 0 on success, 1 on error
int parse_name(const char *name, int * p_ctx, int * p_duration, int *p_width, int *p_height) {

    char *p = (char *) name;
    p = get_number(p, p_ctx);
    if (p == NULL) {
        return 1;
    }
    p = get_number(p, p_duration);
    if (p == NULL) {
        return 2;
    }
    p = get_number(p, p_width);
    if (p == NULL) {
        return 3;
    }
    p = get_number(p, p_height);
    if (p == NULL) {
        return 4;
    }
    return 0;
}

int64_t now_ms_mono() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

int64_t now_ms_real() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

int32_t now_sec_real() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec;
}

void onFileChange(struct inotify_event *p_event) {
    int ctx;
    int duration;
    int width, height;
    long begin_time_mono;
    long begin_time_real;

    // assume end_time = current time = closing time
    long end_time_mono = now_ms_mono();
    long end_time_real = now_ms_real();

    if ((p_event->mask & IN_CLOSE_WRITE) == 0) {
        return;
    }
    const char *name = (const char *) p_event->name;
    // use file naming to deduct context and duration
    // duration is in sec
    // how to get actual duration?
    parse_name(name, &ctx, &duration, &width, &height);
    // ignore if no triggers
    long t = get_trigger(ctx);
    if (t == 0) {
        return;
    }
    clear_trigger(ctx);
    duration *= 1000;   // in ms
    begin_time_mono = end_time_mono - duration;
    begin_time_real = end_time_real - duration;
    printf("name %s, ctx %d, trigger %ld, begin %ld, end %ld\n", name, ctx, t, begin_time_mono, end_time_mono);
    // name 001_030-1920x1056-000.mp4, ctx 1, trigger 439307315, begin 439308497, end 439338497
    // adjust start time and duration if first trigger is before start time
    if (t <= begin_time_mono) {
        long delta = begin_time_mono - t + 200;
        printf("time adjust %ld\n", delta);
        begin_time_mono -= delta;
        begin_time_real -= delta;
        duration = (int) (end_time_mono - begin_time_mono);
        printf("ADJUSTED begin %ld, end %ld, duration %d\n", begin_time_mono, end_time_mono, duration);
    }
    char *json = upload_file_blob(name);
    JSON_Object *root_object;
    JSON_Value *root_value;
    root_value = json_parse_string(json);
    if (root_value == NULL) {
        printf("JSON PARSE ERROR %s\n", json);
        free(json);
        return;
    }
    free(json);
    root_object = json_value_get_object(root_value);
    const char *vid = json_object_dotget_string(root_object, "result.path");
    // fill video info
    struct video_info v;
    if (vid != NULL && strlen(vid) > 0) {
        v.height = height;
        v.width = width;
        v.ctx_id = ctx;
        v.begin_time = begin_time_mono;
        v.begin_time_real = begin_time_real;
        v.duration = duration;
        strcpy(v.path, vid);
        azc_send_video_info(&v);
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
    inotify_rm_watch(fd, wd);
}

// upload is not thread safe yet
char *upload_file_blob(const char *name) {
    pthread_mutex_lock(&upload_lock);
    char *file_name = build_file_name(name);
    char *blob_name = build_blob_name(name);
    //printf("file_name %s\n", file_name);
    char *p = azc_upload(file_name, blob_name);
    char *q = strdup(p);
    printf("upload result %s\n", q);
    free(file_name);
    free(blob_name);
    pthread_mutex_unlock(&upload_lock);
    free(p);
    return q;
}

char *upload_file_curl(const char *name) {
    FILE *fp = NULL;
    char *json = NULL;

    char *file_name = build_file_name(name);
    char *cmd = build_command(file_name);

    do {
        fp = popen(cmd, "r");
        if (fp == NULL) {
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

char *build_file_name(const char *name) {
    char fname[300];
    sprintf(fname, "%s%s", VIDEO_DIR, name);
    return strdup(fname);
}

char *build_blob_name(const char *name) {
    char fname[300];
    char *p = strrchr(name, '/');
    if (p == NULL) {
        strcpy(fname, name);
    } else {
        strcpy(fname, p + 1);
    }
    char *q = strrchr(fname, '.');
    if (q != NULL) {
        *q = 0;
    }
    sprintf(fname + strlen(fname), "-%ld", time(NULL));
    q = strrchr(name, '.');
    if (q != NULL) {
        strcat(fname, q);
    }
    printf("blob_name %s\n", fname);
    return strdup(fname);
}

char *build_command(const char *file_name) {
    char *cmd = (char *) malloc(strlen(CURL) + MAX_FILE_NAME);
    sprintf(cmd, CURL, file_name);
    return cmd;
}
