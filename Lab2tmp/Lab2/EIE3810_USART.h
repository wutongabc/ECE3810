#ifndef __EIE3810_BUZZER_H
#define __EIE3810_BUZZER_H
#include "stm32f10x.h"

void EIE3810_clock_tree_init(void);
void EIE3810_USART2_init(u32, u32);
void EIE3810_USART1_init(u32 pclk1, u32 baudrate);
void USART1_SendChar(u8 ch);
void USART_print(u8 USARTport, char *str);

#endif
