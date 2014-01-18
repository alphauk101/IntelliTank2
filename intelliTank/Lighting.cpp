/*Lighting class which will control lighting levels 
 * changes in level of lighting and possibly feedback
 *errors.*/
#include "Lighting.h"
#include <arduino.h>

static int WHITE1_PIN, WHITE2_PIN, BLUE_PIN;

static int CURRENT_STATE;

static int FULL_POWER = 1023;
static int HALF_POWER = 3098;
static int QUART_POWER = 9345;
static int NIGHT_POWER = 7395;

const int hp = 20; // half power
const int SPEED = 5; //this is the speed at which the lights fade


//constructor set the pins of the leds here
Lighting::Lighting(int WHITE1, int WHITE2, int BLUE)
{
  WHITE1_PIN = WHITE1;
  WHITE2_PIN = WHITE2;
  BLUE_PIN = BLUE;
}

//intialise the pins 
void Lighting::init(void)
{
  pinMode(WHITE1_PIN, OUTPUT);
  pinMode(WHITE2_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);



  //switch into to full power light mode to prevent confusion later
  digitalWrite(BLUE_PIN, LOW);//make sure moon light is off
  switchLight(WHITE1_PIN, true);
  switchLight(WHITE2_PIN, true);

  //set the flag to show were in full power
  CURRENT_STATE = 1023;

}

//with this method we send a number according to mode we want 
//1 = full power
//2 = half power lights go down to half leds
//3 = quarter power remaining leds go down to half
//4 = night mode only blue leds

void Lighting::LightingOnMode(int mode)
{
  /*
  Serial.print("Current state: ");
  Serial.println(CURRENT_STATE);
  Serial.print("Changing to: ");
  Serial.println(mode);
  */
  switch(mode)
  {
    case(1023)://FULL POWER
    //detect the mode and shift accordingly
    if(CURRENT_STATE != mode)//check that the requested mode change is not the same as the current state
    {
      //Serial.println(">>>>>>>> GOING TO FULL POWER MODE");
      //first we need to turn the current mode off and full on
      //if we are in  half or then is just a case of switching the all the lights back on
      if(CURRENT_STATE == 3098)
      {
        //switch WHITE2 back on
        switchLight(WHITE2_PIN,true);

      }
      else if(CURRENT_STATE == 9345)
      {
        //we need to bring WHITE2 back on and WHITE1 back upto full power
        switchLight(WHITE2_PIN, true);
        halfLight(WHITE1_PIN,true);
      }
      else if(CURRENT_STATE == 7395)
      {
        //we need to switch the MOON leds off and the WHITE1 & 2 ON 
        switchLight(BLUE_PIN, false);
        switchLight(WHITE1_PIN, true);
        switchLight(WHITE2_PIN, true);
      }
      //finally set the CURRENT_STATE so we can use it properly
      CURRENT_STATE = mode;

    }//if it is the same ignore the change request

    break;
    case(3098)://HALF POWER
    if(CURRENT_STATE != mode)//check that the requested mode change is not the same as the current state
    {
      //Serial.println(">>>>>>>> GOING TO HALF POWER MODE");
      //first we need to turn the current mode off and full on

        if(CURRENT_STATE == 1023)
      {
        //switch WHITE2 off to go to half power
        switchLight(WHITE2_PIN, false);

      }
      else if(CURRENT_STATE == 9345)
      {
        //this will probably never occur due to the behaviour of the hardware 
        //but we should process accordingly for future developments.

        //switch WHITE2 back up to full and STOP this equals HALF POWER
        halfLight(WHITE1_PIN,true);
      }
      else if(CURRENT_STATE == 7395)
      {
        //Serial.println(">>>>>>>>>>>>>> current mode equals NIGHT");
        //we need to switch the MOON leds off and JUST WHITE2 on (half power)
        switchLight(BLUE_PIN, false);
        switchLight(WHITE1_PIN, true);
      }
      //finally set the CURRENT_STATE so we can use it properly
      CURRENT_STATE = mode;

    }//if it is the same ignore the change request
    break;
    case(9345)://QUARTER POWER

    if(CURRENT_STATE != mode)//check that the requested mode change is not the same as the current state
    {
      //first we need to turn the current mode off and full on

      //Serial.println(">>>>>>>> GOING TO QUARTER POWER MODE");

      if(CURRENT_STATE == 1023)
      {
        switchLight(WHITE2_PIN, false);
        halfLight(WHITE1_PIN, false);
        //switch WHITE2 off and lower power to 50% on WHITE1
      }
      else if(CURRENT_STATE == 3098)
      {
        halfLight(WHITE1_PIN, false);
      }
      else if(CURRENT_STATE == 7395)
      {
        //we need to switch the MOON leds off and JUST WHITE2 on (half power)
        switchLight(BLUE_PIN, false);
        halfLight(WHITE1_PIN, true);
      }
      //finally set the CURRENT_STATE so we can use it properly
      CURRENT_STATE = mode;

    }//if it is the same ignore the change request
    break;
    case(7395)://MOON POWER
    if(CURRENT_STATE != mode)//check that the requested mode change is not the same as the current state
    {
      //Serial.println(">>>>>>>> GOING TO NIGHT MODE");
      //first we need to turn the current mode off and full on
      if(CURRENT_STATE == 1023)
      {
        switchLight(WHITE2_PIN, false);
        switchLight(WHITE1_PIN, false);
        switchLight(BLUE_PIN, true);
        //switch WHITE2 off and lower power to 50% on WHITE1
      }

      if(CURRENT_STATE == 3098)
      {
        switchLight(WHITE1_PIN, false);
        switchLight(BLUE_PIN, true);
      }

      if(CURRENT_STATE == 9345)
      {
        //we need to switch the MOON leds off and JUST WHITE2 on (half power)
        halfLight(WHITE1_PIN, true);
        switchLight(WHITE1_PIN, false);
        switchLight(BLUE_PIN, true);
      }
      //finally set the CURRENT_STATE so we can use it properly
      CURRENT_STATE = mode;

    }//if it is the same ignore the change request
    break;
  default:
    //do nothing because we have been passed a invlaid mode
    break;
  }
}

//this will switch the passed light on or off in a user friendly way.
void Lighting::switchLight(int light, bool switchOn)
{
  if(switchOn)
  {
    //fade light up
    for(int a = 0; a <= 255; a++)
    {
      analogWrite(light,a);
      delay(SPEED);
    }

  }
  else
  {
    //fade down
    for(int a = 255; a >= 0; a--)
    {
      analogWrite(light,a);
      delay(SPEED);
    }
  }
}

//will siwtch a light to half power true is to go to full power
void Lighting::halfLight(int pin, bool switchUp)
{

  if(switchUp)
  {
    //switch to full power

    //go to full power
    for(int a = hp; a <= 255; a++)
    {
      analogWrite(pin, a);
      delay(SPEED);
    }

  }
  else
  {
    //switch down to hp

    //go down to half
    for(int a = 255; a != hp ; a--)
    {
      analogWrite(pin, a);
      delay(SPEED);
    }

  }
}

void Lighting::Storm()
{
  halfLight(BLUE_PIN, true);
  switchLight(WHITE2_PIN, false);
  halfLight(WHITE1_PIN, true);
  //display the lightning action
  int ran = random(1,15);
  for(int a = 0; a <= ran; a++)
  {
    digitalWrite(WHITE2_PIN,LOW);
    digitalWrite(WHITE2_PIN,HIGH);
    delay(50);
    digitalWrite(WHITE2_PIN,LOW);
    delay(50);
    digitalWrite(WHITE2_PIN,HIGH);
    delay(250);
    digitalWrite(WHITE2_PIN,LOW);
    delay(50);
    digitalWrite(WHITE2_PIN,HIGH);
    delay(100);
    digitalWrite(WHITE2_PIN,LOW);
    delay(150);
    digitalWrite(WHITE2_PIN,HIGH);
    delay(50);
    digitalWrite(WHITE2_PIN,LOW);
    delay(70);
    digitalWrite(WHITE2_PIN,HIGH);
    delay(250);
    digitalWrite(WHITE2_PIN,LOW);
    delay(50);
    digitalWrite(WHITE2_PIN,HIGH);
    delay(350);
    digitalWrite(WHITE2_PIN,LOW);
    delay(2000);
  }
}







