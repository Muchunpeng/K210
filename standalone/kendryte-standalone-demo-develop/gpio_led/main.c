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
#include <unistd.h>

#include "fpioa.h"
#include "gpio.h"

int main(void) {
  /* 将IO24设置为普通IO3 */
  fpioa_set_function(24, FUNC_GPIO3);
  /* 初始化IO */
  gpio_init();
  /* 设置GPIO3为输出模式 */
  gpio_set_drive_mode(3, GPIO_DM_OUTPUT);
  /* 设置GPIO3为高电平 */
  gpio_pin_value_t value = GPIO_PV_HIGH;
  gpio_set_pin(3, value);
  while (1) {
    sleep(1);
    /* 电平反转 */
    gpio_set_pin(3, value = !value);
  }
  return 0;
}
