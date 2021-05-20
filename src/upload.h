//
// Created by ivo on 5/18/21.
//

#ifndef AZC_UPLOAD_H
#define AZC_UPLOAD_H

void * upload_thread(void *ptr);

char * upload_file(const char *p, int ctx, long time);
char * build_file_name(const char *p, int ctx, long time);
char * build_command(const char *file_name);

char * upload_file(const char *name, int ctx, long trig_time);

void set_trigger(int context, long time);
long get_trigger(int context);
void clear_trigger(int context);

#endif //AZC_UPLOAD_H
