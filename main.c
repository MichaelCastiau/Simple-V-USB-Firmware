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
			PORTB |= 1; // turn LED on
			}else if (rq->wValue.bytes[0] == 0){
			//O for turning led off
			PORTB &= ~1; // turn LED off
		}
		return 0;
		case USB_GET_STRING:
		buffer[0] = isLedOn();
		usbMsgPtr = buffer;
		return sizeof(buffer);
	}

	return 0; // should not get here
}

int main() {

	MCUCR |= (1u << PUD);	//Disable all pull ups
	DDRD &= ~(1<<PD3);
	DDRD &= ~(1<<PD2);
	DDRB |= (1<<PB0);

	//standard lamp on
	PORTB |= 1;

	usbInit();
	
	usbDeviceDisconnect(); // enforce re-enumeration
	_delay_ms(500);
	usbDeviceConnect();
	
	sei(); // Enable interrupts after re-enumeration
	
	while(true) {
		usbPoll();
	}
	
	return 0;
}