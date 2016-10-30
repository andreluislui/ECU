/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <xc.h> // include processor files - each processor file is guarded.  
#include "Portas.h"

//EEPROM Comands
#define EEPROM_PAGE_SIZE    (unsigned)64
#define EEPROM_PAGE_MASK    (unsigned)0x003f
#define EEPROM_CMD_READ     (unsigned)0b00000011 //
#define EEPROM_CMD_WRITE    (unsigned)0b00000010 //
#define EEPROM_CMD_WRDI     (unsigned)0b00000100 //
#define EEPROM_CMD_WREN     (unsigned)0b00000110 //
#define EEPROM_CMD_RDSR     (unsigned)0b00000101 //
#define EEPROM_CMD_WRSR     (unsigned)0b00000001 //

struct  STATREG{
	unsigned    WIP:1;
	unsigned    WEL:1;
	unsigned    BP0:1;
	unsigned    BP1:1;
	unsigned    RESERVED:3;
	unsigned    WPEN:1;
};

union _EEPROMStatus_{
	struct  STATREG Bits;
	unsigned char	Char;
};

void __attribute__((__interrupt__, __auto_psv__)) _SPI1Interrupt();

void InitSPI();

void EEPROM_SS_HIGH();

void EEPROM_SS_LOW();

unsigned char SPI_Write(unsigned int data);

void EEPROM_Write_Enable();

void EEPROM_Write_Disable();

union _EEPROMStatus_ EEPROM_Read_Status();

void EEPROM_Write_Byte(unsigned char Data, unsigned long Address);

unsigned char EEPROM_Read_Byte(unsigned long Address);

// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

