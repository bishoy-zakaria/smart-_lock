/*
 * EEPROM.h
 *
 * Created: 11/16/2021 7:28:00 PM
 *  Author: Bishoy
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_
#include "ATMEGA32_Regs.h"

#define EEMWE 2
#define EERE  0
#define EEWE  1

void EEPROM_write(unsigned int uiAddress, unsigned int ucData);

unsigned int EEPROM_read(unsigned int uiAddress);

#endif /* EEPROM_H_ */