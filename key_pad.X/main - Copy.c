#include <xc.h>
#include <pic18f4520.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "conbits.h"
#include "uart_layer.h"

const char program_start[]="\r\nProgram start\n\r";
uint8_t print_buffer[25] = {0}; // buffer to print stuff to serial

volatile uint8_t key_value = 0;
volatile uint8_t key_char = 0;
volatile bool key_isr = false;

void keypad_line_scan(void){
    static uint8_t k_scan  = 0x08;
    
    k_scan = k_scan << 1;
    if(k_scan >= 0x80){
        k_scan = 0x08;
    }
    LATB = k_scan;
}

void keypad_to_value_int0(uint8_t port_value,volatile uint8_t *value,volatile uint8_t *value_char){
    switch(port_value){
        case 0x41:
            *value_char = '1';
            *value = 1;
        break;
        case 0x11:
            *value_char = '7';
            *value = 7;
        break;
        case 0x21:
            *value_char = '*';
            *value = 10;
        break;
        case 0x09:
            *value_char = '4';
            *value = 4;
        break;
    }
}
void keypad_to_value_int1(uint8_t port_value,volatile uint8_t *value,volatile uint8_t *value_char){
    switch(port_value){
        
        case 0x42:
            *value_char = '2';
            *value = 2;
            break;
        case 0x12:
            *value_char = '8';
             *value = 8;
            break;
        case 0x21:
            *value_char = '*';
            *value = 10;
        break;
        case 0x0A:
            *value_char = '5';
            *value = 5;
        break;
    }
}
void keypad_to_value_int2(uint8_t port_value,volatile uint8_t *value,volatile uint8_t *value_char){
    switch(port_value){
        case 0x44:
            *value_char = '3';
            *value = 3;
        break;
        case 0x14:
            *value_char = '9';
            *value = 9;
        break;
        case 0x24:
            *value_char = '#';
            *value = 11;
        break;
        case 0x0C:
            *value_char = '6';
            *value = 6;
        break;
    }
}

void keypad_read(void){
        sprintf((char*)print_buffer,"\rKey value: 0x%02x %c",key_value,key_char);
        uart_send_string(print_buffer);
}

void main(void){

    OSCCONbits.IDLEN = 0;
    OSCCONbits.IRCF = 0x07;
    OSCCONbits.SCS = 0x03;
    while(OSCCONbits.IOFS!=1); // 8Mhz
    
    TRISB=0x07;    // b port as output
    LATB=0x01; // b port low

    uart_init(51,0,1,0);//baud 9600
    
    INTCON2bits.INTEDG0 = 1;
    INTCON2bits.INTEDG1 = 1;
    INTCON2bits.INTEDG2 = 1;
    
    INTCON3bits.INT1P = 1;
    INTCON3bits.INT2P = 1;
    
    INTCONbits.INT0E = 1;
    INTCON3bits.INT1E = 1;
    INTCON3bits.INT2E = 1;
    
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;// base interrupt setup
    
    __delay_ms(2000);
    uart_send_string((uint8_t *)program_start); // everything works in setup
    

    for(;;){
        keypad_line_scan();
        if(key_isr){
          keypad_read();
            key_isr = false;  
        };
    } 
}



void __interrupt() high_isr(void){
    INTCONbits.GIEH = 0;
    if(INTCONbits.INT0F || INTCON3bits.INT1F || INTCON3bits.INT2F){
        
        if(INTCONbits.INT0F){
            keypad_to_value_int0(PORTB,&key_value,&key_char);
            key_isr = true;
            INTCONbits.INT0F = 0;
        }
        if(INTCON3bits.INT1F){
            keypad_to_value_int1(PORTB,&key_value,&key_char);
            key_isr = true;
            INTCON3bits.INT1F = 0;
        }
        if(INTCON3bits.INT2F){
            keypad_to_value_int2(PORTB,&key_value,&key_char);
            key_isr = true;
            INTCON3bits.INT2F = 0;
        }

    }
    INTCONbits.GIEH = 1;
}

void __interrupt(low_priority) low_isr(void){
    INTCONbits.GIEH = 0;
    
     
    INTCONbits.GIEH = 1;
}
