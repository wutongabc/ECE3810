#include "stm32f10x.h"
#include "EIE3810_USART.h"
void Delay(u32);

int main(void)
{
    EIE3810_clock_tree_init();
    EIE3810_USART1_init(72, 9600);

    // Wait approximately 1 second after RESET
    Delay(7200000);

    while (1)
    {
        // // Send student ID: 123090704 with simple delay method (no flag checking)
        // USART1->DR = 0x31;
        // Delay(10000);
        // USART1->DR = 0x32;
        // Delay(10000);
        // USART1->DR = 0x33;
        // Delay(10000);
        // USART1->DR = 0x30;
        // Delay(10000);
        // USART1->DR = 0x39;
        // Delay(10000);
        // USART1->DR = 0x30;
        // Delay(10000);
        // USART1->DR = 0x37;
        // Delay(10000);
        // USART1->DR = 0x30;
        // Delay(10000);
        // USART1->DR = 0x34;
        // Delay(10000);
        USART_print(1, "1234567890");
    }
}

void Delay(u32 count)
{
    u32 i;
    for (i = 0; i < count; i++)
        ;
}