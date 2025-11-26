#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_EXTI.h"
#include "EIE3810_LED.h"
#include "EIE3810_USART.h"
#include "EIE3810_Debug.h"
#include "EIE3810_Joyboard.h"

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
// void SysTick_Handler(void);
void Delay(u32 nCount);

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
//     // task1HeartBeat++;
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
