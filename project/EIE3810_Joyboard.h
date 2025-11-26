#ifndef __EIE3810_JOYBOARD_H
#define __EIE3810_JOYBOARD_H

#include "stm32f10x.h"

void JOYPAD_Init(void);
u8 JOYPAD_Read(void);
void JOYPAD_Display_Key(u8 key_value);

#endif

