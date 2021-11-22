/*
 * Smart_Lock.c
 *
 * Created: 11/16/2021 7:26:17 PM
 * Author : Bishoy
            Ali
			Mina
			Mahmoud
 */ 
/**************** Included files ***********************/
#include "LCD.h"
#include "KeyPad.h"
#include "EEPROM.h"
#define F_CPU 8000000
#include <util/delay.h>
#include <avr/interrupt.h>

#define student_num   5
#define name_len     10
#define EEPROM_ID(User_ID)   ((User_ID*4)-400) // this a linear relation between user ID and EEPROM ID as USER ID [100-354] 
#define EEPROM_Pass(User_ID) ((User_ID*4)-398) // this a linear relation between user ID and EEPROM ID that contain password
#define Motor_Port  DIO_PORTD
#define Motor_Pin   DIO_PIN1
#define Motor_ON       1
#define Motor_OFF      0
#define Buzzer_Port  DIO_PORTD
#define Buzzer_Pin   DIO_PIN5
#define Buzzer_ON      1
#define Buzzer_OFF     0

/********************************************************/
typedef struct
{
	uint8 name[name_len];
	uint16 ID;
	uint16 Pass;
}student;


/**************** Prototypes ******************/
void  System_Init(void);
uint16 Key_Pad_enter(uint16*);
int charArrayToInt(uint16 *arr);
uint8 Check_ID(uint16);
uint8 Check_Pass(uint16 , uint16 );
void  Change_Pass(uint16 , uint16 );
void  Admin_Func(void);
void  Student_Func(void);
void  Motor_Trigger(uint8);
void  Buzzer_Trigger(uint8);
/*****************  Data init ******************/
student Data[student_num]={{"Prof",111,203},
{"Ahmed",126,129},
{"Amr",128,325},
{"Adel",130,426},
{"Omar",132,79}};
	
uint8 key_pad_case = 0; // this for hiding the pass code to detect the case of key pad
/******************** main function **********************/
ISR(INT0_vect)
{
	Admin_Func();

}

ISR(INT1_vect)
{
	Student_Func();

}


int main(void)
{
	uint8 keypad_arr[3];
	uint16 KeyPad_Val_ID;
	uint16 KeyPad_Val_PC;
	uint8 enter_val=0;
    System_Init();
	
	/*for (uint16 i=0x00;i<=0xFF;i=i+2) //this step is to check if the EEPROM data
	{
		LCD_WriteInteger(EEPROM_read(i));
		LCD_GoTo(1,0);
		LCD_WriteInteger(i);
		_delay_ms(1000);
		LCD_Clear();
		LCD_GoTo(0,0);
	}*/
	while(1)
	{
		LCD_Clear();
		key_pad_case=0;
		LCD_WriteString("Enter *");
		enter_val= KeyPad_GetValue();
		_delay_ms(250);
		
		if(enter_val == '*')
		{
			LCD_Clear();
			LCD_WriteString("enter ID");
			LCD_GoTo(1,0);
			KeyPad_Val_ID=Key_Pad_enter(&keypad_arr);
			
			if( Check_ID(KeyPad_Val_ID) == 0)
			{
				LCD_Clear();
				LCD_WriteString("wrong ID");
				Buzzer_Trigger(Buzzer_ON);
				_delay_ms(1000);
				Buzzer_Trigger(Buzzer_OFF);
				_delay_ms(1000);
				Buzzer_Trigger(Buzzer_ON);
				_delay_ms(1000);
				Buzzer_Trigger(Buzzer_OFF);
			}
			
			else if (Check_ID(KeyPad_Val_ID) == 1)
			{
				LCD_Clear();
				LCD_WriteString("enter PC");
				LCD_GoTo(1,0);
				key_pad_case=1;
				KeyPad_Val_PC=Key_Pad_enter(&keypad_arr);
				
				if(Check_Pass(KeyPad_Val_PC, KeyPad_Val_ID) == 1)
				{
					LCD_Clear();
					key_pad_case=0;
					for(uint8 i=0 ; i<=4 ; i++)
					{
						if (Data[i].ID == KeyPad_Val_ID )
						{
							LCD_WriteString("welcome");
							LCD_GoTo(1,0);
							LCD_WriteString(Data[i].name);
							Motor_Trigger(Motor_ON);/*open the door */
							_delay_ms(5000);
							Motor_Trigger(Motor_OFF);/*close the door */
							LCD_Clear();
							LCD_WriteString("press*");
							break;
						}
					}
				}
				
				else if(Check_Pass(KeyPad_Val_PC, KeyPad_Val_ID) == 0)
				{
					LCD_Clear();
					LCD_WriteString("wrong PC");
					Buzzer_Trigger(Buzzer_ON);
					_delay_ms(1000);
					Buzzer_Trigger(Buzzer_OFF);
					LCD_Clear();
					enter_val=0;
					
			
				}
			}
			
		}
	}
	
	
}
/********************** functions implimintations *************************/
void System_Init(void)
{
	uint8 Check_Var=1;
	LCD_Init();
	KeyPad_Init();
	DIO_SetPinDir(Motor_Port,Motor_Pin,DIO_PIN_OUTPUT); /* set the motor pin as output*/
	DIO_SetPinDir(Buzzer_Port,Buzzer_Pin,DIO_PIN_OUTPUT);/*set the buzzer bin as output*/
	DIO_WritePin(Motor_Port,Motor_Pin,DIO_PIN_LOW);
	DIO_WritePin(Buzzer_Port,Buzzer_Pin,DIO_PIN_LOW);
	
	DIO_SetPinDir(DIO_PORTD,DIO_PIN2,DIO_PIN_INPUT);
	DIO_WritePin(DIO_PORTD,DIO_PIN2,DIO_PIN_HIGH);
	DIO_SetPinDir(DIO_PORTD,DIO_PIN3,DIO_PIN_INPUT);
	DIO_WritePin(DIO_PORTD,DIO_PIN3,DIO_PIN_HIGH);
	MCUCR=0x0B;
	GICR=0xC0;
	SET_BIT(SREG,7);
	
	//External_INT0_Set_CallBack(&Admin_Func);/* call back function for the interrupt service routine of INT0*/ 
	//External_INT1_Set_CallBack(Student_Func);/* call back function for the interrupt service routine of INT1*/ 
	/*************** Store pass in EEPROM **************/
	for (uint16 i=0x00;i<=0xFF;i++) //this step is to check if the EEPROM is empty or not
	{
		if(EEPROM_read(i) != 0xFF)
			Check_Var=0;
	}
	if(Check_Var==1)/************************************/
	{
		LCD_WriteString("Initializing...");
		for(uint8 i=0 ; i<student_num ; i++)
		{
			EEPROM_write(EEPROM_ID(Data[i].ID) , Data[i].ID);
			EEPROM_write(EEPROM_Pass(Data[i].ID) , Data[i].Pass);
		}
		LCD_Clear();
	}
	else if (Check_Var==0)
		LCD_WriteString("Ready");
		_delay_ms(1000);
		LCD_Clear();
}


uint16 Key_Pad_enter(uint16* ptr)
{
	uint8 counter=0;
	uint16 value=0;
		 while(1)
		 {
			 value=KeyPad_GetValue();
			if(value)   //this loop is for use just on push for the putton of the keypad
				 {
					 if (counter < 2)            //checking if the counter is smaller than 3 and rising its value by one
				 {
					 switch(key_pad_case) // this for hiding the pass code of the user
					 {
						 case 0:
						LCD_WriteChar(value);
						break;
						case 1:
						LCD_WriteChar('*');
						break; 
					 }
					
					 ptr[counter]=value-'0';
					 counter++;
					 _delay_ms(250);
				 }
				 else if (counter == 2)   //checking if the counter is equal to 3 and getting its value to zero
				 {
					 ptr[counter]=value-'0';
					 counter=0;
					 switch(key_pad_case) // this for hiding the pass code of the user
					 {
						  case 0:
						LCD_WriteChar(value);
						break;
						case 1:
						LCD_WriteChar('*');
						break;
					 }
					
					 _delay_ms(250);
					 value=charArrayToInt(ptr);
					 return value;
					 break;
				 }
				 
			 }
		 }
}

int charArrayToInt(uint16 *arr) {
	int i, value;
	i = value = 0;
	
	for( i = 0 ; i<3 ; ++i)
	{
		if( i==0 )
			value=arr[i]*100;
		else if(i==1)
		value+=arr[i]*10;
		else if (i==2)
		value+=arr[i]*1;
	}
	return value;
	
}

uint8 Check_ID(uint16 Enter_ID)
{
	uint8 check =0;
	for (uint16 i=0x00;i<=0xFF;i++) //this step is to check if ID is available and return 0 or 1
	{
		if(EEPROM_read(i) == Enter_ID)
			check=1;
	}
		
	return check;
}

uint8 Check_Pass(uint16 PC, uint16 ID)
{
	uint8 check=0;
	uint16 val= 0;
	val=EEPROM_read(EEPROM_Pass(ID));
	if(val == PC )
		check=1;
	else
		check=0;
	return check;
}




void  Change_Pass(uint16 PC, uint16 ID)
{
	EEPROM_write(EEPROM_Pass(ID) , PC);
}

void  Admin_Func(void)
{
	uint8 keypad_arr[3];
	uint16 KeyPad_Val_ID;
	uint16 KeyPad_Val_PC;
	uint16 New_PC=0;
	uint16 Re_enter=0;
	
	LCD_Clear();
	LCD_WriteString("Welcome Sir");
	_delay_ms(1000);
	LCD_Clear();
	LCD_WriteString("Enter Admin ID");
	KeyPad_Val_ID=0;
	KeyPad_Val_PC=0;
	LCD_GoTo(1,0);
	KeyPad_Val_ID=Key_Pad_enter(&keypad_arr);
	key_pad_case=1;
	if(KeyPad_Val_ID == Data[0].ID)
	{
		LCD_Clear();
		LCD_WriteString("Enter Admin PC");
		LCD_GoTo(1,0);
		KeyPad_Val_PC=Key_Pad_enter(&keypad_arr);
		key_pad_case=0;
		if(Check_Pass(KeyPad_Val_PC,KeyPad_Val_ID))
		{
			LCD_Clear();
			LCD_WriteString("Enter ID");
			LCD_GoTo(1,0);
			KeyPad_Val_ID=0;
			KeyPad_Val_PC=0;
			KeyPad_Val_ID=Key_Pad_enter(&keypad_arr);
			if(Check_ID(KeyPad_Val_ID))
			{
				key_pad_case=1;
				LCD_Clear();
				LCD_WriteString("Enter OLD PC");
				LCD_GoTo(1,0);
				KeyPad_Val_PC=Key_Pad_enter(&keypad_arr);
				if(Check_Pass(KeyPad_Val_PC,KeyPad_Val_ID))
				{
					LCD_Clear();
					LCD_WriteString("Enter New PC");
					LCD_GoTo(1,0);
					New_PC = Key_Pad_enter(&keypad_arr);
					LCD_Clear();
					LCD_WriteString("Renter New PC");
					LCD_GoTo(1,0);
					Re_enter = Key_Pad_enter(&keypad_arr);
					
					if (New_PC == Re_enter)
					{
						Change_Pass(New_PC, KeyPad_Val_ID);
						LCD_Clear();
						LCD_WriteString("PC changed");
						_delay_ms(1000);
					}
					else
					{
						LCD_Clear();
						LCD_WriteString("Not match");
						_delay_ms(1000);
						LCD_Clear();
					}
				}
				else
				{
					LCD_Clear();
					LCD_WriteString("wrong PC");
					_delay_ms(1000);
				}
			}
			else
			{
				LCD_Clear();
				LCD_WriteString("wrong ID");
				_delay_ms(1000);
				LCD_Clear();
			}
				
		}
		else
		{
			LCD_Clear();
			LCD_WriteString("wrong PC");
			_delay_ms(1000);
			LCD_Clear();
		}
	}
	else
	{
		LCD_Clear();
		LCD_WriteString("Wrong ID");
		_delay_ms(1000);
		LCD_Clear();
	}
	
}


void  Student_Func(void)
{
	uint8 keypad_arr[3];
	uint16 KeyPad_Val_ID;
	uint16 KeyPad_Val_PC;
	uint16 New_PC=0;
	uint16 Re_enter=0;
	LCD_Clear();
	LCD_WriteString("Welcome");
	_delay_ms(1000);
	LCD_Clear();
	LCD_WriteString("Enter your ID");
	KeyPad_Val_ID=0;
	KeyPad_Val_PC=0;
	key_pad_case=0;
	LCD_GoTo(1,0);
	KeyPad_Val_ID=Key_Pad_enter(&keypad_arr);
	key_pad_case=1;
	if(Check_ID(KeyPad_Val_ID))
	{
		LCD_Clear();
		LCD_WriteString("Enter your PC");
		LCD_GoTo(1,0);
		KeyPad_Val_PC=Key_Pad_enter(&keypad_arr);
		key_pad_case=1;
		if(Check_Pass(KeyPad_Val_PC,KeyPad_Val_ID))
		{
					LCD_Clear();
					LCD_WriteString("Enter New PC");
					LCD_GoTo(1,0);
					New_PC = Key_Pad_enter(&keypad_arr);
					LCD_Clear();
					LCD_WriteString("Renter New PC");
					LCD_GoTo(1,0);
					Re_enter = Key_Pad_enter(&keypad_arr);
					
					if (New_PC == Re_enter)
					{
						Change_Pass(New_PC, KeyPad_Val_ID);
						LCD_Clear();
						LCD_WriteString("PC changed");
						_delay_ms(1000);
					}
					else
					{
						LCD_Clear();
						LCD_WriteString("Not match");
						_delay_ms(1000);
						LCD_Clear();
					}
		}
				else
				{
					LCD_Clear();
					LCD_WriteString("wrong PC");
					_delay_ms(1000);
					LCD_Clear();
				}
	}
	else
	{
		LCD_Clear();
		LCD_WriteString("Wrong ID");
		_delay_ms(1000);
		LCD_Clear();
	}
}

void  Motor_Trigger(uint8 val)
{
	switch(val)
	{
		case Motor_ON:
			DIO_WritePin(Motor_Port,Motor_Pin,DIO_PIN_HIGH);
			break;
		case Motor_OFF:
			DIO_WritePin(Motor_Port,Motor_Pin,DIO_PIN_LOW);
			break;
	}
}
void  Buzzer_Trigger(uint8 val)
{
	switch(val)
	{
		case Buzzer_ON:
		DIO_WritePin(Buzzer_Port,Buzzer_Pin,DIO_PIN_HIGH);
		break;
		case Buzzer_OFF:
		DIO_WritePin(Buzzer_Port,Buzzer_Pin,DIO_PIN_LOW);
		break;
	}
}


