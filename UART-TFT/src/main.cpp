/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/
/***************************************************************************************************************
 * Este programa está desarrollado para un atmega328 y el framework de Arduino
 * El mismo controla un módulo de pantalla TFT 2.4" táctil (resistivo) Modelo MCUFRIEND
 * Se comunica por puerto serie con un módulo ESP32
 ***************************************************************************************************************/
#include "main.h"
#include "menu.h"
#include "uart.h"
/*
 * Función setup
 */
void setup(void){
  iniciarUart ();
  iniciarMEF();
}
/*
 * Función loop
 */
void loop(void){
  actualizarMEF();
}