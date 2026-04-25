#include "fruit.h"
#include "position.h"
#ifdef ARDUINO
#include "Arduino.h"
#endif

Fruit::Fruit(Snake * snake) {
  this->randomize(snake);
  this->position.x = 10;
  this->position.y = 4;
}

void Fruit::randomize(Snake * snake) {
  uint8_t spaceCnt = 0;
  for(int i = 0; i < Snake::BODY_WIDTH; i++) {
    for(int j = 0; j < Snake::BODY_HEIGHT; j++) {
      if(snake->getBodyAt(i, j) == 0) {
        spaceCnt++;
      }
    }
  }

  uint8_t targetSpace = get_random_value(spaceCnt);
  spaceCnt = 0;
  for(int i = 0; i < Snake::BODY_WIDTH; i++) {
    for(int j = 0; j < Snake::BODY_HEIGHT; j++) {
      if(snake->getBodyAt(i, j) == 0) {
        if(targetSpace == spaceCnt) {
          this->position.x = i;
          this->position.y = j;
          return;
        }
        spaceCnt++;
      }
    }
  }
}

const Position Fruit::getPosition() {
  return this->position;
}
