//
// Created by ivo on 5/10/21.
//

#ifndef GST_AZC_H
#define GST_AZC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// bounding box
struct bbox {
    uint64_t time;
    uint64_t id;
    uint64_t idx;       // class index
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    float confidence;
    float scale_x;
    float scale_y;
};

int azc_init();
int azc_send(size_t num_boxes, struct bbox * boxes);
int azc_reset();


#ifdef __cplusplus
}
#endif

#endif // GST_AZC_H
