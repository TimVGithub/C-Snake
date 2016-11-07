#include <avr/io.h>
#include "cpu_speed.h"
#include <util/delay.h>

int main(void) {

	// Set the CPU speed to 8MHz (you must also be compiling at 8MHz)
	set_clock_speed(CPU_8MHz);

	// SW0 and SW1 are connected to pins F6 and F5. Configure all pins as Input(0)
	DDRF = 0b00000000;

	// Teensy LED is connected to B2, configure pin 2 as an output(1)
	DDRB = 0b00000100;

	// turn OFF LED initially
	PORTB = 0b00000000;


	while (1) {
		// Read input from PORTF.
		// if Pin F5 changes to high then if condition is true and 
		// an output is ent to Port B pin 2
		if (PINB & 0b00000010) {
			// Send output to PORTB LED0(LEFT LED) 
			PORTB = 0b00000100;
		}
		//IF PRESS RIGHT, TURN RIGHT LED ON
		if (PIND & 0b00000001) {
			PORTB = 0b00001000;
		}
		//IF PRESS UP, TURN BOTH LEDS ON
		if (PIND & 0b00000010) {
			PORTB = 0b00001100;
		}

		//IF PRESS DOWN, TURN BOTH LEDS OFF
		if (PINB & 0b10000000) {
			PORTB = 0b00000000;
		}
		//IF PRESS CENTRE, ALTERNATE LEDS
		if (PINB & 0b00000001) {
			PORTB = ~(PORTB);
			_delay_ms(500);
		}
	}
	return 0;
}