#include "renderer.h"

#define PADDLE_COLOR_WIDTH 3

#ifdef ARDUINO
#include "Arduino.h"

namespace Pong {
namespace PongRenderer {

  U8G2* u8g2_ptr = nullptr;

  void initialize(void* u8g) {
    u8g2_ptr = (U8G2*)u8g;
    if(u8g2_ptr) {
      u8g2_ptr->begin();
      u8g2_ptr->setFont(u8g2_font_6x10_tf);
    }
  }

  void renderBorder(int width, int height) {
    if(u8g2_ptr) u8g2_ptr->drawFrame(0, 0, width, height);
  }

  void renderBall(Ball* ball) {
    if(!u8g2_ptr || !ball) return;
    int x = ball->getX();
    int y = ball->getY();
    int r = ball->getRadius();
    u8g2_ptr->drawCircle(x, y, r, U8G2_DRAW_ALL);
  }

  void renderPaddles(Paddle* paddle1, Paddle* paddle2) {
    if(!u8g2_ptr) return;
    
    if(paddle1) {
      int x = paddle1->getX();
      int y = paddle1->getY();
      int w = paddle1->getWidth();
      int h = paddle1->getHeight();
      u8g2_ptr->drawBox(x, y, w, h);
    }
    
    if(paddle2) {
      int x = paddle2->getX();
      int y = paddle2->getY();
      int w = paddle2->getWidth();
      int h = paddle2->getHeight();
      u8g2_ptr->drawBox(x, y, w, h);
    }
  }

  void renderScore(int player1_score, int player2_score) {
    if(!u8g2_ptr) return;
    u8g2_ptr->setFont(u8g2_font_6x10_tf);
    
    // Player 1 score (left side)
    u8g2_ptr->setCursor(10, 10);
    u8g2_ptr->print("P1:");
    u8g2_ptr->print(player1_score);
    
    // Player 2 score (right side)
    u8g2_ptr->setCursor(100, 10);
    u8g2_ptr->print("P2:");
    u8g2_ptr->print(player2_score);
  }

  void renderGameOver(int winner) {
    if(!u8g2_ptr) return;
    
    // Semi-transparent overlay (draw filled rectangle)
    u8g2_ptr->drawBox(32, 20, 64, 22);
    u8g2_ptr->setDrawColor(0);
    u8g2_ptr->setCursor(34, 30);
    u8g2_ptr->print("Game Over!");
    u8g2_ptr->setCursor(34, 40);
    u8g2_ptr->print("Winner: P");
    u8g2_ptr->print(winner);
    u8g2_ptr->setDrawColor(1);
  }

  void renderAll(Ball* ball, Paddle* paddle1, Paddle* paddle2, 
                 int player1_score, int player2_score, int winner) {
    if(!u8g2_ptr) return;
    
    u8g2_ptr->firstPage();
    do {
      renderBorder(128, 64);
      renderScore(player1_score, player2_score);
      renderBall(ball);
      renderPaddles(paddle1, paddle2);
      
      if (winner != 0) {
        renderGameOver(winner);
      }
    } while(u8g2_ptr->nextPage());
  }
}
}

#endif
