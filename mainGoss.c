//Michael Gosselin

#include <avr/io.h>
#include "m_bus.h"
#include "m_general.h"
#include "m_rf.h"
#include <avr/interrupt.h>

#define CHANNEL 1
#define RXADDRESS 0x18
#define TXADDRESS 0x3C
#define PACKET_LENGTH 3

char buffer[PACKET_LENGTH] = {0,0,0};
volatile int n = 0;
volatile int i = 0;
volatile int j = 0;
volatile int freq = 0;

int Array[32]={500,598,691,778,854,916,962,990,999,990,962,916,854,778,691,598,500,402,309,222,146,84,38,10,1,10,38,84,146,222,309,402};

int main(void)
{
	
    sei(); //enable interrupts
    m_clockdivide(0); // 16 MHz
    m_bus_init(); // enable mBUS
    m_rf_open(CHANNEL,RXADDRESS,PACKET_LENGTH); // configure mRF
    //set the clock pre-scaler to make the clock very fast
    //set the clock counter
    OCR3A = 2500;
    //configure pin 6 for output
    set(DDRB,6);
    
    
    
    //set the mode of  timer 1
    set(TCCR1B,WGM13);
    set(TCCR1B,WGM12);
    set(TCCR1A,WGM11);
    set(TCCR1A,WGM10);
    //clear OCR1B at rollover
    set(TCCR1A,COM1B1);
    //timer 1 interrupt handle
    set(TIMSK1,OCIE1A);
    //set the mode of  timer 3
    set(TCCR3B,WGM32);
    //timer 3 interrupt handle
    set(TIMSK3,OCIE3A);
    
    while(1)
    {
//        n=100;
//        i=0;
    }
	
}

ISR(TIMER1_COMPA_vect)
{
	
	//check if the sine wave is at the end of it's period
	if(j<32)
	{
		//get the new value for OCR1B, effectively varying the duty cycle
		OCR1B = (int)((float)Array[j]*OCR1A/1000);
		//increment counter variable
		j++;
	}
	else //start a new period of the sine wave
	{
		//reset the counter variable
		j=0;
	}
}

ISR(INT2_vect)
{
	m_red(TOGGLE);
	m_rf_read(buffer,PACKET_LENGTH); // pull the packet
	
	//start clock 3 at 1/64 scale
	//set(TCCR3B,CS30);
	set(TCCR3B,CS02);
	
	//start clock 1 at 1/1 scale
	set(TCCR1A,COM1B1);
	set(TCCR1B,CS10);
    
	freq = *(int*)&buffer[0];//calculate OCR1A based on the value coming in on buffer's 0th and 1st elements
	OCR1A = (int)(5000000/(float)freq);
	n=(unsigned int)buffer[2];	//get the number of times to continue playing the note from buffer's 2nd element, store in variable n
}

ISR(TIMER3_COMPA_vect)
{
	
	m_red(TOGGLE);
    //check whether the note has already played for enough consecutive times or not
	if(i>n)
	{
        
        //ensure timer 3 does not change
        clear(TCCR1A,COM1B1);
        clear(TCCR1A,COM1B0);
        
        clear(PORTB,6);
        
        //stop timer 1
        clear(TCCR1B,CS10);
        clear(TCCR1B,CS11);
        clear(TCCR1B,CS12);
        
        //stop timer 3
        clear(TCCR3B,CS30);
        clear(TCCR3B,CS31);
        clear(TCCR3B,CS32);
        
        //reset counter variable
        clear(PORTB,6);
        
        i=0;
        
	}
	
	else i++;
}
