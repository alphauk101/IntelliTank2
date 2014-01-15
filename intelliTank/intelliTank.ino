#include "DHT11.h"
#include <LiquidCrystal.h>
#include "screen.h"

#define dhtPIN A4

dht DHT;
Screen screen;

void setup()
{

  Serial.begin(9600);
  delay(1000);
}

void loop()
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

  delay(3000);
}



