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


#define INICIO_CAL          'A' 		
#define FIN_CAL    		    'B'
#define LECTURA_POTENCIAL   'C'
#define GRABA_BUFFER4       'D'
#define GRABA_BUFFER7       'E'
#define GRABA_BUFFER10      'F'
#define INICIO_TIT          'G'
#define LECTURA_PH          'H'
#define CANCELA_TIT         'I'
#define FIN_TIT             "J"
#define INICIO_LIMPIEZA     'K'
#define FIN_LIMPIEZA        'L'
#define LEE_VOLUMEN         'M'
#define GUARDA_VOLUMEN      'N'
#define ESTADO_AGITADOR     'O'
#define HABILITA_AGIT	    'P'
#define DESHABILITA_AGIT    'Q'

void tareaUart(void *arg);
void iniciarUart(void);
int enviarPorUart(const char* data);

#endif