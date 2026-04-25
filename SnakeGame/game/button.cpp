#include "button.h"

void Button::handleInterrupt() {
  pressed = true;
}

bool Button::isPressed() {
  bool res = this->pressed;
  this->pressed = false;
  return res;
}

#ifdef ARDUINO
Button::Button(uint8_t pin, void (*isr)()) {
  this->_pin = pin;
  this->_isr = isr;
}

void Button::begin() {
  pinMode(this->_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(this->_pin), this->_isr, FALLING);
}
#else
Button::Button(){
	this->pressed = false;
}
#endif
