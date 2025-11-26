#include "EIE3810_Debug.h"
#include "EIE3810_TFTLCD.h"
#include <stdarg.h>
#include <stdio.h>

// Display a single character at specified position
void EIE3810_Debug_ShowChar(u16 x, u16 y, char c, u16 color, u16 bgcolor)
{
    EIE3810_TFTLCD_ShowChar(x, y, (u8)c, color, bgcolor);
}

// Display a string at specified position
void EIE3810_Debug_ShowString(u16 x, u16 y, const char *str, u16 color, u16 bgcolor)
{
    u16 current_x = x;
    while (*str != '\0')
    {
        if (*str == '\n')
        {
            // New line
            current_x = x;
            y += 20;
        }
        else if (*str == '\r')
        {
            // Carriage return
            current_x = x;
        }
        else
        {
            EIE3810_TFTLCD_ShowChar(current_x, y, (u8)*str, color, bgcolor);
            current_x += 8;
        }
        str++;
    }
}

// Display a decimal number with specified length (zero-padded if needed)
// Example: ShowNum(10, 10, 123, 5, BLACK, WHITE) displays "00123"
void EIE3810_Debug_ShowNum(u16 x, u16 y, u32 num, u8 len, u16 color, u16 bgcolor)
{
    u8 i;
    u8 temp;
    u8 show_zero = 0;
    u32 divisor = 1;

    // Calculate divisor
    for (i = 1; i < len; i++)
    {
        divisor *= 10;
    }

    // Display each digit
    for (i = 0; i < len; i++)
    {
        temp = (num / divisor) % 10;
        if (temp == 0 && show_zero == 0 && i != (len - 1))
        {
            // Leading zeros - show as space or zero depending on requirement
            EIE3810_TFTLCD_ShowChar(x + i * 8, y, '0', color, bgcolor);
        }
        else
        {
            show_zero = 1;
            EIE3810_TFTLCD_ShowChar(x + i * 8, y, temp + '0', color, bgcolor);
        }
        divisor /= 10;
    }
}

// Display a hexadecimal number with specified length
// Example: ShowHex(10, 10, 0xAB, 2, BLACK, WHITE) displays "AB"
void EIE3810_Debug_ShowHex(u16 x, u16 y, u32 num, u8 len, u16 color, u16 bgcolor)
{
    u8 i;
    u8 temp;
    const char hex_chars[] = "0123456789ABCDEF";

    for (i = 0; i < len; i++)
    {
        temp = (num >> ((len - 1 - i) * 4)) & 0x0F;
        EIE3810_TFTLCD_ShowChar(x + i * 8, y, hex_chars[temp], color, bgcolor);
    }
}

// Display a binary number with specified bits
// Example: ShowBinary(10, 10, 0b1010, 4, BLACK, WHITE) displays "1010"
void EIE3810_Debug_ShowBinary(u16 x, u16 y, u32 num, u8 bits, u16 color, u16 bgcolor)
{
    u8 i;
    u8 bit;

    for (i = 0; i < bits; i++)
    {
        bit = (num >> (bits - 1 - i)) & 0x01;
        EIE3810_TFTLCD_ShowChar(x + i * 8, y, bit + '0', color, bgcolor);
    }
}

// Display a floating point number with specified precision
// Example: ShowFloat(10, 10, 3.14159, 2, BLACK, WHITE) displays "3.14"
void EIE3810_Debug_ShowFloat(u16 x, u16 y, float num, u8 precision, u16 color, u16 bgcolor)
{
    char buffer[32];
    u8 i;
    s32 integer_part;
    u32 decimal_part;
    u32 multiplier = 1;
    u8 is_negative = 0;

    // Handle negative numbers
    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }

    // Calculate multiplier for decimal part
    for (i = 0; i < precision; i++)
    {
        multiplier *= 10;
    }

    // Split into integer and decimal parts
    integer_part = (s32)num;
    decimal_part = (u32)((num - integer_part) * multiplier);

    // Format the string
    if (is_negative)
    {
        if (precision > 0)
            sprintf(buffer, "-%d.%0*d", integer_part, precision, decimal_part);
        else
            sprintf(buffer, "-%d", integer_part);
    }
    else
    {
        if (precision > 0)
            sprintf(buffer, "%d.%0*d", integer_part, precision, decimal_part);
        else
            sprintf(buffer, "%d", integer_part);
    }

    EIE3810_Debug_ShowString(x, y, buffer, color, bgcolor);
}

// Printf-style formatted output (simplified version)
void EIE3810_Debug_Printf(u16 x, u16 y, u16 color, u16 bgcolor, const char *format, ...)
{
    char buffer[128];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    EIE3810_Debug_ShowString(x, y, buffer, color, bgcolor);
}

// Clear a horizontal line
void EIE3810_Debug_Clear_Line(u16 y, u16 color)
{
    EIE3810_TFTLCD_FillRectangle(0, y, 479, y + 15, color);
}

// Clear a rectangular area
void EIE3810_Debug_Clear_Area(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    EIE3810_TFTLCD_FillRectangle(x1, y1, x2, y2, color);
}
