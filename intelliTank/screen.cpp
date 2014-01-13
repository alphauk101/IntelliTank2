#include <LiquidCrystal.h>

#include "screen.h"

LiquidCrystal lcd(2, 4, 7, A0, A1, A2, A3);

Screen::Screen()
{
  lcd.begin(16, 2);
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
