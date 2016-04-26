/* 
 * File:   main.c
 * Author: André
 *
 * Created on 9 de Fevereiro de 2015, 16:32
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#define LED_BR PORTBbits.RB10
#define LED_AZ PORTBbits.RB9
#define INJ_1 PORTDbits.RD9

/*
 * 
 */

unsigned int u16_rpm = 0;
unsigned int u16_tempoentredentes = 0;
unsigned int u16_tempovolta = 0;
unsigned int u16_tempoanterior_cfalha = 0;
unsigned int u16_contador = 0;
unsigned int u16_tempoanterior_volta = 0;

void WriteUART1_U08(unsigned char data){

    while (U1STAbits.TRMT==0);      //Espera desocupar o registrador
        U1TXREG = data;
}

void WriteUART1_U16(unsigned int data){
    while (U1STAbits.TRMT==0);      //Espera desocupar o registrador
        U1TXREG = data;
    while (U1STAbits.TRMT==0);      //Espera desocupar o registrador
        U1TXREG = data>>8;
}

/**
 * @Descricao Função responsáel pelo funcionamento do injetor de combustíveis. 
 * Função gerará uma onda quadrada com inicio no grau expresso no parâmetro "liga"
 * e término no grau expresso no parâmetro "desliga".  
 * @Parametro liga (graus) Tipo: unsigned int | Dados: 0 - 360 | Resol.: 0..1 
 * @Parametro desliga (graus) Tipo: unsigned int | Dados: 0 - 360 | Resol.: 0..1
 */
void injetor_posicao(unsigned int liga, unsigned int desliga){

    float   aux_liga = ((float)u16_tempoanterior_volta/360)*((float)liga), 
            aux_desliga = ((float)u16_tempoanterior_volta/360)*((float)desliga);
    
    // Pulso se mantém dentro de uma volta 
    // 0 -------- Liga ----------- Desliga ------- 360/0
    if(aux_liga <= aux_desliga){
        if( (u16_tempovolta >= aux_liga) && (u16_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = 0;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = 1;
    }
    //Pulso começa em uma volta e acaba em outra
    // 0 ------- Liga ------- 360/0 ------- Desliga ------ 
    else{
        if( (u16_tempovolta >= aux_liga) || (u16_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = 0;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = 1;
    }    
}

/**
 * @Descricao Função responsável pelo funcionamento do injetor de combustíveis.
 * Função gerará o pulso do injetor com fim no grau expresso no parâmetro "fase"
 * e possuir duração expressa pela variável "tempo"
 * @Parametro tempo (ms) Tipo: unsigned char | Dados: 0 - inf+ | Resol.: 0..1
 * @Parametro fase (graus) Tipo: unsigned int | Dados: 0 - 360 | Resol.: 0..1
 */
void injetor_tempo(unsigned char tempo, unsigned int fase){
    
    float           aux_liga = 0, 
                    aux_desliga = 0;
    unsigned int    aux_tempo = 0;
    
    aux_desliga = ((float)u16_tempoanterior_volta/360)*((float)fase);
    aux_liga = u16_tempoanterior_volta - ( ((unsigned int)tempo)*100 - aux_desliga);
    aux_tempo = tempo*100;

    // Pulso se mantém dentro de uma volta 
    // 0 -------- Liga ----------- Desliga ------- 360/0
    if(aux_tempo <= aux_desliga){
        //Posicao do "Liga"
        aux_liga = aux_desliga - ( ((unsigned int)tempo)*100 );
        if( (u16_tempovolta >= aux_liga) && (u16_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = 0;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = 1;
    }
    //Pulso começa em uma volta e acaba em outra
    // 0 ------- Liga ------- 360/0 ------- Desliga ------ 
    else{
        //Posicao do "Liga"
        aux_liga = u16_tempoanterior_volta - ( ((unsigned int)tempo)*100 - aux_desliga);
        if( (u16_tempovolta >= aux_liga) || (u16_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = 0;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = 1;
    }

}

//TESTAR!!!
void ignicao(unsigned char dwell, int avanco){
    
    float           aux_liga = 0, 
                    aux_desliga = 0;
    unsigned int    aux_tempo = 0, 
                    aux_avanco = 0;
    
    aux_avanco = 360 - avanco;
    
    aux_desliga = ((float)u16_tempoanterior_volta/360)*((float)aux_avanco);
    aux_liga = u16_tempoanterior_volta - ( ((unsigned int)dwell)*100 - aux_desliga);
    aux_tempo = dwell*100;

    // Pulso se mantém dentro de uma volta 
    // 0 -------- Liga ----------- Desliga ------- 360/0
    if(aux_tempo <= aux_desliga){
        //Posicao do "Liga"
        aux_liga = aux_desliga - ( ((int)dwell)*100 );
        if( (u16_tempovolta >= aux_liga) && (u16_tempovolta < aux_desliga) )
            PORTBbits.RB12 = 1;
        else 
            PORTBbits.RB12 = 0;
    }
    //Pulso começa em uma volta e acaba em outra
    // 0 ------- Liga ------- 360/0 ------- Desliga ------ 
    else{
        //Posicao do "Liga"
        aux_liga = u16_tempoanterior_volta - ( ((unsigned int)dwell)*100 - aux_desliga);
        if( (u16_tempovolta >= aux_liga) || (u16_tempovolta < aux_desliga) )
            PORTBbits.RB12 = 1;
        else 
            PORTBbits.RB12 = 0;
    }
}

//Interrupcao ADC
void __attribute__((__interrupt__, __auto_psv__)) _ADCInterrupt(void)
{
    IFS0bits.ADIF = 0;

    //WriteUART1(ADCBUF0);
    //Até...
    //WriteUART1(ADCBUFF);

}

//Interrupcao UART1
void __attribute__((__interrupt__, __auto_psv__)) _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = 0;
    WriteUART1_U08(U1RXREG);
}

//Interrupcao timer1 (100ms)
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;

    //PORTDbits.RD0 = !PORTDbits.RD0;
    
    LED_AZ = !LED_AZ;
    //LED_BR = !LED_BR;
    
    //Final do injetor
    if (u16_contador < 360)
        u16_contador++;
    else
        u16_contador = 0;

}

//Interrupcao timer2 (10us)
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;

    //Conta tempo entre as interrupcoes
    u16_tempoentredentes ++;
    u16_tempovolta ++;
    

    
}

//Interrupcao externa INT0
void __attribute__((__interrupt__, __auto_psv__)) _INT0Interrupt(void)
{
    IFS0bits.INT0IF = 0;
    
    if(u16_tempoentredentes > (2*u16_tempoanterior_cfalha) )
    {   
        //Achou a falha
        LED_BR = !LED_BR;
        u16_tempoanterior_volta = u16_tempovolta;
        u16_tempovolta = 0;
    }
    
    u16_tempoanterior_cfalha = u16_tempoentredentes;

    u16_tempoentredentes = 0;

}

int main(int argc, char** argv) {

    //Ports I/O
    //LEDS
    TRISBbits.TRISB10 = 0;
    LED_BR = 1;
    TRISBbits.TRISB9 = 0;
    LED_AZ = 1;
    TRISDbits.TRISD9 = 0;
    INJ_1 = 1;
    

    //Interrupcao Externa 1
    INTCON2bits.INT0EP  = 1;    //Borda de descida
    IEC0bits.INT0IE     = 1;    //Interrupção está Habilitada
    IFS0bits.INT0IF     = 0;    //Reset Flag

    //Timer1
    // Period = PR1 * prescaler * Tcy
    T1CONbits.TCS       = 0;    //Oscilador interno
    T1CONbits.TCKPS     = 3;    //Preescale
    PR1 = 7839;
    //PR1 = 15678;
    //Interrupcao Timer1
    IFS0bits.T1IF       = 0;    //Reset Flag
    IEC0bits.T1IE       = 1;    //Interrupcao Habilitada
    T1CONbits.TON       = 1;    //Timer Ligado

    //Timer2
    // Period = PR1 * prescaler * Tcy
    T2CONbits.TCS       = 0;    //Oscilador interno
    T2CONbits.TCKPS     = 0;    //Prescale
    PR2 = 200;
    //Interrupcao Timer2
    IFS0bits.T2IF       = 0;    //Reset Flag
    IEC0bits.T2IE       = 1;    //Habilita a interrupcao
    T2CONbits.TON       = 1;    //Timer Ligado

    //UART 1
    U1MODEbits.UARTEN   = 1;    //Habilita UART1
    U1MODEbits.ALTIO    = 1;    //Porta alternativa
    U1STAbits.UTXEN     = 1;    //Habilita transmissoes
    U1BRG = 10;                 //Para BaudRate 115200
    //Interrupcao Uart1
    IFS0bits.U1RXIF     = 0;    //Reset Flag
    IEC0bits.U1RXIE     = 1;    //Interrupcao habilitada

     
    //ADC
    ADCON3bits.SAMC     = 15;   //Temporizacao da conversao
    ADCON3bits.ADCS     = 15;
    ADCON2bits.CSCNA    = 1;    //Scanear as portas analogicas
    ADCON2bits.SMPI     = 0b1111; //16 conversoes por interrupcao
    ADCON1bits.SSRC     = 0b111;//Modo de conversao automatico (Amostra -> Conversao)
    ADCON1bits.ASAM     = 1;    //Metodo acima contínuo
    ADCON1bits.SAMP     = 1;    //Inicia as conversoes
    //ADPCFG              = 0b1111110111111111;
    ADPCFG              = 0b1111111111111111;    //-> Colocar 0 nas entradas analógicas
    ADCSSL              = 0b1111111111111111;
    ADCON1bits.ADON     = 1;    //Ativa o conversor
    
    
    //Interrupcao ADC
    IFS0bits.ADIF       = 0;    //Reset flag
    IEC0bits.ADIE       = 1;    //Habilita interrupcao
    
    
    while(1){

        //injetor_posicao(10, 50);
        injetor_tempo(10, 50);
        //injetor_tempo(10, 180);
        //ignicao(1, 20); 

    }

    return (EXIT_SUCCESS);
}