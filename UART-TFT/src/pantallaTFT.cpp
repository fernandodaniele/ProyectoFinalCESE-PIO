/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/

/*=============================================================================
 * Inclusiones de archivos y bibliotecas
 *===========================================================================*/
#include <Adafruit_GFX.h>  //librería gráfica para TFT
#include <MCUFRIEND_kbv.h> //librería especifica del módulo
#include "main.h"
#include "pantallaTFT.h"
#include "panelTactil.h"
#include "uart.h"

/*=============================================================================
 * Definiciones y macros
 *===========================================================================*/
#define BLACK   0x0000    //Colores predefinidos. Se pueden agregar o modificar
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define TXT_BTN_SIZE    2
#define CALIBRAR_BTN    "CALIBRAR"
#define TITULAR_BTN     "TITULAR"
#define CONFIGURAR_BTN  "AJUSTES"
#define WIFI_BTN        "WIFI"
#define B1_BTN          "BUFFER 4"
#define B2_BTN          "BUFFER 7"
#define B3_BTN          "BUFFER 10"
#define REGRESAR_BTN    "REGRESAR"
#define T_INC_DEC        70
#define RETARDO_PANTALLA 1000
#define RETARDO_PIXEL    5000
/*=============================================================================
 * Variables y objetos locales
 *===========================================================================*/
MCUFRIEND_kbv tft;        //Objeto pantalla gráfica

Adafruit_GFX_Button unoBtn, dosBtn, tresBtn, cuatroBtn;
Adafruit_GFX_Button incBtn, decBtn;

char bufferA[6], bufferB[6], bufferC[6]; //variables de calibración en bits formato decimal (0-0V 2047-3.3V)
float bufferAPH = 4.00, bufferBPH = 7.00, bufferCPH = 10.00; //variables de calibración en ph
int pixel_x, pixel_y;   
int volumenCorte = 10;

uint16_t gX, gY; //variables para la curva de titulacion
float ph;
unsigned long T;

/*=============================================================================
 * Definición de funciones globales públicas
 *===========================================================================*/
 //Inicializa el display TFT
 void inicializarTFT(){
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3)
  ID = 0x9486; // write-only shield
  tft.begin(ID);
  tft.setRotation(3); //horizontal 1 o 3
}

//Muestra la pantalla de inicio
void pantallaInicial(){
  tft.fillScreen(BLACK);  //borra cualquier imagen previa

  //Se inician y muestran los botones
                        // (gfx, x, y, w, h, outline, fill, textcolor, nombre, textsize)
  unoBtn.initButton(&tft,  80, 150, 140, 40, WHITE, CYAN, BLACK, CALIBRAR_BTN, TXT_BTN_SIZE); 
  dosBtn.initButton(&tft, 240, 150, 140, 40, WHITE, CYAN, BLACK, TITULAR_BTN, TXT_BTN_SIZE);
  tresBtn.initButton(&tft,  80, 210, 140, 40, WHITE, CYAN, BLACK, CONFIGURAR_BTN, TXT_BTN_SIZE);
  cuatroBtn.initButton(&tft, 240, 210, 140, 40, WHITE, CYAN, BLACK, WIFI_BTN, TXT_BTN_SIZE);
  unoBtn.drawButton(false);//dibuja boton. false es para intercambiar colores
  dosBtn.drawButton(false);
  tresBtn.drawButton(false);
  cuatroBtn.drawButton(false);

  //Se dibuja un rectangulo para escribir dentro
  tft.drawRect(10, 10, 300, 110, WHITE); //(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
  tft.setCursor(15,13);
  tft.setTextColor(WHITE);
  tft.print("...... TITULADOR .......");
  tft.setCursor(15,38);
  tft.print("...... AUTOMATICO ......");
  tft.setCursor(15,61);
  tft.print("Desarrollado por");
  tft.setCursor(15,86);
  tft.print("U.T.N. San Francisco.");
}

//Consulta si se presionó algún botón en la pantalla inicial
int consultaTactil(){
  //Consulta si se presionó el tactil y guarda las coordenadas
  bool presionado = Touch_getXY(&pixel_x, &pixel_y);
  
  //Compara el dato obtenido con los botones de la pantalla
  unoBtn.press(presionado && unoBtn.contains(pixel_x, pixel_y));
  dosBtn.press(presionado && dosBtn.contains(pixel_x, pixel_y));
  tresBtn.press(presionado && tresBtn.contains(pixel_x, pixel_y));
  cuatroBtn.press(presionado && cuatroBtn.contains(pixel_x, pixel_y));

  //Si alguno de los botones fue soltado, retorna el valor correspondiente
  if (unoBtn.justReleased()) {
    return 1;
  }
  else if (dosBtn.justReleased()) {
    return 2; 
  }
  else if (tresBtn.justReleased()) {
    return 3;
  }
  else if (cuatroBtn.justReleased()) {
    return 4; 
  }
  else{
    return 0;
  } 
}

//Muestra la pantalla con el valor actual de cada buffer y da la opción de elegir 
//cada uno de ellos por separado para realizar la calibración
void pantallaElegirBuffer(){
    //Limpia la pantalla y muestra el valor actual de los buffers
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setCursor(10,20);
    tft.print("1. Coloque el electrodo\n   en el buffer\n");
    tft.print("2. Seleccione la opcion\n   correspondiente al\n   buffer utilizado");

    //Dibuja los botones correspondientes a esta pantalla
    unoBtn.initButton(&tft,  80, 150, 140, 40, WHITE, CYAN, BLACK, B1_BTN, TXT_BTN_SIZE);
    dosBtn.initButton(&tft, 240, 150, 140, 40, WHITE, CYAN, BLACK, B2_BTN, TXT_BTN_SIZE);
    tresBtn.initButton(&tft,  80, 210, 140, 40, WHITE, CYAN, BLACK, B3_BTN, TXT_BTN_SIZE);
    cuatroBtn.initButton(&tft, 240, 210, 140, 40, WHITE, CYAN, BLACK, REGRESAR_BTN, TXT_BTN_SIZE);
    unoBtn.drawButton(false);
    dosBtn.drawButton(false);
    tresBtn.drawButton(false);
    cuatroBtn.drawButton(false);
}

//Muestra la pantalla con el valor actual de ph que será guardado como valor para calibrar el instrumento
void pantallaCalibrar (){
  tft.fillScreen(BLACK);
  dosBtn.initButton(&tft,  80, 210, 140, 40, WHITE, CYAN, BLACK, "CANCELAR", TXT_BTN_SIZE);
  unoBtn.initButton(&tft, 240, 210, 140, 40, WHITE, CYAN, BLACK, "GUARDAR", TXT_BTN_SIZE);
  dosBtn.drawButton(false);
  unoBtn.drawButton(false);
  tft.setTextColor(WHITE);
  tft.setCursor(10,10);
  tft.println("Realice la medicion");
  tft.println("Presione guardar cuando el");
  tft.println("valor en mV se estabilice");
}

//Consulta si se presionó algún botón en la pantalla de elección de buffer
int tactilCalibrar(){
  float tempMV = ph;
  //Acá tengo que leer el valor del electrodo desde el ESP
  if(leerPotencial(&ph)==0){
      imprimirError();
      return 4;
  }
  else
  {
  tft.setCursor(100,110);
  tft.setTextColor(BLACK);
  tft.print(tempMV);
  tft.print(" pH");
  //Acá tengo que mostrar en pantalla ese valor leido
  tft.setCursor(100,110);
  tft.setTextColor(WHITE);
  tft.print(ph);
  tft.print(" pH");
  }

  bool  ab = Touch_getXY(&pixel_x, &pixel_y);
  unoBtn.press(ab && unoBtn.contains(pixel_x, pixel_y));
  dosBtn.press(ab && dosBtn.contains(pixel_x, pixel_y));

  if (unoBtn.justReleased()){
    return 3;
  }
  else if (dosBtn.justReleased()){
    return 4;
  }
  else{
    return 0;
  }
}

//Muestra la pantalla con la curva de titulación y el valor actual de pH
void pantallaMedir(){
 
  tft.fillScreen(BLACK);
  unoBtn.initButton(&tft, 240, 210, 140, 40, WHITE, CYAN, BLACK, "FINALIZAR", TXT_BTN_SIZE);
  unoBtn.drawButton(false);

  //Dibuja el grafico
  tft.drawLine(20, 30, 20, 160, WHITE);
  tft.drawLine(20, 160, 310, 160, WHITE);
  tft.setTextColor(WHITE);
  tft.setTextSize(1); 
  tft.setCursor(5,5);
  tft.print("pH");
  tft.setCursor(5,30);
  tft.print("14");
  tft.setCursor(5,91);
  tft.print(" 7");
  tft.setCursor(5,153);
  tft.print(" 0");
  tft.setCursor(20,163);
  tft.print("0");
  tft.setCursor(280,163);
  tft.print("tiempo");
  tft.setTextSize(2);

  T= millis();
  gX = 20;
}

//Actualiza el grafico y consulta si se presionó el boton de finalizar
int tactilMedir(){
  
  if(millis()>T+RETARDO_PIXEL){

    float tmpPH = ph;
    //Acá debería consultar al ESP el valor del pH
    if(leerPotencial(&ph)==0){
      imprimirError();
      return 4;
    }

    gY =(ph * (30 - 160)/14+ 160);
    tft.drawPixel(gX,gY,YELLOW);

    tft.setCursor(20,200);
    tft.setTextColor(BLACK);
    tft.print(tmpPH);
    tft.print(" pH");
    tft.setCursor(20,200);
    tft.setTextColor(WHITE);
    tft.print(ph);
    tft.print(" pH");
    T = millis();
    gX++;
  }
  
  bool  d = Touch_getXY(&pixel_x, &pixel_y);
  unoBtn.press(d && unoBtn.contains(pixel_x, pixel_y));
  
  if (unoBtn.justReleased()){
    return 4;
  }
  else{
    return 0;
  }

}

//Muestra la pantalla con la opciones para configurar los buffers a utilizar, el volumen de corte y para ejecutar la limpieza de la bomba
void pantallaAjustes()
{
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setCursor(10,20); 
    tft.print ("Seleccione una opcion");

    unoBtn.initButton(&tft,  80, 150, 140, 40, WHITE, CYAN, BLACK, "VOLUMEN", TXT_BTN_SIZE); 
    dosBtn.initButton(&tft, 240, 150, 140, 40, WHITE, CYAN, BLACK, "LIMPIEZA", TXT_BTN_SIZE);
    tresBtn.initButton(&tft,  80, 210, 140, 40, WHITE, CYAN, BLACK, "AGITADOR", TXT_BTN_SIZE);
    cuatroBtn.initButton(&tft, 240, 210, 140, 40, WHITE, CYAN, BLACK, "REGRESAR", TXT_BTN_SIZE);
    unoBtn.drawButton(false);//dibuja boton. false es para intercambiar colores
    dosBtn.drawButton(false);
    tresBtn.drawButton(false);
    cuatroBtn.drawButton(false);
}

//Muestra la pantalla que permite ajustar el valor del volumen de corte
void pantallaVolumenCorte()
{
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setCursor(10,15);
  tft.print("Configurar el volumen de corte");
  tft.setCursor(10,65);
  if(leerVolumenCorte(&volumenCorte) == 0)
  {
    tft.print("Error");
  }
  else
  {
  tft.print("Volumen = ");
  tft.setCursor(125,65);
  tft.print(volumenCorte);
  }

  unoBtn.initButton(&tft,  80, 150, 140, 40, WHITE, CYAN, BLACK, "+", TXT_BTN_SIZE); 
  dosBtn.initButton(&tft, 240, 150, 140, 40, WHITE, CYAN, BLACK, "-", TXT_BTN_SIZE);
  tresBtn.initButton(&tft,  80, 210, 140, 40, WHITE, CYAN, BLACK, "GUARDAR", TXT_BTN_SIZE);
  cuatroBtn.initButton(&tft, 240, 210, 140, 40, WHITE, CYAN, BLACK, "REGRESAR", TXT_BTN_SIZE);
  unoBtn.drawButton(false);//dibuja boton. false es para intercambiar colores
  dosBtn.drawButton(false);
  tresBtn.drawButton(false);
  cuatroBtn.drawButton(false);
 }

//Permite configurar el volumen de corte y consulta si se presionó el boton de guardar
int  tactilVolumenCorte()
{
  bool  presionado = Touch_getXY(&pixel_x, &pixel_y);
  unoBtn.press(presionado && unoBtn.contains(pixel_x, pixel_y));
  dosBtn.press(presionado && dosBtn.contains(pixel_x, pixel_y));
  tresBtn.press(presionado && tresBtn.contains(pixel_x, pixel_y));
  cuatroBtn.press(presionado && cuatroBtn.contains(pixel_x, pixel_y));  
  if (unoBtn.justReleased()){
    delay(T_INC_DEC); 
    tft.setCursor(125,65);
    tft.setTextColor(BLACK);
    tft.print(volumenCorte);
    volumenCorte = volumenCorte + 1;
    tft.setTextColor(WHITE);
    tft.setCursor(125,65);
    tft.print(volumenCorte);
  }
  else if (dosBtn.justReleased()){
    delay(T_INC_DEC);
    tft.setCursor(125,65);
    tft.setTextColor(BLACK);
    tft.print(volumenCorte);
    volumenCorte = volumenCorte - 1;
    tft.setTextColor(WHITE);
    tft.setCursor(125,65);
    tft.print(volumenCorte);
  }
  else if (tresBtn.justReleased()){
    imprimirGuardando();
    char volumenStr [6];
    sprintf(volumenStr, "%d", volumenCorte);
    if(guardarVolumenCorte(volumenStr)==0)
    {
      imprimirError();
    }
    return 4;
  }
  else if (cuatroBtn.justReleased()){
    return 4;
  }
  else
  {
    return 0;
  }
}

//Muestra la pantalla de limpieza de la bomba
void pantallaLimpieza ()
{
  tft.fillScreen(BLACK);
  unoBtn.initButton(&tft,  80, 210, 140, 40, WHITE, CYAN, BLACK, "FINALIZAR", TXT_BTN_SIZE);
  unoBtn.drawButton(false);

  tft.setTextColor(WHITE);
  tft.setCursor(10,10);
  tft.println("Realizando limpieza");
}

//Consulta si se presionó el boton de finalizar
int tactilLimpieza()
{
  bool  presionado = Touch_getXY(&pixel_x, &pixel_y);
  unoBtn.press(presionado && unoBtn.contains(pixel_x, pixel_y));

  if (unoBtn.justReleased()){ 
    imprimirFinalizando();
    return 4;
  }
  else{
    return 0;
  }
}

//Muestra la pantalla con los datos para conectarse al WiFi
void pantallaWIFI ()
{
  tft.fillScreen(BLACK);
  unoBtn.initButton(&tft,  80, 210, 140, 40, WHITE, CYAN, BLACK, "FINALIZAR", 2); // (gfx, x, y, w, h, outline, fill, textcolor, "Calibracion", textsize)
  unoBtn.drawButton(false);

  tft.setTextColor(WHITE);
  tft.setCursor(10,10);
  tft.println("Pantalla wifi");
}

//Consulta si se presionó el boton de finalizar
int tactilWIFI()
{
  bool  presionado = Touch_getXY(&pixel_x, &pixel_y);
  unoBtn.press(presionado && unoBtn.contains(pixel_x, pixel_y));

  if (unoBtn.justReleased()){
    return 4;
  }
  else{
    
    return 0;
  }
}

void imprimirFinalizando()
{
  tft.setCursor(15,100);
  tft.setTextColor(WHITE);
  tft.fillScreen(BLACK);
  tft.print("Finalizando...");
  delay(RETARDO_PANTALLA);
}

void imprimirError(){
  tft.setCursor(15,100);
  tft.setTextColor(WHITE);
  tft.fillScreen(BLACK);
  tft.print("Error");
  delay(RETARDO_PANTALLA);
}

void imprimirGuardando()
{
  tft.setCursor(15,100);
  tft.setTextColor(WHITE);
  tft.fillScreen(BLACK);
  tft.print("Guardando...");
  delay(RETARDO_PANTALLA);
}

void imprimirResultado(float resultado)
{
  tft.setCursor(15,100);
  tft.setTextColor(WHITE);
  tft.fillScreen(BLACK);
  tft.print("Volumen = ");
  tft.print(resultado);
  tft.print(" [mL] ");
  delay(RETARDO_PANTALLA*5);
}

void pantallaCalibrarA()
{
  imprimirGuardando();
  //Cuando se de guardar hay que enviar un señal para que se guarde ese valor
  if(calibrarBufferA()==0)
  {
     imprimirError();
  }
  else
  {
      //leer valor guardado y mostrar en pantalla
  }
}

void pantallaCalibrarB()
{
  imprimirGuardando();
  //Cuando se de guardar hay que enviar un señal para que se guarde ese valor
  if(calibrarBufferB()==0)
  {
     imprimirError();
  }
  else
  {
      //leer valor guardado y mostrar en pantalla
  }
}

void pantallaCalibrarC()
{
  imprimirGuardando();
  //Cuando se de guardar hay que enviar un señal para que se guarde ese valor
  if(calibrarBufferC()==0)
  {
    imprimirError();
  }
  else
  {
      //leer valor guardado y mostrar en pantalla
  }
}

void pantallaAgitador()
{
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setCursor(10,20); 
    tft.println ("Seleccione una opcion");
    unoBtn.initButton(&tft,  80, 150, 140, 40, WHITE, CYAN, BLACK, "ON", TXT_BTN_SIZE); // (gfx, x, y, w, h, outline, fill, textcolor, "Calibracion", textsize)
    dosBtn.initButton(&tft, 240, 150, 140, 40, WHITE, CYAN, BLACK, "OFF", TXT_BTN_SIZE);
    cuatroBtn.initButton(&tft, 160, 210, 300, 40, WHITE, CYAN, BLACK, "REGRESAR", TXT_BTN_SIZE);
    unoBtn.drawButton(false);
    dosBtn.drawButton(false);
    cuatroBtn.drawButton(false);
}

int tactilAgitador(){
    bool  presionado = Touch_getXY(&pixel_x,&pixel_y);
    unoBtn.press(presionado && unoBtn.contains(pixel_x, pixel_y));
    dosBtn.press(presionado && dosBtn.contains(pixel_x, pixel_y));
    cuatroBtn.press(presionado && cuatroBtn.contains(pixel_x, pixel_y));
    if (unoBtn.justReleased()){
      return 1;
    }
    else if (dosBtn.justReleased()){
      return 2;
    }
    else if (cuatroBtn.justReleased()){
      return 4; 
    }
    else{
      return 0;
    }
  
}

void habilitoAgitador()
{
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setCursor(10,20); 
    tft.println ("Agitador habilitado");
    delay(RETARDO_PANTALLA);
}

void deshabilitoAgitador()
{
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);
    tft.setCursor(10,20); 
    tft.print("Agitador deshabilitado");
    delay(RETARDO_PANTALLA);
}