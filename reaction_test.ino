#include <Arduino.h>
#include <U8g2lib.h>

// ── Externs from AJBoy.ino ──────────────────────────────────────────────────
extern U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2;
extern const uint8_t c_up;
extern const uint8_t c_down;
extern const uint8_t c_button1;
extern const uint8_t c_left;
extern const uint8_t c_right;

// ── Timing constants ──────────────────────────────────────────────────────────
const unsigned long BOOT_DELAY_MS      = 1500;  // Minimum wait after boot
const unsigned long RANDOM_EXTRA_MAX   = 3500;  // Extra random wait (0–3500 ms)
const unsigned long TOO_SOON_LOCKOUT   = 1000;  // Penalty lock if pressed too early
const unsigned long RESULT_DISPLAY_MS  = 3000;  // How long to show the result

// ── State machine ─────────────────────────────────────────────────────────────
enum ReactionState {
  STATE_BOOT,        // Initial boot screen
  STATE_WAITING,     // Waiting for the random moment to show the cue
  STATE_GO,          // Cue is showing — player should press now
  STATE_TOO_SOON,    // Player pressed before the cue — penalty
  STATE_RESULT       // Displaying reaction time result
};

static ReactionState reactionState = STATE_BOOT;
static unsigned long stateStart = 0;   // millis() when the current state began
static unsigned long waitDelay  = 0;   // Total random delay for STATE_WAITING
static unsigned long reactionMs = 0;   // Measured reaction time

// Button debounce
static bool lastBtnRaw      = HIGH;
static bool btnPressed      = false;  // True for exactly one loop() after a clean press

// ── Helpers ───────────────────────────────────────────────────────────────────

// Returns true on the rising-edge of a button press (after debounce).
static bool readButtonPress() {
  bool raw = digitalRead(c_button1);
  bool pressed = false;

  if (raw == LOW && lastBtnRaw == HIGH) {
    delay(20); // Simple software debounce
    if (digitalRead(c_button1) == LOW) {
      pressed = true;
    }
  }
  lastBtnRaw = raw;
  return pressed;
}

// Centred text helper
static void drawCentred(const char* text, uint8_t y) {
  int16_t w = u8g2.getStrWidth(text);
  u8g2.drawStr((128 - w) / 2, y, text);
}

// ── Screen drawing functions ──────────────────────────────────────────────────

static void drawBoot() {
  u8g2.setFont(u8g2_font_logisoso16_tr);
  drawCentred("REACTION", 24);
  drawCentred("TEST", 44);
  u8g2.setFont(u8g2_font_6x10_tr);
  drawCentred("Press BTN1 to start", 58);
}

static void drawWaiting() {
  u8g2.setFont(u8g2_font_logisoso16_tr);
  drawCentred("WAIT...", 36);
  u8g2.setFont(u8g2_font_6x10_tr);
  drawCentred("Do NOT press yet!", 55);
}

static void drawGo() {
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 64);
  u8g2.setDrawColor(0);            // Invert text colour
  u8g2.setFont(u8g2_font_logisoso28_tr);
  drawCentred("PRESS!", 46);
  u8g2.setDrawColor(1);            // Restore
}

static void drawTooSoon() {
  u8g2.setFont(u8g2_font_logisoso16_tr);
  drawCentred("TOO SOON!", 28);
  u8g2.setFont(u8g2_font_6x10_tr);
  drawCentred("Wait for the signal", 44);
  drawCentred("Restarting...", 58);
}

static void drawResult(unsigned long ms) {
  char buf[24];
  u8g2.setFont(u8g2_font_logisoso16_tr);
  drawCentred("NICE!", 18);
  u8g2.setFont(u8g2_font_logisoso28_tr);
  snprintf(buf, sizeof(buf), "%lu ms", ms);
  drawCentred(buf, 50);
  u8g2.setFont(u8g2_font_6x10_tr);
  const char* rating;
  if      (ms < 150) rating = "Superhuman!";
  else if (ms < 200) rating = "Excellent!";
  else if (ms < 250) rating = "Very Good";
  else if (ms < 350) rating = "Good";
  else if (ms < 500) rating = "Average";
  else               rating = "Keep practising";
  drawCentred(rating, 62);
}

// ── Transition helper ─────────────────────────────────────────────────────────
static void enterWaiting() {
  waitDelay = BOOT_DELAY_MS + (unsigned long)random(0, RANDOM_EXTRA_MAX + 1);
  reactionState = STATE_WAITING;
  stateStart = millis();
}

// ── Game Entry Point ─────────────────────────────────────────────────────────
void reaction_test_start() {
  // Initialization
  randomSeed(analogRead(A0));
  reactionState = STATE_BOOT;
  stateStart = millis();
  lastBtnRaw = HIGH;

  while (true) {
    // Check for exit condition (Left button)
    if (digitalRead(c_left) == LOW) {
      delay(200); // debounce exit
      return; 
    }

    btnPressed = readButtonPress();
    unsigned long now = millis();

    // ── Logic Update ────────────────────────────────────────────────────────
    switch (reactionState) {
      case STATE_BOOT:
        if ((now - stateStart >= BOOT_DELAY_MS) && btnPressed) {
          enterWaiting();
        }
        break;

      case STATE_WAITING:
        if (btnPressed) {
          reactionState = STATE_TOO_SOON;
          stateStart = now;
        } else if (now - stateStart >= waitDelay) {
          reactionState = STATE_GO;
          stateStart = now;
        }
        break;

      case STATE_GO:
        if (btnPressed) {
          reactionMs = now - stateStart;
          reactionState = STATE_RESULT;
          stateStart = now;
        }
        break;

      case STATE_TOO_SOON:
        if (now - stateStart >= TOO_SOON_LOCKOUT) {
          enterWaiting();
        }
        break;

      case STATE_RESULT:
        if (now - stateStart >= RESULT_DISPLAY_MS || btnPressed) {
          enterWaiting();
        }
        break;
    }

    // ── Rendering ───────────────────────────────────────────────────────────
    u8g2.firstPage();
    do {
      switch (reactionState) {
        case STATE_BOOT:     drawBoot(); break;
        case STATE_WAITING:  drawWaiting(); break;
        case STATE_GO:       drawGo(); break;
        case STATE_TOO_SOON: drawTooSoon(); break;
        case STATE_RESULT:   drawResult(reactionMs); break;
      }
    } while (u8g2.nextPage());
    
    // Tiny delay to prevent pegged CPU if needed, though u8g2 timing usually handles it
    delay(10);
  }
}
