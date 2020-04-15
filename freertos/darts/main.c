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
#include <devices.h>
#include <stdio.h>
#include <string.h>
#include "project_cfg.h"
#include "task.h"
#include "imu.h"

static void start_task(void *pvParameters);
static void bsp_init();

#define START_TASK_PRIO		1
#define START_STK_SIZE 		128
TaskHandle_t StartTask_Handler;


#define IMU_TASK_PRIO		3
#define IMU_STK_SIZE 		128
TaskHandle_t IMU_Handler;

#define VISUAL_TASK_PRIO		3
#define VISUAL_STK_SIZE 		128
TaskHandle_t VISUAL_Handler;
void visual_task(void *pvParameters);

#define SERVO_TASK_PRIO		4
#define SERVO_STK_SIZE 		128
TaskHandle_t SERVO_Handler;
void servo_task(void *pvParameters);


void bsp_init()
{
    imu_init();
    servo_init();
    visual();
}
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建陀螺仪任务
    xTaskCreate((TaskFunction_t )imu_task,
                (const char*    )"imu_task",
                (uint16_t       )IMU_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )IMU_TASK_PRIO,
                (TaskHandle_t*  )&IMU_Handler);
    //创建视觉处理任务
    xTaskCreate((TaskFunction_t )visual_task,
                (const char*    )"visual_task",
                (uint16_t       )VISUAL_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )VISUAL_TASK_PRIO,
                (TaskHandle_t*  )&VISUAL_Handler);
    //创建电机控制任务
    xTaskCreate((TaskFunction_t )servo_task,
                (const char*    )"servo_task",
                (uint16_t       )SERVO_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )SERVO_TASK_PRIO,
                (TaskHandle_t*  )&SERVO_Handler);
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区

}
int main()
{
    bsp_init();
    //创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄
    vTaskStartScheduler();          //开启任务调度
    while (1);
}
