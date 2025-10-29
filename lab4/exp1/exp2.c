#include "EIE3810_LED.h"
#include "EIE3810_Key.h"
#include "EIE3810_EXTI.h"
#include "stm32f10x.h"

// LED control macros - DS0 on PB5, DS1 on PE5
#define DS0_ON GPIOB->BRR = 1 << 5   // Turn on DS0 (set PB5 high)
#define DS0_OFF GPIOB->BSRR = 1 << 5 // Turn off DS0 (set PB5 low)
#define DS1_ON GPIOE->BSRR = 1 << 5  // Turn on DS1 (set PE5 high)
#define DS1_OFF GPIOE->BRR = 1 << 5  // Turn off DS1 (set PE5 low)

void Delay(u32 count);
void EXTI2_IRQHandler(void);
void EXTI0_IRQHandler(void);

void Delay(u32 count)
{
    u32 i;
    for (i = 0; i < count; i++)
        ;
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

void EXTI0_IRQHandler(void)
{
    u8 i;

    for (i = 0; i < 5; i++)
    {
        DS1_ON;
        Delay(2000000);
        DS1_OFF;
        Delay(2000000);
    }
    EXTI->PR = 1 << 0; // Clear EXTI0 pending bit
}

int main(void)
{
    u32 count = 0;
    EIE3810_LED_Init();
    EIE3810_NVIC_SetPriorityGrouping(5); // Set priority grouping to 5
    EIE3810_Key2_EXTIInit();             // Initialize Key2 as external interrupt
    EIE3810_KeyUp_EXTIInit();            // Initialize KeyUp as external interrupt
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
