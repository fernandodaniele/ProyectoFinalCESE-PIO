#include "electrodo.h"
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include <stdio.h>
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "flash.h"
#include "uart.h"
#include "sd.h"

#define DUTY_US     50
#define PWM_FREQ    10000
#define PIN_DIR     27
#define PIN_PASO    12

void rectaRegresion();

int valorAdc;
int valorBufferA;
int valorBufferB;
int valorBufferC;

float m,b; //pendiente y ordenada de la recta de regresion

int motorBomba = 0;
int procesoTitulacion =0; //variable para inciar/ finalizar el proceso de titulacion
int lecturaTitulacion [1000];
float titulacionPH [1000];
float derivada1 [1000];
float derivada2 [1000];

int volumenActual = 0;
extern int16_t volumenCorte;
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

void electrodo_task (void *arg)
{
    //Configuración
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11);
    int sumaAdc = 0;
    //Bucle infinito
    while(1)
    {
        taskENTER_CRITICAL(&myMutex);
        for (int i = 0; i < 50; i++)
        {
            sumaAdc = sumaAdc + adc1_get_raw(ADC1_CHANNEL_6);
            //vTaskDelay(1/portTICK_PERIOD_MS);
        }
        taskEXIT_CRITICAL(&myMutex);
        valorAdc = sumaAdc / 50;
        sumaAdc = 0;
        
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void tareaEjemploPWM(void *arg)
{
    gpio_pad_select_gpio(PIN_DIR);
    gpio_set_direction(PIN_DIR, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_DIR, 0);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_PASO);    //Set GPIO PIN_PASO as PWM0A, to which servo is connected

    mcpwm_config_t pwm_config;
    pwm_config.frequency = PWM_FREQ;
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    while (1) 
    {
       if(procesoTitulacion == 1)
        {
            escribeSD("Nueva titulación\n");
            for(int volumenActual =1 ; volumenActual <volumenCorte+1;volumenActual++)
            {
                mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, DUTY_US);
                vTaskDelay(1150 /portTICK_PERIOD_MS); //con 13333, 10 K y duty 50 inyecta 1 mL, probar si con 1333 inyecta 0,1
                mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
                vTaskDelay(100 /portTICK_PERIOD_MS); //para que el valor se estabilice
                lecturaTitulacion[volumenActual] = valorAdc;
                ESP_LOGI("Recta de regresion", "%d", volumenActual);
            }
            //convertir valores de v a Ph
            rectaRegresion();
            for(int vol =1; vol <volumenCorte+1; vol++)
            {
                titulacionPH [vol] = m * lecturaTitulacion[vol] + b;
                ESP_LOGI("Valor PH", "%f pH", titulacionPH [vol]);
            }
            //calcular primera derivada
            for(int vol =2; vol < (volumenCorte); vol++)
            {
                derivada1 [vol] = (titulacionPH[vol+1]-titulacionPH[vol-1])/((vol+1)-(vol-1));
            }
            //calcular segunda derivada
            for(int vol =3; vol < (volumenCorte-1); vol++)
            {
                derivada2 [vol] = (derivada1[vol+1]-derivada1[vol-1])/((vol+1)-(vol-1));
            }
            //ver cual es el volumen para el cual la derivada segunda se hace 0
            //guardar todos lo valores en la SD
            escribeSD("Volumen[mL]\tpH\t\tDerivada 1\t\tDerivada 2\n");
            float volumen;
            for(int vol = 1; vol < (volumenCorte+1); vol++)
            {
                volumen = (float) vol / 10;
                escribeSDFloat(volumen);
                escribeSD("\t\t");
                escribeSDFloat(titulacionPH[vol]);
                escribeSD("\t\t");
                if((vol<2)||(vol>(volumenCorte-2)))
                {
                    escribeSD("Sin dato\t\tSin dato");
                }
                else
                {
                    escribeSDFloat(derivada1 [vol]);
                    escribeSD("\t\t");
                    escribeSDFloat(derivada2 [vol]);
                }
                escribeSD("\n");
            }
            procesoTitulacion = 0;

        }
        else if(motorBomba == 1) //para la limpieza o para purgar
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, DUTY_US);

        }
        else
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0); //se detiene bomba
        }
        
        vTaskDelay(100 /portTICK_PERIOD_MS);
    }
}




void rectaRegresion()
{
    
    //Declaracion de variables 
    int n=3,i;
    float sumax,sumay,sumaxy,sumax2;
    int16_t x[3];
    float y[3];

    y[0] = 4; //ph
    y[1] = 7;
    y[2] = 10;

    //Lectura de los valores calibrados
    if(leerFlash("CALIBREA", &x[0]))
    {
    }

    if(leerFlash("CALIBREB", &x[1]))
    {
    }

    if(leerFlash("CALIBREC", &x[2]))
    {
    }

    //Sumatorias
    sumax=sumay=sumaxy=sumax2=0;
    for (i=0;i<3;i++)
    {
        sumaxy += x[i]*y[i];
        sumax2 += x[i]*x[i];
        sumax += x[i];
        sumay += y[i];
    }
    /* Calculo de la pendiente (m) y la interseccion (b)*/
    m = (n*sumaxy - sumax*sumay) / (n*sumax2 - sumax*sumax);
    b = (sumay - m*sumax) / n;
}