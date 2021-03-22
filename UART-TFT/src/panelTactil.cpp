/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/

#include "main.h"
#include "pantallaTFT.h"
#include "panelTactil.h"
#include <TouchScreen.h>   //librería para la parte táctil del módulo
#define MINPRESSURE 150   //Valor previo: 200- Rango de detección eje z táctil
#define MAXPRESSURE 1000

//const int XP=12,XM=15,YP=33,YM=13;                                                         
const int XP=8,XM=A2,YP=A3,YM=9; //240x320 ID=0x9341 //Para Arduino UNO y MEGA
//const int TS_LEFT=130,TS_RT=906,TS_TOP=905,TS_BOT=126;  //Valores obtenidos del ejemplo de calibración de la librería. Para arduino UNO usar const int TS_LEFT=66,TS_RT=883,TS_TOP=924,TS_BOT=108;  
const int TS_LEFT=906,TS_RT=130,TS_TOP=126,TS_BOT=905; //Para la rotacion de pantalla 3 en UNO
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);      //Objeto para pantalla táctil ¿porqué 300?

/*
 * Función para leer la pantalla táctil
 */

bool Touch_getXY(int *pixel_X,int *pixel_Y){           //Touch_getXY() updates global vars
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);          //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);       //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
      *pixel_X = map(p.y, TS_LEFT, TS_RT, 0, 320);
      *pixel_Y = map(p.x, TS_TOP, TS_BOT, 0, 240);
    }
    return pressed;
}