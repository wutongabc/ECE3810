#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_EXTI.h"
#include "EIE3810_LED.h"
#include "EIE3810_USART.h"

#define LED0_PWM_VAL TIM3->CCR2

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
    EIE3810_LED_Init();              // Initialize LED
    EIE3810_TIME3_PWMInit(9999, 71); // Initialize TIM3 for PWM on DS0 (PB4) at 1kHz
    while (1)
    {
        Delay(1500);
        if (dir)
            LED0PWMVal++; // Adjust brightness value
        else
            LED0PWMVal--; // Adjust brightness value
        if (LED0PWMVal >= 5000)
            dir = 0; // Change direction at max
        if (LED0PWMVal == 0)
            dir = 1;               // Change direction
        LED0_PWM_VAL = LED0PWMVal; // Update PWM duty cycle
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
    {                          // Check update interrupt flag
        GPIOB->ODR ^= 1 << 5;  // Toggle DS0 (PB5) only
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
