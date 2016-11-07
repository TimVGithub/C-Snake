/*	CAB202: Tutorial 8
*	Question 2 - Template
*
*	B.Talbot, April 2016
*	Queensland University of Technology
*/
#include <avr/io.h>
#include <util/delay.h>

#include "cpu_speed.h"
#include <graphics.h>
#include <lcd.h>
#include "sprite.h"

#define LCD_X 84
#define LCD_Y 48

volatile Sprite block;

unsigned char block1[] = {
0b11111111,
0b11111111,
0b11111111,
0b11111111,
0b11111111,
0b11111111,
0b11111111,
0b11111111,};


void create_screen();

/*
* Main function - this is all done for you... no need to modify
*/

int main(void) {

	// Set the CPU speed to 8MHz (you must also be compiling at 8MHz)
	set_clock_speed(CPU_8MHz);

	// Initialise the LCD screen
	lcd_init(LCD_DEFAULT_CONTRAST);

	create_screen();

	return 0;
}


void create_screen() {
	// Text
	draw_line(LCD_X-1, 0, LCD_X-1, LCD_Y);
	draw_string(LCD_X/4-5, (LCD_Y/2)-4, "Martin-Timothy Vu");
	draw_string(LCD_X/4, (LCD_Y/2)+4, "N9454870");
	init_sprite((Sprite*) &block1[8], 0, 0, 8, 8, block1);
	draw_sprite((Sprite*) &block );
	show_screen();
}
