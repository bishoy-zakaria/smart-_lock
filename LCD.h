/*
 * IncFile1.h
 *
 * Created: 04/07/2020 07:33:51 ص
 *  Author: Ali
 */ 


#ifndef LCD_H_
#define LCD_H_

#include "LCD_Cfg.h"
#include "BIT_Math.h"
#include "STD.h"



void LCD_Init(void);

void LCD_WriteCommand(uint8 cmd);

void LCD_WriteChar(uint8 data);

void LCD_GoTo(uint8 row, uint8 col);

void LCD_WriteString(uint8* str);

void LCD_Clear(void);

void LCD_WriteInteger(uint32 int_num);






#endif /* INCFILE1_H_ */