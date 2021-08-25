//==================== Inclusiones ==========================
#include "../include/wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include "electrodo.h"

//==================== Definiciones ==========================
#define WIFI_SSID      "Fibertel WiFi154 2.4GHz"
#define WIFI_PASS      "Roberkira1"
#define MAXIMUM_RETRY  2
// El event group permite múltiples bits para cada evento, pero solo nos interesan dos:
#define WIFI_CONNECTED_BIT BIT0 //está conectado al AP con una IP
#define WIFI_FAIL_BIT      BIT1 //no pudo conectarse después de la cantidad máxima de reintentos

//================== Prototipos =======================
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static httpd_handle_t start_webserver(void);
static esp_err_t principal_get_handler(httpd_req_t *req);
static esp_err_t mostrarPagina(httpd_req_t *req);
static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

extern int volumenInyectado [1000];
extern float titulacionPH [1000];
extern float derivada1 [1000];
extern float derivada2 [1000];
extern uint16_t cont;
extern float volumenFinal;

//================== Variables y constantes =======================
static EventGroupHandle_t s_wifi_event_group;
static const char *TAG = "HTTP_WEBSERVER";
static int s_retry_num = 0;
 
static const httpd_uri_t principal = {
    .uri       = "/principal",
    .method    = HTTP_GET,
    .handler   = principal_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL 
};

//================== Funciones =======================
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
 
void iniciarWifi(void)
{
    s_wifi_event_group = xEventGroupCreate();
 
    //tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_netif_init()); //Inicializa el TCP/IP stack
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
 
    //Crea WIFI STA por defecto. En caso de cualquier error de inicialización, 
    //esta API se cancela.
    //La API crea el objeto esp_netif con la configuración predeterminada de la estación WiFi,
    //adjunta el netif a wifi y registra los controladores de wifi predeterminados.
    esp_netif_create_default_wifi_sta(); 

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
 
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
 
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS ,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
 
    ESP_LOGI(TAG, "wifi_init_sta finished.");
 
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
 
    /* xEventGroupWaitBits() devuelve los bits antes de que se devolviera la llamada,
    por lo que podemos probar qué evento sucedió realmente. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Conectado al AP SSID:%s password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Fallo al conectar al AP SSID:%s, password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "Evento inesperado");
    }
 
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

// Inicia el servidor web
void inicarServidor()
{
    static httpd_handle_t server = NULL;

    esp_netif_init();
 
    //Registra controladores de eventos para detener el servidor cuando se desconecta Wi-Fi y reinicia al conectarse
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
 
    server = start_webserver(); //Inicia el servidor por primera vez
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
 
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &principal);
        return server;
    }
 
    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}
 
static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}
 
static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}
 
static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

/* An HTTP GET handler */
static esp_err_t principal_get_handler(httpd_req_t *req)
{
   ESP_LOGI(TAG, "Pagina principal");
   return mostrarPagina(req);
}

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /principal and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /principal or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /principal). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/principal", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/principal URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } /*else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        //Return ESP_FAIL to close underlying socket
        return ESP_FAIL;
    }*/
    // For any other URI send 404 and close socket
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static esp_err_t mostrarPagina(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
 
    //Obtiene la longitud de la cadena del header y asigna memoria con un byte adicional para la terminación null
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        // Copia el string en el búfer
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Header => Host: %s", buf);
        }
        free(buf);
    }
 
    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Header => Test-Header-2: %s", buf);
        }
        free(buf);
    }
 
    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }
 
    //Obtiene la longitud Query string de la URL solitada
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }
 
    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");
  
    char* data1="<!DOCTYPE html><html>";
    char* data2="<head><title>Titulador</title></head>";
    char* data3="<body><h1>Titulador potenciometrico automatico</h1><p>Desarrollado por UTN San Francisco.</p>";
    char* data4 = "<p>Volumen en punto de equivalencia = %.1f [mL]</p>";
    char* data8= "<p>Datos de la titulacion</p>";
    char* data9= "<table class=""default""><tr><th>Volumen[mL]</th><th>pH</th></tr>";
    char* data10= "<tr><td>%.1f</td><td>%.1f</td></tr>";
    char* data11= "</table>";
    char* end_data = "";
    char valores[100];
    
    httpd_resp_send_chunk(req,data1,strlen(data1));
    httpd_resp_send_chunk(req,data2,strlen(data2));
    httpd_resp_send_chunk(req,data3,strlen(data3));

    sprintf(valores,data4,volumenFinal);
    httpd_resp_send_chunk(req,valores,strlen(valores));

    httpd_resp_send_chunk(req,data8,strlen(data8));
    httpd_resp_send_chunk(req,data9,strlen(data9));
    
    for(int vol = 1; vol <(cont); vol++)
    {
        sprintf(valores,data10,volumenInyectado[vol]/10.0,titulacionPH[vol]);
        httpd_resp_send_chunk(req,valores,strlen(valores));
    }
    httpd_resp_send_chunk(req,data9,strlen(data11));
    httpd_resp_send_chunk(req,end_data,strlen(end_data));
 
    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}