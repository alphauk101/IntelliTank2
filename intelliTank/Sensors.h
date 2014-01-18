#ifndef Sensors_h
#define Sensors_h
#include "screen.h" 


class Sensors
{
  public:
  void SetPins(int,int); 
  void init(Screen);
  bool getStatus(int);
  int LDR;
  int PIR;
  int getLDRValue(void);
  private:
  int _getLDRValue();
};

#endif

