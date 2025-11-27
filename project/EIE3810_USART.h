#ifndef __EIE3810_USART_H
#define __EIE3810_USART_H
#include "stm32f10x.h"

void EIE3810_clock_tree_init(void);
void EIE3810_USART2_init(u32, u32);
void EIE3810_USART1_init(u32 pclk1, u32 baudrate);
// void USART1_SendChar(u8 ch); // Removed: Not implemented in .c
void USART_print(u8 USARTport, char *str);
void EIE3810_USART1_EXTIInit(void);

#endif
