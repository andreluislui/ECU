/*
 * File:   SPI.c
 * Author: Andre
 *
 * Created on 21 de Abril de 2016, 20:18
 */

#include "SPI.h"

void __attribute__((__interrupt__, __auto_psv__)) _SPI1Interrupt()
{	
	IFS0bits.SPI1IF = 0;
}

void InitSPI(){
    SPI1CONbits.FRMEN   = 0;    //not framed
    SPI1CONbits.SPIFSD  = 0;    // master
    SPI1CONbits.DISSDO  = 0;
    SPI1CONbits.MODE16  = 0;
    SPI1CONbits.SMP     = 0;    //input sampled at end of output time
    SPI1CONbits.CKE     = 0;
    SPI1CONbits.SSEN    = 0;    //Pin SS Disabled
    SPI1CONbits.CKP     = 0;
    SPI1CONbits.MSTEN   = 1;    //Master
    //SPI1CONbits.SPRE    = 0b110;//Secundary Prescale 2:1
    //SPI1CONbits.PPRE    = 0b11; //Primary Prescale 1:1   
    //Interrupcao SPI
    IFS0bits.SPI1IF = 0;	// clear interrupt flag
	IEC0bits.SPI1IE = 1;	// enable interrupt
    SPI1STATbits.SPIEN = 1;    //enable module
}

void EEPROM_SS_HIGH(){
    EEPROM_SS = 1;
}

void EEPROM_SS_LOW(){
    EEPROM_SS = 0;
}

unsigned char SPI_Write(unsigned int data){
    SPI1BUF = data;                 //Write in buffer 
    while(!SPI1STATbits.SPIRBF);    //While for transfer complete 
    return SPI1BUF;
}

void EEPROM_Write_Enable(){
    unsigned char var;
    EEPROM_SS_LOW();
    var = SPI_Write(EEPROM_CMD_WREN);
    EEPROM_SS_HIGH();
}

void EEPROM_Write_Disable(){
    unsigned char var;
    EEPROM_SS_LOW();
    var = SPI_Write(EEPROM_CMD_WRDI);
    EEPROM_SS_HIGH();
}

union _EEPROMStatus_ EEPROM_Read_Status()
{
	unsigned char var;
    EEPROM_SS_LOW();
    var = SPI_Write(EEPROM_CMD_RDSR);
    var = SPI_Write(0);
    EEPROM_SS_HIGH();
    
    return (union _EEPROMStatus_)var;
}

void EEPROM_Write_Byte(unsigned char Data, unsigned long Address)
{
	unsigned char var;
    
    EEPROM_Write_Enable();
    
    EEPROM_SS_LOW();
    var = SPI_Write(EEPROM_CMD_WRITE);
    var = SPI_Write( (unsigned char)((Address>>16)&0x000000ff) );
    var = SPI_Write( (unsigned char)((Address>> 8)&0x000000ff) );
    var = SPI_Write( (unsigned char)((Address)&0x000000ff) );
    var = SPI_Write(Data);
    EEPROM_SS_HIGH();

    // wait for completion of previous write operation
    while(EEPROM_Read_Status().Bits.WIP);
    
    EEPROM_Write_Disable();
}

unsigned char EEPROM_Read_Byte(unsigned long Address)
{
	unsigned char var;

    EEPROM_SS_LOW();
    var = SPI_Write(EEPROM_CMD_READ);
    var = SPI_Write( (unsigned char)((Address>>16)&0x000000ff) );
    var = SPI_Write( (unsigned char)((Address>> 8)&0x000000ff) );
    var = SPI_Write( (unsigned char)((Address)&0x000000ff) );
    var = SPI_Write(0);
    EEPROM_SS_HIGH();
    
    return var;
}