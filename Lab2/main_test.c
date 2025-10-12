#include "stm32f10x.h"
void EIE3810_clock_tree_init(void);
void EIE3810_USART1_init(u32 pclk2, u32 baudrate);
void Delay(u32);
void USART1_SendChar(u8 ch);

int main(void)
{
    EIE3810_clock_tree_init();
    EIE3810_USART1_init(72, 9600);

    // Wait approximately 1 second after RESET
    Delay(7200000);

    while (1)
    {
        // Send student ID: 123090704 with simple delay method (no flag checking)
        USART1->DR = 0x31;
        Delay(10000);
        USART1->DR = 0x32;
        Delay(10000);
        USART1->DR = 0x33;
        Delay(10000);
        USART1->DR = 0x30;
        Delay(10000);
        USART1->DR = 0x39;
        Delay(10000);
        USART1->DR = 0x30;
        Delay(10000);
        USART1->DR = 0x37;
        Delay(10000);
        USART1->DR = 0x30;
        Delay(10000);
        USART1->DR = 0x34;
        Delay(10000);
    }
}

viod USART_print(u8 USARTport, char *str)
{
    u8 i = 0;
    while (str[i] != 0x00)
    {
        if (USARTport == 1)
        {
            USART1 - DR = str[i];
        }
        if (USARTport == 2)
        {
            USART2 - DR = str[i];
        }
        Delay(50000);
        if (i == 255)
            break;
        i++;
    }
}

void Delay(u32 count)
{
    u32 i;
    for (i = 0; i < count; i++)
        ;
}

void EIE3810_clock_tree_init(void)
{
    u8 PLL = 7;
    u8 temp = 0;
    RCC->CR |= 0x00010000;
    while (!((RCC->CR >> 17) & 0x1))
        ;
    RCC->CFGR &= 0xFFFDFFFF;
    RCC->CFGR |= 1 << 16;
    RCC->CFGR |= PLL << 18;
    RCC->CR |= 0x01000000;
    while (!(RCC->CR >> 25))
        ;
    RCC->CFGR &= 0xFFFFFFFE;
    RCC->CFGR |= 0x00000002;
    while (temp != 0x02)
    {
        temp = RCC->CFGR >> 2;
        temp &= 0x03;
    }
    RCC->CFGR &= 0xFFFFFC0F;
    RCC->CFGR |= 0x00000400;
    FLASH->ACR = 0x32;
    RCC->APB1ENR |= 1 << 17;
    RCC->CFGR &= ~(0x7 << 11);
    RCC->APB2ENR |= 1 << 14;
}

void EIE3810_USART1_init(u32 pclk2, u32 baudrate)
{
    float temp;
    u16 mantissa;
    u16 fraction;

    temp = (float)(pclk2 * 1000000) / (baudrate * 16);
    mantissa = temp;
    fraction = (temp - mantissa) * 16;
    mantissa <<= 4;
    mantissa += fraction;

    RCC->APB2ENR |= (1 << 2); // GPIOA Clock
    RCC->APB2ENR |= (1 << 0); // AFIO Clock

    GPIOA->CRH &= 0xFFFFF00F;
    GPIOA->CRH |= 0x000004B0; // PA10: Input floating, PA9: Alt func push-pull 50MHz

    RCC->APB2RSTR |= (1 << 14);
    RCC->APB2RSTR &= ~(1 << 14);

    USART1->BRR = mantissa;
    USART1->CR1 = 0x0000;
    USART1->CR2 = 0x0000;
    USART1->CR3 = 0x0000;

    USART1->CR1 |= (1 << 3);  // TE
    USART1->CR1 |= (1 << 13); // UE
}
