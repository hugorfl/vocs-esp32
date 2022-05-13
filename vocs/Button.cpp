#include "Button.h"

Button::Button(int pin) : pin(pin), isPressed(false) { }

void Button::init(int mode) {
  pinMode(pin, mode);
  
}

void Button::addInterrupt(void (*interruptFn)(), int mode) {
  attachInterrupt(pin, interruptFn, mode);
}

bool Button::getIsPressed() const {
  return isPressed;
}

void Button::pressButton() {
  isPressed = true;
}

void Button::releaseButton() {
  isPressed = false;
}
