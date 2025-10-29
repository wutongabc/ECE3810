#include "stm32f10x.h"
#include "EIE3810_USART.h"

void EIE3810_clock_tree_init(void)
{
    u8 PLL = 7;
    u8 temp = 0;
    RCC->CR |= 0x00010000; // Turn on HSE clock
    while (!((RCC->CR >> 17) & 0x1))
        ;                    // Wait for HSE ready
    RCC->CFGR &= 0xFFFDFFFF; // PLL not devided
    RCC->CFGR |= 1 << 16;    // HSE oscillator selected as PLL input clock
    RCC->CFGR |= PLL << 18;  // Set PLLCLK to 72Mhz (f{PLL} * 8Mhz)
    RCC->CR |= 0x01000000;   // turn on PLLON
    while (!(RCC->CR >> 25))
        ;                    // wait until PLLRDY is locked
    RCC->CFGR &= 0xFFFFFFFE; // clear these bits
    RCC->CFGR |= 0x00000002; // Let PLL selected as system clock
    while (temp != 0x02)     // wait until PLL has been system clock
    {
        temp = RCC->CFGR >> 2;
        temp &= 0x03; // read the status of system clock and reset temp
    }
    RCC->CFGR &= 0xFFFFFC0F; // clear APB1 prescaler
    // and set AHB presacler to make SYSCLK not devided.
    RCC->CFGR |= 0x00000400; // set APB1 prescaler. HCLK devided by 2
    FLASH->ACR = 0x32;       // Set FLASH with 2 wait states
    RCC->APB1ENR |= 1 << 17; // Enable APB1 output 36MHz of PCLK1 to USART2

    RCC->CFGR &= ~(0x7 << 11); // Clear APB2 prescaler bits
                               // APB2 prescaler = 1 (not divided), so APB2 = 72MHz
    RCC->APB2ENR |= 1 << 14;   // Enable USART1 clock
}

void EIE3810_USART2_init(u32 pclk1, u32 baudrate)
{
    // USART2
    float temp;
    u16 mantissa;
    u16 fraction;
    temp = (float)(pclk1 * 1000000) / (baudrate * 16);
    mantissa = temp;
    fraction = (temp - mantissa) * 16;
    mantissa <<= 4;
    mantissa += fraction;
    RCC->APB2ENR |= 1 << 2;   // Enable IO port A clock
    GPIOA->CRL &= 0xFFFF00FF; // Clear GPIOA port3 and port2
    GPIOA->CRL |= 0x00008B00; // PA3 to input with pull-up / pull-down
    // PA2 to Alternate function output Open-drain
    RCC->APB1RSTR |= 1 << 17;    // Reset USART2
    RCC->APB1RSTR &= ~(1 << 17); // stop reset USART2
    USART2->BRR = mantissa;      // Set the rate of USART2 to 9600 baud
    USART2->CR1 = 0x2008;        // enable transmitter, disable parity and inhibit interrupt
}

void EIE3810_USART1_init(u32 pclk1, u32 baudrate)
{
    // USART1 on APB2
    float temp;
    u16 mantissa;
    u16 fraction;

    // Calculate baud rate register value
    temp = (float)(pclk1 * 1000000) / (baudrate * 16);
    mantissa = temp;
    fraction = (temp - mantissa) * 16;
    mantissa <<= 4;
    mantissa += fraction;

    // Enable GPIOA and AFIO clocks
    RCC->APB2ENR |= (1 << 2); // GPIOA Clock
    RCC->APB2ENR |= (1 << 0); // AFIO Clock (Alternative Function IO)

    // Configure PA9 (TX) and PA10 (RX)
    GPIOA->CRH &= 0xFFFFF00F; // Clear PA10 and PA9 configuration bits (bits 4-11)
    GPIOA->CRH |= 0x000004B0; // PA10[11:8]=0100 (Input floating)
                              // PA9[7:4]=1011 (Alt func push-pull 50MHz)

    // Reset and configure USART1
    RCC->APB2RSTR |= (1 << 14);  // Reset USART1
    RCC->APB2RSTR &= ~(1 << 14); // Release reset

    // Configure USART1 before enabling
    USART1->BRR = mantissa; // Set baud rate FIRST
    USART1->CR1 = 0x0000;   // Clear CR1
    USART1->CR2 = 0x0000;   // 1 stop bit (default)
    USART1->CR3 = 0x0000;   // No hardware flow control

    USART1->CR1 |= (1 << 3);  // TE: Transmitter Enable
    USART1->CR1 |= (1 << 2);  // RE: Receiver Enable
    USART1->CR1 |= (1 << 5);  // RXNEIE: RXNE Interrupt Enable (Receive interrupt)
    USART1->CR1 |= (1 << 13); // UE: USART Enable (MUST be last!)
}

void USART_print(u8 USARTport, char *str)
{
    u8 i = 0;
    while (str[i] != 0x00)
    {
        if (USARTport == 1)
        {
            USART1->DR = str[i];
        }
        if (USARTport == 2)
        {
            USART2->DR = str[i];
        }
        // Delay(50000);
        //  checks the TXE flag in the SR register
        if (USARTport == 1)
        {
            while (!((USART1->SR >> 7) & 0x1))
                ;
        }
        if (USARTport == 2)
        {
            while (!((USART2->SR >> 7) & 0x1))
                ;
        }

        if (i == 255)
            break;
        i++;
    }
}

void EIE3810_USART1_EXTIInit(void)
{
    NVIC->IP[37] = 0x65;     // Set priority for USART1 interrupt
    NVIC->ISER[1] |= 1 << 5; // Enable USART1 interrupt in NVIC
}