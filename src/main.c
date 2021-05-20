
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <time.h>
#include <pthread.h>
#include <parson.h>
//#include "azc.h"

#include "hub.h"
#include "upload.h"

char * echo (char *s);

void test();

int main(int argc, char *argv[]) {
//    test();

    pthread_t thread1, thread2;
    printf("azc v.1.5.19.3 enter\n");
    //printf("c++ called with 'foo' - result %s\n", echo("foo"));
    pthread_create( &thread1, NULL, hub_thread, NULL);
    pthread_create( &thread2, NULL, upload_thread, NULL);
    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);
    printf("azc exit");
    return 0;
}

void test() {
    JSON_Object * root_object;
    JSON_Value  * root_value;
    const char * json = "{\"result\": {\"uid\": \"f6cfa8416d2e4132b8d8d85914df51c6\"}}";
    root_value = json_parse_string(json);
    if (root_value == NULL) {
        printf("JSON PARSE ERROR\n");
        return;
    }
    root_object = json_value_get_object(root_value);
    const char * vid = json_object_dotget_string(root_object, "result.uid");
    char *v = strdup(vid);
    json_value_free(root_value);
    printf("VIDEO UUID %s\n", v);

    JSON_Value *rootv = json_value_init_object();
    JSON_Object *root = json_value_get_object(rootv);
    json_object_set_string(root, "vid", v);
    char * msg = json_serialize_to_string(rootv);
    free(v);
    printf("msg %s\n", msg);
}