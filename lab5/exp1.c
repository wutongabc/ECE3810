#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_EXTI.h"
#include "EIE3810_LED.h"

void EIE3810_TIM3_Init(u16 arr, u16 psc);

int main(void)
{
    EIE3810_clock_tree_init();
    EIE3810_LED_Init(); // Initialize LED
    EIE3810_NVIC_SetPriorityGrouping(5);
    EIE3810_TIM3_Init(4999, 7199); // Initialize TIM3
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
    TIM3->DIER |= 1 << 0;   // Enable update interrupt
    TIM3->CR1 |= 0x01;      // Enable TIM3

    NVIC->IP[29] = 0x45;      // Set TIM3 interrupt priority
    NVIC->ISER[0] |= 1 << 29; // Enable TIM3 interrupt in NVIC
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1 << 0))
    {                          // Check update interrupt flag
        GPIOB->ODR ^= 1 << 5;  // Toggle LED5
        TIM3->SR &= ~(1 << 0); // Clear update interrupt flag
    }
}
