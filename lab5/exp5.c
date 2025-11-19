#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_EXTI.h"
#include "EIE3810_LED.h"
#include "EIE3810_USART.h"
#include "EIE3810_Debug.h"

#define LED0_PWM_VAL TIM3->CCR2

// Global variables for JOYPAD handling
volatile u8 joypad_current = 0;
volatile u8 joypad_last = 0xFF;

void EIE3810_TIM3_Init(u16 arr, u16 psc);
void EIE3810_TIM4_Init(u16 arr, u16 psc);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void EIE3810_SYSTICK_Init(void);
void EIE3810_TIM3_PWMInit(u16 arr, u16 psc);
void JOYPAD_Init(void);
void JOYPAD_Delay(u16 t);
u8 JOYPAD_Read(void);
// void SysTick_Handler(void);
void Delay(u32 nCount);
void JOYPAD_Display_Key(u8 key_value);
void Display_Hex(u16 x, u16 y, u8 value);

int main(void)
{
    u16 LED0PWMVal = 0;
    u8 dir = 1;

    EIE3810_clock_tree_init();
    EIE3810_LED_Init();    // Initialize LED
    EIE3810_TFTLCD_Init(); // Initialize LCD
    // Set the background to white
    EIE3810_TFTLCD_Clear(WHITE);
    EIE3810_TIM3_PWMInit(9999, 71); // Initialize TIM3 for PWM on DS0 (PB4) at 1kHz (not used for PWM anymore)
    JOYPAD_Init();                  // Initialize JOYPAD

    // Configure TIM3 to trigger interrupt at 100Hz for JOYPAD reading
    // ARR = 9999, PSC = 71 gives frequency: 72MHz / (71+1) / (9999+1) ≈ 100Hz
    EIE3810_TIM3_Init(9999, 71);

    // Clear LCD and display initial message
    EIE3810_TFTLCD_Clear(WHITE);
    EIE3810_TFTLCD_ShowChar(10, 10, 'P', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(18, 10, 'R', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(26, 10, 'E', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(34, 10, 'S', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(42, 10, 'S', BLACK, WHITE);

    while (1)
    {
        // CPU can do other tasks while JOYPAD is being read in interrupt
        Delay(100000);
    }
}

void EIE3810_TIM3_Init(u16 arr, u16 psc)
{
    RCC->APB1ENR |= 1 << 1; // Enable TIM3 clock
    TIM3->ARR = arr;        // Set auto-reload value
    TIM3->PSC = psc;        // Set prescaler value
    // TIM3->CNT = 0;          // Start from 0
    TIM3->DIER |= 1 << 0; // Enable update interrupt
    TIM3->CR1 |= 0x01;    // Enable TIM3

    NVIC->IP[29] = 0x45;      // Set TIM3 interrupt priority
    NVIC->ISER[0] |= 1 << 29; // Enable TIM3 interrupt in NVIC
}

void EIE3810_TIM4_Init(u16 arr, u16 psc)
{
    RCC->APB1ENR |= 1 << 2; // Enable TIM4 clock
    TIM4->ARR = arr;        // Set auto-reload value
    TIM4->PSC = psc;        // Set prescaler value
    // TIM4->CNT = arr / 2;    // Start from half period (2500) for 180° phase shift
    TIM4->DIER |= 1 << 0; // Enable update interrupt
    TIM4->CR1 |= 0x01;    // Enable TIM4

    NVIC->IP[30] = 0x45;      // Set TIM4 interrupt priority
    NVIC->ISER[0] |= 1 << 30; // Enable TIM4 interrupt in NVIC
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1 << 0))
    { // Check update interrupt flag
        // Read JOYPAD at 100Hz frequency
        joypad_current = JOYPAD_Read();

        // Check if key state has changed (key pressed down)
        if (joypad_current != joypad_last)
        {
            joypad_last = joypad_current;
            // Call display function to show which key was pressed
            JOYPAD_Display_Key(joypad_current);
        }

        TIM3->SR &= ~(1 << 0); // Clear update interrupt flag
    }
}

// Helper function to display hex value for debugging (now using Debug module)
void Display_Hex(u16 x, u16 y, u8 value)
{
    EIE3810_Debug_ShowHex(x, y, value, 2, RED, WHITE);
}

void TIM4_IRQHandler(void)
{
    if (TIM4->SR & (1 << 0))
    {                          // Check update interrupt flag
        GPIOE->ODR ^= 1 << 5;  // Toggle DS1 (PE5) only
        TIM4->SR &= ~(1 << 0); // Clear update interrupt flag
    }
}

void EIE3810_SYSTICK_Init(void)
{
    SysTick->CTRL = 0;                                                                                // Disable SysTick
    SysTick->LOAD = 72000 - 1;                                                                        // Set reload for 1ms (assuming 72MHz clock)
    SysTick->VAL = 0;                                                                                 // Clear current value
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; // Enable SysTick with interrupt
}

// void SysTick_Handler(void)
// {
//     // SysTick interrupt handler (if needed)
//     task1HeartBeat++;
// }

void EIE3810_TIM3_PWMInit(u16 arr, u16 psc)
{
    RCC->APB2ENR |= 1 << 3;   // Enable GPIOA clock
    GPIOB->CRL &= 0xFF0FFFFF; // Clear PB4 mode bits
    GPIOB->CRL |= 0x00B00000; // Set PB4
    RCC->APB2ENR |= 1 << 0;   // Enable AFIO clock
    AFIO->MAPR &= 0xFFFFF3FF; // Clear TIM3 remap bits
    AFIO->MAPR |= 1 << 11;    // Remap TIM3 CH1 to PB4
    RCC->APB1ENR |= 1 << 1;   // Enable TIM3 clock
    TIM3->ARR = arr;          // Set auto-reload value
    TIM3->PSC = psc;          // Set prescaler value
    TIM3->CCMR1 |= 7 << 12;   // PWM mode 1 on CH2
    TIM3->CCMR1 |= 1 << 11;   // Enable CH2 output
    TIM3->CCER |= 1 << 4;     // Enable CH2
    TIM3->CR1 = 0x0080;       // Enable auto-reload preload
    TIM3->CR1 |= 1 << 0;      // Enable TIM3
}

void Delay(u32 nCount)
{
    for (; nCount != 0; nCount--)
        ;
}

void JOYPAD_Init(void)
{
    RCC->APB2ENR |= 1 << 3;   // Enable GPIOA clock
    RCC->APB2ENR |= 1 << 5;   // Enable GPIOC clock
    GPIOB->CRH &= 0xFFFF00FF; // Clear PB8-PB11 mode bits
    GPIOB->CRH |= 0x00003800; // Set PB8-PB11 as output push-pull
    GPIOB->ODR |= 3 << 10;    // Set PB8-PB11 high
    GPIOD->CRL &= 0xFFFF0FFF; // Clear PD0-PD3 mode bits
    GPIOD->CRL |= 0x00003000; // Set PD0-PD3 as input floating
    GPIOD->ODR |= 1 << 3;     // Set PD0-PD3 high
}

void JOYPAD_Delay(u16 t)
{
    while (t--)
        ;
}

u8 JOYPAD_Read(void)
{
    vu8 temp = 0;
    u8 t;
    GPIOB->BSRR |= 1 << 11; // Set PB11 low
    Delay(80);              // Delay for signal stabilization
    GPIOB->BSRR |= 1 << 27; // Set PB11 high
    for (t = 0; t < 8; t++)
    {
        temp >>= 1; // Shift temp to prepare for next bit
        if ((((GPIOB->IDR) >> 10) & 0x01) == 0)
            temp |= 0x80;       // Read bit and set MSB if low
        GPIOD->BSRR |= 1 << 3;  // Set PB8 low
        Delay(80);              // Delay for signal stabilization
        GPIOD->BSRR |= 1 << 19; // Set PB8 high
        Delay(80);              // Delay before next bit
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
    Display_Hex(400, 10, key_value);

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