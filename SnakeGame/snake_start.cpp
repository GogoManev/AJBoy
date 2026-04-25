#include "snake_start.h"
#include "game/button.h"
#include "game/fruit.h"
#include "renderer.h"
#include <Arduino.h>
#ifndef ARDUINO
#include "stm32f3xx_hal.h"
#include <stdlib.h>
#endif

#ifdef ARDUINO
CREATE_INTERRUPT_BUTTON(buttonEnd, 5);
CREATE_INTERRUPT_BUTTON(buttonUp, 4);
CREATE_INTERRUPT_BUTTON(buttonDown, 3);
CREATE_INTERRUPT_BUTTON(buttonLeft, 6);
CREATE_INTERRUPT_BUTTON(buttonRight, 7);
#else
Button buttonUp;
Button buttonDown;
Button buttonLeft;
Button buttonRight;
Button buttonEnd;
#endif
Snake snake;
Fruit fruit(&snake);


void snake_start(void* u8g2) {
#ifdef ARDUINO
	Serial.println("Snake Game Started");
	buttonEnd.begin();
	buttonUp.begin();
	buttonDown.begin();
	buttonLeft.begin();
	buttonRight.begin();
	Renderer::initialize(u8g2);
	
	if(!snake.isAlive()) {
		snake.restart();
	}
	
	unsigned long lastPrintTime = millis();
	
	while(1){
		if (buttonEnd.isPressed()) {
			Serial.println("Game Ended by Button");
			break;
		}
		if (buttonUp.isPressed()) {
			snake.turn(UP);
			Serial.println("Turn UP");
		}
		if (buttonDown.isPressed()) {
			snake.turn(DOWN);
			Serial.println("Turn DOWN");
		}
		if (buttonLeft.isPressed()) {
			snake.turn(LEFT);
			Serial.println("Turn LEFT");
		}
		if (buttonRight.isPressed()) {
			snake.turn(RIGHT);
			Serial.println("Turn RIGHT");
		}

		bool resetFruit = false;
		if(snake.nextHeadPosition() == fruit.getPosition()) {
			snake.grow();
			resetFruit = true;
			Serial.print("Score: ");
			Serial.println(snake.getPoints());
		}

		snake.advance();
		if(resetFruit) fruit.randomize(&snake);
		  
		Renderer::renderAll(&snake, &fruit);
		if(!snake.isAlive()) {
			Serial.print("Game Over! Final Score: ");
			Serial.println(snake.getPoints());
		}
		
		// Print game status periodically
		if (millis() - lastPrintTime >= 2000) {
			Serial.print("Snake alive: ");
			Serial.println(snake.isAlive());
			lastPrintTime = millis();
		}
		
		delay(500);
	}
	Serial.println("Snake Game Exited");
#else
	if(!snake.isAlive()) {
		snake.restart();
	}
	Renderer::initialize(u8g2);
	while(1){
		if (buttonEnd.isPressed()) 		break;
		if (buttonUp.isPressed()) 		snake.turn(UP);
		if (buttonDown.isPressed()) 	snake.turn(DOWN);
		if (buttonLeft.isPressed()) 	snake.turn(LEFT);
		if (buttonRight.isPressed()) 	snake.turn(RIGHT);

		bool resetFruit = false;
		if(snake.nextHeadPosition() == fruit.getPosition()) {
			snake.grow();
			resetFruit = true;
		}

		snake.advance();
		if(resetFruit) fruit.randomize(&snake);
		  
		Renderer::renderAll(&snake, &fruit);
		
		delay(500);
	}
#endif
}

//This function is called by STM32 button interrupt with proper parameter corresponding the right button
//You can use this approach or make yours
#ifndef ARDUINO
void snake_button(short control){
	if(control == END) 		buttonEnd.pressed = true;
	if(control == UP)		buttonUp.pressed = true;
	if(control == DOWN) 	buttonDown.pressed = true;
	if(control == RIGHT)	buttonRight.pressed = true;
	if(control == LEFT)		buttonLeft.pressed = true;
}
#endif
//Fill these functions
// void delay(int delay){
// #ifdef ARDUINO
// 	delay(delay);
// #else
// 	HAL_Delay(delay);
// #endif
// }

//Fill these functions
// long micros(){
// #ifdef ARDUINO
// 	return micros();
// #else
// 	return HAL_GetTick()*1000;
// #endif
// }

//Fill these functions
int get_random_value(int spaceCnt){
#ifdef ARDUINO
	return random(0, spaceCnt);
#else
	srand (HAL_GetTick()); //not the best
	return rand() % spaceCnt;
	return 0;
#endif

}
