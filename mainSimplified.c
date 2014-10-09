/* L4a_csorice.c
 * Created: 10/3/2013 9:09:34 PM
 *  Author: Cristina & Eza */
// cd Desktop/M2/Speaker

#define F_CPU 16000000

#include <avr/io.h>
#include "m_general.h"
#include <avr/interrupt.h>
#include "m_rf.h"
#include "m_bus.h"

//Variables
char buffer[3]={0,0,0};
volatile int i,deciFreq,y,duration;

//LOOKUP TABLE
int wave[360] = {130, 132, 134, 136, 139, 141, 143, 145, 148, 150, 152, 154, 156, 158, 161, 163, 165, 167, 169, 171, 173, 175, 178, 180, 182, 184, 186, 188, 190, 192, 193, 195, 197, 199, 201, 203, 205, 206, 208, 210, 211, 213, 215, 216, 218, 220, 221, 223, 224, 226, 227, 228, 230, 231, 232, 234, 235, 236, 237, 238, 239, 241, 242, 243, 244, 244, 245, 246, 247, 248, 249, 249, 250, 251, 251, 252, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 256, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 252, 251, 251, 250, 249, 249, 248, 247, 246, 245, 244, 244, 243, 242, 241, 239, 238, 237, 236, 235, 234, 232, 231, 230, 228, 227, 226, 224, 223, 221, 220, 218,	216, 215, 213, 211,	210, 208, 206, 205, 203,	201, 199, 197, 195, 193, 192, 190, 188, 186,184, 182, 180, 178, 175, 173, 171, 169, 167, 165, 163, 161, 158,156, 154, 152, 150, 148, 145, 143, 141, 139, 136, 134, 132, 130,128, 125, 123, 121, 119, 116, 114, 112, 110, 107, 105, 103, 101,99,	97,	94,	92,	90,	88,	86,	84,	82,	80,	77,	75,	73,	71,	69,	67,	65,	64,	62,	60,	58,	56,	54,	52,	50,	49,	47,	45,	44,	42,	40,	39,	37,	35,	34,	32,	31,	29,	28,	27,	25,	24,	23,	21,	20,	19,	18,	17,	16,	14,	13,	12,	11,	11,	10,	9, 8, 7, 6,	6,	5,	4,	4,	3,	3,	2,	2,	1,	1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	2,	2,	3,	3,	4,	4,	5,	6,	6,	7,	8,	9,	10	,11,	11,	12,	13,	14,	16,	17,	18,	19,	20,	21,	23,	24,	25,	27,	28,	29,	31,	32,	34,	35,	37,	39,	40,	42,	44,	45,	47,	49,	50,	52,	54,	56,	58,	60,	62,	64,	65,	67,	69,	71,	73,	75,	77,	80,	82,	84,	86,	88,	90,	92,	94,	97,	99,	101,103,105,107,110,112,114,116,119,121,123,125,128};

// Subroutines
void init1(void);
//void init0(void);

int main(void) //Prototype
{
    //interrupt whenever (TCNT0 matches OCR0A), set the TIMSK0 : OCIE0A bit
    //set(TIMSK0,OCIE0A); //enables Timer 0 interrupts
    //m_rf_open(1,0x18,3);//m_rf_open(channel,RXaddress,packet_length)
    i=0;
    y=0;
    duration = 10;//centiseconds
    deciFreq=1000;//1kHz
//    OCR1A = ((int)F_CPU/(sizeof(wave)*deciFreq));//new freq in cycles
    OCR1A = 256;
    
    init1(); //Initialize Timers
    m_green(ON);
    //init0();
    
    while(1){
        m_red(TOGGLE);
        m_wait(100);
        i++; //increment
        if (i>=sizeof(wave))i=0;//reset
        OCR1B=wave[i]; //set OCR1B (for duty cycle)
    }
}


//ISR(TIMER0_COMPA_vect) //whenever (TCNT0 matches OCR0A)
//{
//    if (y>=duration) //check for duration
//    {
//        clear(DDRB,6); //turn off output to speaker
//        m_green(OFF);
//        y=0;
//    }
//    else
//    {
//        set(DDRB,6); //Open the output
//        m_green(ON);
//        y++; //continue to overflow
//    }
//}



////Use Timer 0 for duration
//void init0(void){
//    m_clockdivide(0); //16 MHz
//    OCR0A = 0; //counts/cycles
//    
//    set(TCCR0B,CS02); //divide timer0 by 1024=15626 Hz
//    clear(TCCR0B,CS01);
//    set(TCCR0B,CS00);
//    
//    clear(TCCR0B,WGM02); //up to OCROA
//    set(TCCR0A,WGM01);
//    clear(TCCR0A,WGM00);
//    
//    //8 bits: maximum representable value 2^8 − 1 = 255
//}

//Use Timer 1 for PWM control
void init1(void){
    m_clockdivide(0);//16MHz
    
	clear(TCCR1B,CS12);	//prescalar of 1 = 16MHz = FAST
	clear(TCCR1B,CS11);
	set (TCCR1B,CS10);
    
	set(TCCR1B,WGM13);	//mode15 (UP to OCR1A, PWM mode)
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10);
    
	set(TCCR1A,COM1B1);	//clear at OCR1B, set at rollover
	clear(TCCR1A,COM1B0);
        
    //16 bits: maximum representable value 2^16 − 1 = 65,535
}