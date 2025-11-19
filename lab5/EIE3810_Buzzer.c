#include "stm32f10x.h"
#include "EIE3810_Buzzer.h"

// put your procedure and code here
void EIE3810_Buzzer_init()
	{
	RCC->APB2ENR |= 1<<3;
	// Init Buzzer
	GPIOB->CRH &= 0x0;
	GPIOB->CRH |= 0x3;
}
