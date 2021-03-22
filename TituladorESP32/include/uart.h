#ifndef UART_H
#define UART_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include <stdint.h>

void rx_task(void *arg);
void iniciarUart(void);

#endif