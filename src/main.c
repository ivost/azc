
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <parson.h>

#include "msgq.h"
#include "watch.h"

int parse_name(const char *name, int * p_ctx, int * p_duration);
void test();

int main(int argc, char *argv[]) {
/*
    test();
*/
    pthread_t thread1;
    pthread_t thread2;
    printf("azc v.1.5.22.3 enter\n");
    pthread_create(&thread1, NULL, msgRecvThread, NULL);
    pthread_create(&thread2, NULL, watchThread, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}

void test() {
    const char name[] = "A99-123-004.mp4";
    int ctx;
    int dur;
    parse_name(name, &ctx, &dur);
    printf("name %s, ctx %d, dur %d\n", name, ctx, dur);


//    JSON_Object * root_object;
//    JSON_Value  * root_value;
//    const char * json = "{\"result\": {\"uid\": \"f6cfa8416d2e4132b8d8d85914df51c6\"}}";
//    root_value = json_parse_string(json);
//    if (root_value == NULL) {
//        printf("JSON PARSE ERROR\n");
//        return;
//    }
//    root_object = json_value_get_object(root_value);
//    const char * vid = json_object_dotget_string(root_object, "result.uid");
//    char *v = strdup(vid);
//    json_value_free(root_value);
//    printf("VIDEO UUID %s\n", v);
//
//    JSON_Value *rootv = json_value_init_object();
//    JSON_Object *root = json_value_get_object(rootv);
//    json_object_set_string(root, "vid", v);
//    char * msg = json_serialize_to_string(rootv);
//    free(v);
//    printf("msg %s\n", msg);
}