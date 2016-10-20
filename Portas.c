/*
 * File:   Portas.c
 * Author: Andre
 *
 * Created on 21 de Abril de 2016, 18:05
 */

#include <p30F4013.h>

#include "Portas.h"

void InitPortas(){
    //Ports I/O
    //LEDS
    TRISBbits.TRISB10 = 0;
    TRISBbits.TRISB9 = 0;
    TRISDbits.TRISD2 = 0;
    TRISBbits.TRISB12 = 0;
    TRISDbits.TRISD9 = 0;
    
    ADPCFG              = 0b1111111111111111;    //-> Colocar 0 nas entradas analógicas
    
    LED_BR = 1;
    LED_AZ = 1;
    EEPROM_SS = 1;
    INJ_1 = INJ_OFF;
    IGN_1 = IGN_OFF;
    
}
