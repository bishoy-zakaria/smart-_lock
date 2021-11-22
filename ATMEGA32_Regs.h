


#ifndef ATMEGA32_REG_H_
#define ATMEGA32_REG_H_

#include "STD.h"

/************************************* GPIO **********************************/

#define PORTA (*(volatile uint8*)(0x3B))
#define PINA  (*(volatile uint8*)(0x39))
#define DDRA  (*(volatile uint8*)(0x3A))

#define PORTB (*(volatile uint8*)(0x38))
#define PINB  (*(volatile uint8*)(0x36))
#define DDRB  (*(volatile uint8*)(0x37))

#define PORTC (*((volatile uint8*)(0x35)))
#define PINC  (*((volatile uint8*)(0x33)))
#define DDRC  (*((volatile uint8*)(0x34)))

#define PORTD (*(volatile uint8*)(0x32))
#define PIND  (*(volatile uint8*)(0x30))
#define DDRD  (*(volatile uint8*)(0x31))

/* ************************************* INTRRURT *************************** */

#define SREG   (*(volatile uint8*)(0x5F))
#define GICR   (*(volatile uint8*)(0x5B))
#define GIFR   (*(volatile uint8*)(0x5A))
#define MCUCR  (*(volatile uint8*)(0x55))
#define MCUCSR (*(volatile uint8*)(0x54))

/*************************************** EEPROM ******************************/
#define EECR (*(volatile uint8*)(0x3C))
#define EEAR (*(volatile uint8*)(0x3E))
#define EEDR (*(volatile uint8*)(0x3D))

#endif /* ATMEGA32_REG_H_ */