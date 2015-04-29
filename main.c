//test
//counter on PIC with LCD 8x2


#include "main.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
void conv_pulse(void) //single pulse for dc-converter
{
CONV_H;
__delay_us(10);
CONV_L;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void system_calc(void) //calculation
{
uint16_t temp=0;
for(uint8_t i=0; i<GEIGER_TIME; i++) temp+=pulsBuff[i]; //calculation the dose rate
//if(temp>=19125) temp=19125; //overflow
doseRate=temp;

for(uint8_t k=GEIGER_TIME-1; k>0; k--) pulsBuff[k]=pulsBuff[k-1]; //shift array
pulsBuff[0]=0; //clear old data

doseTot=(pulsTot*GEIGER_TIME/3600); //dose

if(++timesec>59) //seconds
	{
	if(++timemin>59) //minutes
		{
		if(++timehrs>23) timehrs=0; //hours
		timemin=0;
		}
	timesec=0;
	}

SCR_REFR_SET; //set the flag for screen update
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void interrupt handler(void)
{
if(INT2IF) // external interrupt 2  /*** pulse from Geiger detector ***/
	{
	INT2IF=0;

	if(SYS_READY)
		{
		if(pulsBuff[0]!=255) pulsBuff[0]++; //counting pulses
		if(++pulsTot>999999UL*3600/GEIGER_TIME) pulsTot=999999UL*3600/GEIGER_TIME;
		conv_pulse(); //voltage boost
		}
	}

if (TMR0IF) //the timer 0 overflow interrupt  /*** general purpose timer 1kHz ***/
	{
	TMR0IF=0;
	TMR0=TMR0_LOAD; //preload for overflow 1 kHz

	static uint16_t cntCon=0;
	static uint16_t cntSys=0;

	if(++cntCon>=(1000/convFreq))  /*** pulses for DC-DC converter ***/
		{
		cntCon=0;
		conv_pulse(); //single pulse
		}

	if(++cntSys>=1000)  /*** 1Hz measurement time ***/
		{
		cntSys=0;
		if(SYS_READY) system_calc(); //calculation of measured values
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void delay_ms(uint16_t val)  //delay milliseconds (correct only for Fosc 4MHz or 8MHz)
{
while(val--) __delay_ms(1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void system_screen(void)
{
if(doseRate<1000) sprintf(strBuff,"%3u uR/h",doseRate); //"XXX uR/h"
else sprintf(strBuff,"%2u.%1umR/h",doseRate/1000,(doseRate%1000)/100); //"XX.X mR/h"

lcd_goto(0,0);
lcd_print(strBuff);

lcd_goto(1,0);
sprintf(strBuff,"%02u:%02u:%02u",timehrs,timemin,timesec);
lcd_print(strBuff);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void system_init(void)
{
IRCF2=1; IRCF1=1; IRCF0=1; //8MHz internal RC oscillator

ADCON1=0b1111; //disable analog inputs

LED_PIN; //LED indicator control pin
LED_RED; //

SYS_READY_CLR; //clear the "system ready" flag

CONV_PIN; //dc-converter control pin as output
CONV_L;

RBPU=0; //pull-up enable

/*** external interrupt 2 settigs ***/
INTEDG2=0; //on falling edge
INT2IE=1; //enable

/*** timer 0 settigs ***/
//TMR0ON//T08BIT//T0CS//T0SE//PSA//T0PS2:T0PS0
T0CON=0b10010011; //prescaler 011 - 1:16  Fosc/4=2000, 2000/16=125kHz
TMR0=TMR0_LOAD; //preload
TMR0IE=1; //timer overflow intterrupt enable

PEIE=1; //peripheral interrupt enable
GIE = 1; //global interrupts enable

lcd_init();

lcd_goto(0,0); //hi there
lcd_print("Wait a");
lcd_goto(1,0);
lcd_print("moment");

convFreq=1000; //voltage boost 1000kHz
delay_ms(2000); //wait 2 sec
convFreq=CONV_FREQ;

lcd_clear();

BL_ON; //LCD backlight
LED_GREEN;

SCR_REFR_SET;
SYS_READY_SET;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void main(void)
{
system_init();

for(;;)
	{
	if(SCR_REFR)
		{
		system_screen();
		SCR_REFR_CLR;
		}
	}
}
