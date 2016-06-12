/*
*	nokia_3310_lcd.h
*/

#ifndef _NOKIA_3310_LCD_H
#define _NOKIA_3310_LCD_H

#if ARDUINO >= 100
  #include <Arduino.h> // Arduino 1.0
  #define WRITE_RESULT size_t
  #define WRITE_RETURN return 1;
#else
  #include <WProgram.h> // Arduino 0022
  #define WRITE_RESULT void
  #define WRITE_RETURN
#endif

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "Print.h"


// Define SPI port
#define LCD_RST PORTB1
#define SPI_SS  PORTB2	// must be high whilst SPCR is set.

// Default CS for nuelectronics shield
#define SPI_CS  PORTB2
#define SPI_CS_PORT  PORTB
#define SPI_CS_DDR  DDRB
// Alternative Digital Pin 6 if you want to modify shield to use Ethernet at same time
//#define SPI_CS  PORTD6
//#define SPI_CS_PORT  PORTD
//#define SPI_CS_DDR  DDRD

#define SPI_MOSI PORTB3
#define SPI_SCK PORTB5
#define LCD_DC  PORTB0
#define LCD_BL  PORTD7		// Backlight control

//display mode -- normal / highlight
#define MENU_NORMAL	0
#define MENU_HIGHLIGHT 1
#define PIXEL_OFF 0
#define PIXEL_ON  1
#define PIXEL_XOR 2


class Nokia_3310_lcd : public Print
{
  private:
	void writeCommand( unsigned char );
	void writeData( unsigned char );

  public:
  
  	Nokia_3310_lcd();
  	
  	// Init/Clear/position functions
	void init(void);
  	virtual WRITE_RESULT write(uint8_t byte);
	void clear(void);
	void gotoXY(unsigned char x, unsigned char y);
	void update(void);

	// String and character functions
	void writeString(unsigned char x,unsigned char y,char *s, char mode);
	void writeStringP(unsigned char x,unsigned char y,
			const char *s, char mode);
	void writeStringBig ( unsigned char x,unsigned char y,
			char *string, char mode );
	void writeCharBig (unsigned char x,unsigned char y,
			unsigned char ch, char mode);
	void writeChar(unsigned char ch, char mode);


};

#endif //_NOKIA_3310_LCD_H

