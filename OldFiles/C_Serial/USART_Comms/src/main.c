#define F_CPU 16000000UL

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "i2cmaster.h"
#include "lcd.h"
#include "lm75.h"

#include "usart.h"

#define BAUDRATE 9600      
#define BAUD_PRESCALER (((F_CPU / (BAUDRATE * 16UL))) -1)

typedef enum {
    memory_bit,
    message_bit,
} message;

void usart_init();
void usart_out(unsigned char data);
unsigned char usart_in(void);

volatile unsigned char data = 0;
volatile unsigned char length = 0;
volatile unsigned char* str;
volatile message msg = 0;
volatile unsigned int counter = 0;

ISR(USART_RX_vect) {
    data = UDR0;
    if(msg == message_bit) {
        printf("%d ", data);
        str[counter] = data;
    }
    else if(msg == memory_bit) {
        length = data * 10;
        msg = message_bit;
        str = malloc(length * sizeof(unsigned char));
        for(int i = 0; i < length; i++)
            str[i] = 0;
    }
}


int main(void) {    

    //uart_init(); // open the communication to the microcontroller
	//io_redirect(); // redirect input and output to the communication

    i2c_init();
    LCD_init();
    //lm75_init();

    //The buttons are PC0, PC1, PC2, PC3
    //The LEDs are PD7, PD6, PD5, PD4

    DDRC = 0xF0; //Setting (button) ports to input (0)
    PORTC = 0x3F; //Setting up pull-up resistors
    DDRD = 0xF0; //Setting (LED) ports to output
    //PORTD = 0xF0; //Setting LEDs on
    
    sei();
    usart_init();

    while(1) {
        
    }

    return 0;
}

void usart_init() {
    UBRR0H = (uint8_t)(BAUD_PRESCALER >> 8);
    UBRR0L = (uint8_t)(BAUD_PRESCALER);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << TXCIE0); 
    UCSR0C = (1 << UCSZ01) | (1 <<  UCSZ00);
}

void usart_out(unsigned char data) {
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

unsigned char usart_in(void) {
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}
