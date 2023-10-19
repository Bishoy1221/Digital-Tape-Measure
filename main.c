/*
 ?   LAB_6: Digital Tape Measure
 
     Author:  Leandro DaSilva Neves
 ????Partner: Bishoy Mikhail

ATmega------LCD
PD7---------D7
PD6---------D6
PD5---------D5
PD4---------D4
PD3---------D3
PD2---------D2
PD1---------D1
PD0---------D0
PC5---------E
PC4---------RS
GND---------R/W (we are only writing)
GND---------Vss
Vcc---------Vdd

+5--\/\/\/\/--GND
????   V0

Optional:
Vcc---------A
GND---------K

ATmega---------Sensor
PB0(ICP1)------Echo
PC3------------Trig
*/

/****************************************************/ 
 #include <avr/io.h>
 #include <stdio.h>
 #include <avr/interrupt.h>
 #define F_CPU 16000000
 #include <util/delay.h>
/****************************************************/
 void InitLCD();
 void WriteCommand(uint8_t d);
 void WriteData(uint8_t d);
 void PrintString(char s[]);
 void GoToXY(uint8_t x, uint8_t y);
 /****************************************************/
 volatile uint16_t new_value = 0;
 volatile uint16_t previous_value = 0;
/****************************************************/
 #define EBIT 5   //Set E to PC5
 #define RSBIT 4  //Set RS to PC4
 #define TBIT 3   //Set trigger to PC3
 #define EcoPIN 0 //Set Echo to PB0
/****************************************************/
 inline void E_HIGH() {PORTC = PORTC |= 1<<EBIT;}     //Setting E to high/low
 inline void E_LOW() {PORTC = PORTC &= ~(1<<EBIT);}
 inline void RS_HIGH() {PORTC = PORTC |= 1<<RSBIT;}   //Setting RS to high/low
 inline void RS_LOW() {PORTC = PORTC &= ~(1<<RSBIT);}  
 inline void T_HIGH() {PORTC = PORTC |= 1<<TBIT;}     //Setting Trigger to high/low
 inline void T_LOW() {PORTC = PORTC &= ~(1<<TBIT);}
/****************************************************/
 inline void delay250ns()
 {
	asm volatile(    //Defining LCD Function and Setup
	"swap r16\t\n"
	"swap r16\t\n"
	"swap r16\t\n"   //Use swap R16 to have compiler not optimize delay250
	"swap r16\t\n");
 }

 int main(void)
 {
	InitLCD();

	DDRB = 0b10;
	TCCR1A = 0b00<<COM1A0 | 0b00<<COM1B0 | 0b00<<WGM10;
	TCCR1B = 0<<ICNC1 | 1<<ICES1 | 0b011<<CS10 | 0b00<<WGM12; //Initializing and setting up Interrupt Registers and ports for input and outputting
	TIMSK1 = 1<<ICIE1;
	sei(); //Also enabling interrupts

	while (1)
	{
		float distance = 0.0; //initialize meas so it clears each time and doesn't get FUNKY
		T_HIGH();
		_delay_us(10); //Send the 8 pulses
		T_LOW();
		_delay_ms(38); //Wait for pulses to return
		 GoToXY(0,0); 
		PrintString("                ");
		GoToXY(0,1); //Clearing the display
		PrintString("                ");
		cli(); //Disable interrupts so you can do the MATHS
		uint16_t diff = new_value - previous_value;
		distance = (3.5f * diff) / 148.0f;
		sei(); //Re-enable interrupts for wonderful interrupt-ness
		char S[20];
		if((distance >= 6)&(distance <= 84))
		{
			sprintf(S, "%2.3f inches", distance);
			GoToXY(0, 0);
			PrintString(S);
		}
		else
		{
			GoToXY(0,0); PrintString("Out of Range");
		}
		TCNT1 = 0;  //reset timer so it don't blow up
		_delay_ms(1000);
	}
 }

 ISR (TIMER1_CAPT_vect)
 {
	previous_value = new_value;
	new_value = ICR1;
	TCCR1B ^= 1 << ICES1;
 }

 void InitLCD()
 {
	DDRC |= 0xFF; //D is output
	_delay_ms(100);
	DDRD |= 0xFF;
	WriteCommand(0x30);
	_delay_ms(5);
	WriteCommand(0x38);
	_delay_us(100);
	WriteCommand(0x08);
	WriteCommand(0x01);
	_delay_ms(2);
	WriteCommand(0x06);
	WriteCommand(0x0F);
 }

 void WriteCommand(uint8_t d)
 {
	E_LOW();
	RS_LOW();
	E_HIGH();
	PORTD = (d & 0xFF);
	delay250ns();
	E_LOW();
	delay250ns();
	_delay_us(39);
 }

 void WriteData(uint8_t d)
 {
	E_LOW();
	RS_HIGH();
	E_HIGH();
	PORTD = (d & 0xFF);
	delay250ns();
	E_LOW();
	delay250ns();
	_delay_us(39);
 }

 void PrintString(char s[])
 {
	uint8_t i=0;
	while (s[i])
	WriteData(s[i++]);
 }

 void GoToXY(uint8_t x, uint8_t y)
 {
	WriteCommand(0x80 | (y*0x40 + x));
 }