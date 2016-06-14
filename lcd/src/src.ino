#include "nokia_3310_lcd.h"
#include <VirtualWire.h> // library for RF RX/TX

Nokia_3310_lcd lcd=Nokia_3310_lcd();


int variableA, variableB, variableC, variableD, variableE;
float A,B,C,D,E;

void setup()
{
   //---------------------------------------------------------
  // Reciever setup
  //---------------------------------------------------------
  vw_set_ptt_inverted(true);          // Required for DR3100
  vw_setup(2000);	              // Bits per sec
  vw_set_rx_pin(6);
  vw_rx_start();                      // Start
  //---------------------------------------------------------


  lcd.init();
  lcd.clear();

  Serial.begin(9600);
  Serial.println("BeeMonV2.5 lcd");

    lcd.writeString(0,0,"Bee Monitor V2", MENU_HIGHLIGHT );
    lcd.writeString(0,3.5,"NO DATA", MENU_NORMAL );
    lcd.writeString(0,4.5,"RECEIVED", MENU_NORMAL );
    lcd.writeString(0,5.5,":-(", MENU_HIGHLIGHT );


}

void loop()
{




 if (readDataRF())
   
  {

    A=variableA/100.0;
    B=variableB/100.0;
    C=variableC/100.0;
    D=variableD/100.0;
    E=variableE/100.0;

    Serial.print(A); Serial.print(" ");
    Serial.print(B); Serial.print(" ");
    Serial.print(C); Serial.print(" ");
    Serial.println(D); Serial.print(" ");
    Serial.print(E);
    
    lcd.clear();
    lcd.writeString(0,0,"Bee Monitor V2", MENU_HIGHLIGHT );
    lcd.writeString(0,1.5,"Crown:", MENU_NORMAL );
    lcd.writeString(0,2.5,"Ambient:", MENU_NORMAL );
    lcd.writeString(0,3.5,"Super:", MENU_NORMAL );
    lcd.writeString(0,4.0,"Cluster:", MENU_NORMAL );
    lcd.writeString(0,5.0,"Battery:", MENU_NORMAL );

    char str[4];
    dtostrf(A,0,1, str);
    strcat(str,"C");
    lcd.writeString(48,1.5,str, MENU_NORMAL );

    dtostrf(B,0,1, str);
    strcat(str,"C");
    lcd.writeString(48,2.5,str, MENU_NORMAL );

    dtostrf(C,0,1, str);
    strcat(str,"C");
    lcd.writeString(48,3.5,str, MENU_NORMAL );

    dtostrf(D,0,1, str);
    strcat(str,"C");
    lcd.writeString(48,4.0,str, MENU_NORMAL );



    dtostrf(E,0,1, str);
    strcat(str,"V");
    lcd.writeString(48,5.0,str, MENU_NORMAL );
  }

}
