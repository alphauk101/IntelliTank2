/*Sensor to control behaves of the attached sensors*/
#include "Sensors.h"
#include <arduino.h>

static int NIGHT_LEVEL = 600;
char blank = 219;
//int PIR = 20;
//int LDR = 21;
int PIR;
int LDR;

static int PIR_PIN, LDR_PIN;
/*sets the pins ready for init*/
void Sensors::SetPins(int mPIR, int mLDR)
{
  PIR_PIN = mPIR;
  LDR_PIN = mLDR;
  /*
  Serial.print("PIR pin: ");
  Serial.println(PIR_PIN);
  Serial.print("LDR pin: ");
  Serial.println(LDR_PIN);
  **/
}

void Sensors::init(Screen screen)
{
  //set the pins to input mode
  pinMode(PIR_PIN,INPUT);
  pinMode(LDR_PIN,INPUT);
  
  LDR = 2014;
  PIR = 1321;
  String track = String(blank);
  //Serial.println("Intialising PIR");
  for(int a = 0; a < 5; a++)//<<<<<<<<< Change this back to 30 in production
  {
    if(a % 2)
    {
      track = track + blank;
    }
    screen.screenManager("Calibrating:",track);

    delay(1000);
  }
  
  //LED to show status
  pinMode(31,OUTPUT);
  
  
}

int Sensors::getLDRValue()
{
  return analogRead(LDR_PIN);
}

bool Sensors::getStatus(int sensor)
{
  if(sensor == LDR)
  {

    //check the pir status
    //here we need to check that the light levels are above 
    //what we would consider night if not switch to night mode
    //Serial.println(analogRead(LDR_PIN));
    
    if(_getLDRValue() >= NIGHT_LEVEL)
    {
      //Serial.println("LDR true day");
      return true;
    }
    else
    {
      //Serial.println("LDR false night");
      return false;
    }
  }else if(sensor == PIR)
  {
    //Serial.println(analogRead(PIR_PIN));
    //check the ldr status
    if(analogRead(PIR_PIN) > 400)
    {
      return true;
    }else
    {
      return false;
    }
  }
}
//does a quick average of 10 readings to makesure that there a anomally reading is used
int Sensors::_getLDRValue()
{
  //we shall take 10 readings in
  int med = 0; 
  for(int a = 0; a < 10; a++)
  {
    med += analogRead(LDR_PIN);
    delay(5);
  }
  int res  = (med / 10);
  //Serial.println(res);
  return res;
}
