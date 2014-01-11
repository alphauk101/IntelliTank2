#include <LiquidCrystal.h>
#include <dht.h>

#define dhtPIN A4

dht DHT;
LiquidCrystal lcd(2, 4, 7, A0, A1, A2, A3);

void setup()
{
  lcd.begin(16, 2);
  //lcd.autoscroll();//makes the lcd scroll if text to long
  lcd.print("Temp and humidity");
  Serial.begin(9600);
  delay(1000);
}

void loop()
{
  DHT.read11(dhtPIN);//this sets properties within itself 

  double temp = DHT.temperature;
  char* tempStr = (char*) malloc(5);
  dtostrf(temp, 5, 2, tempStr);

  String tempS = String(tempStr);

  String line1 = "Temp: " + tempS + "C";
  free(tempStr);
  
   double hum = DHT.humidity;
  char* humStr = (char*) malloc(5);
  dtostrf(hum, 5, 2, humStr);

  String humS = String(humStr);
  
  String line2 = "Hum: " + humS + "%";
  free(humStr);

  screenManager(line1,line2);

  delay(3000);
  
}

void screenManager(String line1, String line2)//we pass both lines
{
  //whenever a new text arrive remove old text
  lcd.clear();
  //reset the cursor
  lcd.setCursor(0,0);//column 0 line 0
  //print top line
  lcd.print(line1);
  //go to next line
  lcd.setCursor(0,1);
  lcd.print(line2);
  //done
}

