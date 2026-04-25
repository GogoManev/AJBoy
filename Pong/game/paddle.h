#pragma once

#include "position.h"

namespace Pong {

class Paddle {
  public:
    Paddle(int x, int width, int height, int playfield_height);
    ~Paddle();
    
    void moveUp();
    void moveDown();
    void update();
    void reset();
    
    Position getPosition() const;
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    
    // Check if ball collides with paddle
    bool checkCollision(int ball_x, int ball_y, int ball_radius) const;
    
    static const int PADDLE_SPEED = 2;
    
  private:
    Position position;
    int width;
    int height;
    int playfield_height;
    int velocity_y = 0;
    int start_x;
    int start_y;
};

} // namespace Pong
