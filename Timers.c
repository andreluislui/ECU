/*
 * File:   Timer.c
 * Author: Andre
 *
 * Created on 21 de Abril de 2016, 17:48
 */

#include "Timers.h"

void InitTimer(){
    
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

    //Timer3
    // Period = PR1 * prescaler * Tcy
    T3CONbits.TCS       = 0;    //Oscilador interno
    T3CONbits.TCKPS     = 3;    //Prescale
    PR3 = 7839;
    //Interrupcao Timer3
    IFS0bits.T3IF       = 0;    //Reset Flag
    IEC0bits.T3IE       = 1;    //Habilita a interrupcao
    T3CONbits.TON       = 1;    //Timer Ligado
    
}
