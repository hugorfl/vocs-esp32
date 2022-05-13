#ifndef Button_h
#define Button_h

#include <Arduino.h>

class Button {
  private:
    bool isPressed;
    int pin;
  public:
    Button(int pin);
    void init(int mode);
    void addInterrupt(void (*interruptFn)(), int mode);
    bool getIsPressed() const;
    void pressButton();
    void releaseButton();
};

#endif
