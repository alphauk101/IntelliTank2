#ifndef Lighting_h
#define Lighting_h

class Lighting
{
  public:
    int FULL_POWER;
    int HALF_POWER;
    int QUART_POWER;
    int NIGHT_POWER;
    void LightingOnMode(int);
    Lighting(int, int, int);
    void init(void);
    void Storm();
  private:
    void switchLight(int, bool);
    void halfLight(int, bool);
};

#endif

