#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_EXTI.h"
#include "EIE3810_LED.h"
#include "EIE3810_USART.h"

void EIE3810_TIM3_Init(u16 arr, u16 psc);
void EIE3810_TIM4_Init(u16 arr, u16 psc);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void Delay(u32 nCount);

int main(void)
{
    EIE3810_clock_tree_init();
    EIE3810_LED_Init(); // Initialize LED

    // Set initial LED states - DS0 ON, DS1 OFF for opposite state
    GPIOB->BSRR = 1 << 5; // Turn ON DS0
    GPIOE->BRR = 1 << 5;  // Turn OFF DS1

    EIE3810_NVIC_SetPriorityGrouping(5);
    EIE3810_TIM3_Init(4999, 7199); // Initialize TIM3 for DS0 - 1 Hz
    EIE3810_TIM4_Init(4999, 7199); // Initialize TIM4 for DS1 - 1 Hz
    while (1)
    {
        ;
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

void Delay(u32 nCount)
{
    for (; nCount != 0; nCount--)
        ;
}
