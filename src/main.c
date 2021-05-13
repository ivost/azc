
#include <stdlib.h>
#include "azc.h"
#include "azure_c_shared_utility/threadapi.h"

int main(int argc, char *argv[]) {
    azc_init();

    size_t num_boxes = 10;

    struct bbox * pb = calloc(num_boxes, sizeof(struct bbox));
    struct bbox * pbt = pb;
    for (int i=0; i<num_boxes; i++) {
        pbt->x = 1 + 10*i;
        pbt->y = 2 + 20*i;
        pbt->confidence = 0.5 + i/2.;
        pbt++;
    }

    azc_send(num_boxes, pb);

//    for (int i=0; i<10; i++) {
//        azc_send();
//        ThreadAPI_Sleep(1000);
//    }

    azc_reset();
}
