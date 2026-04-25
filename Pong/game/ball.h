#pragma once

#include "position.h"

namespace Pong {

class Paddle; // Forward declaration

class Ball {
  public:
    Ball(int playfield_width, int playfield_height);
    ~Ball();
    
    void update(Paddle* paddle1, Paddle* paddle2);
    void reset();
    
    Position getPosition() const;
    int getX() const;
    int getY() const;
    int getRadius() const;
    
    // Getters for velocity (useful for debugging)
    int getVelocityX() const;
    int getVelocityY() const;
    
    static const int BALL_RADIUS = 2;
    static const int INITIAL_SPEED = 2;
    
  private:
    Position position;
    int velocity_x = INITIAL_SPEED;
    int velocity_y = 1;
    int playfield_width;
    int playfield_height;
    int start_x;
    int start_y;
    
    void bounceOffWalls();
    void increaseDifficulty();
};

} // namespace Pong
