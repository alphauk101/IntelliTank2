#include <OneWire.h>

#include "DHT11.h"
#include <LiquidCrystal.h>
#include "screen.h"

#define dhtPIN A4
OneWire ds(8);

dht DHT;
Screen screen;

void setup()
{

  Serial.begin(9600);
  delay(1000);
}

void loop()
{

  //delay(3000);
  //getHoodAtmos();
  delay(1000);
  getWaterTemp();
}

void getWaterTemp()
{
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  //get address
  if ( !ds.search(addr)) {
    ds.reset_search();
    return;
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

  int tempInt = ((MSB << 8) | LSB);
  if(tempInt & 0x8000)
  {
    tempInt = (tempInt ^ 0xffff) + 1;//for minus purposes only
  }
  tempInt = (6 * tempInt) + tempInt / 4;
  
  int frac = tempInt % 100;
  tempInt = tempInt / 100;
   

  String tempStr = String(tempInt);
  String fracStr = String(frac);
  tempStr = "Water:" + tempStr + "." + fracStr + "C";
  screen.screenManager(tempStr, "No Alerts");

}

void getHoodAtmos()
{
  DHT.read11(dhtPIN);//this sets properties within itself 

  double temp = DHT.temperature;
  char* tempStr = (char*) malloc(5);
  dtostrf(temp, 2, 2, tempStr);

  String tempS = String(tempStr);

  String line1 = "Temp: " + tempS + "C";
  free(tempStr);

  double hum = DHT.humidity;
  char* humStr = (char*) malloc(5);
  dtostrf(hum, 2, 2, humStr);

  String humS = String(humStr);

  String line2 = "Hum: " + humS + "%";
  free(humStr);

  screen.screenManager(line1,line2);

}



