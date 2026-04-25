#pragma once

typedef unsigned char uint8_t;

struct Position {
  int x;
  int y;
  
  Position() : x(0), y(0) {}
  Position(int x, int y) : x(x), y(y) {}
  
  bool operator==(const Position& other) const {
    return x == other.x && y == other.y;
  }
};
