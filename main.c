/* 
 * File:   main.c
 * Author: André
 *
 * Created on 9 de Fevereiro de 2015, 16:32
 * 
 * To Do List:
 * - Mudar a variável de contagem de tempo de u16 para u32 (unsigned long)
 * - Determinar onde o hardware falha para determinar o momento onde o motor desliga
 */

//Teste

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "Portas.h"
#include "Serial.h"
#include "Timers.h"
#include "SPI.h"

//VARIAVEIS DE CONTROLE DE INJETOR E CENTELHA
unsigned int u16_ctrl_tempoentredentes = 0;         //Tempo entre dentes (10us)
unsigned int u16_ctrl_tempoanterior_cfalha = 0;     //Tempo entre dentes qualquer (10us)
unsigned int u16_ctrl_tempovolta = 0;               //Contagem do tempo da volta atual
                                                    //Varia com o tempo (10us)
unsigned int u16_ctrl_tempoanterior_volta = 0;      //Contagem do tempo da volta anterior 
                                                    //Fixo no tempo (10us)
unsigned char u08_ctrl_motorligado = 0;             //Variavel indica se o motor está ligado

//PARAMETROS DE CONTROLE DE INJETOR E CENTELHA
unsigned char u08_prmt_tempoinjecao = 1;            //Tempo de injecao
unsigned int  u16_prmt_faseinjecao = 2;             //Fase do termino da injecao
unsigned char u08_prmt_dwellignicao = 3;            //Tempo de carga da bobina de ignicao
int s16_prmt_avancoignicao = 4;                     //Posicao do disparo da centelha

//VARIAVEIS TELEMETRIA
float f32_tlmt_rpm = 0;

//VARIAVEIS BUFFERS ADC 
unsigned int u16_adc_iat = 0;           //AN0
unsigned int u16_adc_ect = 0;           //AN1
unsigned int u16_adc_tps = 0;           //AN3
unsigned int u16_adc_map = 0;           //AN4
unsigned int u16_adc_lbd = 0;           //AN5
unsigned int u16_adc_bat = 0;           //AN8
unsigned int u16_adc_tec = 0;           //AN12

//FLAGS DE CONTROLE
unsigned char u08_flg_modorecebe = 0;
unsigned char u08_cnt_modorecebe = 0;

void SalvarDadosEEPROM(){
    unsigned char aux;
    EEPROM_Write_Byte(u08_prmt_tempoinjecao, 0x000);
    aux = (u16_prmt_faseinjecao>>8) & 0xFF;
    EEPROM_Write_Byte(aux, 0x001);
    EEPROM_Write_Byte((unsigned char) u16_prmt_faseinjecao, 0x002);
    EEPROM_Write_Byte(u08_prmt_dwellignicao, 0x003);
    aux = (s16_prmt_avancoignicao>>8) & 0xFF;
    EEPROM_Write_Byte(aux, 0x004);
    EEPROM_Write_Byte((unsigned char) s16_prmt_avancoignicao, 0x005);
}

void LerDadosEEPROM(){
    //Leitura dos dados da EEPROM
    //Endereços
    u08_prmt_tempoinjecao = EEPROM_Read_Byte(0x000);
    u16_prmt_faseinjecao = EEPROM_Read_Byte(0x001);
    u16_prmt_faseinjecao = u16_prmt_faseinjecao<<8;
    u16_prmt_faseinjecao = u16_prmt_faseinjecao | EEPROM_Read_Byte(0x002);
    u08_prmt_dwellignicao = EEPROM_Read_Byte(0x003);
    s16_prmt_avancoignicao = EEPROM_Read_Byte(0x004);
    s16_prmt_avancoignicao = s16_prmt_avancoignicao<<8;
    s16_prmt_avancoignicao = s16_prmt_avancoignicao | EEPROM_Read_Byte(0x005);
}

void EnviaDadosUSB(){
    //Dados para o programa do PC 
            WriteUART1_U16( (unsigned int)f32_tlmt_rpm );
            WriteUART1_U08(u08_prmt_tempoinjecao);
            WriteUART1_U16(u16_prmt_faseinjecao);
            WriteUART1_U08(u08_prmt_dwellignicao);
            WriteUART1_U16(s16_prmt_avancoignicao);
            WriteUART1_U16(u16_adc_iat);
            WriteUART1_U16(u16_adc_ect);
            WriteUART1_U16(u16_adc_tps);
            WriteUART1_U16(u16_adc_map);
            WriteUART1_U16(u16_adc_lbd);
            WriteUART1_U16(u16_adc_bat);
            WriteUART1_U16(u16_adc_tec);
}

/**
 * @Descricao Função responsáel pelo funcionamento do injetor de combustíveis. 
 * Função gerará uma onda quadrada com inicio no grau expresso no parâmetro "liga"
 * e término no grau expresso no parâmetro "desliga".  
 * @Parametro liga (graus) Tipo: unsigned int | Dados: 0 - 360 | Resol.: 0..1 
 * @Parametro desliga (graus) Tipo: unsigned int | Dados: 0 - 360 | Resol.: 0..1
 */
void injetor_posicao(unsigned int liga, unsigned int desliga){

    float   aux_liga = ((float)u16_ctrl_tempoanterior_volta/360)*((float)liga), 
            aux_desliga = ((float)u16_ctrl_tempoanterior_volta/360)*((float)desliga);
    
    // Pulso se mantém dentro de uma volta 
    // 0 -------- Liga ----------- Desliga ------- 360/0
    if(aux_liga <= aux_desliga){
        if( (u16_ctrl_tempovolta >= aux_liga) && (u16_ctrl_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = INJ_ON;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = INJ_OFF;
    }
    //Pulso começa em uma volta e acaba em outra
    // 0 ------- Liga ------- 360/0 ------- Desliga ------ 
    else{
        if( (u16_ctrl_tempovolta >= aux_liga) || (u16_ctrl_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = INJ_ON;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = INJ_OFF;
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
    
    aux_desliga = ((float)u16_ctrl_tempoanterior_volta/360)*((float)fase);
    aux_liga = u16_ctrl_tempoanterior_volta - ( ((unsigned int)tempo)*100 - aux_desliga);
    aux_tempo = tempo*100;

    // Pulso se mantém dentro de uma volta 
    // 0 -------- Liga ----------- Desliga ------- 360/0
    if(aux_tempo <= aux_desliga){
        //Posicao do "Liga"
        aux_liga = aux_desliga - ( ((unsigned int)tempo)*100 );
        if( (u16_ctrl_tempovolta >= aux_liga) && (u16_ctrl_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = INJ_ON;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = INJ_OFF;
    }
    //Pulso começa em uma volta e acaba em outra
    // 0 ------- Liga ------- 360/0 ------- Desliga ------ 
    else{
        //Posicao do "Liga"
        aux_liga = u16_ctrl_tempoanterior_volta - ( ((unsigned int)tempo)*100 - aux_desliga);
        if( (u16_ctrl_tempovolta >= aux_liga) || (u16_ctrl_tempovolta < aux_desliga) )
            //PORTDbits.RD1 = 0;
            INJ_1 = INJ_ON;
        else 
            //PORTDbits.RD1 = 1;
            INJ_1 = INJ_OFF;
    }

}

//TESTAR!!!
void ignicao_avanco(unsigned char dwell, int avanco){
    
    float           aux_liga = 0, 
                    aux_desliga = 0;
    unsigned int    aux_tempo = 0, 
                    aux_avanco = 0;
    
    if(avanco >= 0)
        aux_avanco = 360 - avanco;
    else 
        aux_avanco = abs(avanco);
        
    aux_desliga = ((float)u16_ctrl_tempoanterior_volta/360)*((float)aux_avanco);
    aux_liga = u16_ctrl_tempoanterior_volta - ( ((unsigned int)dwell)*100 - aux_desliga);
    aux_tempo = dwell*100;

    // Pulso se mantém dentro de uma volta 
    // 0 -------- Liga ----------- Desliga ------- 360/0
    if(aux_tempo <= aux_desliga){
        //Posicao do "Liga"
        aux_liga = aux_desliga - ( ((int)dwell)*100 );
        if( (u16_ctrl_tempovolta >= aux_liga) && (u16_ctrl_tempovolta < aux_desliga) )
            IGN_1 = IGN_OFF;
        else 
            IGN_1 = IGN_ON;
    }
    //Pulso começa em uma volta e acaba em outra
    // 0 ------- Liga ------- 360/0 ------- Desliga ------ 
    else{
        //Posicao do "Liga"
        aux_liga = u16_ctrl_tempoanterior_volta - ( ((unsigned int)dwell)*100 - aux_desliga);
        if( (u16_ctrl_tempovolta >= aux_liga) || (u16_ctrl_tempovolta < aux_desliga) )
            IGN_1 = IGN_OFF;
        else 
            IGN_1 = IGN_ON;
    }
}

/**
 * @Descricao INTERRUPÇÂO DO CONVERSOR AD
 * Leitura das portas analógicas ocorre em um esquema de varredura. 
 * As portas setadas como analógicas, periodicamente são varridas e seus valores
 * armazenados nos buffer's ADCBUF0..ADCBUFF. Ao fim dessa varredura, essa 
 * interrupção ocorre. 
 */
void __attribute__((__interrupt__, __auto_psv__)) _ADCInterrupt(void)
{
    IFS0bits.ADIF = 0;
    
    u16_adc_iat = ADCBUF0;
    u16_adc_ect = ADCBUF1;
    u16_adc_tps = ADCBUF3;
    u16_adc_map = ADCBUF4;
    u16_adc_lbd = ADCBUF5;
    u16_adc_bat = ADCBUF8;
    u16_adc_tec = ADCBUFC;

}

/**
 * @Descricao INTERRUPÇÂO DA INTERFACE SERIAL 1
 * A interrupção ocorre quando a interface serial recebe algum dado. 
 * Valor lido é obtido por meio do registrador U1RXREG.
 */
void __attribute__((__interrupt__, __auto_psv__)) _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = 0;
    
    if(! u08_flg_modorecebe){
        switch (U1RXREG){
        case 'D':
            EnviaDadosUSB();
            break; 
        case 'U':
            //Atualiza parametros via PC
            u08_flg_modorecebe = 1;
            u08_cnt_modorecebe = 0;
            break;
        case 'E':
            //Grava parametros na EEPROM
            SalvarDadosEEPROM();
            break;
        default:
            break;
        }          
    }
    else 
    {
        switch (u08_cnt_modorecebe){
            case 0:
                u08_prmt_tempoinjecao = U1RXREG;
                u08_cnt_modorecebe++;
                break;
            case 1:
                u16_prmt_faseinjecao = U1RXREG;
                u16_prmt_faseinjecao = u16_prmt_faseinjecao<<8;
                u08_cnt_modorecebe++;
                break;
            case 2:
                u16_prmt_faseinjecao = u16_prmt_faseinjecao | U1RXREG;
                u08_cnt_modorecebe++;
                break;
            case 3:
                u08_prmt_dwellignicao = U1RXREG;
                u08_cnt_modorecebe++;
                break;
            case 4:
                s16_prmt_avancoignicao = U1RXREG;
                s16_prmt_avancoignicao = s16_prmt_avancoignicao<<8;
                u08_cnt_modorecebe++;
                break;
            case 5:
                s16_prmt_avancoignicao = s16_prmt_avancoignicao | U1RXREG;
                u08_cnt_modorecebe++;
                break;
        }
        
        if(u08_cnt_modorecebe == 6)
        {
            u08_flg_modorecebe = 0;
            WriteUART1_U08('O');
            WriteUART1_U08('K');
            WriteUART1_U08('!');
        }           
    } 
}

/**
 * @Descricao INTERRUPÇÂO DE TIMER 1
 * A interrupção de Timer 1 está setada para ocorrer com intervalo de 100ms.
 */
void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    LED_AZ = !LED_AZ;
    
    //Envia dados
    //EnviaDadosUSB();
    
}

/**
 * @Descricao INTERRUPÇÂO DE TIMER 2
 * A interrupção de Timer 2 está setada para ocorrer com intervalo de 10us.
 * Nesse momento ocorre o incremento das variáveis que contam o tempo entre
 * dentes da roda fônica e o tempo de uma volta. 
 */
void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    
    //Conta tempo entre as interrupcoes
    u16_ctrl_tempoentredentes ++;
    u16_ctrl_tempovolta ++; 
}

/**
 * @Descricao INTERRUPÇÂO DE TIMER 3
 * A interrupção de Timer 3 está setada para ocorrer com intervalo de 100ms.
 * Nesse momento ocorre a atribuição de False para a flag de motor ligado. 
 * Essa variavel é setada para True quando ocorre a interrupção externa 1.
 */
void __attribute__((__interrupt__, __auto_psv__)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;
    LED_BR = !LED_BR;
    u08_ctrl_motorligado = 0;
}

/**
 * @Descricao INTERRUPÇÂO EXTERNA 1
 * Por meio de um sensor indutivo, fixado próximo à roda-fonica, ocorre 
 * a interrupção por meio de pulsos externos. 
 * Ocorre a atualização de variáveis. 
 */
void __attribute__((__interrupt__, __auto_psv__)) _INT0Interrupt(void)
{
    IFS0bits.INT0IF = 0;
    
    //Verifica se a falha foi encontrada 
    if(u16_ctrl_tempoentredentes > (2*u16_ctrl_tempoanterior_cfalha) )
    {
        //LED_BR = !LED_BR;
        u16_ctrl_tempoanterior_volta = u16_ctrl_tempovolta;
        u16_ctrl_tempovolta = 0;
    }
    
    u16_ctrl_tempoanterior_cfalha = u16_ctrl_tempoentredentes;
    u16_ctrl_tempoentredentes = 0;
    
    //Verificação se o motor está ligado 
    u08_ctrl_motorligado = 1;
    //TMR3 = 7839;             //Não deixa o timer disparar
    TMR3 = 0;             //Não deixa o timer disparar
    
}

int main(int argc, char** argv) {

    //Ports I/O
    InitPortas();
    
    //Timer1
    InitTimer();
    
    //UART 1
    InitSerial();
    
    //SPI - Testar
    InitSPI();

    //Interrupcao Externa 1
    INTCON2bits.INT0EP  = 1;    //Borda de descida
    IEC0bits.INT0IE     = 1;    //Interrupção está Habilitada
    IFS0bits.INT0IF     = 0;    //Reset Flag
    
    ////UART 1
    //U1MODEbits.UARTEN   = 1;    //Habilita UART1
    //U1MODEbits.ALTIO    = 1;    //Porta alternativa
    //U1STAbits.UTXEN     = 1;    //Habilita transmissoes
    //U1BRG = 10;                 //Para BaudRate 115200
    ////Interrupcao Uart1
    //IFS0bits.U1RXIF     = 0;    //Reset Flag
    //IEC0bits.U1RXIE     = 1;    //Interrupcao habilitada

    //ADC
    ADCON3bits.SAMC     = 15;   //Temporizacao da conversao
    ADCON3bits.ADCS     = 15;
    ADCON2bits.CSCNA    = 1;    //Scanear as portas analogicas
    ADCON2bits.SMPI     = 0b1111; //16 conversoes por interrupcao
    ADCON1bits.SSRC     = 0b111;//Modo de conversao automatico (Amostra -> Conversao)
    ADCON1bits.ASAM     = 1;    //Metodo acima contínuo
    ADCON1bits.SAMP     = 1;    //Inicia as conversoes
    //ADPCFG              = 0b1111110111111111;
    //ADPCFG              = 0b1111111111111111;    //-> Colocar 0 nas entradas analógicas   //Portas.c
    ADCSSL              = 0b1111111111111111;
    ADCON1bits.ADON     = 1;    //Ativa o conversor
    //Interrupcao ADC
    IFS0bits.ADIF       = 0;    //Reset flag
    IEC0bits.ADIE       = 1;    //Habilita interrupcao
    
    LerDadosEEPROM();
    
    while(1){
        
        if(u08_ctrl_motorligado){
            //void injetor_tempo(unsigned char tempo, unsigned int fase)
            //(10, 50) -> Passa pelo zero
            //(5, 90)  -> Fora do Zero
            injetor_tempo(5, 90);
            
            //void ignicao_avanco(unsigned char dwell, int avanco)
            //(10,50)  -> Avanço positivo
            //(10, -50)-> Carga no zero 
            //(5, -100)-> Carga após zero 
            ignicao_avanco(5, -100);
            
            //Calculo de parametros;
            //f32_tlmt_rpm = (6000000/u16_ctrl_tempoanterior_volta);     
        }
        else{
            INJ_1 = INJ_OFF;
            IGN_1 = IGN_OFF;
            u16_ctrl_tempoentredentes = 0;
            u16_ctrl_tempoanterior_cfalha = 0;    
            u16_ctrl_tempovolta = 0;
            u16_ctrl_tempoanterior_volta = 0;                  
            u08_ctrl_motorligado = 0;
            
            f32_tlmt_rpm = 0;
        }

    }

    return (EXIT_SUCCESS);
}