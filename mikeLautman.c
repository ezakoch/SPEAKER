/* Eza
 * Created: 10/3/2013 9:09:34 PM
 *  Author: Cristina & Eza */

#define F_CPU 16000000

#include <avr/io.h>
#include "m_general.h"
#include <avr/interrupt.h>
#include "m_rf.h"
#include "m_bus.h"

//Variables
char buffer[3]={0,0,0};
volatile char freq_1=0, freq_2=0, duration=0;
volatile int i, x, y = 0;
volatile int freq_deciHz,length_of_play,new_freq = 0;
volatile float time, length_of_play_clock=0;

//LOOKUP TABLE
int wave[90 ] = {130, 139, 148, 156, 165, 173, 182, 190, 197, 205, 211, 218, 224, 230, 235, 239, 244, 247, 250, 252, 254, 255, 255, 255, 255, 253, 251, 249, 245, 242,	237,	232,	227,	221,	215,	208,	201,	193,	186,	178,	169,	161,	152,	143,	134,	125,	116,	107,	99,	90,	82,	73,	65,	58,	50,	44,	37,	31,	25,	20,	16,	11,8,	5,	3,	1,	0,	0,	0,	0,	2,	4,	6,	10	,	13,	18,	23,	28,	34,	40,	47,	54,	62,	69,	77,	86,	94,	103,	112,	121};

//Initialize Timer Initialization Subroutines
void timer_init1(void);
void timer_init3(void);

volatile int cycles=0;
volatile int durationCycle=100;
volatile int freq = 400; //SET DESIRED FREQUENCY HERE
int main(void)
{
	m_clockdivide(0); //16 MHz
	set(DDRB,6); //set B6 to output
	m_bus_init();
	m_rf_open(1,0x4C,3);//m_rf_open(channel,RXaddress,packet_length)
	m_green(ON);

	timer_init1(); //Inititalize Timers 1 and 3
	timer_init3();
	int index =0;
	while(1)
	{ 
		while(!check(TIFR3, OCF3A));
		set(TIFR3, OCF3A);
		m_green(2);
		index++ ;
		if (index >= 90){
			cycles++;
			index=0;
			if (cycles > durationCycle){
				cycles=0;
			}
		}
		int tmp = wave[index];
		if (tmp == 0 ) tmp=1;
		if (tmp == 256) tmp = 255;
		OCR1B=tmp;
	}
}

//Use Timer 1 for PWM control
void timer_init1(void){
	//TIMER 1
	clear(TCCR1B,CS12);	//divide by 8
	clear(TCCR1B,CS11);
	set(TCCR1B,CS10);

	set(TCCR1B,WGM13);	//mode15
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10);


	set(TCCR1A,COM1B1);	//clear at OCR1B, set at rollover
	clear(TCCR1A,COM1B0);

	OCR1A = 256;
	OCR1B = 255;
}

//Use Timer 3 to go through lookup table
void timer_init3(void){
	//TIMER 3
	//divideby1
	clear(TCCR3B,CS32);
	set(TCCR3B,CS31);
	clear(TCCR3B,CS30);
	//mode: up to OCR3A, mode 4
	clear(TCCR3B,WGM33);
	set(TCCR3B,WGM32);
	clear(TCCR3A,WGM31);
	clear(TCCR3A,WGM30);
}

//interrupt for wireless
ISR(INT2_vect)
{
	cli();
	m_red(TOGGLE);
	m_rf_read(buffer,3);
	freq = (buffer[0]<<8)+buffer[1];
	OCR3A=(int)(22222/500);
	durationCycle=buffer[2];
	sei();
} 
