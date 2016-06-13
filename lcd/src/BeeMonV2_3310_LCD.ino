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

    lcd.writeString(0,0,"Crown", MENU_NORMAL );
    lcd.writeString(0,1.5,"Ambient", MENU_NORMAL );
    lcd.writeString(0,2.0,"Super", MENU_NORMAL );
    lcd.writeString(0,2.5,"Cluster", MENU_NORMAL );
    lcd.writeString(0,3.0,"Battery", MENU_NORMAL );


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

    lcd.writeString(36,0,"^Cluster", MENU_NORMAL );
    lcd.writeString(36,9,"Ambient", MENU_NORMAL );
    lcd.writeString(36,18,"^Crown", MENU_NORMAL );
    lcd.writeString(36,27,"Cluster", MENU_NORMAL );
    lcd.writeString(35  ,37,"Battery", MENU_NORMAL );

    char str[4];
    dtostrf(A,0,1, str);
    strcat(str,"a");
    lcd.writeString(0,0,str, MENU_NORMAL );

    dtostrf(B,0,1, str);
    strcat(str,"e");
    lcd.writeString(0,9,str, MENU_NORMAL );

    dtostrf(C,0,1, str);
    strcat(str,"k");
    lcd.writeString(0,18,str, MENU_NORMAL );

    dtostrf(D,0,1, str);
    strcat(str,"m");
    lcd.writeString(0,27,str, MENU_NORMAL );



    dtostrf(E,0,1, str);
    strcat(str,"u");
    lcd.writeString(0,37,str, MENU_NORMAL );
  }



}
