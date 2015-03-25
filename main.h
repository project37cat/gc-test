
#define _XTAL_FREQ 8000000

#include <htc.h>
#include <stdio.h>

#include "lcd.h"


__CONFIG(1, RCIO );
__CONFIG(2, BORDIS & WDTDIS );


typedef unsigned int uint16_t; //stdint
typedef unsigned char uint8_t;
typedef unsigned long int uint32_t;


//output pin for converter control
#define CONV_PIN TRISB3=0;
#define CONV_H RB3=1;
#define CONV_L RB3=0;

//bicolor LED
#define LED_PIN  { TRISC4=0; TRISC5=0; }
#define LED_GREEN  { RC5=0; RC4=1; }
#define LED_RED  { RC5=1; RC4=0; }


#define CONV_FREQ 5 //pulse frequency in the normal mode of DC-DC converter 
#define TMR0_LOAD 65413 //adjust the measurement time
#define GEIGER_TIME 75


uint8_t pulsBuff[GEIGER_TIME]; //array of pulses from Geiger detector

char strBuff[17]; //string buffer

uint16_t doseRate; //dose rate
uint32_t doseTot; //total dose
uint32_t pulsTot; //total sum of pulses

uint16_t convFreq=CONV_FREQ;

uint8_t timehrs,timemin,timesec;


#define BIT_IS_SET(reg, bit) ((reg>>bit)&1)

#define SET_BIT(reg, bit) (reg |= (1<<bit))
#define CLR_BIT(reg, bit) (reg &= (~(1<<bit)))
#define INV_BIT(reg, bit) (reg ^= (1<<bit))

uint8_t sysReg = 0b00000000;

#define SCR_REFR_SET  SET_BIT(sysReg,0)
#define SCR_REFR_CLR  CLR_BIT(sysReg,0)
#define SCR_REFR      BIT_IS_SET(sysReg,0)

#define SYS_READY_SET  SET_BIT(sysReg,1)
#define SYS_READY_CLR  CLR_BIT(sysReg,1)
#define SYS_READY      BIT_IS_SET(sysReg,1)
