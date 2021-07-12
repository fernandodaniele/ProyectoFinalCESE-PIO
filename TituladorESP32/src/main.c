/* 
Sistema embebido para titulador potenciometrico autómatico
Desarrollado por Ing. Fernando Ezequiel Daniele como proyecto final de la Especialización en Sistemas Embebidos de la FIUBA
Este sistema se enmarca dentro del proyecto de investigación y desarrollo de un Titulador automatico llevado a cabo por el grupo GISAI de UTN San Francisco
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
    iniciarFlash();
    iniciarUart();
    xTaskCreatePinnedToCore (tareaUart, "tareaUart", 1024*2, NULL, configMAX_PRIORITIES, NULL, 1);
    xTaskCreatePinnedToCore (tareaElectrodo, "tareaElectrodo", 1024*2, NULL, configMAX_PRIORITIES, NULL, 0);
    xTaskCreatePinnedToCore (tareaBomba, "ejemploPWM", 1024*2, NULL, configMAX_PRIORITIES-1, NULL, 0);
}