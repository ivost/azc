//
// Created by ivo on 5/18/21.
//

#ifndef AZC_WATCH_H
#define AZC_WATCH_H

_Noreturn void *watchThread(void *ptr);

int64_t now_ms_mono();

int64_t now_ms_real();

int32_t now_sec_real();

char *upload_file_curl(const char *file_name);

char *upload_file_blob(const char *file_name);

char *build_file_name(const char *file_name);

char *build_blob_name(const char *file_name);

char *build_command(const char *file_name);

void set_trigger(int context, long time);

long get_trigger(int context);

void clear_trigger(int context);

#endif //AZC_WATCH_H
