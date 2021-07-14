//============================ Inclusiones ===================================
#include "uart.h"
#include "electrodo.h"
#include "flash.h"

//============================ Definiciones ===================================
#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

//============================== Variables ===================================
static const int RX_BUF_SIZE = 1024;
extern float valorAdc;
extern portMUX_TYPE myMutex;
extern int motorBomba;
extern int procesoTitulacion;
extern int procesoCalibracion;
extern int16_t valorBuffer[3];

char bufferA[7];
int16_t bufferAInt = 400;
char bufferB[7];
int16_t bufferBInt = 700;
char bufferC[7];
int16_t bufferCInt = 1000;
char electrodoA[7];
int16_t electrodoAInt = 500;
char electrodoB[7];
int16_t electrodoBInt = 750;
char electrodoC[7];
int16_t electrodoCInt = 1100;
char electrodoStr[7];
int16_t electrodoVal = 800;
char volumenStr[7];
int16_t volumenCorte = 10;

//============================ Funciones ===================================
void iniciarUart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

int enviarPorUart(const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    uart_write_bytes(UART_NUM_1, "/", 1);
    //ESP_LOGI("Envío por UART", "%s", data);
    return txBytes;
}

void tareaUart(void *arg)
{
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    int64_t mProv, bProv;
    float m = 1, b = 1;            
    
    while (true) 
    {
        int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 500 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
			switch (data[0])
			{
                case INICIO_CAL:{
                    enviarPorUart("K");
                    procesoCalibracion = 1;
                    break;
                }
                case FIN_CAL:{
                    enviarPorUart("K");
                    procesoCalibracion = 0;
                    rectaRegresion();
                    break;
                }
                case LECTURA_POTENCIAL:{
                    leerFlash ("M", &mProv);
                    leerFlash ("B", &bProv);
                    m = mProv/1000000.0;
                    b = bProv/1000000.0;
                    float valorPH = m * valorAdc + b;
                    sprintf(electrodoStr,"%.2f",valorPH);
                    enviarPorUart (electrodoStr); //acá seria el valor del electrodo
                    break;
                }
                case GRABA_BUFFER4:{
                   /* if(guardarFlash("CALIBREA", valorAdc)) //esto debería ir en seecion critica
                    {
                        ESP_LOGI(RX_TASK_TAG, "%d Guardado en Flash", valorAdc);
                    }*/
                    portENTER_CRITICAL(&myMutex);
                    valorBuffer[0] = valorAdc;
                    portEXIT_CRITICAL(&myMutex);
                    enviarPorUart("K");
                    break;
                }
                case GRABA_BUFFER7:{
                    portENTER_CRITICAL(&myMutex);
                    valorBuffer[1] = valorAdc;
                    portEXIT_CRITICAL(&myMutex);
                    enviarPorUart("K");
                    break;
                }
                case GRABA_BUFFER10:{
                    portENTER_CRITICAL(&myMutex);
                    valorBuffer[2]= valorAdc;
                    portEXIT_CRITICAL(&myMutex);
                    enviarPorUart("K");
                    break;
                }	
                case INICIO_TIT:{
                    //iniciar titualacion
                    enviarPorUart("K");
                    procesoTitulacion = 1;
                    break;
                }
                case LECTURA_PH:{
                    break;
                }
                case CANCELA_TIT:{
                    //finalizar titulacion
                    enviarPorUart("K");
                    procesoTitulacion = 0;
                    break;
                }
                case INICIO_LIMPIEZA:{
                    //iniciar limpieza
                    enviarPorUart("K");
                    motorBomba = 1;
                    break;
                }
                case FIN_LIMPIEZA:{
                    //finalizar limpieza
                    enviarPorUart("K");
                    motorBomba = 0;
                    break;
                }
                case GUARDA_VOLUMEN:{
                    for(int i = 1; i<rxBytes; i++)
                    {
                        volumenStr [i-1] = data [i];
                    }
                    volumenCorte = 10 * (atoi (volumenStr)) ;
                    enviarPorUart("K");
                    printf("Volumen corte recibido: '%d'", volumenCorte);
                    break;
                }
			    case LEE_VOLUMEN:{
                    sprintf(volumenStr,"%d",(volumenCorte/10));
                    enviarPorUart (volumenStr);
                    printf("Volumen corte enviado: '%s'", volumenStr);
                    break;
                }
                case ESTADO_AGITADOR:{
                    break;
                }
                case HABILITA_AGIT:{
                    //habilitar agitador
                    enviarPorUart("K");
                    break;
                }
                case DESHABILITA_AGIT:{
                    //deshabilitar agitador
                    enviarPorUart("K");
                    break;
                }
                default:{
                    break;
                }
			}
        }    
    }
    free(data);
}