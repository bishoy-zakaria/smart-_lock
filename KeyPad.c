/*
 * KeyPad.c
 *
 * Created: 04/07/2020 01:40:28 م
 *  Author: Ali
 */ 

#include "KeyPad.h"
#define F_CPU 8000000
#include <util/delay.h>


#define  COL_INIT    0
#define  COL_FINAL   2

#define  ROW_INIT    4
#define  ROW_FINAL   7

#define NOT_PRESSED  0



const uint8 KeyPad_Vlues[4][3] = {  {'1','2','3'},
                                    {'4','5','6'},
								    {'7','8','9'},
								    {'*','0','#'} };
											  
											  
										  
void KeyPad_Init(void)
{
	DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_COLUM_0, DIO_PIN_OUTPUT);
	DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_COLUM_1, DIO_PIN_OUTPUT);
	DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_COLUM_2, DIO_PIN_OUTPUT);
	//DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_COLUM_3, DIO_PIN_OUTPUT);
	
	DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_ROW_0, DIO_PIN_INPUT);
	DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_ROW_1, DIO_PIN_INPUT);
    DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_ROW_2, DIO_PIN_INPUT);
	DIO_SetPinDir(KEYPAD_PORT ,KEYPAD_ROW_3, DIO_PIN_INPUT);
	
	DIO_SetPullUp(KEYPAD_PORT ,KEYPAD_ROW_0 );
	DIO_SetPullUp(KEYPAD_PORT ,KEYPAD_ROW_1 );
	DIO_SetPullUp(KEYPAD_PORT ,KEYPAD_ROW_2 );
	DIO_SetPullUp(KEYPAD_PORT ,KEYPAD_ROW_3 );
	
	DIO_WritePin(KEYPAD_PORT ,KEYPAD_COLUM_0, DIO_PIN_HIGH);
	DIO_WritePin(KEYPAD_PORT ,KEYPAD_COLUM_1, DIO_PIN_HIGH);
	DIO_WritePin(KEYPAD_PORT ,KEYPAD_COLUM_2, DIO_PIN_HIGH);
	//DIO_WritePin(KEYPAD_PORT ,KEYPAD_COLUM_3, DIO_PIN_HIGH);
	
}

uint8 KeyPad_GetValue(void)
{
	
	uint8 LOC_Coloum = 0;
	
	uint8 LOC_ROW = 0;
	
	uint8 Value = 0;
	
	uint8 Temp = 0;
	
	for(LOC_Coloum = COL_INIT ; LOC_Coloum <= COL_FINAL ; LOC_Coloum++)
	{
		
		DIO_WritePin(KEYPAD_PORT ,LOC_Coloum , DIO_PIN_LOW);
		
		for(LOC_ROW = ROW_INIT ; LOC_ROW <= ROW_FINAL ; LOC_ROW++)
		{
			
			DIO_ReadPin (KEYPAD_PORT ,LOC_ROW , &Temp);
			
			if(!Temp)
			{
				Value = KeyPad_Vlues[LOC_ROW - ROW_INIT][LOC_Coloum - COL_INIT];
				
				while(!Temp)
				{
					DIO_ReadPin (KEYPAD_PORT ,LOC_ROW , &Temp);
				}
				
				_delay_ms(10);
			}		
			
		}
		
		DIO_WritePin(KEYPAD_PORT ,LOC_Coloum , DIO_PIN_HIGH);
	}
	
	return Value ;
	
}