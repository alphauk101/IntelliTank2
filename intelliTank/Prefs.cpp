#include <EEPROM.h>

/*Allows access to EEPROM primarily to save settings etc*/
#include "Prefs.h"
#include <arduino.h>
#include "defines.h"



//everything is store MSB - LSB
//constructor not sure if this is necessary
Prefs::Prefs()
{
  
}

int Prefs::getValue(int which)
{
  byte a;
  byte b;
  int result = 0;
  switch(which)
  {
    case(MEM_LIGHT_LEVEL):
    //get the level 
      a = EEPROM.read(MEM_LIGHT_LEVEL);
      b = EEPROM.read(MEM_LIGHT_LEVEL + 1);
      result = (a << 8) | b;
      return result;
    break;
    
    case(MEM_ALARM_TEMP):
    //get the level 
      a = EEPROM.read(MEM_ALARM_TEMP);
      b = EEPROM.read(MEM_ALARM_TEMP + 1);
      result = (a << 8) | b;
      return result;
    break;
  }
}

void Prefs::storeValue(int where, int value)
{
  
}
