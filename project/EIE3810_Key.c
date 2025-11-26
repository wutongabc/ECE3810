#include "stm32f10x.h"
#include "EIE3810_KEY.h"


// put your procedure and code here

void EIE3810_Key_init(){
	RCC->APB2ENR |= 1<<6;
	RCC->APB2ENR |= 1<<2;
	
	// reset GPIO
	GPIOE->CRL &= 0x0 << (4 * 4);
	GPIOA->CRL &= 0x0;

	// Init
	GPIOA->CRL |= 0x8; //UP 
	GPIOE->CRL |= 0x8 << (4 * 2); //key2 PE2
	GPIOE->CRL |= 0x8 << (4 * 3); //key1 PE3
	GPIOE->CRL |= 0x8 << (4 * 4); //key0 PE4


}
