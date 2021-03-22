#include "flash.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

nvs_handle_t nvsHandle; //Handle para el NVS
esp_err_t err; //variable para el control de errores 

void iniciarFlash (void)
{
    err = nvs_flash_init(); //inicializa el guardado no volatil en flash
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        //La partición NVS se truncó y debe borrarse
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init(); //Reintenta iniciar NVS
    }
    ESP_ERROR_CHECK( err );
}

int leerFlash (const char *valorNombre, int16_t *valorLeido)
{ 
    err = nvs_open("storage", NVS_READWRITE, &nvsHandle); //Abre el handle del NVS
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return -1;
    } 
    else 
    {
        err = nvs_get_i16(nvsHandle, valorNombre, valorLeido);
        switch (err) {
            case ESP_OK:
                return 1;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                return 0;
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
                return -1;
        }

        nvs_close(nvsHandle); //Cierra el handle
    }
}

int guardarFlash(const char *valorNombre, int16_t valorGuardar)
{ 
    err = nvs_open("storage", NVS_READWRITE, &nvsHandle); //Abre el handle del NVS
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return -1;
    } 
    else 
    {
        err = nvs_set_i16(nvsHandle, valorNombre, valorGuardar); 
        if(err != ESP_OK){
            printf("Error (%s) al guardar!\n", esp_err_to_name(err));
            return -1;
        }

        err = nvs_commit(nvsHandle); //Escribe los cambios en la memoria flash
        if(err != ESP_OK){
            printf("Error (%s) al escribir!\n", esp_err_to_name(err));
            return -1;
        }

        nvs_close(nvsHandle);
        return 1;
    }
}