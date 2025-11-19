#ifndef __EIE3810_DEBUG_H
#define __EIE3810_DEBUG_H
#include "stm32f10x.h"

// Debug display functions
void EIE3810_Debug_ShowChar(u16 x, u16 y, char c, u16 color, u16 bgcolor);
void EIE3810_Debug_ShowString(u16 x, u16 y, const char *str, u16 color, u16 bgcolor);
void EIE3810_Debug_ShowNum(u16 x, u16 y, u32 num, u8 len, u16 color, u16 bgcolor);
void EIE3810_Debug_ShowHex(u16 x, u16 y, u32 num, u8 len, u16 color, u16 bgcolor);
void EIE3810_Debug_ShowBinary(u16 x, u16 y, u32 num, u8 bits, u16 color, u16 bgcolor);
void EIE3810_Debug_ShowFloat(u16 x, u16 y, float num, u8 precision, u16 color, u16 bgcolor);

// Helper functions
void EIE3810_Debug_Printf(u16 x, u16 y, u16 color, u16 bgcolor, const char *format, ...);
void EIE3810_Debug_Clear_Line(u16 y, u16 color);
void EIE3810_Debug_Clear_Area(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);

#endif
