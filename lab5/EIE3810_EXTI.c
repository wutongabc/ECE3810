#include "EIE3810_EXTI.h"
#include "stm32f10x.h"

void EIE3810_NVIC_SetPriorityGrouping(u8 prigroup)
{
    u32 temp1, temp2;
    temp2 = prigroup & 0x00000007;
    temp2 <<= 8;         // Shift to bits 10:8
    temp1 = SCB->AIRCR;  // Read current AIRCR value
    temp1 &= 0x0000F8FF; // Clear PRIGROUP bits
    temp1 |= 0x05FA0000; // Set VECTKEY
    temp1 |= temp2;      // Set new PRIGROUP
    SCB->AIRCR = temp1;  // Write back to AIRCR
}

void EIE3810_Key2_EXTIInit(void)
{
    RCC->APB2ENR |= 1 << 6;   // Enable GPIOE clock
    GPIOE->CRL &= 0xFFFFF0FF; // Configure PE2 as input floating
    GPIOE->CRL |= 0x00000800;
    GPIOE->ODR |= 1 << 2;          // Set PE2 high (pull-up)
    RCC->APB2ENR |= 0x01;          // Enable AFIO clock
    AFIO->EXTICR[0] &= 0xFFFFF0FF; // Map EXTI2 to PE2
    AFIO->EXTICR[0] |= 0x00000400;
    EXTI->IMR |= 1 << 2;     // Unmask EXTI2
    EXTI->FTSR |= 1 << 2;    // Trigger on falling edge for EXTI2
    NVIC->IP[8] = 0x65;      // Set priority for EXTI2 interrupt
    NVIC->ISER[0] |= 1 << 8; // Enable EXTI2 interrupt in NVIC
}

void EIE3810_KeyUp_EXTIInit(void)
{
    RCC->APB2ENR |= 1 << 2;   // Enable GPIOA clock
    GPIOA->CRL &= 0xFFFFFFF0; // Configure PA0 as input floating
    GPIOA->CRL |= 0x00000008;
    GPIOA->ODR &= ~(1 << 0);       // Set PA0 low (no pull-up, KeyUp is active high)
    RCC->APB2ENR |= 0x01;          // Enable AFIO clock
    AFIO->EXTICR[0] &= 0xFFFFFFF0; // Map EXTI0 to PA0
    AFIO->EXTICR[0] |= 0x00000000; // PA0 = 0000
    EXTI->IMR |= 1 << 0;           // Unmask EXTI0
    EXTI->RTSR |= 1 << 0;          // Trigger on rising edge for EXTI0 (key press)
    NVIC->IP[6] = 0x75;            // Set priority for EXTI0 interrupt (lower priority than EXTI2)
    NVIC->ISER[0] |= 1 << 6;       // Enable EXTI0 interrupt in NVIC
}
