
#include <stdlib.h>
#include <time.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azc.h"

int main(int argc, char *argv[]) {
    int rc;
    struct objdet_result r;
    struct bbox * pbt;
    azc_init();

    for (int i=0; i<5; i++) {
        r.ctx_id = 100 + i*10;
        r.time = time(NULL);
        r.numbb = i+1;
        r.bb = pbt = calloc(r.numbb, sizeof(struct bbox));
        for (int i=0; i<r.numbb; i++) {
            pbt->x = 1 + i;
            pbt->y = 2 + i;
            pbt->width = 10 + 20*i;
            pbt->height = 20 + 40*i;
            pbt->conf = 5 + i;
            pbt->cat = 1;
            pbt++;
        }
        rc = azc_send_result(&r);
        (void) rc;
        //ThreadAPI_Sleep(100);
        free(r.bb);
    }

    ThreadAPI_Sleep(1000);

    azc_reset();
}


