/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/
#include "Arduino.h"

void iniciarUart ();
int estadoTitulacion(float *val);

int iniciarCalibracion();
int finalizarCalibracion();
int leerPotencial(float *val);
int calibrarBufferA();
int calibrarBufferB();
int calibrarBufferC();
//int leerPotencialA(char *val);
//int leerPotencialB(char *val);
//int leerPotencialC(char *val);
int iniciarTitulacion();
int cancelarTitulacion();
int finalizarTitulacion();
int iniciarLimpieza();
int finalizarLimpieza();
int leerVolumenCorte(int *val);
int guardarVolumenCorte(char * val);
int estadoAgitador(char *val); //val puede ser N o S
int habilitarAgitador();
int deshabilitarAgitador();