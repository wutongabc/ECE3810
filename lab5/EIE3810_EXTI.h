#ifndef __EIE3810_EXTI_H
#define __EIE3810_EXTI_H
#include "stm32f10x.h"

// Function declarations
void EIE3810_NVIC_SetPriorityGrouping(u8 prigroup);
void EIE3810_Key2_EXTIInit(void);
void EIE3810_KeyUp_EXTIInit(void);

#endif
