/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/
#include "uart.h"
#include "menu.h"
#include "main.h"
#include "pantallaTFT.h"


// Variable para el estado actual
pantalla_t pantalla;
int opcion =0;

// Función para controlar errores de la MEF (Error handler)
void errorMEF( void )
{
   iniciarMEF();
}

// Función para iniciar la MEF
void iniciarMEF( void )
{
   pantalla = MENU_INICIAL; 
   inicializarTFT();
   pantallaInicial();
}

// Función para actualizar la MEF
void actualizarMEF( )
{
   switch( pantalla ){
      case MENU_INICIAL:
         opcion = consultaTactil();
    	   switch (opcion){
            case 1:
               pantalla = MENU_ELEGIR_BUFFER;
               pantallaElegirBuffer();
               break;
            case 2:
               pantalla = MENU_TITULACION;
               pantallaMedir();
               if(iniciarTitulacion()){   //envia señal para iniciar la titulacion
                  //Serial.println("Se inicio titulacion");
               }
               else
               {
                  //acá ver que hacer en caso de error
                  //Serial.println("Error en el inicio de titulacion");
               }
               break;
            case 3:
               pantalla = MENU_AJUSTES;
               pantallaAjustes();
               break;
            case 4:
               pantalla = MENU_CONEXION;
               pantallaWIFI ();
               break;
            default:
               pantalla = MENU_INICIAL;
               break;
    	   }
    	   break;

      case MENU_ELEGIR_BUFFER:
         opcion = consultaTactil();
         switch (opcion){
            case 1:
               pantalla = MENU_CALIBRAR_A;
               pantallaCalibrar ();
               break;
            case 2:
               pantalla = MENU_CALIBRAR_B;
               pantallaCalibrar ();
               break;
            case 3:
               pantalla = MENU_CALIBRAR_C;
               pantallaCalibrar ();
               break;
            case 4:
               pantalla = MENU_INICIAL;
               pantallaInicial();
               break;
            default:
               pantalla = MENU_ELEGIR_BUFFER;
               break;
    	   }
    	   break;

      case MENU_CALIBRAR_A:
         opcion = tactilCalibrar();
    	   if(opcion==3){
            calibrarBufferA();
            pantalla = MENU_ELEGIR_BUFFER;
            pantallaElegirBuffer();
    	   }
         else if(opcion==4){
            pantalla = MENU_ELEGIR_BUFFER;
            pantallaElegirBuffer();
    	   }
    	   break;

      case MENU_CALIBRAR_B:
         opcion =  tactilCalibrar();
    	   if(opcion==3){
            calibrarBufferB();
            pantalla = MENU_ELEGIR_BUFFER;
            pantallaElegirBuffer();
    	   }
         else if(opcion==4){
            pantalla = MENU_ELEGIR_BUFFER;
            pantallaElegirBuffer();
    	   }
    	   break;

      case MENU_CALIBRAR_C:
         opcion =  tactilCalibrar();
    	   if(opcion==3){
            calibrarBufferC();
            pantalla = MENU_ELEGIR_BUFFER;
            pantallaElegirBuffer();
    	   }
         else if(opcion==4){
            pantalla = MENU_ELEGIR_BUFFER;
            pantallaElegirBuffer();
    	   }
    	   break;

      case MENU_TITULACION:
         opcion = tactilMedir();
    	   if(opcion==4){
            if(finalizarTitulacion()){
              // Serial.println("Se finalizo titulacion");
            }
            else
            {
              // Serial.println("Error en finalizar titulacion");
            }
            pantalla = MENU_INICIAL;
            pantallaInicial();
    	   }
    	   break;

      case MENU_AJUSTES:
         opcion = consultaTactil();
         switch (opcion){   
            case 1:
               pantalla = MENU_VOLUMEN;
               pantallaVolumenCorte();
               break;
            case 2:
               pantalla = MENU_LIMPIEZA;
               pantallaLimpieza();
               if(iniciarLimpieza()){
                 // Serial.println("Se inicio limpieza");
               }
               else
               {
                 // Serial.println("Error en inicio limpieza");
               }
               break;
            case 3:
               pantalla = MENU_AGITADOR;
               pantallaAgitador();
               break;
            case 4:
               pantalla = MENU_INICIAL;
               pantallaInicial();
               break;
            default:
               pantalla = MENU_AJUSTES;
               break;
    	   }
         break;

      case MENU_VOLUMEN:
         opcion = tactilVolumenCorte();
    	   if(opcion==4){
            //Acá enviar al ESP el valor del volumen de corte
            pantalla = MENU_AJUSTES;
            pantallaAjustes();
    	   }
    	   break;

      case MENU_LIMPIEZA:
         opcion = tactilLimpieza();
    	   if(opcion==4){
            pantalla = MENU_AJUSTES;
            if(finalizarLimpieza()){
              // Serial.println("Se finalizo limpieza");
            }
            else
            {
               //Serial.println("Error en finalizar limpieza");
            }
            pantallaAjustes();
    	   }
    	   break;

      case MENU_AGITADOR:
         opcion = tactilAgitador();
         switch (opcion)
         {
         case 1:
               if(habilitarAgitador()){
                  habilitoAgitador();
               }
               else
               {
                  imprimirError();
               }
            pantalla = MENU_AJUSTES;
            pantallaAjustes();
            break;
         case 2:
               if(deshabilitarAgitador()){
                 deshabilitoAgitador();
               }
               else
               {
                  imprimirError();
               }
            pantalla = MENU_AJUSTES;
            pantallaAjustes();
            break;
         case 4:
            pantalla = MENU_AJUSTES;
            pantallaAjustes();
            break;
         default:
            break;
         }
    	   break;

      case MENU_CONEXION:
         opcion = tactilWIFI();
    	   if(opcion==4){
            pantalla = MENU_INICIAL;
            pantallaInicial();
    	   }
         break;

      default:
    	 errorMEF();
    	 break;
   }
}