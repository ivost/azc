//
// Created by ivo on 5/10/21.
//

#ifndef GST_AZC_H
#define GST_AZC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FIELDS (100)
#define MAX_BB (100)

// context
struct cam_context {
    uint32_t ctx_id;
    uint16_t cam;
    uint16_t width;
    uint16_t height;
    uint16_t model;
    // scale is multiplied by 10000 and rounded to 4 digits precision
    float scale_x;
    float scale_y;
    // bbox field list - e.g. "x,y,w,h,conf,cat"
    char  fields[MAX_FIELDS];
};

// bounding box
// 16 bytes
struct bbox {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t cat;       // class index
    uint16_t res0;
    float    conf;
};

typedef struct bbox BB;

// object detection result
// 16 bytes + nbb*16 - max 336 b
struct objdet_result {
    uint64_t time;
    uint32_t ctx_id;
    uint16_t numbb;
    uint16_t res1;
    BB       bb[MAX_BB];
};

int azc_init();

int azc_send_context(struct cam_context * ctx);
int azc_send_result(struct objdet_result * res);
int azc_send_video_id(int ctx_id, long trig_time, const char * uuid);

int azc_reset();


#ifdef __cplusplus
}
#endif


#endif // GST_AZC_H
