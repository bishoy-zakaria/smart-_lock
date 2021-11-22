/*
 * EEPROM.c
 *
 * Created: 11/16/2021 7:28:21 PM
 *  Author: Bishoy
 */ 

#include "EEPROM.h"
#define F_CPU 8000000
#include <util/delay.h>
void EEPROM_write(unsigned int uiAddress, unsigned int ucData)
{
	int data=0;
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE))
	;
	/* Set up address and data registers */
	if (ucData<=255)
	{
		EEAR = uiAddress;
		EEDR = ucData;
		/* Write logical one to EEMWE */
		EECR |= (1<<EEMWE);
		/* Start eeprom write by setting EEWE */
		EECR |= (1<<EEWE);
		_delay_ms(10);
		
		EEAR = uiAddress+1;
		EEDR = 0x00;
		/* Write logical one to EEMWE */
		EECR |= (1<<EEMWE);
		/* Start eeprom write by setting EEWE */
		EECR |= (1<<EEWE);
		_delay_ms(10);
	}
	
	else if (ucData > 255)
	{
		data= ucData-255;
		EEAR = uiAddress;
		EEDR = 255;
		/* Write logical one to EEMWE */
		EECR |= (1<<EEMWE);
		/* Start eeprom write by setting EEWE */
		EECR |= (1<<EEWE);
		_delay_ms(10);
		
		EEAR = uiAddress+1;
		EEDR = data;
		/* Write logical one to EEMWE */
		EECR |= (1<<EEMWE);
		/* Start eeprom write by setting EEWE */
		EECR |= (1<<EEWE);
		_delay_ms(10);
	}
	
}

unsigned int EEPROM_read(unsigned int uiAddress)
{
	int data=0;
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE))
	;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from data register */
	if(EEDR ==0)
	return data;
	else
	data+=EEDR;
	
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE))
	;
	/* Set up address register */
	EEAR = uiAddress+1;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from data register */
	data+=EEDR;
	return data;
	
}

unsigned char EEPROM_read_Sum(unsigned int uiAddress)
{
	return (EEPROM_read(uiAddress)+EEPROM_read((uiAddress+1)));
}