/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "uart.h"
#include "electrodo.h"
#include "flash.h"
#include "sd.h"

void app_main(void)
{
    inicializarSD();
    iniciarUart();
    iniciarFlash();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(electrodo_task, "electrodo_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tareaEjemploPWM, "ejemploPWM", 1024*2, NULL, 5, NULL);
}


