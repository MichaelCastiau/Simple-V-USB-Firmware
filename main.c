/*
* SimpleUSBExample.c
*
* Created: 16/10/2015 11:52:22
* Author : Michael Castiau
*/
#define false 0
#define true 1
#define USB_COMMAND_SWITCH_LED 0
#define USB_GET_STRING 1
#define F_CPU 12000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "usb_driver/usbdrv.h"
#include <util/delay.h>

static unsigned char buffer[64];
void initializeTimer(){
	/*TCCR1B |= (1 << CS10) | ( 1 << WGM12);	//Enable timer, no prescaler, enable CTC mode with compare register OCR1A
	TCNT1 = 0;
	OCR1A = 60000;	//Reset the timer value after comparing against 157
	TIMSK |= (1 << OCIE1A);*/
	
	TCCR1A = (1<<WGM10)|(1<<COM1A1)|(1<<COM1B1);  //Fast PWM, 8 bit mode
	TCCR1B = (1<<WGM12)|(1<<CS10);               //no prescaler, clear timer after compare
	//50% duty cycle
	//total cycle = 2 * 157
	OCR1A = 157;
	OCR1B = 157;
}
ISR (TIMER1_COMPA_vect)
{
	//Timer was compared
	//Switch led
	PORTC ^= (1 << PC0);
}
uint8_t isLedOn(){
	//Evaluate the state of the Port
	uint8_t isOn = (1 & PORTB);
	//If isOn == 1 , means it's on
	return isOn;
}
// this gets called when custom control message is received
USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data; // cast data to correct type for a more user friendly field access
	switch(rq->bRequest) { // custom command is in the bRequest field
		case USB_COMMAND_SWITCH_LED:
		if(rq->wValue.bytes[0] == 1){
			//1 for turning led on
			PORTB |=(1 << PINB0); // turn LED on
			}else if (rq->wValue.bytes[0] == 0){
			//O for turning led off
			PORTB &= ~(1 << PINB0); // turn LED off
		}
		return 0;
		case USB_GET_STRING:
		buffer[0] = isLedOn();
		usbMsgPtr = buffer;
		return sizeof(buffer);
	}

	return 0; // should not get here
}
void initializePorts(){
	MCUCR |= (1u << PUD);	//Disable all pull ups
	DDRD &= ~(1<<PD3);	//D3 as input
	DDRD &= ~(1<<PD2);	//D2 as input
	DDRB |= (1<<PB0) | ( 1 << PB1);
	DDRC |= ( 1 << PC0);

	//standard lamp on
	PORTB |= (1 << PINB0);
	//Wavelength lamp off
	PORTB &= ~(1 << PINB1);
}

int main() {

	initializePorts();

	usbInit();
	
	usbDeviceDisconnect(); // enforce re-enumeration
	_delay_ms(500);
	usbDeviceConnect();
	
	sei(); // Enable interrupts after re-enumeration
	initializeTimer();
	while(true) {
		usbPoll();
	}
	
	return 0;
}