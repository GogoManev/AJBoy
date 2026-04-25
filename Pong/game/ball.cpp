#include "ball.h"
#include "paddle.h"

namespace Pong {

Ball::Ball(int playfield_width, int playfield_height)
    : playfield_width(playfield_width), playfield_height(playfield_height) {
  start_x = playfield_width / 2;
  start_y = playfield_height / 2;
  position.x = start_x;
  position.y = start_y;
}

Ball::~Ball() {
}

void Ball::update(Paddle* paddle1, Paddle* paddle2) {
  // Update position
  position.x += velocity_x;
  position.y += velocity_y;
  
  // Bounce off top and bottom walls
  bounceOffWalls();
  
  // Check paddle collisions
  if (paddle1 && paddle1->checkCollision(position.x, position.y, BALL_RADIUS)) {
    velocity_x = INITIAL_SPEED;
    position.x = paddle1->getX() + paddle1->getWidth() + BALL_RADIUS;
    velocity_y += (paddle1->getY() + paddle1->getHeight() / 2 - position.y) / 8;
    increaseDifficulty();
  }
  
  if (paddle2 && paddle2->checkCollision(position.x, position.y, BALL_RADIUS)) {
    velocity_x = -INITIAL_SPEED;
    position.x = paddle2->getX() - BALL_RADIUS;
    velocity_y += (paddle2->getY() + paddle2->getHeight() / 2 - position.y) / 8;
    increaseDifficulty();
  }
}

void Ball::bounceOffWalls() {
  if (position.y - BALL_RADIUS <= 0) {
    position.y = BALL_RADIUS;
    velocity_y = -velocity_y;
  } else if (position.y + BALL_RADIUS >= playfield_height) {
    position.y = playfield_height - BALL_RADIUS;
    velocity_y = -velocity_y;
  }
}

void Ball::increaseDifficulty() {
  if (velocity_x > 0) {
    velocity_x++;
  } else {
    velocity_x--;
  }
}

void Ball::reset() {
  position.x = start_x;
  position.y = start_y;
  velocity_x = INITIAL_SPEED;
  velocity_y = 1;
}

Position Ball::getPosition() const {
  return position;
}

int Ball::getX() const {
  return position.x;
}

int Ball::getY() const {
  return position.y;
}

int Ball::getRadius() const {
  return BALL_RADIUS;
}

int Ball::getVelocityX() const {
  return velocity_x;
}

int Ball::getVelocityY() const {
  return velocity_y;
}

} // namespace Pong
