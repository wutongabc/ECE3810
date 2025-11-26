#include "EIE3810_Joyboard.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_Debug.h"

// Local delay function used by JOYPAD_Read
static void JOYPAD_Delay(u32 t)
{
    while (t--)
        ;
}

void JOYPAD_Init(void)
{
    RCC->APB2ENR |= 1 << 3;   // Enable GPIOB clock
    RCC->APB2ENR |= 1 << 5;   // Enable GPIOD clock
    
    GPIOB->CRH &= 0xFFFF00FF; // Clear PB8-PB11 mode bits
    GPIOB->CRH |= 0x00003800; // Set PB8-PB11 as output push-pull
    GPIOB->ODR |= 3 << 10;    // Set PB8-PB11 high
    
    GPIOD->CRL &= 0xFFFF0FFF; // Clear PD0-PD3 mode bits
    GPIOD->CRL |= 0x00003000; // Set PD0-PD3 as input floating
    GPIOD->ODR |= 1 << 3;     // Set PD0-PD3 high
}

u8 JOYPAD_Read(void)
{
    vu8 temp = 0;
    u8 t;
    GPIOB->BSRR |= 1 << 11; // Set PB11 low
    JOYPAD_Delay(80);       // Delay for signal stabilization
    GPIOB->BSRR |= 1 << 27; // Set PB11 high
    for (t = 0; t < 8; t++)
    {
        temp >>= 1; // Shift temp to prepare for next bit
        if ((((GPIOB->IDR) >> 10) & 0x01) == 0)
            temp |= 0x80;       // Read bit and set MSB if low
        GPIOD->BSRR |= 1 << 3;  // Set PB8 low
        JOYPAD_Delay(80);       // Delay for signal stabilization
        GPIOD->BSRR |= 1 << 19; // Set PB8 high
        JOYPAD_Delay(80);       // Delay before next bit
    }
    return temp;
}

// Display the pressed key name on LCD
// According to Table 1: keys are at bit positions 0-7 for A, B, SELECT, START, UP, DOWN, LEFT, RIGHT
void JOYPAD_Display_Key(u8 key_value)
{
    // Static variables to keep track of display position
    static u16 display_x = 10;
    static u16 display_y = 50;

    u8 key_name[20] = "";

    // Display the hex value for debugging (top right corner)
    // Using EIE3810_Debug_ShowHex directly instead of Display_Hex wrapper
    EIE3810_Debug_ShowHex(400, 10, key_value, 2, RED, WHITE);

    // Map key value (each bit represents a key)
    // Bit 0: A, Bit 1: B, Bit 2: SELECT, Bit 3: START, Bit 4: UP, Bit 5: DOWN, Bit 6: LEFT, Bit 7: RIGHT
    // Note: When pressed, bit is HIGH (1), not LOW (0)
    if (key_value & 0x01) // Bit 0 is active (high)
        key_name[0] = 'A';
    else if (key_value & 0x02) // Bit 1 is active (high)
        key_name[0] = 'B';
    else if (key_value & 0x04) // Bit 2 is active (high)
    {
        EIE3810_TFTLCD_ShowChar(display_x, display_y, 'S', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 8, display_y, 'E', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 16, display_y, 'L', BLACK, WHITE);
        display_x += 24;
        return;
    }
    else if (key_value & 0x08) // Bit 3 is active (high)
    {
        EIE3810_TFTLCD_ShowChar(display_x, display_y, 'S', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 8, display_y, 'T', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 16, display_y, 'A', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 24, display_y, 'R', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 28, display_y, 'T', BLACK, WHITE);
        display_x += 40;
        return;
    }
    else if (key_value & 0x10) // Bit 4 is active (high)
    {
        EIE3810_TFTLCD_ShowChar(display_x, display_y, 'U', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 8, display_y, 'P', BLACK, WHITE);
        display_x += 16;
        return;
    }
    else if (key_value & 0x20) // Bit 5 is active (high)
    {
        EIE3810_TFTLCD_ShowChar(display_x, display_y, 'D', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 8, display_y, 'O', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 16, display_y, 'W', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 24, display_y, 'N', BLACK, WHITE);
        display_x += 32;
        return;
    }
    else if (key_value & 0x40) // Bit 6 is active (high)
    {
        EIE3810_TFTLCD_ShowChar(display_x, display_y, 'L', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 8, display_y, 'E', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 16, display_y, 'F', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 24, display_y, 'T', BLACK, WHITE);
        display_x += 32;
        return;
    }
    else if (key_value & 0x80) // Bit 7 is active (high)
    {
        EIE3810_TFTLCD_ShowChar(display_x, display_y, 'R', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 8, display_y, 'I', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 16, display_y, 'G', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 24, display_y, 'H', BLACK, WHITE);
        EIE3810_TFTLCD_ShowChar(display_x + 32, display_y, 'T', BLACK, WHITE);
        display_x += 40;
        return;
    }
    else
    {
        // No key is pressed (value is 0x00)
        return;
    }

    // Display single character key names
    EIE3810_TFTLCD_ShowChar(display_x, display_y, key_name[0], BLACK, WHITE);
    display_x += 8;

    // Wrap to next line if needed
    if (display_x > 400)
    {
        display_x = 10;
        display_y += 20;
    }

    // Clear screen if y position exceeds display
    if (display_y > 700)
    {
        EIE3810_TFTLCD_Clear(WHITE);
        display_x = 10;
        display_y = 50;
    }
}

