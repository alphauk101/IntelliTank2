/*
*Intelli-Tank 2
 AVRDUDE exe line
 C:\arduino-1.0.3\hardware\tools\avr\bin>avrdude.exe -C C:\arduino-1.0.3\hardware\tools\avr\etc\avrdude.conf -c avrispmkII -P usb -p ATmega328P  -U flash:w:\Users\Lee\Desktop\mainActivity.hex -F
 */
#include <OneWire.h>
#include "DHT11.h"
#include <LiquidCrystal.h>
#include "screen.h"
#include "Lighting.h"
#include "Sensors.h"

/*PIN DEFINITIONS*/
#define dhtPIN A4
//light driver pins
int SBLUE_PIN = 9;
int SWHITE1_PIN = 11;
int SWHITE2_PIN = 10;
OneWire ds(8);
int red_led = 6;
int yellow_led = 5;
int green_led = A0;
int PIR_pin = A1;
int LDR_pin = A5;
int buzzer = 3;
int relay = 13;
/*PIN DEFINITIONS EOD*/

static int FULLPOWER = 1023;
static int HALFPOWER = 3098;
static int QUARTPOWER = 9345;
static int NIGHTPOWER = 7395;

boolean DAYTIME = true;
char degreeSymbol = 223;

float ALERT_TEMP = 0;//if the temp is anything other than zro show

unsigned long HALFPOWER_TIMER = 3600000; 
unsigned long QUARTPOWER_TIMER =3600000;
unsigned long CURRENT_TIME;

float ALARM_LOWTEMP = 15;//temp when alarm is triggered by cold water

//hood temp probe
dht DHT;
Screen screen;

Lighting light = Lighting(SWHITE1_PIN,SWHITE2_PIN,SBLUE_PIN);//lighting class
Sensors sensor;

void setup()
{
  Serial.begin(9600);//++++++++++++++++
 
 pinMode(relay,OUTPUT);
  delay(1000);
  screen.screenManager("Calibrating","Sensors");
  light.init();//start the lighting class
  pinMode(green_led, OUTPUT);//power led
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
  digitalWrite(green_led,HIGH);
  sensor.SetPins(PIR_pin,LDR_pin);//PIR,LDR
  sensor.init(screen);
  screen.screenManager("Calibrating","Done");
  showInitDone();

  CURRENT_TIME = millis();//set the timer so we have something to use
  delay(1000);
}

void loop()
{
  checkLight();//Checks the LDR and sets the appropriate flag to show day or night.

  if(DAYTIME)//now processes that flag
  {
    //Serial.println("DAY TIME");
    //its daytime so we need to manage half power etc
    if(timerOne(false))
    {
      //Serial.println("Half Power timer triggered");
      //we can assume that timer one has ALWAYS triggered if timer two has triggered
      if(timerTwo(false))//check the quater power trigger
      {
        //Serial.println("Quarter Power timer triggered");
        light.LightingOnMode(QUARTPOWER);
      }
      else
      {
        //go down to half power
        light.LightingOnMode(HALFPOWER);
      }
    }
    else
    {
      //Serial.println("going to full power");
      digitalWrite(relay,HIGH);
      light.LightingOnMode(FULLPOWER);
    }
  }
  else
  {
    //its night time s do to night time the lighting class will manage the duplicate requests
    //Serial.println("NIGHT TIME");
    light.LightingOnMode(NIGHTPOWER);

    int ran = random(0,10000);
    if(ran == 501) //random number to excute storm  
    {
      light.Storm();
    }
  }


  if(sensor.getStatus(sensor.PIR))//now time to check if the PIR has been triggered
  {
    //the PIR has seen something so reset the timers
    //Serial.println(">>>>>>>>>>>>>> PIR SENSOR TRIGGERED <<<<<<<<<<<<<<<");
    digitalWrite(red_led,HIGH);
    timerOne(true); 
    timerTwo(true);//reset the timers
    delay(250);
    digitalWrite(red_led,LOW);
  }

  doTempReadings();//gets the temps from the sensors
  delay(500);
  
  
  //Serial.println(analogRead(LDR_pin));
}

boolean WHICH_DISP = true;//decides which to show 
//Reads the sensors and then does the appropriates actions
void doTempReadings()
{
  //Once we have the readings we should check which to display to the user ALSO send to the BT when its functioning
  getHoodAtmos();//we now have a DHT object with temp hum properties

  float waterTemp = getWaterTemp();
  checkWaterTemp(waterTemp);//sound and store alarms
  
  if(WHICH_DISP)
  {
    //if true show hood temp
    int temp = DHT.temperature;
    String tempS = String(temp);
    String line1 = "Temp: " + tempS + degreeSymbol+"C";
    int hum = DHT.humidity;
    String humS = String(hum);
    String line2 = "Hum: " + humS + "%";
    screen.screenManager(line1,line2);
  }
  else
  {
    char* tempStr = (char*) malloc(4);
    dtostrf(waterTemp, 3, 1, tempStr);

  ///Serial playing
  byte btData[4];
  btData[0] = 0x01;
  btData[1] = 0x01;
  btData[2] = waterTemp;
  Serial.write(btData,3);
  Serial.flush();

    String temp = String(tempStr);
    temp = temp + degreeSymbol+"C";
    
    if(ALERT_TEMP != 0)//as in its got a reading
    {
      
      char* alertStr = (char*) malloc(4);
      dtostrf(ALERT_TEMP, 3, 1, alertStr);
  
      String tempAl = String(alertStr);
      tempAl = tempAl + degreeSymbol+"C";
      
      screen.screenManager("Water Temp: " + temp, "ALERT: " + tempAl);
      free(alertStr);
      
    }else
    {
      screen.screenManager("Water Temp: " , temp);
    }
    
    free(tempStr);
  }
  WHICH_DISP = ! WHICH_DISP;
}

void checkWaterTemp(float temp)
{
  if(temp < ALARM_LOWTEMP)
  {
    //sound alarm and probably log it!
    alarm();//sound alarm
    //probably save temp so when can display this
    if((temp < ALERT_TEMP) || (ALERT_TEMP == 0))
    {
      ALERT_TEMP = temp;
    }
  }
}

void alarm()
{
  for(int a = 0; a < 5; a++)
  {
    analogWrite(buzzer,500);
    delay(250);
    analogWrite(buzzer,0);
    delay(250);
  }
  analogWrite(buzzer,0);//makesure the alarm is off
}
//checks the light level in the room and set DAYTIME flags
void checkLight()
{
  if(sensor.getStatus(sensor.LDR))//if its day
  { 
    if(! DAYTIME)
    { 
      delay(3000);
      if(sensor.getStatus(sensor.LDR))
      {
        DAYTIME = true;
      }
    }
  }
  else //its night
  {
    if(DAYTIME)
    {
      delay(3000);
      if(! sensor.getStatus(sensor.LDR))//returns night again!
      {
        DAYTIME = false;
      }
    }
  }
  if(DAYTIME)
  {
    digitalWrite(yellow_led, LOW);
  }
  else
  {
    for(int a = 0; a < 10; a++)
    {
      digitalWrite(yellow_led, HIGH);
      delay(100);
      digitalWrite(yellow_led, LOW);
      delay(100);
    }
    digitalWrite(yellow_led, LOW);//make sure the yellow LED is off as it disturpts the night mode.
  }
}



double getWaterTemp()
{
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  //get address
  if ( !ds.search(addr)) {
    ds.reset_search();
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); //begin temp reading

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  //we have our two bytes of temoerature readings
  byte LSB = data[0];
  byte MSB = data[1];
  /*
  int tempInt = ((MSB << 8) | LSB);
   tempInt = (6 * tempInt) + tempInt / 4;
   
   int frac = tempInt % 100;
   tempInt = tempInt / 100;
   
   String tempStr = String(tempInt);
   String fracStr = String(frac);
   tempStr = "Water: " + tempStr + "." + fracStr + degreeSymbol+"C";
   screen.screenManager(tempStr, "No Alerts");
   */
  int tempInt = ((MSB << 8) | LSB);
  float tempD = ((6 * tempInt) + tempInt / 4);
  tempD = tempD / 100;

  return tempD;
}

void getHoodAtmos()
{
  DHT.read11(dhtPIN);//this sets properties within itself 
  /*
  int temp = DHT.temperature;
   String tempS = String(temp);
   String line1 = "Temp: " + tempS + degreeSymbol+"C";
   int hum = DHT.humidity;
   String humS = String(hum);
   
   String line2 = "Hum: " + humS + "%";
   
   screen.screenManager(line1,line2);
   */
}


void showInitDone()
{
  for(int a = 0 ;a < 5; a++)
  {
    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);
    digitalWrite(yellow_led, LOW);

    digitalWrite(red_led,HIGH);
    delay(250);
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, HIGH);
    delay(250);
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, HIGH);
    delay(250);
  }
  digitalWrite(red_led, LOW);
  digitalWrite(green_led, LOW);
  digitalWrite(yellow_led, LOW);
}


/**************************
 * TIMERS
 ***************************/
//if a true is passed then timer is reset is a false is passed then we are just updating
boolean timerOne(boolean reset)
{
  //if a true is passed then timer is reset is a false is passed then we are just updating

  if(reset)
  {
    //reset the timer
    CURRENT_TIME = millis();

  }
  else
  {
    //Serial.print((millis() - CURRENT_TIME) / 1000);
    //Serial.println(" Seconds passed");
    //if time up then return true else return false
    if((millis() - CURRENT_TIME) > HALFPOWER_TIMER)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  //If a true is returned then the timer is up
}

//if a true is passed then timer is reset is a false is passed then we are just updating
boolean timerTwo(boolean reset)
{
  //if a true is passed then timer is reset is a false is passed then we are just updating

  if(reset)
  {
    //reset the timer
    CURRENT_TIME = millis();
  }
  else
  {
    //if time up then return true else return false
    if((millis() - CURRENT_TIME) > QUARTPOWER_TIMER)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  //If a true is returned then the timer is up
}


