#include <LiquidCrystal.h>

#include "screen.h"

LiquidCrystal lcd(2, 4, 7, A0, A1, A2, A3);

Screen::Screen()
{
  lcd.begin(16, 2);
  lcd.print("  Intelli-Tank");
  //delay(1000);
}

void Screen::screenManager(String line1, String line2)//we pass both lines
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

void Screen::transist()
{
  for(int count = 0; count < 16; count++)
  {
    String line1;
    String line2;
    
    for(int inner = 0; inner <= count; inner++)
    {
      line1 += "-";
      line2 += "-";
    }
    
    screenManager(line1,line2);
  }
}
