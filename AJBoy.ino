/*
  AJBoy - Main Project File
*/
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/13, /* data=*/11, /* cs=*/10, /* dc=*/9, /* reset=*/8);
// End of constructor list


//snake stuff
#include "SnakeGame/game/button.cpp"
#include "SnakeGame/game/position.cpp"
#include "SnakeGame/game/fruit.cpp"
#include "SnakeGame/game/snake.cpp"
#include "SnakeGame/renderer.cpp"
#include "SnakeGame/snake_start.cpp"

// c_ for controls
const uint8_t c_up = 3;
const uint8_t c_down = 4;
const uint8_t c_button1 = 2;
const uint8_t c_left = 5;
const uint8_t c_right = 6;

//space trash
#include "SpaceTrash/spacetrash.h"

// Pong 
#include "Pong/game/position.cpp"
#include "Pong/game/ball.cpp"
#include "Pong/game/paddle.cpp"
#include "Pong/renderer.cpp"
#include "Pong/pong.cpp"

void spacetrash_start();
void flappybird_start();
void reaction_test_start();
void AJBoy_text();


void setup(void) {
  pinMode(c_up, INPUT_PULLUP);
  pinMode(c_down, INPUT_PULLUP);
  pinMode(c_button1, INPUT_PULLUP);
  Serial.begin(115200);
  u8g2.begin();
}

uint8_t gameMode = 0;
/*
0 - main manu
1 - game 1 (space something)
*/

const char *menuLabels[] = {
  "Space Trash",
  "Snake",
  "Flappy Bird",
  "Pong",
  "Reaction Test",
  "AJBoy text",
};

const int VISIBLE_ITEMS = 3;
uint8_t topIndex = 0;
uint8_t menuIndex = 0;         // which item is highlighted
const unsigned int MENU_ITEMS = sizeof(menuLabels) / sizeof(menuLabels[0]);

char menuIndexCharBuff[5];
char menuCharBuffer[30];
void drawMenu() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tr);

    // Title
    itoa(menuIndex, menuIndexCharBuff, 10);
    strcpy(menuCharBuffer, "    === AJBoy === ");
    strcat(menuCharBuffer, menuIndexCharBuff);
    u8g2.drawStr(0, 10, menuCharBuffer);

    // Menu items
    for (uint8_t i = 0; i < VISIBLE_ITEMS; i++) {
      uint8_t itemIndex = topIndex + i;
      if (itemIndex >= MENU_ITEMS) break;
      
      int yPos = 28 + i * 14;
      
      if (itemIndex == menuIndex) {
        u8g2.drawStr(0, yPos, ">");
      }

      u8g2.drawStr(10, yPos, menuLabels[itemIndex]);
    }
  } while (u8g2.nextPage());
}

void handleMenuSelect(int grrrrr) {
  switch (menuIndex) {
    case 0:
      gameMode = 1;
      break;
    case 1:
      //snake game
      gameMode = 2;
      break;
    case 2:
      // pong
      gameMode = 3;
      break;
    case 3:
      gameMode = 4;
      break;
    case 4:
      gameMode = 5;
      break;
    case 5:
      gameMode = 6;
      break;
  }
}

void loop(void) {
  switch (gameMode) {
    case 0:
      drawMenu();

      if (digitalRead(c_down) == LOW) {
        // menuIndex = (menuIndex + 1) % MENU_ITEMS;
        
        menuIndex = max(menuIndex - 1, 0);

        
        if (menuIndex < topIndex + VISIBLE_ITEMS) {
        //   topIndex = menuIndex - VISIBLE_ITEMS + 1;
            topIndex = menuIndex;
        }
        delay(150);  // simple debounce
      }
      if (digitalRead(c_up) == LOW) {
        // menuIndex = (menuIndex - 1 + MENU_ITEMS) % MENU_ITEMS;
        menuIndex = min(menuIndex + 1, MENU_ITEMS - 1);

        if (menuIndex >= topIndex + VISIBLE_ITEMS) {
          topIndex = menuIndex;
        }

        delay(150);
      }
      if (digitalRead(c_button1) == LOW) {
        handleMenuSelect(menuIndex);
        delay(150);
      }
      break;
    case 1:
      spacetrash_start();
      gameMode = 0;
      break;
    case 2:
      snake_start(&u8g2);
      break;
    case 3:
      flappybird_start();
      gameMode = 0;
      break;
    case 4:
      pong_start(&u8g2, c_down, c_up, c_button1, c_left);
      gameMode = 0;
      break;
    case 5:
      reaction_test_start();
      gameMode = 0;
      break;
    case 6:
      AJBoy_text();
      gameMode = 0;
      break;
    default:
      gameMode = 0;
  }
}