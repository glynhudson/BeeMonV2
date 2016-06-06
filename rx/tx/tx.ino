/*-------------------------------------------------------------
Bee Monitor V2.0
Glyn Hudson 28/03/2016
--------------------------------------------------------------*/
#include <Ports.h>
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#include <OneWire.h>
#include <DallasTemperature.h>

#include <VirtualWire.h>             // library for RF RX/TX [VirtualWire 1.3] http://download.milesburton.com/Arduino/VirtualWire/VirtualWire.rar

#define DEBUG

#define ONE_WIRE_BUS 2
#define LEDpin 13
#define HUMIDITYpin1 4
#define HUMIDITYpin2 3
#define RFpin 5
#define batteryADC 1

#define temp_resolution 12

DeviceAddress address_T1 = { 0x28, 0x28, 0xCF, 0xE0, 0x03, 0x00, 0x00, 0xE9 };
DeviceAddress address_T2 = { 0x28, 0xF5, 0x88, 0xE0, 0x03, 0x00, 0x00, 0xDF };
DeviceAddress address_T3 = { 0x28, 0x6F, 0xAB, 0xE0, 0x03, 0x00, 0x00, 0x17 };
DeviceAddress address_T4 = { 0x28, 0x4E, 0x94, 0x7A, 0x03, 0x00, 0x00, 0x87 };

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
int numberOfDevices; // Number of temperature devices found
// arrays to hold device address
DeviceAddress tempDeviceAddress;


int temp1=0;
int temp2=0;
int temp3=0;
int temp4=0;
int temp5=0;
int battery=0;

  int variableA = temp1;
  int variableB = temp2;
  int variableC = temp3;
  int variableD = temp4;
  int variableE = battery;


//-----------------------------------------------------------------------------------------------------------------------------------
// Send data via RF
//----------------------------------------------------------------------------------------------------------------------------------
  void rfWrite(const char *msg)
{
  vw_send((uint8_t *)msg, strlen(msg));
  vw_wait_tx();                              // Wait until the whole message is gone
  delay(10);
}
//-------------------------------------------------------------------------------------------------------------------

//********************************************************************
//SETUP
//********************************************************************
void setup(){
  
  Serial.begin(9600);
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,HIGH);
  
  Serial.println("Bee Monitor V2 315Mhz");
  Serial.println("openenergymonitor.org");
  
  sensors.begin();
  
  numberOfDevices = sensors.getDeviceCount(); Serial.print("Temperature sensors found: "); Serial.println(numberOfDevices);
  for(int i=0;i<numberOfDevices; i++)
  {
    if (sensors.getAddress(tempDeviceAddress, i)) sensors.setResolution(tempDeviceAddress, temp_resolution);
  }
  
  //-----------------------------------------------------------
  // Setup the transmitter - 315Mhz
  //------------------------------------------------------------
  vw_set_tx_pin(RFpin);
  vw_set_ptt_inverted(true);                 // Required for DR3100
  vw_setup(2000);	                     // Bits per
  //------------------------------------------------------------
  
delay(3000);
digitalWrite(LEDpin,LOW);

}


//********************************************************************
//LOOP
//********************************************************************
void loop(){
  
  
 
 sensors.requestTemperatures();
 
  
 
  temp1 = sensors.getTempC(address_T1) * 100;
  temp2 = sensors.getTempC(address_T2) * 100;
  temp3 = sensors.getTempC(address_T3) * 100;
  temp4 = sensors.getTempC(address_T4) * 100;
 
  battery=((((analogRead(batteryADC))*0.00488)*2.766))*100; //4.7V @ 13V = multiple ADC V by 2.766 to give battery voltage
//assuming 1024 ADC abd 5V VCC, 4.88mV per ADC
//4.7V = 953 ADC
    
    //------------------------------------------------------------
  // Send Data Via RF
  //------------------------------------------------------------
  
  //define variables to send and convert float to integer
  variableA = temp1;
  variableB = temp2;
  variableC = temp3;
  variableD = temp4;
  variableE = battery;
  
  
  char str_rf[30];                          //create string
  
  itoa(variableA,str_rf,10);               //Convert to string
  strcat(str_rf,"A");                      //Add identifier character
  rfWrite(str_rf);                         //Send the string

  itoa(variableB,str_rf,10);               //Convert to string
  strcat(str_rf,"B");                      //Add identifier character
  rfWrite(str_rf);                         //Send the string
  
  itoa(variableC,str_rf,10);               //Convert to string
  strcat(str_rf,"C");                      //Add identifier character
  rfWrite(str_rf);                         //Send the string
  
  itoa(variableD,str_rf,10);               //Convert to string
  strcat(str_rf,"D");                      //Add identifier character
  rfWrite(str_rf);                         //Send the string
  
    itoa(variableE,str_rf,10);               //Convert to string
  strcat(str_rf,"E");                      //Add identifier character
  rfWrite(str_rf);                         //Send the string

  //------------------------------------------------------------


#ifdef DEBUG
Serial.print(temp1); Serial.print(" ");
Serial.print(temp2); Serial.print(" ");
Serial.print(temp3); Serial.print(" ");
Serial.print(temp4); Serial.print(" ");
Serial.println(battery);
delay(200);
#endif


digitalWrite(LEDpin, HIGH);
delay(100);
digitalWrite(LEDpin, LOW);



Sleepy::loseSomeTime(60000);  //JeeLabs power save function: enter low power mode and update Arduino millis
//only be used with time ranges of 16..65000 milliseconds, and is not as accurate as when running normally.http://jeelabs.org/2010/10/18/tracking-time-in-your-sleep/
}




  

