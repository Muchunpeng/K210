#include "servo.h"
#include "osdefs.h"

static void servo_init();
static void servo_task(void *pvParameters);

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

#define SERVO_CHANNEL 0

handle_t gio;
handle_t pwm0;

void servo_init()
{
    pwm0 = io_open("/dev/pwm0");
    configASSERT(pwm0);
    pwm_set_frequency(pwm0, 200000); //set 200KHZ
    pwm_set_active_duty_cycle_percentage(pwm0, SERVO_CHANNEL, 1); //duty 50%
    pwm_set_enable(pwm0, SERVO_CHANNEL, 1);

}

void servo_task(void *pvParameters)
{
    while (1);
}
