#include <nokia_3310_lcd.h>

// Temp/humidity display using nokia 3310 LCD display shield from nuelectronics.com

#include "SHT1x.h"
#include "Andrew.h"
#include "pacman.h"

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  5
#define clockPin 6
SHT1x sht1x(dataPin, clockPin);

//keypad debounce parameter
#define DEBOUNCE_MAX 15
#define DEBOUNCE_ON  10
#define DEBOUNCE_OFF 3 

#define NUM_KEYS 5

#define NUM_MENU_ITEM	5

// joystick number
#define UP_KEY 3
#define LEFT_KEY 0
#define CENTER_KEY 1
#define DOWN_KEY 2
#define RIGHT_KEY 4

// Pin used by Backlight, so we can turn it on and off. Pin setup in LCD init function
#define BL_PIN 7

// menu starting points
#define MENU_X	10		// 0-83
#define MENU_Y	0		// 0-5

// adc preset value, represent top value,incl. noise & margin,that the adc reads, when a key is pressed
// set noise & margin = 30 (0.15V@5V)
int  adc_key_val[5] ={
  30, 150, 360, 535, 760 };

// debounce counters
byte button_count[NUM_KEYS];
// button status - pressed/released
byte button_status[NUM_KEYS];
// button on flags for user program 
byte button_flag[NUM_KEYS];

// menu definition
char menu_items[NUM_MENU_ITEM][12]={
  "TEMPERATURE",
  "HUMIDITY",
  "BACKLIGHT",
  "DEMO",
  "ABOUT"	
};

void (*menu_funcs[NUM_MENU_ITEM])(void) = {
  temperature,
  humidity,
  backlight,
  demo,
  about
};

char current_menu_item;
int blflag = 1;  // Backlight initially ON
char degree = 0x7b;  // Degree symbol


Nokia_3310_lcd lcd=Nokia_3310_lcd();

void setup()
{
  // setup interrupt-driven keypad arrays  
  // reset button arrays
  for(byte i=0; i<NUM_KEYS; i++){
    button_count[i]=0;
    button_status[i]=0;
    button_flag[i]=0;
  }

  // Setup timer2 -- Prescaler/256
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
  TCCR2B &= ~(1<<WGM22);
  TCCR2B = (1<<CS22)|(1<<CS21);      

  ASSR |=(0<<AS2);

  // Use normal mode  
  TCCR2A =0;    
  //Timer2 Overflow Interrupt Enable  
  TIMSK2 |= (0<<OCIE2A);
  TCNT2=0x6;  // counting starts from 6;  
  TIMSK2 = (1<<TOIE2);    

  SREG|=1<<SREG_I;

  lcd.init();
  lcd.clear();

  //menu initialization
  init_MENU();
  current_menu_item = 0;	
}


/* loop */
void loop() {
  byte i;
  for(i=0; i<NUM_KEYS; i++) {
    if(button_flag[i] !=0) {

      button_flag[i]=0;  // reset button flag
      switch(i){
      case UP_KEY:
        // current item to normal display
        lcd.writeString(MENU_X, MENU_Y + current_menu_item, menu_items[current_menu_item], MENU_NORMAL );
        current_menu_item -=1;
        if(current_menu_item <0)  current_menu_item = NUM_MENU_ITEM -1;
        // next item to highlight display
        lcd.writeString(MENU_X, MENU_Y + current_menu_item, menu_items[current_menu_item], MENU_HIGHLIGHT );
        break;
      case DOWN_KEY:
        // current item to normal display
        lcd.writeString(MENU_X, MENU_Y + current_menu_item, menu_items[current_menu_item], MENU_NORMAL );
        current_menu_item +=1;
        if(current_menu_item >(NUM_MENU_ITEM-1))  current_menu_item = 0;
        // next item to highlight display
        lcd.writeString(MENU_X, MENU_Y + current_menu_item, menu_items[current_menu_item], MENU_HIGHLIGHT );
        break;
      case LEFT_KEY:
        init_MENU();
        current_menu_item = 0;
        break;   
      case RIGHT_KEY:
        lcd.clear();
        (*menu_funcs[current_menu_item])();
        lcd.clear();
        init_MENU();
        current_menu_item = 0;           
        break;	
      }
    }
  }
}

/* menu functions */
void init_MENU(void) {

  byte i;
  lcd.clear();
  lcd.writeString(MENU_X, MENU_Y, menu_items[0], MENU_HIGHLIGHT );

  for (i=1; i<NUM_MENU_ITEM; i++) {
    lcd.writeString(MENU_X, MENU_Y+i, menu_items[i], MENU_NORMAL);
  }
}

// waiting for center key press
void waitfor_OKkey() {
  byte i;
  byte key = 0xFF;
  while (key!= CENTER_KEY){
    for(i=0; i<NUM_KEYS; i++){
      if(button_flag[i] !=0){
        button_flag[i]=0;  // reset button flag
        if(i== CENTER_KEY) key=CENTER_KEY;
      }
    }
  }
}

// Check if joystick is moved or pressed
byte checkKeypressed() {
  byte key = 0xFF;

  for(int i=0; i<NUM_KEYS; i++){
    if(button_flag[i] !=0){
      button_flag[i]=0;  // reset button flag
      if(i== CENTER_KEY) key=CENTER_KEY;
    }
  }
  return key;
}

char numStr[8];

// Format a number to 2 decimal places
void formatNum( int num ) {
  // First part before decimalpoint
  itoa( num / 100, numStr, 10 );
  int pos = strlen( numStr );
  numStr[pos++] = '.';
  int decimal = num % 100;
  if( decimal > 9 ) {
    itoa( decimal, &numStr[pos], 10 );
  } else {
    numStr[pos++] = '0';
    itoa( decimal, &numStr[pos], 10 );
  }
}

// Display temperature in big digits, humidity in small digits underneath
void temperature() {
  int temp, humid;

  // Display non changing text, there is a slight delay while first reading is taken
  lcd.gotoXY( 62,1 );
  lcd.print( degree );
  lcd.print( "C" );
//  lcd.writeString(67, 1, "C", MENU_NORMAL);
  lcd.writeString(38, 5, "OK", MENU_HIGHLIGHT );
  lcd.writeString(40, 3, "%RH", MENU_NORMAL);

  long lastUpdate = 0;  // Force update
  byte i;
  byte key = 0xFF;

  // Loop to display temperaure/humidity with check for key press to exit
  while (key!= CENTER_KEY) {
    // Update temp
    if( millis() > lastUpdate + 1000) {
      // Read temperature and humidity
      temp = (int)(sht1x.readTemperatureC() * 100);
      humid = (int)(sht1x.readHumidity() * 100);

      // Display temp
      formatNum( temp );
      lcd.writeStringBig(8, 0, numStr, MENU_NORMAL);

      // Display Humidity
      formatNum( humid );
      lcd.writeString(10, 3, numStr, MENU_NORMAL);

      lastUpdate = millis();
    }
    key = checkKeypressed();
  }
}

// Display humidity in big digits, temperature in small digits underneath
void humidity() {
  int temp, humid;

  // Display non changing text
  lcd.writeString(66, 1, "%RH", MENU_NORMAL);
  lcd.gotoXY( 40,3 );
  lcd.print( degree );
  lcd.print( "C" );
//  lcd.writeString(40, 3, "C", MENU_NORMAL);
  lcd.writeString(38, 5, "OK", MENU_HIGHLIGHT );

  long lastUpdate = 0;  // Force update
  byte i;
  byte key = 0xFF;
  // Loop to display temperaure/humidity with check for key press to exit

  while (key!= CENTER_KEY){
    // Update temp
    if( millis() > lastUpdate + 1000) {
      // Read temperature and humidity
      temp = (int)(sht1x.readTemperatureC() * 100);
      humid = (int)(sht1x.readHumidity() * 100);
      
      // Display humidity
      formatNum( humid );
      lcd.writeStringBig(8, 0, numStr, MENU_NORMAL);

      // Display temperature
      formatNum( temp ); 
      lcd.writeString(10, 3, numStr, MENU_NORMAL);

      lastUpdate = millis();
    }
    key = checkKeypressed();
  }
}

// Display the about information
void about(){

  lcd.gotoXY( 0, 1 );
  lcd.print("Temp/Humidity" );
  lcd.gotoXY( 0, 2 );
  lcd.print( "Nokia 3110 LCD" );
  lcd.gotoXY( 0, 3 );
  lcd.print( "A D Lindsay" );
  lcd.writeString(38, 5, "OK", MENU_HIGHLIGHT );
  waitfor_OKkey();
}

// Display the simple graphics demo
void demo(){
  lcd.writeString( 0, 1, "Text Demo", MENU_NORMAL);
  lcd.writeString(24, 5, "START", MENU_HIGHLIGHT );
  waitfor_OKkey();
  lcd.clear();
  
  lcd.writeStringBig( 0, 0, "123456", MENU_NORMAL );
  lcd.writeStringBig( 0, 3, "7890+-.", MENU_NORMAL );
  delay(10000);
  
  lcd.writeStringBig( 0, 0, "123456", MENU_HIGHLIGHT );
  lcd.writeStringBig( 0, 3, "7890+-.", MENU_HIGHLIGHT );
  delay(10000);

  lcd.clear();

  lcd.writeString( 0, 1, "Graphic Demo", MENU_NORMAL);
  lcd.writeString(24, 5, "START", MENU_HIGHLIGHT );
  waitfor_OKkey();
  lcd.clear();
  // Draw some circles pulsing in and out
  for(int a=0; a< 4; a++) {
    for( int r = 1; r < 49; r+=1 ) {
      lcd.drawCircle(42, 24, r, PIXEL_ON );
      delay(10);
    }
    delay(10);
    for( int r = 48; r >0; r-=1 ) {
      delay(10);
      lcd.drawCircle(42, 24, r, PIXEL_OFF );
    }
  }

  // More circles
  for( int xc = 10; xc < 85; xc+=15 ) {
    lcd.drawCircle(xc, 24, 20, PIXEL_ON );
  }
  delay( 2000 );
    
  // Draw diagonal lines using XOR colour
  lcd.drawLine(0,0,83,47, PIXEL_XOR);
  lcd.drawLine(0,43,83,0, PIXEL_XOR);

  delay( 2000 );
  
  // Draw a rectangle
  lcd.drawRectangle(5,5,78,42, PIXEL_ON);

  delay( 2000 );

  // Draw 2 filled rectangles
  lcd.drawFilledRectangle(5,3,78,20, PIXEL_ON);
  lcd.drawFilledRectangle(5,25,78,42, PIXEL_ON);

  delay( 2000 );

  // Draw bitmap image
  lcd.drawBitmapP( 0,0, &Andrew[0],84,48);
  delay(5000);

  // Pacman animation
  lcd.clear();
  int px = 0;
  int py = 1;
  int pause=70;
  for( int p=0; p <9; p++) {
    lcd.drawBitmapP( px,py, &pacman1[0],32,32);
    delay( pause );
    lcd.clearBitmap( px++,py, 32,32);
    lcd.drawBitmapP( px,py, &pacman2[0],32,32);
    delay( pause ); 
    lcd.clearBitmap( px++,py, 32,32);
    lcd.drawBitmapP( px,py, &pacman3[0],32,32);
    delay( pause ); 
    lcd.clearBitmap( px++,py, 32,32);
    lcd.drawBitmapP( px,py, &pacman4[0],32,32);
    delay( pause ); 
    lcd.clearBitmap( px++,py, 32,32);
    lcd.drawBitmapP( px,py, &pacman3[0],32,32);
    delay( pause );
    lcd.clearBitmap( px++,py, 32,32);
    lcd.drawBitmapP( px,py, &pacman2[0],32,32);
    delay( pause ); 
    lcd.clearBitmap( px++,py, 32,32);
  }
  lcd.drawBitmapP( px,py, &pacman1[0],32,32);

  delay( 5000 );

  lcd.clear();

  lcd.writeString( 0, 1, "Graphic Demo", MENU_NORMAL);
  lcd.writeString( 0, 3, "The End!!", MENU_NORMAL);
  lcd.writeString(38, 5, "OK", MENU_HIGHLIGHT );
  waitfor_OKkey();
}

// Allow backlight to be turned on and off
void backlight() {

  lcd.writeString( 0, 1, "Toggle", MENU_NORMAL);
  lcd.writeString( 0, 2, "Backlight", MENU_NORMAL);
  if( blflag != 0 ) {
    lcd.writeString( 60, 2, "Off", MENU_HIGHLIGHT);
  } 
  else {
    lcd.writeString( 60, 2, "On", MENU_HIGHLIGHT);
  }

  waitfor_OKkey();
  
  if( blflag != 0 ) {
    blflag=0;
    digitalWrite( BL_PIN, LOW );
  }   else {
    blflag = 1;
    digitalWrite( BL_PIN, HIGH );
  }
}

// The followinging are interrupt-driven keypad reading functions
//  which includes DEBOUNCE ON/OFF mechanism, and continuous pressing detection


// Convert ADC value to key number
char get_key(unsigned int input) {
  char k;

  for (k = 0; k < NUM_KEYS; k++) {
    if (input < adc_key_val[k]) {
      return k;
    }
  }

  if (k >= NUM_KEYS)
    k = -1;     // No valid key pressed

  return k;
}

void update_adc_key() {
  int adc_key_in;
  char key_in;
  byte i;

  adc_key_in = analogRead(0);
  key_in = get_key(adc_key_in);
  for(i=0; i<NUM_KEYS; i++) {
    if(key_in==i) { //one key is pressed 
      if(button_count[i]<DEBOUNCE_MAX)       {
        button_count[i]++;
        if(button_count[i]>DEBOUNCE_ON)         {
          if(button_status[i] == 0)           {
            button_flag[i] = 1;
            button_status[i] = 1; //button debounced to 'pressed' status
          }
        }
      }
    } else  { // no button pressed
      if (button_count[i] >0) {  
        button_flag[i] = 0;	
        button_count[i]--;
        if(button_count[i]<DEBOUNCE_OFF) {
          button_status[i]=0;   //button debounced to 'released' status
        }
      }
    }
  }
}

// Timer2 interrupt routine -
// 1/(160000000/256/(256-6)) = 4ms interval

ISR(TIMER2_OVF_vect) {  
  TCNT2  = 6;
  update_adc_key();
}

