/* Created: 10/24/2013
 Author: Eza I. Koch
 cd Desktop/M2/Speaker */

#define F_CPU 16000000
#define RANGE 1000
#define PRESCALAR 64
#define HI 256
#define CHANNEL 1
#define ADDRESS 0x18
#define packetLength 3

#include <avr/io.h>
#include "m_general.h"
#include <avr/interrupt.h>
#include "m_rf.h"
#include "m_bus.h"
#include "m_usb.h"

//Variables
char buffer[3]={0,0,0};
volatile int i=0; //elements of the wave
volatile char y=0; //Timer 0 cycles
volatile unsigned char duration;
volatile int deciFreq=0;

//LOOKUP TABLE: Sine wave from 0 to 1000 (RANGE)
int wave[32]={500,598,691,778,854,916,962,990,999,990,962,916,854,778,691,598,500,402,309,222,146,84,38,10,1,10,38,84,146,222,309,402};

// Subroutines
void init3(void){
    
    clear(TCCR3B,CS32); // Timer3/64=250kHz
    set(TCCR3B,CS31);
    set(TCCR3B,CS30);
    
    clear(TCCR3B,WGM33); //up to OCR3A
    set(TCCR3B,WGM32);
    clear(TCCR3A,WGM31);
    clear(TCCR3A,WGM30);
    
}//Use Timer 0 for duration
void init1(void){
    
	clear(TCCR1B,CS12);	//Timer1/1 = 16MHz
	clear(TCCR1B,CS11);
	set (TCCR1B,CS10);
    
	set(TCCR1B,WGM13);	//mode15 (UP to OCR1A, PWM mode)
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10);
    
	set(TCCR1A,COM1B1);	//clear at OCR1B, set at rollover
	clear(TCCR1A,COM1B0);
}//Use Timer 1 for PWM control
void startTimers(void){
    
    //Timer1/1 = 16MHz
    clear(TCCR1B,CS12);
	clear(TCCR1B,CS11);
	set (TCCR1B,CS10);
    
    // Timer3/64=250kHz
    clear(TCCR3B,CS32);
    set(TCCR3B,CS31);
    set(TCCR3B,CS30);
}
void stopTimers(void){
    //ensure timer 1 does not change
    clear(TCCR1A,COM1B1);
    clear(TCCR1A,COM1B0);
    
    //stop timer 1
    clear(TCCR1B,CS10);
    clear(TCCR1B,CS11);
    clear(TCCR1B,CS12);
    
    //stop timer 3
    clear(TCCR3B,CS30);
    clear(TCCR3B,CS31);
    clear(TCCR3B,CS32);
}

int main(void) //Prototype
{
    m_usb_init(); //enable usb comm
    //while(!m_usb_isconnected()); //wait unless usb is connected
    sei(); //enable interrupts
    m_clockdivide(0);//16MHz
    set(TIMSK3,OCIE3A); //enables Timer 0 interrupts
    set(TIMSK1,OCIE1A); //enables Timer 1 interrupts
    m_rf_open(CHANNEL,ADDRESS,packetLength); //open wireless comm
    set(DDRB,6); //Open the output
    OCR3A = F_CPU/(PRESCALAR*100); //about 1 centisecond
    
    //Initialize Timers
    init1();
    init3();
    
    while(1){}; // JUST CHIIIILLL HERE
}

// THREE INTERRUPTS TO CONTROL EVENTS

ISR(TIMER1_COMPA_vect)
{
    if (i<(sizeof(wave)/sizeof(int))) //don't exceed ends of vector... sizeof(wave)
    {
        OCR1B=(int)((float)wave[i]*OCR1A/RANGE); //go to next array element
        //OCR1B is a percentage of OCR1A
        i++; //increment
    }
    else i=0; //reset
}

ISR(INT2_vect) //receive new packet
{
    m_red(TOGGLE);
    m_rf_read(buffer,packetLength);
    startTimers(); //get em going
    //deciFreq = ((int)buffer[0] + (HI*(int)buffer[1])); //deciHz
    deciFreq = (*(int*)&buffer[0]); //deciHz
    //duration = (long)(15626.0*(float)buffer[2]/100.0); //csec to cycles
    duration = (unsigned char)buffer[2]; //centiseconds casted
    //OCR1A = (long)((float)F_CPU/(360*deciFreq));//freq cycles ... sizeof(wave)
    OCR1A = (long)((F_CPU/(sizeof(wave)/sizeof(int)))/(float)(deciFreq/10));//freq
    
    //m_usb_tx_int(duration); //print duration
    m_usb_tx_string("\n"); //new line
    
}

ISR(TIMER3_COMPA_vect) //whenever (TCNT0 matches OCR0A)
{
    if (((unsigned char)y)>((unsigned char)duration)) //check for duration
    {
        //stopTimers(); //turn off timers
        m_green(TOGGLE); //green
        clear(PORTB,6); //turn off output to speaker
    }
    else y++;
}