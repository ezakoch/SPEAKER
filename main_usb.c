// Eza Koch
// MEAM 510
// 10/07/13
// cd Desktop/M2/ADC
/* connect the wiper of a 10k potentiometer spanning a 5V supply to ADC0 (F0) and have one of the onboard LEDs turn off when the voltage is below 2.5V and turn on when it is above 2.5V */

#include "m_general.h" // library of useful functions
#include "m_usb.h"
void init();
void loop();

void main(void)
{
    init();
    loop();
}

void loop(){
    while (TRUE){ //continue to read Analog value
        set(ADCSRA,ADSC); // begin conversion; auto clear when done
        set(ADCSRA:ADIE); // call interrupt when finished
        if (ADC>0x200){ // Vcc = 5, 2.5 is halfway; scaled for hex
            m_red(ON);// Onboard red LED ON
        }
        else {
                m_red(OFF); // Onboard red LED OFF
        }
    }
}

void init(){
    m_usb_init(); //initialize usb communication
    while(!m_usb_isconnected()); // wait for a connection
    m_clockdivide(0); // 16Mhz
    m_red (OFF); // Start with the LED off
    
    // Voltage Reference
    clear(ADMUX,REFS1); // Vcc
	set(ADMUX,REFS0);   // ^
    
    // ADC Prescalar
	set(ADCSRA,ADPS2); // prescaler 128 to 125kHz
	set(ADCSRA,ADPS1); // ^
	set(ADCSRA,ADPS0); // ^
	
    // Disable other inputs
	set(DIDR0,ADC0D); // disable F0
	set(DIDR0,ADC1D); // disable F1
	set(DIDR0,ADC4D); // disable F4
	set(DIDR0,ADC5D); // disable F5
    set(DIDR0,ADC6D); // disable F6
    set(DIDR0,ADC7D); // disable F7
    
    // Set Desired Pin Input
    clear(ADCSRB, MUX5); //set pin to F0
    clear(ADCSRB, MUX2); //^
    clear(ADCSRB, MUX1); //^
    clear(ADCSRB, MUX0); //^
    
    // Enable Conversion
    set(ADCSRA,ADEN); // enable conversion
}

//Interrupt
ISR(ADC){
    m_usb_tx_int(ADC); //print the result
}