/*=============================================================================
 * Autor: Fernando Ezequiel Daniele <fernandodaniele1993@gmai.com>
 * Fecha: 2020/12/21
 *===========================================================================*/
#include "Arduino.h"
#include "uart.h"

#define INICIO_CAL          'A' 		
#define FIN_CAL    		    'B'
#define LECTURA_POTENCIAL   'C'
#define GRABA_BUFFER4       'D'
#define GRABA_BUFFER7       'E'
#define GRABA_BUFFER10      'F'
#define INICIO_TIT          'G'
#define LECTURA_PH          'H'
#define CANCELA_TIT         'I'
#define FIN_TIT             "J"
#define INICIO_LIMPIEZA     'K'
#define FIN_LIMPIEZA        'L'
#define LEE_VOLUMEN         'M'
#define GUARDA_VOLUMEN      'N'
#define ESTADO_AGITADOR     'O'
#define HABILITA_AGIT	    'P'
#define DESHABILITA_AGIT    'Q'

void iniciarUart ()
{
    Serial.begin(115200);
}

int estadoTitulacion(float *val)
{
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 500))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == FIN_TIT)
    {
        unsigned long tOut = millis ();
        /*while(Serial.available()==0)
        {
            if(millis()> (tOut + 1000))
            {
                return 0;
            }
        }*/
        String temp = Serial.readStringUntil('/');
        *val = temp.toFloat();
        return 1;   //finalizo la titulacion
    }
    else
    {
        return 0;   //continua la titulacion
    } 
}

int iniciarCalibracion(){
    Serial.write(INICIO_CAL);    //envia comando
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }  
}

int finalizarCalibracion(){
    Serial.write(FIN_CAL);    //envia comando
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }  
}

int leerPotencial(float *val){
    Serial.write(LECTURA_POTENCIAL);    //envia comando
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readStringUntil('/');
    *val = temp.toFloat();
    return 1;
}

int calibrarBufferA()
{
    Serial.write(GRABA_BUFFER4);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
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
    Serial.write(GRABA_BUFFER7);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
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
    Serial.write(GRABA_BUFFER10);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int iniciarTitulacion()
{
    Serial.write(INICIO_TIT);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int leerPH(float *val){
    Serial.write(LECTURA_PH);    //envia comando
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readStringUntil('/');
    *val = temp.toFloat();
    *val = *val/100;
    return 1;
}

int cancelarTitulacion()
{
    Serial.write(CANCELA_TIT);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
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
    Serial.write(INICIO_LIMPIEZA);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
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
    Serial.write(FIN_LIMPIEZA);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }       
}

int leerVolumenCorte(int *val){
    Serial.write(LEE_VOLUMEN);    //envia comando para leer el valor de buffer A
    Serial.flush();
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String temp = Serial.readStringUntil('/');
    *val = temp.toInt();
    return 1;
}

int guardarVolumenCorte(char * val)
{
    Serial.write(GUARDA_VOLUMEN);
    Serial.write(val);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}

int estadoAgitador(char *val){
    Serial.write(ESTADO_AGITADOR);    //envia comando para leer el valor de buffer A
    Serial.flush();
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    *val = Serial.read();
    return 1;
}

int habilitarAgitador()
{
    Serial.write(HABILITA_AGIT);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
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
    Serial.write(DESHABILITA_AGIT);
    unsigned long tOut = millis ();
    while(Serial.available()==0)
    {
        if(millis()> (tOut + 1000))
        {
            return 0;
        }
    }
    String ack = Serial.readStringUntil('/');
    if (ack == "K")
    {
        return 1;   //se escribió correctamente
    }
    else
    {
        return 0;   //hubo error
    }    
}