#include "pong.h"
#include "renderer.h"

#ifdef ARDUINO
#include "Arduino.h"
#endif

// Game state
Ball* g_ball = nullptr;
Paddle* g_paddle1 = nullptr;
Paddle* g_paddle2 = nullptr;

int g_player1_score = 0;
int g_player2_score = 0;
bool g_game_over = false;
int g_winner = 0;

unsigned long g_last_update = 0;
const unsigned long UPDATE_INTERVAL = 30; // milliseconds

// Button states
int g_p1_up_pin = -1;
int g_p1_down_pin = -1;
int g_p2_up_pin = -1;
int g_p2_down_pin = -1;

#ifdef ARDUINO

void pong_start(void* u8g, int p1_up, int p1_down, int p2_up, int p2_down) {
  // Initialize renderer
  Renderer::initialize(u8g);
  
  // Store button pins
  g_p1_up_pin = p1_up;
  g_p1_down_pin = p1_down;
  g_p2_up_pin = p2_up;
  g_p2_down_pin = p2_down;
  
  // Set button pins as inputs
  pinMode(g_p1_up_pin, INPUT_PULLUP);
  pinMode(g_p1_down_pin, INPUT_PULLUP);
  pinMode(g_p2_up_pin, INPUT_PULLUP);
  pinMode(g_p2_down_pin, INPUT_PULLUP);
  
  // Create game objects
  if (g_ball) delete g_ball;
  if (g_paddle1) delete g_paddle1;
  if (g_paddle2) delete g_paddle2;
  
  g_ball = new Ball(128, 64);
  
  // Paddle 1: left side, width 3, height 12
  g_paddle1 = new Paddle(2, 3, 12, 64);
  
  // Paddle 2: right side, width 3, height 12
  g_paddle2 = new Paddle(128 - 5, 3, 12, 64);
  
  g_player1_score = 0;
  g_player2_score = 0;
  g_game_over = false;
  g_winner = 0;
  g_last_update = millis();
  
  // Main game loop
  while (!g_game_over) {
    unsigned long now = millis();
    
    if (now - g_last_update >= UPDATE_INTERVAL) {
      g_last_update = now;
      
      // Read button inputs
      if (digitalRead(g_p1_up_pin) == LOW) {
        g_paddle1->moveUp();
      }
      if (digitalRead(g_p1_down_pin) == LOW) {
        g_paddle1->moveDown();
      }
      if (digitalRead(g_p2_up_pin) == LOW) {
        g_paddle2->moveUp();
      }
      if (digitalRead(g_p2_down_pin) == LOW) {
        g_paddle2->moveDown();
      }
      
      // Update game state
      g_paddle1->update();
      g_paddle2->update();
      g_ball->update(g_paddle1, g_paddle2);
      
      // Check for scoring (ball out of bounds)
      if (g_ball->getX() < 0) {
        // Player 2 scores
        g_player2_score++;
        if (g_player2_score >= 11) {
          g_game_over = true;
          g_winner = 2;
        }
        g_ball->reset();
      } else if (g_ball->getX() > 128) {
        // Player 1 scores
        g_player1_score++;
        if (g_player1_score >= 11) {
          g_game_over = true;
          g_winner = 1;
        }
        g_ball->reset();
      }
    }
    
    // Render
    if (g_game_over) {
      Renderer::renderGameOver(g_player1_score, g_player2_score, g_winner);
    } else {
      Renderer::renderAll(g_ball, g_paddle1, g_paddle2, g_player1_score, g_player2_score);
    }
  }
}

void pong_button(int button_pin, bool pressed) {
  // Can be used for alternative control method
  (void)button_pin;
  (void)pressed;
}

#endif
