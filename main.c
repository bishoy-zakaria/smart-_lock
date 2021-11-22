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
#include "LCD.h" // include the LCD driver
#include "KeyPad.h" // include the key pad driver
#include "EEPROM.h" // include EEPROM driver
#define F_CPU 8000000 // define the frequency of the oscillator for delay_ms function
#include <util/delay.h> // include this library for using delay_ms function
#include <avr/interrupt.h> // include this library for using ISR for INT0,INT1

#define student_num   5 // maximum number of the students
#define name_len     10 // maximum number for the name length of the data set
#define EEPROM_ID(User_ID)   ((User_ID*4)-400) // this a linear relation between user ID and EEPROM ID as USER ID [100-354] 
#define EEPROM_Pass(User_ID) ((User_ID*4)-398) // this a linear relation between user ID and EEPROM ID that contain password
#define Motor_Port  DIO_PORTD // port of the motor that is defined in DIO.h
#define Motor_Pin   DIO_PIN1 // pin of the motor that is defined in DIO.h
#define Motor_ON       1   // this macro that will pass in Motor_Trigger to turn the motor on
#define Motor_OFF      0   // this macro that will pass in Motor_Trigger to turn the motor off
#define Buzzer_Port  DIO_PORTD // port of the buzzer that is defined in DIO.h
#define Buzzer_Pin   DIO_PIN5  // pin of the buzzer that is defined in DIO.h
#define Buzzer_ON      1 // this macro that will pass in Buzzer_Trigger to turn the motor on
#define Buzzer_OFF     0 // this macro that will pass in Buzzer_Trigger to turn the motor off

/********************** data type as struct *******************************/
typedef struct
{
	uint8 name[name_len];
	uint16 ID;
	uint16 Pass;
}student;


/************************* Prototypes ********************************/
void  System_Init(void); // system initialization for LCD, keypad, ext_interrupt, motor and buzzer
uint16 Key_Pad_enter(uint16*); //this function takes an array as a buffer and store in it the entered characters from keypad and return it as integer
int charArrayToInt(uint16 *arr);//this function called in kay_pad_enter to convert the array of characters to integer 
uint8 Check_ID(uint16); // this function check if the entered ID is stored in eeprom and return with 1 if true or 0 if false
uint8 Check_Pass(uint16 , uint16 ); //this function takes the id and the password and checked if this id has the same password and return 1 if true or 0 if false
void  Change_Pass(uint16 , uint16 );//this function takes the id and the the new password to change its value in eeprom
void  Admin_Func(void); // this function for the professor and will be called in INT0 ISR
void  Student_Func(void);// this function for the student for changing his password and will be called in INT1 ISR
void  Motor_Trigger(uint8); //this function trigger the motor status to open and close the door
void  Buzzer_Trigger(uint8); // this function trigger the buzzer status 

/*****************  Data set initialization ******************/
student Data[student_num]={{"Prof",111,203},
{"Ahmed",126,129},
{"Amr",128,325},
{"Adel",130,426},
{"Omar",132,79}};
uint8 key_pad_case = 0; // this for hiding the pass code to detect the case of key pad as when it will be 1 write * in pc and if 0 write the normal character that is entered in keypad

/******************** ISR **********************/
ISR(INT0_vect) // interrupt INT0 service routine 
{
	Admin_Func();
}

ISR(INT1_vect)// interrupt INT1 service routine 
{
	Student_Func();
}

/******************** main function **********************/
int main(void)
{
	uint8 keypad_arr[3];//this array will store the data that entered from the keypad
	uint16 KeyPad_Val_ID;//this variable will store the data of ID that comes from the keypad
	uint16 KeyPad_Val_PC;//this variable will store the data of PC that comes from the keypad
	uint8 enter_val=0; // this variable will store the value of * when the user enter it in the beginning of the while(1)
    System_Init();

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
				Buzzer_Trigger(Buzzer_ON); // two peeps
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
				key_pad_case=1; // crypt the following data the entered using * as it is the PC
				KeyPad_Val_PC=Key_Pad_enter(&keypad_arr);
				
				if(Check_Pass(KeyPad_Val_PC, KeyPad_Val_ID) == 1)
				{
					LCD_Clear();
					key_pad_case=0;// show the following data on LCD as it is the ID
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
					Buzzer_Trigger(Buzzer_ON); // one peep
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
	LCD_Init(); //initialize LCD
	KeyPad_Init();//initialize Keypad
	DIO_SetPinDir(Motor_Port,Motor_Pin,DIO_PIN_OUTPUT); /* set the motor pin as output*/
	DIO_SetPinDir(Buzzer_Port,Buzzer_Pin,DIO_PIN_OUTPUT);/*set the buzzer bin as output*/
	DIO_WritePin(Motor_Port,Motor_Pin,DIO_PIN_LOW);//set this pin low as initialized
	DIO_WritePin(Buzzer_Port,Buzzer_Pin,DIO_PIN_LOW);//set this pin low as initialized
	
	/*initialize the external interrupt0 and external interrupt1*/
	DIO_SetPinDir(DIO_PORTD,DIO_PIN2,DIO_PIN_INPUT);
	DIO_WritePin(DIO_PORTD,DIO_PIN2,DIO_PIN_HIGH);//enable pull up resistor 
	DIO_SetPinDir(DIO_PORTD,DIO_PIN3,DIO_PIN_INPUT);
	DIO_WritePin(DIO_PORTD,DIO_PIN3,DIO_PIN_HIGH);//enable pull up resistor
	MCUCR=0x0B;// set modes of INT0 as rising edge and INT1 as falling edge
	GICR=0xC0; //enable INT0 and INT1
	SET_BIT(SREG,7);//global interrupt enable

	/*************** Store pass in EEPROM **************/
	for (uint16 i=0x00;i<=0xFF;i++) //this step is to check if the EEPROM is empty or not 
	{
		if(EEPROM_read(i) != 0xFF)
			Check_Var=0;
	}
	if(Check_Var==1)/**************** if the eeprm is empty enter in it *************/
	{
		LCD_WriteString("Initializing...");
		for(uint8 i=0 ; i<student_num ; i++)
		{
			EEPROM_write(EEPROM_ID(Data[i].ID) , Data[i].ID); //write the ID of the students and professor
			EEPROM_write(EEPROM_Pass(Data[i].ID) , Data[i].Pass);//write the PC of the students and professor
		}
		LCD_Clear();
	}
	else if (Check_Var==0) // if the eeprom is initialized and and has any section its value is not 0xFF
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
			if(value)   //this loop is for use just on push for the button of the keypad
				 {
					 if (counter < 2)            //checking if the counter is smaller than 3 and rising its value by one
				 {
					 switch(key_pad_case) // this for hiding the pass code of the user as it checks on the global variable key_pad_case
					 {
						 case 0:
						LCD_WriteChar(value);
						break;
						case 1:
						LCD_WriteChar('*');
						break; 
					 }
					
					 ptr[counter]=value-'0';//this for convert from ASCI to integer
					 counter++;
					 _delay_ms(250);
				 }
				 else if (counter == 2)   //checking if the counter is equal to 3 and getting its value to zero
				 {
					 ptr[counter]=value-'0'; //this for convert from ASCI to integer
					 counter=0;
					 switch(key_pad_case) // this for hiding the pass code of the user as it checks on the global variable key_pad_case
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


