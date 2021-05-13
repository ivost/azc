
#include <stdlib.h>
#include <time.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azc.h"

int main(int argc, char *argv[]) {
    int rc;
    struct cam_context ctx;
    struct objdet_result r;
    struct bbox * pbt;

    azc_init();

    ctx.ctx_id = 100;
    ctx.cam = 2;
    ctx.model = 3;
    ctx.width = 640;
    ctx.height = 480;
    ctx.scale_x = 0.521;
    ctx.scale_y = 0.4333;
    strcpy(ctx.fields, "x,y,w,h,conf,cat");

    rc = azc_send_context(&ctx);
    (void) rc;

    r.ctx_id = ctx.ctx_id;

    for (int i=0; i<2; i++) {
        r.time = time(NULL);
        r.numbb = i+1;
        r.bb = pbt = calloc(r.numbb, sizeof(struct bbox));
        for (int i=0; i<r.numbb; i++) {
            pbt->x = 1 + i;
            pbt->y = 2 + i;
            pbt->width = 10 + 20*i;
            pbt->height = 20 + 40*i;
            pbt->conf = 0.543666666;
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


