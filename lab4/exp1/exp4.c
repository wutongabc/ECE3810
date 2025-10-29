#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_USART.h"
#include "EIE3810_LED.h"
#include "Font.h"

// LED control macros - DS0 on PB5
#define DS0_ON GPIOB->BRR = 1 << 5	 // Turn on DS0 (set PB5 low for common anode)
#define DS0_OFF GPIOB->BSRR = 1 << 5 // Turn off DS0 (set PB5 high)

void Delay(u32);
void exp1(void);
void exp2(void);
void exp3(void);
void exp4(void);
void USART1_IRQHandler(void);

int main(void)
{
	EIE3810_clock_tree_init();
	EIE3810_LED_Init(); // Initialize LED
	EIE3810_TFTLCD_Init();
	Delay(5000000);
	EIE3810_TFTLCD_Clear(WHITE);
	EIE3810_NVIC_SetPriorityGrouping(5);
	EIE3810_USART1_init(72, 9600); // Initialize USART1 with baud rate 9600
	EIE3810_USART1_EXTIInit();	   // Enable USART1 receive interrupt
	DS0_OFF;					   // Turn off LED0 initially
	USART_print(1, "1234567890");
	while (1)
	{
		USART_print(1, "EIE3810_Lab4");
		while (!((USART1->SR >> 7) & 0x1))
			;
		Delay(1000000);
	}

	// EIE3810_TFTLCD_Init();
	// Delay(1000000);

	// // Clear screen to white background (initial setup)
	// EIE3810_TFTLCD_Clear(WHITE);
	// exp1();
	// exp2();
	// exp4();
	// exp3();
}

void Delay(u32 count) // Use looping for delay
{
	u32 i;
	for (i = 0; i < count; i++)
		;
}

void exp1(void)
{
	int i, line;
	u16 colors[5] = {RED, YELLOW, GREEN, BLUE, BLACK}; // 5 colors for 5 lines
	u16 y_positions[5] = {10, 20, 30, 40, 50};		   // y positions for 5 lines

	// Draw 5 horizontal lines with different colors
	for (line = 0; line < 5; line++)
	{
		for (i = 0; i < 20; i++) // 20 dots per line
		{
			EIE3810_TFTLCD_DrawDot(10 + i, y_positions[line], colors[line]);
		}
	}
}
void exp2(void)
{
	EIE3810_TFTLCD_FillRectangle(50, 700, 100, 800, YELLOW);
}

void exp3()
{
	// Countdown loop: display digits from 9 to 0 periodically
	while (1)
	{
		u8 digit;
		u16 center_x = 202; // Center position x = (480 - 75) / 2 = 202
		u16 center_y = 340; // Center position y = (800 - 120) / 2 = 340

		// Count down from 9 to 0
		for (digit = 9; digit <= 9; digit--) // digit is u8, so it wraps after 0
		{
			// Clear the digit area by drawing a white rectangle
			EIE3810_TFTLCD_FillRectangle(center_x, center_y, center_x + 75, center_y + 140, WHITE);

			// Display the current digit in blue color
			EIE3810_TFTLCD_SevenSegment(center_x, center_y, digit, BLUE);

			// Delay for approximately 1 second
			Delay(10000000);

			// Break the loop when digit reaches 0 (before wrapping to 255)
			if (digit == 0)
				break;
		}
	}
}

void exp4()
{
	// Convert number 123090704 to ASCII and display at the top center of the screen
	// The digits are: 1, 2, 3, 0, 9, 0, 7, 0, 4
	// ASCII codes: '0'=48, '1'=49, '2'=50, ..., '9'=57
	u8 digits[] = {1, 2, 3, 0, 9, 0, 7, 0, 4};
	u8 ascii_codes[9]; // Array to store ASCII codes
	u8 i;
	u8 message_len = 9; // Length of the message
	// u8 test_code = 48;

	// Convert digits to ASCII codes
	for (i = 0; i < message_len; i++)
	{
		ascii_codes[i] = digits[i] + 48; // Add 48 to convert digit to ASCII code
										 // Example: digit 1 + 48 = 49 (ASCII code for '1')
	}

	// Calculate starting x position to center the message
	// Each character is 8 pixels wide
	// Total width = 9 characters * 8 pixels = 72 pixels
	// Screen width = 480 pixels
	// Center position: (480 - 72) / 2 = 204
	u16 start_x = 30;

	// Place at top center of screen (y = 50 for some margin from top)
	u16 start_y = 200;

	// EIE3810_TFTLCD_ShowChar(start_x, start_y, test_code, BLUE, WHITE);

	// Display each character using ASCII code
	for (i = 0; i < message_len; i++)
	{
		// Display character at position (start_x + i*8, start_y)
		// Character color: BLUE, Background color: WHITE
		EIE3810_TFTLCD_ShowChar(start_x + i * 16, start_y, ascii_codes[i], WHITE, BLACK);
	}
}

void USART1_IRQHandler(void)
{
	u32 buffer;
	// Check if RXNE (Receive Not Empty) flag is set
	if (USART1->SR & (1 << 5))
	{
		buffer = USART1->DR; // Read received data (clears RXNE flag)
		if (buffer == 'Q')
		{
			DS0_ON; // Turn on LED0 when 'Q' is received
		}
		else if (buffer == 'H')
		{
			DS0_OFF; // Turn off LED0 when 'H' is received
		}
	}
}