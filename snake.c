//Snek Game CAB 202
//Made by: Martin-Timothy Vu
//Date: 21/10/16
//Description: A game of snek to be played on TeensyPEW

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

#include <math.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "sprite.h"

const int left = -1;
const int right = 1;
const int up = -2;
const int down = 2;
const int maxSnakeLength = 25;

double delayMultiplier;
int previousDirection = 0;
int direction = 0;
int score = 0;
int lives = 5;
int snekLength = 0;
int initialSnekLength = 2;
bool isFirstFood = true;
bool isSnekKill = false;
bool isWalls = false;
volatile unsigned long ovf_count = 0;
uint16_t adcResult;

unsigned char snekImg[] = {
	0b11100000,
	0b10100000,
	0b11100000
}; 

unsigned char foodImg[] = {
	0b01000000,
	0b11100000,
	0b01000000
};

unsigned char wall1Img[] = {
	0b11111111
};

unsigned char wall2Img[] = {
	0b11111111
};

char scoreBuff[80];
char livesBuff[80];

Sprite snek[25];
Sprite wall1[3];
Sprite wall2[3];
Sprite food;

void InitHardware (void);
void DrawName (void);
void InitialiseSnek (void);
void InitialiseFood (void);
void InitialiseWalls (void);
void UpdateSnek (void);
bool CheckCollided (Sprite* spriteA, Sprite* spriteB);
bool CheckCollidedWall (Sprite* spriteA, Sprite* spriteB);
double GetSystemTime (void);
void ShowScoreAndLives (void);
void ShowSnek (void);
void ShowWalls (void);
void ShowGame (void);
void Setup(void);
void Process(void);
void ShowGameOver(void);
void setup_adc ();
uint16_t adc_read (uint8_t);

//Sets up the teensy hardware
void InitHardware (void) {
	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();

	DDRC = 1 << PIN7;
	PORTC = 1 << PIN7;
	TCCR1B &= ~((1 << WGM02));
	TCCR1B |= (1 << CS02) | (1 << CS00);
	TCCR1B &= ~((1 << CS01));

	TCCR0B &= ~((1<<WGM02));
	TCCR0B |= (1<<CS11);
	TIMSK0 |= 1<<TOIE0;
	sei();
}
//End InitHardware

//Displays title of game, name, and student number
void DrawName (void) {
	draw_string(0,0, "CAB202 Snek Game");
	draw_string(0, 10, "Timothy Vu");
	draw_string(0, 20, "N9454870");
	show_screen();
	_delay_ms(2000);
}
//End DrawName

//Initialises a food sprite at a random location 
void InitialiseFood (void) {
	if (isFirstFood) {
		init_sprite(&food, rand() % (LCD_X-3), rand() %(LCD_Y-3-8)+8, 3,3, foodImg);
		isFirstFood = false;
	} else{
	srand(GetSystemTime());
	init_sprite(&food, rand() % (LCD_X-3), rand() %(LCD_Y-3-8)+8, 3,3, foodImg);
	}
}
//End CreateFood

//Initialise the train of snek sprites
void InitialiseSnek (void) {
	snekLength = 2;
	for (int i = 0; i < maxSnakeLength; i++) {
		init_sprite(&snek[i], LCD_X/2-(i*3), LCD_Y/2, 3, 3, snekImg);
	}
}
//End InitialiseSnek

//Initialises the walls
void InitialiseWalls (void) {
	for (int i = 0; i < 3; i++) {
		init_sprite(&wall1[i], 0, LCD_Y - (10+i*11), 8, 1, wall1Img);
		init_sprite(&wall2[i], 8, LCD_Y - (10+i*11), 8, 1, wall2Img);
	}
}
//End InitialiseWalls

//Updates the position of the snek
void UpdateSnek(void) {
	if (direction == -(previousDirection) && (direction != 0) ) {
		isSnekKill = true;
	} else {
		if (direction != 0) {

			for (int i = maxSnakeLength-1; i > 0; i--) {
				snek[i].x = snek [i-1].x;
				snek[i].y = snek[i-1].y;
			}

			if(direction == left) {
				snek[0].x -= 3;
			} else if (direction == right) {
				snek[0].x += 3;
			} else if (direction == up) {
				snek[0].y -= 3;
			} else if (direction == down) {
				snek[0].y += 3;
			}
		}

		previousDirection = direction;
	}

	if (snek[0].x >= LCD_X){
		snek[0].x = 1;
	}

	if (snek[0].x < 0){
		snek[0].x = LCD_X;
	}

	if (snek[0].y >= LCD_Y){
		snek[0].y = 7;
	}

	if (snek[0].y <= 6) {
		snek[0].y = LCD_Y;
	}

}
//End UpdateSnek

//Checks to see whether spriteA and spriteB have ollided and returns a bool
bool CheckCollided (Sprite* spriteA, Sprite* spriteB) {
	if (((*spriteA).x >= (*spriteB).x - 2) && ((*spriteA).x <= (*spriteB).x+2) && ((*spriteA).y >= (*spriteB).y-2) && ((*spriteA).y <= (*spriteB).y + 2)) {
		return true;
	}
	else {
		return false;
	}
}
//End is collided

bool CheckCollidedWall (Sprite* spriteA, Sprite* spriteB) {
	if (((*spriteA).x >= (*spriteB).x-2) && ((*spriteA).x <= (*spriteB).x + 7) &&  ((*spriteA).y >= (*spriteB).y-2) && ((*spriteA).y <= (*spriteB).y)) {
		return true;
	} else {
		return false;
	}
}

//Returns the system time as a double
double GetSystemTime(void) {
	return (ovf_count * 65536 + TCNT1) * 8000000/1024;
}
//End GetSystemTime

//Converts score/lives to a string and draws on the lcd
void ShowScoreAndLives (void) {
	sprintf(scoreBuff, "%02d", score);
	sprintf(livesBuff, "(%01d)", lives);
	draw_string(0,0, scoreBuff);
	draw_string(20,0, livesBuff);
}
//End ShowScoreAndLives

//Draws the required parts of the snek on the LCD
void ShowSnek (void) {
	for (int i = 0; i < snekLength; i++) {
		draw_sprite(&snek[i]);
	}
}
//End ShowSnek

//Shows the walls if isWalls is true
void ShowWalls (void) {
	if (isWalls) {
		for (int i = 0; i < 3; i++) {
			draw_sprite(&wall1[i]);
			draw_sprite(&wall2[i]);
		}
	}
}
//End ShowWalls

//Draws the game
void ShowGame (void) {
	clear_screen();
	ShowScoreAndLives();
	ShowSnek();
	draw_sprite(&food);
	ShowWalls();
	show_screen();
}
//End ShowGame

//Performs the initial setup of the game
void Setup (void) {
	snekLength = initialSnekLength;
	isFirstFood = true;
	isSnekKill = false;
	InitialiseSnek();
	InitialiseFood();
	InitialiseWalls();
	_delay_ms(100);
	previousDirection = 0;
	direction = 0;
}
//End Setup

//Main function that calls game processes
void Process (void) {
	UpdateSnek();
	clear_screen();
	ShowGame();
	if (CheckCollided(&snek[0], &food)) {
		InitialiseFood();
		if(isWalls) {
			score++;
		}
		score++;
		if(snekLength < maxSnakeLength)
		snekLength++;
	}

	for (int i = 1; i < snekLength; i++) {
		if (CheckCollided(&snek[0], &snek[i])) {
			isSnekKill = true;
		}
	}

	if (isWalls) {
		for (int i = 0; i<3; i++) {
			if (CheckCollidedWall(&snek[0], &wall1[i]) || CheckCollidedWall(&snek[0], &wall2[i])) {
				isSnekKill = true;
			}
			if (CheckCollidedWall(&food, &wall1[i]) || CheckCollidedWall(&food, &wall2[i])) {
				food.y +=6;
			}
		}
	}

	if(isSnekKill) {
		lives--;
		clear_screen();
		Setup();
	}
}
//End Process

//Shows the game over screen
void ShowGameOver(void) {
	clear_screen();
	draw_string(0,0, "GAME OVER!");
	show_screen();
	_delay_ms(1000);
}

//Main loop
int main (void) {
	setup_adc();
	set_clock_speed(CPU_8MHz);

	InitHardware();

	while(1) {
		DrawName();
		Setup();

		while(lives > 0) {
			// if (PINB >> PIN1 & 0b1) {
			// 	direction = left;	
			// }
			// if (PIND >> PIN0 & 0b1) {
			// 	direction = right;
			// }
			// if (PIND >> PIN1 & 0b1) {
			// 	direction = up;
			
			// }
			// if (PINB >> PIN7 & 0b1) 20
			// 	direction = down;
			// }
			
			Process();		
			_delay_ms(100 - (adc_read(0)/25));
		}
		//PUT SHIT HERE PLS LOOK
		//NFJANJKFNAS
		//IADSUNSI
		//OKAJSFONAJSFN

		ShowGameOver();
		lives = 5;
		score = 0;
	}
}

void setup_adc(){
    // AREF = AVcc
    ADMUX = (1<<REFS0);
 
    // ADC Enable and pre-scaler of 128
    // 8000000/128 = 62500
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
 
    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);
 
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));
 
    return (ADC);
}

ISR (TIMER0_OVF_vect) {
	if (PINB >> PIN1 & 0b1) {
		direction = left;	
		}
	if (PIND >> PIN0 & 0b1) {
		direction = right;
		}
	if (PIND >> PIN1 & 0b1) {
		direction = up;
		}
	if (PINB >> PIN7 & 0b1) {
		direction = down;
		}
	if ((PINF>>5)&1) {
		if(!isWalls) {
			isWalls = true;
		}
	}
	if ((PINF>>6)&1) {
		if (isWalls) {
			isWalls = false;
		}
	}
}



