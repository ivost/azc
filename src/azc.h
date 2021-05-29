//
// Created by ivo on 5/10/21.
//

#ifndef GST_AZC_H
#define GST_AZC_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FIELDS (100)
#define MAX_BB (100)
#define MAX_PATH (300)

// context
struct cam_context {
    uint32_t ctx_id;
    uint16_t cam;
    uint16_t width;
    uint16_t height;
    uint16_t model;
    // scale is multiplied by 1000 and rounded to 4 digits precision
    float scale_x;
    float scale_y;
    // bbox field list - e.g. "x,y,w,h,conf,cat"
    char fields[MAX_FIELDS];
};

//Id        int64    `db:"id" json:"id"`
//Vid       string   `db:"vid" json:"vid"`
//CtxId     int16    `db:"ctx_id" json:"cid"`
//BeginTime int64    `db:"start_time" json:"s"`
//EndTime   int64    `db:"end_time" json:"t"`
//Width     int16    `db:"width" json:"w"`
//Height    int16    `db:"height" json:"h"`
//Duration  int16    `db:"duration" json:"d"`

// video clip
struct video_info {
    uint64_t begin_time;        // monotonic time in ms
    uint64_t begin_time_real;   // real time in ms
    uint32_t duration;          // in ms
    uint32_t ctx_id;
    uint16_t width;
    uint16_t height;
    // blob path, i.e. https://edgestorage01.blob.core.windows.net/video/C610/A01-001-1622319571.mp4"
    char path[MAX_PATH];
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
    BB bb[MAX_BB];
};

int azc_init();

int azc_send_context(struct cam_context *ctx);

int azc_send_result(struct objdet_result *res);

int azc_send_video_info(struct video_info *info);

int azc_reset();

char *azc_upload(const char *file_name, const char *blob_path);

#ifdef __cplusplus
}
#endif


#endif // GST_AZC_H
