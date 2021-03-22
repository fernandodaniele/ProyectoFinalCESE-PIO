/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/08/12
 *===========================================================================*/

#ifndef MISPROGRAMAS_PDM_TP_FINAL_INC_MENU_H_
#define MISPROGRAMAS_PDM_TP_FINAL_INC_MENU_H_

#include <Arduino.h>
// Funciones de la MEF
void errorMEF( void );
void iniciarMEF( void );
void actualizarMEF();

// Nombres de los estados de la MEF
typedef enum{
   MENU_INICIAL,
   MENU_ELEGIR_BUFFER,
   MENU_CALIBRAR_A,
   MENU_CALIBRAR_B,
   MENU_CALIBRAR_C,
   MENU_TITULACION,
   MENU_AJUSTES,
   MENU_VOLUMEN,
   MENU_LIMPIEZA,
   MENU_AGITADOR,
   MENU_CONEXION,
} pantalla_t;

#endif /* MISPROGRAMAS_PDM_TP_FINAL_INC_MENU_H_ */