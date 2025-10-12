#include "stm32f10x.h"
#include "EIE3810_LED.h"

// put your code here
void EIE3810_LED_Init(){
	RCC->APB2ENR |= 1<<3;
	RCC->APB2ENR |= 1<<6;
	
	GPIOE->CRL &= 0x0 << (4 * 5);
	GPIOB->CRL &= 0x0 << (4 * 5);
	// Init light DS0 PB5
	GPIOB->CRL |= 0x3 << (4 * 5);
	
	// Init light DS1 PE5
	GPIOE->CRL |= 0x3 << (4 * 5);
}
