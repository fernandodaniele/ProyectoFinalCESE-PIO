/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/
//#include <avr/interrupt.h>
//#include <avr/io.h>
#include "Arduino.h"
#include "uart.h"

void iniciarUart ()
{
    Serial.begin(115200);
}

int leerElectrodo(float *val){
    Serial.write("E");    //envia comando
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readString();
    *val = temp.toFloat();
    *val = *val/100;
    return 1;
}

int calibrarBufferA()
{
    Serial.write("CA");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int calibrarBufferB()
{
    Serial.write("CB");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int calibrarBufferC()
{
    Serial.write("CC");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int leerElectrodoA(char *val){
    Serial.write("VA");    //envia comando para leer el valor de buffer A
    Serial.flush();
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readString();
    temp.toCharArray(val, 5);
    return 1;
}

int leerElectrodoB(char *val){
    Serial.write("VB");    //envia comando
    Serial.flush();
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readString();
    temp.toCharArray(val, 5);
    return 1;
}

int leerElectrodoC(char *val){
    Serial.write("VC");    //envia comando
    Serial.flush();
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readString();
    temp.toCharArray(val, 5);
    return 1;
}

int iniciarTitulacion()
{
    Serial.write("TI");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int finalizarTitulacion()
{
    Serial.write("TF");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int iniciarLimpieza()
{
    Serial.write("LI");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }           
}

int finalizarLimpieza()
{
    Serial.write("LF");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }       
}

int habilitarAgitador()
{
    Serial.write("AI");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int deshabilitarAgitador()
{
    Serial.write("AF");
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int leerVolumenCorte(int *val){
    Serial.write("RV");    //envia comando para leer el valor de buffer A
    Serial.flush();
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readString();
    *val = temp.toInt();
    return 1;
}

int guardarVolumenCorte(char * val)
{
    Serial.write("WV");
    Serial.write(val);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readString();
    if (ack == "OK")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

/* //Esto es para usar interrupciones
void iniciarUart (){
    UBRR1 = 8; // 8 for configuring baud rate of 115200bps, 103 for 9600
    UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10); 
    // Use 8-bit character sizes
    UCSR1B |= (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);  
    // Turn on the transmission, reception, and Receive interrupt     
    sei();// enable global interrupt
}

ISR(USART1_RX_vect)
{ 
  char datoActual = UDR1; // Lee el rado recibido desde el puerto serie
  
  if(datoActual == '\n'){
    recepcionTerminada = true;
    for(int i =0; i<indice; i++){
          datosRecibidos[i] = datos [i];
    }
    cantidad = indice;
    indice=0;
  }
  else
  {
    datos[indice] = datoActual;
    indice++;
  }
}
*/