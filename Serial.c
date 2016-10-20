/*
 * File:   Serial.c
 * Author: Andre
 *
 * Created on 21 de Abril de 2016, 17:37
 */

#include"Serial.h"

void InitSerial(){
    
    U1MODEbits.UARTEN   = 1;    //Habilita UART1
    U1MODEbits.ALTIO    = 1;    //Porta alternativa
    U1STAbits.UTXEN     = 1;    //Habilita transmissoes
    U1BRG = 10;                 //Para BaudRate 115200
    //Interrupcao Uart1
    IFS0bits.U1RXIF     = 0;    //Reset Flag
    IEC0bits.U1RXIE     = 1;    //Interrupcao habilitada
    
}

/**
 * @Descricao Função responsáel pelo envio de dados com 1 bytes pela interface 
 * serial 1. 
 * @Parametro data Tipo: unsigned int | Dados: 0 - 255 | Resol.: 0..1 
 */
void WriteUART1_U08(unsigned char data){

    while (U1STAbits.TRMT==0);      //Espera desocupar o registrador
        U1TXREG = data;
}

/**
 * @Descricao Função responsáel pelo envio de dados com 2 bytes pela interface 
 * serial 1. 
 * @Parametro data Tipo: unsigned int | Dados: 0 - 65535 | Resol.: 0..1 
 */
void WriteUART1_U16(unsigned int data){
    while (U1STAbits.TRMT==0);      //Espera desocupar o registrador
        U1TXREG = data>>8;
    while (U1STAbits.TRMT==0);      //Espera desocupar o registrador
        U1TXREG = data;
}
