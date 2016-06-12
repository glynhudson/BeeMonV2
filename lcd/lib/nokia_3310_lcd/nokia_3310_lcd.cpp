/* LCD library for noka 3110 display
 * Date: 27 Oct 2009
 * Author: A Lindsay
 * Credits: based on original nuelectronics library extensively tidied up
 * and with functions re-named and recoded.
 * Additional graphics code added with options to disnable graphics if
 * not required, see nokia_3110_lcd.h
 *
 * Updated: 10th November by John Crouchley
 * 1) Allow for multiple SPI devices by
 *    a) allowing SPI_CS to reside on any port
 *    b) set SPCR as part of LCDENABLE
 * 2) move get_key into the class
 * 3) Fix bug in writeChar caused column 6 of any character not to be updated
 * 4) add code to ensure values for cursor_row and cursor_col are kept within array bounds
 * 5) Inherit Nokia_3310_lcd class from Print and add a write function
 *    This enables all normal print behaviour, including numbers
 *
 * Updated: 14th November by Andrew Lindsay
 * 1) Create new big font that is cleaner.
 *
 * Updated: 9th December 2009 by John Crouchley
 * 1) PORTB2 (Arduino digital 10) must be set to OUTPUT and HIGH whilst SPCR is being changed
 *    Failure to do this can lead to intermittent SPI errors.
 *    If there is another SPI device already using PORTB2 then it will naturally be high already
 *    Warning - if you use this pin for anything else (other than another SPI device enable)
 *    then you may have problems.
 *
 * Updated: 26th January 2010 by John Crouchley
 * 1) Initialisation values (contrast and temp coefficient) updated for later LCDs (got a black screen)
 *    values taken from updated driver on Nuelectronics site. This still works with the earlier lcds.
 *
 * Updated: 2nd September 2010 by Andrew Lindsay based on changes supplied
 * by jmccrohan to add extra characters to big_font
 *
 * Updated: 11/12/2011, Andrew Lindsay. Updates for Arduino 1.0 compatibility
 */

#include <avr/pgmspace.h>
#include <avr/io.h>
//#include <WConstants.h>
#include "nokia_3310_lcd.h"
#include "font_5x7.hpp"

// defines for accessing lcd
#define SPI_INIT		SPCR = 0x51
//#define LCDENABLE		SPI_INIT; SPI_CS_PORT &= ~(1<<SPI_CS)	// Enable LCD
#define LCDENABLE		SPI_CS_PORT &= ~(1<<SPI_CS)	// Enable LCD
#define LCDDISABLE	SPI_CS_PORT |= (1<<SPI_CS)	// disable LCD
#define LCDCMDMODE	PORTB &= ~(1<<LCD_DC)	// Set LCD in command mode
#define LCDDATAMODE	PORTB |= (1<<LCD_DC)	// Set LCD to Data mode

#define LCDCOLMAX	84
#define LCDROWMAX	6
#define LCDPIXELROWMAX	48

// current cursor postition
static unsigned char cursor_row = 0; /* 0-5 */
static unsigned char cursor_col = 0; /* 0-83 */

#ifdef USE_GRAPHIC
static unsigned char lcd_buffer[LCDROWMAX][LCDCOLMAX];
#endif

// Default constructor
Nokia_3310_lcd::Nokia_3310_lcd() {
}
  	

/*
 * Name         : init
 * Description  : Main LCD initialisation function
 * Argument(s)  : None
 * Return value : None
 */
void Nokia_3310_lcd::init(void){
    DDRB |= (1<<LCD_DC)|(1<<LCD_RST)|(1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_SS);
    DDRD |= (1<<LCD_BL);
    SPI_CS_DDR |= (1<<SPI_CS);
    //
    // even if we don't use SPI_SS for enabling the SPI device it must be high whilst
    // changing SPCR. This code assumes that if it is re-used it is only by another SPI device
    // and thus it will always be high whilst that device is not in use, hence it is safe to set high here
    // we would need to set it high on every LCDENABLE otherwise.
    //
    PORTB |= (1<<SPI_SS);
    LCDDISABLE;
    PORTB &= ~(1<<LCD_RST);
  	
    delayMicroseconds(1);

    PORTB |= (1<<LCD_RST);
    SPI_INIT;   			// enable SPI master, fosc/16 = 1MH
    PORTD |= (1<<LCD_BL);  	// turn on backlight

    writeCommand(0x21);		// LCD Extended Commands
    //writeCommand(0xe0); 	// Set LCD Vop (Contrast)
    writeCommand(0xC0); 	// Set LCD Vop (Contrast) **NEW**
    //writeCommand(0x04);  	// Set temp coefficient
    writeCommand(0x06);  	// Set temp coefficient   **NEW**
    writeCommand(0x13);		// LCD bias mode1:48
    writeCommand(0x20);		// LCD Standard Commands, Horizontal addressing mode
    writeCommand(0x0c);		// LCD in normal mode
    clear();
}


/*
 * Name         : writeCommand
 * Description  : Sends command to display controller
 * Argument(s)  : command - The command to be sent
 * Return value : none
 */
void Nokia_3310_lcd::writeCommand(unsigned char command ) {
    LCDENABLE;
    LCDCMDMODE;
    SPDR = command;		// Send data to display controller

    while (!(SPSR & 0x80));   	// Wait until Tx register empty

    LCDDISABLE;
}


/*
 * Name         : writeData
 * Description  : Sends data to the display controller
 * Argument(s)  : Date - Data to be sent
 * Return value : none
 */
void Nokia_3310_lcd::writeData(unsigned char data ) {
    LCDENABLE;
    LCDDATAMODE;
    SPDR = data;		// Send data

    while (!(SPSR & 0x80));	// Wait until Tx register empty

    LCDDISABLE;
}


/*
 * Name         : writeString
 * Description  : Write a string to the LCD from current position
 * Argument(s)  : x - Column to start at, 0-83
 * 		  y - Row to start, 0-6
 * 		  s - Pointer to start of string
 * 		  mode - Mode of string, reverse or normal
 *
 * Return value : none
 */
void Nokia_3310_lcd::writeString(unsigned char x,unsigned char y,char *string, char mode){
    gotoXY(x,y);
    while (*string)
	 writeChar(*string++, mode);
}


/*
 * Name         : writeStringP
 * Description  : Write a string stored in PROGMEM to the LCD
 * 		  from current position
 * Argument(s)  : x - Column to start at, 0-83
 * 		  y - Row to start, 0-6
 * 		  s - Pointer to start of string
 * 		  mode - Mode of string, reverse or normal
 * Return value : none
 */
void Nokia_3310_lcd::writeStringP ( unsigned char x,unsigned char y,const char *string, char mode )
{
    char c;
    gotoXY(x,y);
    while ( ( c = pgm_read_byte( string++) ) )
	writeChar( c, mode );
}


/*
 * Name         : writeStringBig
 * Description  : Write a string using big font to position x,y
 * 		  Note: bigfont only includes digits, '.', '+', '-'
 * Argument(s)  : x,y - starting position on screen, x=0-83, y=0-6
 *                string - string pointer of data to display
 *                mode - reverse or normal
 * Return value : none
 */
void Nokia_3310_lcd::writeStringBig ( unsigned char x,unsigned char y,
		char *string, char mode ){
    while ( *string ){
        writeCharBig( x, y, *string , mode );
        
        // For decimal point use 5 pixel gap instead of 12 to not
	// make spacing look odd
        if(*string++ == '.')
          x += 5;
        else
          x += 12;
    }
}


/*
 * Name         : writeCharBig
 * Description  : Write a single big character to screen
 * 		  Note: bigfont only includes digits, '.', '+', '-'
 * Argument(s)  : x,y - starting position on screen, x=0-83, y=0-6
 *                ch - character to display
 *                mode - reverse or normal
 * Return value : none
 */




/*
 * Name         : writeChar
 * Description  : Write a single normal font character to screen
 * 		  at current cursor position
 * Argument(s)  : ch - character to display
 *                mode - reverse or normal
 * Return value : none
 */
void Nokia_3310_lcd::writeChar(unsigned char ch, char mode) {
	unsigned char j;
  
#ifdef USE_GRAPHIC
      if (cursor_col > LCDCOLMAX - 6) cursor_col = LCDCOLMAX - 6; // ensure space is available for the character
      if (cursor_row > LCDROWMAX - 1) cursor_row = LCDROWMAX - 1; // ensure space is available for the character
	lcd_buffer[cursor_row][cursor_col] = 0x00;
	for(j=0; j<5; j++) {
		lcd_buffer[cursor_row][cursor_col + j] =  pgm_read_byte(&(smallFont [(ch-32)*5 + j] ));
        }

	lcd_buffer[cursor_row][cursor_col + 5] = 0x00;

	for(j=0; j< 6; j++) {
		if( mode == MENU_NORMAL )
			writeData(lcd_buffer[cursor_row][cursor_col++]);
		else
			writeData(lcd_buffer[cursor_row][cursor_col++] ^ 0xff);
		if (cursor_col >= LCDCOLMAX)
		{
			cursor_col=0;
			cursor_row++;
			if (cursor_row >= LCDROWMAX) cursor_row=0;
		}
	}
#else
	for(j=0; j<5; j++) {
		if( mode == MENU_NORMAL )
			writeData( pgm_read_byte(&(smallFont [(ch-32)*5 + j] )) );
		else
			writeData( pgm_read_byte(&(smallFont [(ch-32)*5 + j] )) ^ 0xff );
	}
	writeData( (mode == MENU_NORMAL) ? 0x00 : 0xff );
#endif
}

/*
 * Name         : write
 * Description  : write a character - override function in Print class
 * Argument(s)  : character to write
 * Return value : none
 */
WRITE_RESULT Nokia_3310_lcd::write(uint8_t b)
{
	writeChar(b,0);
	WRITE_RETURN
}

/*
 * Name         : gotoXY
 * Description  : Move text position cursor to specified position
 * Argument(s)  : x, y - Position, x = 0-83, y = 0-6
 * Return value : none
 */
void Nokia_3310_lcd::gotoXY(unsigned char x, unsigned char y) {
      if (x > LCDCOLMAX - 1) x = LCDCOLMAX - 1 ; // ensure within limits
      if (y > LCDROWMAX - 1) y = LCDROWMAX - 1 ; // ensure within limits

	writeCommand (0x80 | x);   //column
	writeCommand (0x40 | y);   //row

	cursor_row = y;
	cursor_col = x;
}


/*
 * Name         : clear
 * Description  : Clear the screen and display buffer
 * Argument(s)  : none
 * Return value : none
 */
void Nokia_3310_lcd::clear(void) {
	int i,j;
	
	gotoXY (0,0);  	//start with (0,0) home position

	for(i=0; i< LCDROWMAX; i++) {
		for(j=0; j< LCDCOLMAX; j++) {
			writeData( 0x00 );
#ifdef USE_GRAPHIC
			lcd_buffer[i][j] = 0x00;
#endif
		}
	}
   
      gotoXY (0,0);	//bring the XY position back to (0,0)
}


/*
 * Name         : update
 * Description  : Write the screen buffer to the display memory
 * Argument(s)  : none
 * Return value : none
 */
void Nokia_3310_lcd::update(void) {
#ifdef USE_GRAPHIC
	int i,j;

	for(i=0; i< LCDROWMAX; i++) {
		gotoXY (0,i);
		for(j=0; j< LCDCOLMAX; j++) {
			writeData(lcd_buffer[i][j]);
		}
	}
#endif
	gotoXY (0,0);	//bring the XY position back to (0,0)
}


#ifdef USE_GRAPHIC
/*
 * Name         : setPixel
 * Description  : Set a single pixel either on or off, update display buffer.
 * Argument(s)  : x,y - position, x = 0-83, y = 0-6
 *                c - colour, either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void Nokia_3310_lcd::setPixel( unsigned char x, unsigned char y, unsigned char c ) {
unsigned char value;
unsigned char row;
	
	if( x < 0 || x >= LCDCOLMAX || y < 0 || y >= LCDPIXELROWMAX ) return;

	row = y / 8;

	value = lcd_buffer[row][x];
	if( c == PIXEL_ON ) {
		value |= (1 << (y % 8));
	} else if( c == PIXEL_XOR ) {
		value ^= (1 << (y % 8));
	} else {
		value &= ~(1 << (y % 8));
	}

	lcd_buffer[row][x] = value;

	gotoXY (x,row);
	writeData(value);
}


/*
 * Name         : drawLine
 * Description  : Draws a line between two points on the display.
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for line origin.
 *                x2, y2 - Absolute pixel coordinates for line end.
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void Nokia_3310_lcd::drawLine(unsigned char x1, unsigned char y1,
		unsigned char x2, unsigned char y2, unsigned char c) {
    int dx, dy, stepx, stepy, fraction;

    /* Calculate differential form */
    /* dy   y2 - y1 */
    /* -- = ------- */
    /* dx   x2 - x1 */

    /* Take differences */
    dy = y2 - y1;
    dx = x2 - x1;

    /* dy is negative */
    if ( dy < 0 ) {
        dy    = -dy;
        stepy = -1;
    } else {
        stepy = 1;
    }

    /* dx is negative */
    if ( dx < 0 ) {
        dx    = -dx;
        stepx = -1;
    } else {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    /* Draw initial position */
    setPixel( x1, y1, c );

    /* Draw next positions until end */
    if ( dx > dy ) {
        /* Take fraction */
        fraction = dy - ( dx >> 1);
        while ( x1 != x2 ) {
            if ( fraction >= 0 ) {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;

            /* Draw calculated point */
            setPixel( x1, y1, c );
        }
    } else {
        /* Take fraction */
        fraction = dx - ( dy >> 1);
        while ( y1 != y2 ) {
            if ( fraction >= 0 ) {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;

            /* Draw calculated point */
            setPixel( x1, y1, c );
        }
    }
}


/*
 * Name         : drawRectangle
 * Description  : Draw a rectangle given to top left and bottom right points
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for top left corner
 *                x2, y2 - Absolute pixel coordinates for bottom right corner
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void Nokia_3310_lcd::drawRectangle(unsigned char x1, unsigned char y1,
		unsigned char x2, unsigned char y2, unsigned char c){
	drawLine( x1, y1, x2, y1, c );
	drawLine( x1, y1, x1, y2, c );
	drawLine( x1, y2, x2, y2, c );
	drawLine( x2, y1, x2, y2, c );
}


/*
 * Name         : drawFilledRectangle
 * Description  : Draw a filled rectangle given to top left and bottom right points
 * 		  just simply draws horizontal lines where the rectangle would be
 * Argument(s)  : x1, y1 - Absolute pixel coordinates for top left corner
 *                x2, y2 - Absolute pixel coordinates for bottom right corner
 *                c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : none
 */
void Nokia_3310_lcd::drawFilledRectangle(unsigned char x1, unsigned char y1,
		unsigned char x2, unsigned char y2, unsigned char c) {
	for(int i=y1; i <= y2; i++ ) {
		drawLine( x1, i, x2, i, c );
	}
}


/*
 * Name         : drawCircle
 * Description  : Draw a circle using Bresenham's algorithm.
 * 		  Some small circles will look like squares!!
 * Argument(s)  : xc, yc - Centre of circle
 * 		  r - Radius
 * 		  c - either PIXEL_ON, PIXEL_OFF or PIXEL_XOR
 * Return value : None
 */
void Nokia_3310_lcd::drawCircle(unsigned char xc, unsigned char yc,
		unsigned char r, unsigned char c) {
	int x=0;
	int y=r;
	int p=3-(2*r);

        setPixel( (byte)(xc+x),(byte)(yc-y), c);

	for(x=0;x<=y;x++) {
		if (p<0) {
			y=y;
			p=(p+(4*x)+6);
		} else {
			y=y-1;
			p=p+((4*(x-y)+10));
		}

		setPixel((byte)(xc+x),(byte)(yc-y), c);
		setPixel((byte)(xc-x),(byte)(yc-y), c);
		setPixel((byte)(xc+x),(byte)(yc+y), c);
		setPixel((byte)(xc-x),(byte)(yc+y), c);
		setPixel((byte)(xc+y),(byte)(yc-x), c);
		setPixel((byte)(xc-y),(byte)(yc-x), c);
		setPixel((byte)(xc+y),(byte)(yc+x), c);
		setPixel((byte)(xc-y),(byte)(yc+x), c);
	}
}
#endif

#ifdef USE_JOYSTICK
// Convert ADC value to key number
// adc preset value, represent top value,incl. noise & margin,that the adc reads, when a key is pressed
// set noise & margin = 30 (0.15V@5V)
int  adc_key_val[NUM_KEYS] ={
  30, 150, 360, 535, 760 };

char Nokia_3310_lcd::get_key() {
  int k;
  int input = analogRead(0);

  for (k = 0; k < NUM_KEYS; k++) {
    if (input < adc_key_val[k]) {
      return k;
    }
  }

  k = -1;     // No valid key pressed

  return k;
}
#endif
