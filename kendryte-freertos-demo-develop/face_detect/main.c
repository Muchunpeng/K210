/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <devices.h>
#include "dvp_camera.h"
#include "lcd.h"
#include "ov5640.h"
#include "region_layer.h"
#include "image_process.h"
#include "w25qxx.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"
#include "iomem.h"
#include <time.h>
#include <sys/time.h>
#include <printf.h>
#include <string.h>

image_t kpu_image, display_image0, display_image1;
handle_t model_context;
camera_context_t camera_ctx;
static region_layer_t face_detect_rl;
static obj_info_t face_detect_info;
#define ANCHOR_NUM 5
static float anchor[ANCHOR_NUM * 2] = {1.889,2.5245,  2.9465,3.94056, 3.99987,5.3658, 5.155437,6.92275, 6.718375,9.01025};

#define LOAD_KMODEL_FROM_FLASH 1

#if LOAD_KMODEL_FROM_FLASH
handle_t spi3;
#define KMODEL_SIZE (380 * 1024)
uint8_t *model_data;
#else
INCBIN(model, "../src/face_detect/detect.kmodel");
#endif
static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    volatile uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;

    if (x1 <= 0)
        x1 = 1;
    if (x2 >= 319)
        x2 = 318;
    if (y1 <= 0)
        y1 = 1;
    if (y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 8) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    addr4 = gram + (320 * (y2 - 1) + x2 - 8) / 2;
    for (uint32_t i = 0; i < 4; i++)
    {
        *addr1 = data;
        *(addr1 + 160) = data;
        *addr2 = data;
        *(addr2 + 160) = data;
        *addr3 = data;
        *(addr3 + 160) = data;
        *addr4 = data;
        *(addr4 + 160) = data;
        addr1++;
        addr2++;
        addr3++;
        addr4++;
    }
    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    addr3 = gram + (320 * (y2 - 8) + x1) / 2;
    addr4 = gram + (320 * (y2 - 8) + x2 - 2) / 2;
    for (uint32_t i = 0; i < 8; i++)
    {
        *addr1 = data;
        *addr2 = data;
        *addr3 = data;
        *addr4 = data;
        addr1 += 160;
        addr2 += 160;
        addr3 += 160;
        addr4 += 160;
    }
}

void detect()
{
    int time_count = 0;
    struct timeval get_time[2];

    gettimeofday(&get_time[0], NULL);

    while (1) {
        while (camera_ctx.dvp_finish_flag == 0)
            ;
        camera_ctx.dvp_finish_flag = 0;
        uint32_t *lcd_gram = camera_ctx.gram_mux ? (uint32_t *)camera_ctx.lcd_image1->addr : (uint32_t *)camera_ctx.lcd_image0->addr;

        if (kpu_run(model_context, camera_ctx.ai_image->addr) != 0) {
            printf("Cannot run kmodel.\n");
            exit(-1);
        }

        float *output;
        size_t output_size;
        kpu_get_output(model_context, 0, (uint8_t **)&output, &output_size);

        face_detect_rl.input = output;
        region_layer_run(&face_detect_rl, &face_detect_info);
        for (uint32_t face_cnt = 0; face_cnt < face_detect_info.obj_number; face_cnt++) {
            draw_edge(lcd_gram, &face_detect_info, face_cnt, RED);
        }

        lcd_draw_picture(0, 0, 320, 240, lcd_gram);
        camera_ctx.gram_mux ^= 0x01;
        time_count ++;
        if(time_count == 100)
        {
            gettimeofday(&get_time[1], NULL);;
            printf("SPF:%fms Byte\n", ((get_time[1].tv_sec - get_time[0].tv_sec)*1000*1000 + (get_time[1].tv_usec - get_time[0].tv_usec))/1000.0/100);
            memcpy(&get_time[0], &get_time[1], sizeof(struct timeval));
            time_count = 0;
        }
    }
    io_close(model_context);
    image_deinit(&kpu_image);
    image_deinit(&display_image0);
    image_deinit(&display_image1);
}
int main(void)
{
    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 240;
    image_init(&kpu_image);
    
    display_image0.pixel = 2;
    display_image0.width = 320;
    display_image0.height = 240;
    image_init(&display_image0);

    display_image1.pixel = 2;
    display_image1.width = 320;
    display_image1.height = 240;
    image_init(&display_image1);

    camera_ctx.dvp_finish_flag = 0;
    camera_ctx.ai_image = &kpu_image;
    camera_ctx.lcd_image0 = &display_image0;
    camera_ctx.lcd_image1 = &display_image1;
    camera_ctx.gram_mux = 0;

    face_detect_rl.anchor_number = ANCHOR_NUM;
    face_detect_rl.anchor = anchor;
    face_detect_rl.threshold = 0.7;
    face_detect_rl.nms_value = 0.3;
    region_layer_init(&face_detect_rl, 20, 15, 30, camera_ctx.ai_image->width, camera_ctx.ai_image->height);

    printf("lcd init\n");
    lcd_init();
    printf("DVP init\n");
    dvp_init(&camera_ctx);
    ov5640_init();

#if LOAD_KMODEL_FROM_FLASH
    model_data = (uint8_t *)malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t *)(((uintptr_t)model_data+255)&(~255));
    spi3 = io_open("/dev/spi3");
    configASSERT(spi3);
    w25qxx_init(spi3);
    w25qxx_read_data(0xA00000, model_data_align, KMODEL_SIZE);
#else
    uint8_t *model_data_align = model_data;
#endif
    model_context = kpu_model_load_from_buffer(model_data_align);
    xTaskCreate(detect, "detect", 2048*2, NULL, 3, NULL);
    vTaskDelete(NULL);
}
