#include "EIE3810_LED.h"
#include "EIE3810_Key.h"
#include "stm32f10x.h"

// LED control macros - DS0 on PB5, DS1 on PE5
#define DS0_ON  GPIOB->BSRR = 1 << 5   // Turn on DS0 (set PB5 high)
#define DS0_OFF GPIOB->BRR = 1 << 5    // Turn off DS0 (set PB5 low)
#define DS1_ON  GPIOE->BSRR = 1 << 5   // Turn on DS1 (set PE5 high)
#define DS1_OFF GPIOE->BRR = 1 << 5    // Turn off DS1 (set PE5 low)

void Delay(u32 count);
void EIE3810_Key2_EXITInit(void);
void EIE3810_NVIC_SetPriorityGrouping(u8 prigroup);
void EXTI2_IRQHandler(void);

void Delay(u32 count)
{
    u32 i;
    for (i = 0; i < count; i++)
        ;
}

void EIE3810_Key2_EXITInit(void)
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

void EXTI2_IRQHandler(void)
{
    u8 i;

    for (i = 0; i < 10; i++)
    {
        DS0_ON;
        Delay(3000000);
        DS0_OFF;
        Delay(3000000);
    }
    EXTI->PR = 1 << 2; // Clear EXTI2 pending bit
}

int main(void)
{
    u32 count = 0;
    EIE3810_LED_Init();
    EIE3810_NVIC_SetPriorityGrouping(5); // Set priority grouping to 5
    EIE3810_Key2_EXITInit();             // Initialize Key2 as external interrupt
    DS0_OFF;
    DS1_OFF;
    while (1)
    {
        Delay(5000000);
        DS1_ON;
        Delay(5000000);
        DS1_OFF;
        count++;
    }
}