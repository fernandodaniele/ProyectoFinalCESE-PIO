/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/
#include "Arduino.h"

void iniciarUart ();
int leerElectrodo(float *val);
int calibrarBufferA();
int calibrarBufferB();
int calibrarBufferC();
int leerElectrodoA(char *val);
int leerElectrodoB(char *val);
int leerElectrodoC(char *val);
int iniciarTitulacion();
int finalizarTitulacion();
int iniciarLimpieza();
int finalizarLimpieza();
int habilitarAgitador();
int deshabilitarAgitador();

int leerVolumenCorte(int *val);
int guardarVolumenCorte(char * val);