#include "stm32f10x.h"
#include "EIE3810_LED.h"

// put your code here
void EIE3810_LED_Init()
{
	// Enable GPIOB and GPIOE clocks
	RCC->APB2ENR |= 1 << 3; // GPIOB clock
	RCC->APB2ENR |= 1 << 6; // GPIOE clock

	// Configure PB5 as output (DS0) - 50MHz push-pull output
	GPIOB->CRL &= ~(0xF << (4 * 5)); // Clear bits for PB5
	GPIOB->CRL |= (0x3 << (4 * 5));	 // Set as 50MHz output, push-pull

	// Configure PE5 as output (DS1) - 50MHz push-pull output
	GPIOE->CRL &= ~(0xF << (4 * 5)); // Clear bits for PE5
	GPIOE->CRL |= (0x3 << (4 * 5));	 // Set as 50MHz output, push-pull

	// Turn off both LEDs initially
	GPIOB->BRR = 1 << 5; // DS0 OFF
	GPIOE->BRR = 1 << 5; // DS1 OFF
}
