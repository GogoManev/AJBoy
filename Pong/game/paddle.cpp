#include "paddle.h"

Paddle::Paddle(int x, int width, int height, int playfield_height)
    : width(width), height(height), playfield_height(playfield_height), 
      start_x(x), start_y(playfield_height / 2 - height / 2) {
  position.x = start_x;
  position.y = start_y;
}

Paddle::~Paddle() {
}

void Paddle::moveUp() {
  velocity_y = -PADDLE_SPEED;
}

void Paddle::moveDown() {
  velocity_y = PADDLE_SPEED;
}

void Paddle::update() {
  position.y += velocity_y;
  velocity_y = 0;
  
  // Clamp to playfield
  if (position.y < 0) {
    position.y = 0;
  } else if (position.y + height > playfield_height) {
    position.y = playfield_height - height;
  }
}

void Paddle::reset() {
  position.x = start_x;
  position.y = start_y;
  velocity_y = 0;
}

Position Paddle::getPosition() const {
  return position;
}

int Paddle::getX() const {
  return position.x;
}

int Paddle::getY() const {
  return position.y;
}

int Paddle::getWidth() const {
  return width;
}

int Paddle::getHeight() const {
  return height;
}

bool Paddle::checkCollision(int ball_x, int ball_y, int ball_radius) const {
  // Check if ball is within paddle bounds
  if (ball_x - ball_radius < position.x + width &&
      ball_x + ball_radius > position.x &&
      ball_y - ball_radius < position.y + height &&
      ball_y + ball_radius > position.y) {
    return true;
  }
  return false;
}
