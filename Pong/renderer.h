#pragma once

#include "game/ball.h"
#include "game/paddle.h"
#include "U8g2lib.h"

namespace Pong {

namespace PongRenderer {
  void initialize(void*);
  void renderBorder(int width, int height);
  void renderBall(Ball* ball);
  void renderPaddles(Paddle* paddle1, Paddle* paddle2);
  void renderScore(int player1_score, int player2_score);
  void renderGameOver(int winner);
  void renderAll(Ball* ball, Paddle* paddle1, Paddle* paddle2, 
                 int player1_score, int player2_score, int winner = 0);
}

} // namespace Pong
