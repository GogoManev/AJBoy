#pragma once

#include "game/ball.h"
#include "game/paddle.h"

#ifdef __cplusplus
extern "C" {
#endif

void pong_start(void*, int p1_up, int p1_down, int p2_up, int p2_down);
void pong_button(int button_pin, bool pressed);

#ifdef __cplusplus
}
#endif
