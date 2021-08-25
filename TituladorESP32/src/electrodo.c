//============================ Inclusiones ===================================
#include <stdio.h>
#include <math.h>
#include "electrodo.h"
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "flash.h"
#include "uart.h"
#include "sd.h"

//============================ Definiciones ===================================
#define DUTY_US     50
#define PWM_FREQ    10000
#define PIN_DIR     27
#define PIN_PASO    12
#define UMBRAL_PH   40 //0.2 pH - diferencia en el valor del ADC por arriba del cual se acelera el proceso de medicion
#define N_MUESTRAS  100 //Muestras que toma el ADC para promediar
#define T_MUESTRAS  10  //tiempo entre muestras
#define T_INYEC_CORTO 1150 //1150 para 0,1 mL
#define T_INYEC_LARGO 11500 //11500 para 1 mL
#define T_ESPERA    5000 //Tiempo que espera para que estabilice el pH

//============================== Variables ===================================
float valorAdc;
int16_t valorBuffer[3] = {3466,2844,2271};
int motorBomba = 0;
int procesoTitulacion =0; //variable para iniciar/ finalizar el proceso de titulacion
int procesoCalibracion = 0;
int lecturaTitulacion [1000];
int volumenInyectado [1000];
float titulacionPH [1000];
float derivada1 [1000];
float derivada2 [1000];
float volumenFinal = 0;
uint16_t cont = 0;

int volumenActual = 0;
extern int16_t volumenCorte;
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
//============================ Funciones ===================================

void tareaElectrodo (void *arg)
{
    //Configuración
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11);
    uint32_t sumaAdc = 0;
    //Bucle infinito
    while(true)
    {
    for (int i=0; i< N_MUESTRAS; i++)
    {
        sumaAdc = sumaAdc + adc1_get_raw(ADC1_CHANNEL_6);  
        vTaskDelay(T_MUESTRAS/portTICK_PERIOD_MS);
    } 
    portENTER_CRITICAL(&myMutex); 
    valorAdc = sumaAdc / N_MUESTRAS;
    portEXIT_CRITICAL(&myMutex);
    sumaAdc = 0;
    ESP_LOGI("Valor promedio ADC: ", "%f", valorAdc);
    }
}

void tareaBomba(void *arg)
{
    //Configuración
    //int min=0;
    int max=0;
    float m=-0.005,b=21.3;

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
    //Bucle infinito
    while (true) 
    {
       if(procesoTitulacion == 1)
        {
            escribeSD("Nueva titulación\n");
            uint16_t tiempoInyectado = T_INYEC_CORTO;
            cont = 0;
            uint16_t jota = 0;
            int dif=0;

            for(int volumenActual =1 ; volumenActual <volumenCorte+1;volumenActual++)
            {
                mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, DUTY_US);
                vTaskDelay(tiempoInyectado /portTICK_PERIOD_MS); //con 13333, 10 K y duty 50 inyecta 1 mL, probar si con 1333 inyecta 0,1
                mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
                vTaskDelay(T_ESPERA /portTICK_PERIOD_MS); //para que el valor se estabilice
                portENTER_CRITICAL(&myMutex); 
                lecturaTitulacion[cont] = valorAdc;
                volumenInyectado[cont] = volumenActual;
                portEXIT_CRITICAL(&myMutex);
                if(cont >0)
                {
                    dif = (abs(lecturaTitulacion[cont]-lecturaTitulacion[cont-1]));
                }                
                if(dif < UMBRAL_PH)
                {
                    if(tiempoInyectado == T_INYEC_CORTO)
                    {
                        jota++;
                        if( jota == 10)
                        {
                            tiempoInyectado = T_INYEC_LARGO;//acá podría cambiar la velocidad de la bomba
                        }
                    }
                    else{
                        volumenActual += 9; //el volumen aumenta de 10 en diez
                    }
                }
                else
                {
                    if(tiempoInyectado == T_INYEC_CORTO)
                    {
                        volumenActual += 9;
                    }
                    tiempoInyectado = T_INYEC_CORTO;
                    jota = 0;
                }
                ESP_LOGI("Volumen actual", "%d", volumenActual);
                if(procesoTitulacion == 0){
                    escribeSD("Titulación cancelada\n");
                    break;
                }
                cont++;
            }
            //aviso que finalizó la titulación para cambiar de pantalla
            enviarPorUart(FIN_TIT);
            int64_t mProv, bProv;
            leerFlash ("M", &mProv);
            leerFlash ("B", &bProv);
            m = mProv/1000000.0;
            b = bProv/1000000.0;
            //convertir valores de v a Ph
            for(int vol =0; vol <cont; vol++)
            {
                titulacionPH [vol] = m * lecturaTitulacion[vol] + b;
                ESP_LOGI("Valor PH", "%f pH", titulacionPH [vol]);
            }
            //calcular primera derivada
            for(int vol =1; vol < cont-1; vol++)
            {
                derivada1 [vol] = (titulacionPH[vol+1]-titulacionPH[vol-1])/(volumenInyectado[vol+1]-volumenInyectado[vol-1]);
                if (vol == 3 || max < fabs(derivada1 [vol])){
                    max = fabs(derivada1 [vol]);                  //verifico cual es el valor para el cual el volumen se hace 0
                    volumenFinal = volumenInyectado[vol]/10.0;
                }
            }
            //calcular segunda derivada
            for(int vol =2; vol < (cont-2); vol++)
            {
                derivada2 [vol] = (derivada1[vol+1]-derivada1[vol-1])/(volumenInyectado[vol+1]-volumenInyectado[vol-1]);
                ESP_LOGI("Volumen inyectado", "%d pH", volumenInyectado [vol]);
                ESP_LOGI("Derivada segunda", "%f pH", derivada2 [vol]);
               /* if (vol == 3 || min > derivada2 [vol]){
                    min = derivada2 [vol];                  //verifico cual es el valor para el cual el volumen se hace 0
                    volumenFinal = volumenInyectado[vol]/10.0;
                }*/
            }
            //ver que pasa si tengo dos volumen final

            ESP_LOGI("Volumen en punto de equivalencia:", "%f mL", volumenFinal);
            
            //Guarda el resultado en la SD (faltaría en WiFi)
            escribeSD("Volumen en punto de equivalencia = ");
            escribeSDFloat(volumenFinal);
            escribeSD("\n");

            //guarda todos lo valores en la SD -- Esto no haría falta
            escribeSD("Volumen[mL]\tpH\t\tDerivada 1\t\tDerivada 2\n");
            
            for(int vol = 1; vol < (cont); vol++)
            {
                escribeSDFloat(volumenInyectado[vol]/10.0);
                escribeSD("\t\t");
                escribeSDFloat(titulacionPH[vol]);
                escribeSD("\t\t");
                if((vol<2)||(vol>(cont-2)))
                {
                    escribeSD("Sin dato\t\tSin dato");
                }
                else
                {
                    escribeSDFloat(derivada1 [vol]);
                    escribeSD("\t\t\t");
                    escribeSDFloat(derivada2 [vol]);
                }
                escribeSD("\n");  
            }
            escribeSD("Fin titulación\n\n");
            //hasta acá borrar
            procesoTitulacion = 0;
        }
        else if(motorBomba == 1) //para la limpieza o para purgar
        {
            //esto debería hacerlo la primera vez o durante un tiempo
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, DUTY_US);
        }
        else
        {
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0); //se detiene bomba
        }
        
        vTaskDelay(1000 /portTICK_PERIOD_MS);
    }
}

void rectaRegresion()
{
    //Declaracion de variables 
    int n=3,i;
    float sumax,sumay,sumaxy,sumax2;
    float y[3];
    float m=-0.005,b=21.3; //pendiente y ordenada de la recta de regresion
    ESP_LOGI("m", "entre a la recta");
    y[0] = 4; //ph
    y[1] = 7;
    y[2] = 10;

    //Sumatorias
    sumax=sumay=sumaxy=sumax2=0;
    for (i=0;i<3;i++)
    {
        sumaxy += valorBuffer[i]*y[i];
        sumax2 += valorBuffer[i]*valorBuffer[i];
        sumax += valorBuffer[i];
        sumay += y[i];
    }
    /* Calculo de la pendiente (m) y la interseccion (b), faltaría guardarlos en flash*/
    m = (n*sumaxy - sumax*sumay) / (n*sumax2 - sumax*sumax);
    b = (sumay - m*sumax) / n;
    if(guardarFlash("M", (m*1000000))) //esto debería ir en seecion critica
    {
        ESP_LOGI("m", "%f Guardado en Flash", m);
    }
     if(guardarFlash("B", (b*1000000))) //esto debería ir en seecion critica
    {
        ESP_LOGI("b", "%f Guardado en Flash", b);
    }
}