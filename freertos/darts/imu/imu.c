#include "imu.h"
#include "osdefs.h"

#define IMU_UART /dev/uart1
static void imu_init();
static void imu_task(void *pvParameters);
static uint8_t imu_decode();

handle_t uart;
void imu_init()
{
    uint8_t recv = 0;
    uart = io_open("IMU_UART");
    uart_config(uart, 115200, 8, UART_STOP_1, UART_PARITY_NONE);
    uart_set_read_timeout(uart, 10*1000);
}

void imu_task(void *pvParameters)
{
    while (1);
}
uint8_t imu_decode()
{
    uint8_t *recv;
    while (1)
    {
        if(io_read(uart, &recv, 1))
        io_write(uart, &recv, 1);
    }
}
