#include "renderer.h"

#define CELL_SIZE 8

#ifdef ARDUINO
#include "Arduino.h"
namespace Renderer {

  U8G2* u8g2_ptr = nullptr;

  void initialize(void* u8g) {
    u8g2_ptr = (U8G2*)u8g;
    if(u8g2_ptr) {
      u8g2_ptr->begin();
      u8g2_ptr->setFont(u8g2_font_6x10_tf);
    }
  }

  unsigned long time_total = 0;
  unsigned long time_last = 0;
  float framerate() {
    time_last = time_total;
    time_total = millis();
    return 1.0 / ((time_total - time_last) / 1000000.0);
  }

  void renderBorder() {
    if(u8g2_ptr) u8g2_ptr->drawFrame(0, 0, 128, 64);
  }

  void renderSnake(Snake *snake) {
    if(!u8g2_ptr) return;
    for(int i = 0; i < Snake::BODY_WIDTH; i++) {
      for(int j = 0; j < Snake::BODY_HEIGHT; j++) {
        if(snake->getBodyAt(i, j) > 0) {
          u8g2_ptr->drawBox(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE);
        }
      }
    }
  }

  void renderFruit(Fruit * fruit) {
    if(!u8g2_ptr) return;
    Position position = fruit->getPosition();
    u8g2_ptr->drawFrame(position.x * CELL_SIZE + 1, position.y * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2);
  }

  void renderGameOver(Snake *snake) {
    if(!u8g2_ptr) return;
    u8g2_ptr->drawBox(32, 20, 64, 22);
    u8g2_ptr->setDrawColor(0);
    u8g2_ptr->setCursor(34, 30);
    u8g2_ptr->print("Game Over!");
    u8g2_ptr->setCursor(34, 40);
    u8g2_ptr->print("Points: ");
    u8g2_ptr->print(snake->getPoints());
    u8g2_ptr->setDrawColor(1);
  }

  void renderAll(Snake *snake, Fruit *fruit) {
    if(!u8g2_ptr) return;
    u8g2_ptr->firstPage();
    do {
      renderBorder();
      renderSnake(snake);
      renderFruit(fruit);
      if(!snake->isAlive()) {
        renderGameOver(snake);
      }
    } while(u8g2_ptr->nextPage());
  }

}
#else
namespace Renderer {

	u8g2_t u8g2;
  void initialize(void * u8) {
	u8g2 = *(u8g2_t*)u8;
    u8g2_SetFont(&u8g2,u8g2_font_6x10_tf);
  }

  unsigned long time_total = 0;
  unsigned long time_last = 0;
  float framerate() {
    time_last = time_total;
    time_total = millis();
    return 1.0 / ((time_total - time_last) / 1000000.0);
  }

  void renderBorder() {
    u8g2_DrawFrame(&u8g2,0, 0, 128, 64);
  }
  
  void renderSnake(Snake *snake) {
    for(int i = 0; i < Snake::BODY_WIDTH; i++) {
      for(int j = 0; j < Snake::BODY_HEIGHT; j++) {
        if(snake->getBodyAt(i, j) > 0) {
          u8g2_DrawBox(&u8g2, i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE);
        }
      }
    }
  }

  void renderFruit(Fruit * fruit) {
    Position position = fruit->getPosition();
    u8g2_DrawFrame(&u8g2, position.x * CELL_SIZE + 1, position.y * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2);
  }

  void renderGameOver(Snake *snake) {
    u8g2_DrawBox(&u8g2, 32, 20, 64, 22);
    u8g2_SetDrawColor(&u8g2,0);
    u8g2_DrawStr(&u8g2, 34, 30, "Game Over!");
    u8g2_DrawStr(&u8g2, 34, 40, "Points: ");
    char c[4];
    intToStr(snake->getPoints(), c, 2);
    u8g2_DrawStr(&u8g2, 76, 40, c); //its not good in that way
    u8g2_SetDrawColor(&u8g2, 1);
  }

  void renderAll(Snake *snake, Fruit *fruit) {
    u8g2_ClearBuffer(&u8g2);
    renderBorder();
    renderSnake(snake);
    renderFruit(fruit);
    if(!snake->isAlive()) {
      renderGameOver(snake);
    }
    u8g2_SendBuffer(&u8g2);
  }

}

// Reverses a string 'str' of length 'len'
void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    if(x == 0) {
    	str[0] = '0';
    	i++;
    }
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
#endif
