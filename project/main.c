#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_Key.h"
#include "EIE3810_Joyboard.h"
#include "EIE3810_USART.h"
#include "EIE3810_LED.h"
#include "EIE3810_Buzzer.h"
#include "EIE3810_Debug.h"

#define MY_BAUD_RATE 4800

// Game States
typedef enum {
    WELCOME,
    DIFFICULTY,
    WAIT_SEED,
    COUNTDOWN,
    PLAYING,
    GAMEOVER
} GameState;

// Difficulty Levels
typedef enum {
    DIFF_EASY,
    DIFF_HARD
} Difficulty;

// Global Variables
GameState current_state = WELCOME;
Difficulty current_difficulty = DIFF_EASY;

// Phase 2 Global Variables
volatile u8 random_seed = 0;
volatile u8 seed_received = 0;

// Helper Functions

// Simple delay function
void Delay(u32 count) {
    while(count--) {
        // Empty loop for delay
    }
}

// System Initialization
void System_Init(void) {
    // Initialize System Clock (usually handled by startup code, but we can ensure peripherals are enabled)
    // If EIE3810_clock_tree_init() exists in USART.h/c, it might be used here, but often SystemInit() is automatic.
    // Assuming standard library setup.
    
    // Initialize Peripherals
    EIE3810_Key_init();      // Note: Lowercase 'i' based on actual header
    EIE3810_LED_Init();
    EIE3810_Buzzer_init();   // Note: Lowercase 'i' based on actual header
    
    // Phase 2 Initialization
    EIE3810_USART1_init(72, MY_BAUD_RATE);
    EIE3810_USART1_EXTIInit();

    // this must be initialized after the USART is initialized
    // otherwise the JOYPAD and showing char will not work
    EIE3810_TFTLCD_Init();
    JOYPAD_Init();

}

// Draw Welcome Screen
void Draw_Welcome(void) {
    EIE3810_TFTLCD_Clear(WHITE); // Clear screen with white background
    EIE3810_TFTLCD_ShowChar(50, 100, 'W', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(58, 100, 'E', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(66, 100, 'L', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(74, 100, 'C', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(82, 100, 'O', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(90, 100, 'M', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(98, 100, 'E', BLACK, WHITE);
    
    // Show "ECE3080 Project"
    // Using simple char display for now
}

// Draw Difficulty Selection Screen
void Draw_Difficulty(Difficulty diff) {
    EIE3810_TFTLCD_Clear(WHITE);
    
    // Draw Title
    // "SELECT DIFFICULTY"
    EIE3810_TFTLCD_ShowChar(20, 50, 'S', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(28, 50, 'E', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(36, 50, 'L', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(44, 50, 'E', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(52, 50, 'C', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(60, 50, 'T', BLACK, WHITE);
    
    // Colors for selection
    u16 easy_bg = (diff == DIFF_EASY) ? YELLOW : WHITE;
    u16 hard_bg = (diff == DIFF_HARD) ? YELLOW : WHITE;
    
    // Option 1: EASY
    EIE3810_TFTLCD_FillRectangle(50, 100, 150, 130, easy_bg);
    EIE3810_TFTLCD_ShowChar(60, 105, 'E', BLACK, easy_bg);
    EIE3810_TFTLCD_ShowChar(68, 105, 'A', BLACK, easy_bg);
    EIE3810_TFTLCD_ShowChar(76, 105, 'S', BLACK, easy_bg);
    EIE3810_TFTLCD_ShowChar(84, 105, 'Y', BLACK, easy_bg);
    
    // Option 2: HARD
    EIE3810_TFTLCD_FillRectangle(50, 150, 150, 180, hard_bg);
    EIE3810_TFTLCD_ShowChar(60, 155, 'H', BLACK, hard_bg);
    EIE3810_TFTLCD_ShowChar(68, 155, 'A', BLACK, hard_bg);
    EIE3810_TFTLCD_ShowChar(76, 155, 'R', BLACK, hard_bg);
    EIE3810_TFTLCD_ShowChar(84, 155, 'D', BLACK, hard_bg);
    
    // Instruction
    EIE3810_TFTLCD_ShowChar(20, 250, 'U', BLACK, WHITE); // UP
    EIE3810_TFTLCD_ShowChar(28, 250, '/', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(36, 250, 'D', BLACK, WHITE); // DOWN
    EIE3810_TFTLCD_ShowChar(44, 250, 'W', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(52, 250, 'N', BLACK, WHITE);
    
    EIE3810_TFTLCD_ShowChar(20, 270, 'S', BLACK, WHITE); // SEL
    EIE3810_TFTLCD_ShowChar(28, 270, 'E', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(36, 270, 'L', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(44, 270, '-', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(52, 270, '>', BLACK, WHITE);
    EIE3810_TFTLCD_ShowChar(60, 270, 'O', BLACK, WHITE); // OK
    EIE3810_TFTLCD_ShowChar(68, 270, 'K', BLACK, WHITE);
}

int main(void) {
    u8 joy_val;
    u8 prev_joy_val = 0;
    u8 show_wait_msg = 1; // Flag to show wait message once
    
    System_Init();
    
    while(1) {
        switch(current_state) {
            case WELCOME:
                Draw_Welcome();
                // Wait for approx 2 seconds
                Delay(20000000); 
                
                // Auto jump to DIFFICULTY
                current_state = DIFFICULTY;
                // Initial draw for difficulty screen
                Draw_Difficulty(current_difficulty);
                break;
                
            case DIFFICULTY:
                // Read Joypad
                joy_val = JOYPAD_Read();
                
                if (joy_val != prev_joy_val && joy_val != 0) {
                    // Bit 4: UP
                    if (joy_val & 0x10) { 
                        if (current_difficulty == DIFF_HARD) {
                            current_difficulty = DIFF_EASY;
                            Draw_Difficulty(current_difficulty);
                        }
                    }
                    // Bit 5: DOWN
                    else if (joy_val & 0x20) {
                        if (current_difficulty == DIFF_EASY) {
                            current_difficulty = DIFF_HARD;
                            Draw_Difficulty(current_difficulty);
                        }
                    }
                    // Bit 2: SELECT
                    else if (joy_val & 0x04) {
                        current_state = WAIT_SEED;
                        EIE3810_TFTLCD_Clear(WHITE);
                        show_wait_msg = 1; // Reset display flag
                        seed_received = 0; // Reset seed flag
                    }
                    
                    Delay(1000000); // Small debounce delay
                }
                prev_joy_val = joy_val;
                break;
                
            case WAIT_SEED:
                if (show_wait_msg) {
                    // Show "Waiting for PC..."
                    EIE3810_TFTLCD_ShowChar(20, 100, 'W', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(28, 100, 'a', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(36, 100, 'i', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(44, 100, 't', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(52, 100, 'i', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(60, 100, 'n', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(68, 100, 'g', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(76, 100, '.', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(84, 100, '.', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(92, 100, '.', BLACK, WHITE);
                    show_wait_msg = 0;
                }
                
                if (seed_received) {
                    // Show Received Seed
                    EIE3810_TFTLCD_Clear(WHITE);
                    EIE3810_TFTLCD_ShowChar(50, 100, 'S', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(58, 100, 'e', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(66, 100, 'e', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(74, 100, 'd', BLACK, WHITE);
                    EIE3810_TFTLCD_ShowChar(82, 100, ':', BLACK, WHITE);
                    
                    // Show number (0-9 assumption for single digit seed)
                    EIE3810_TFTLCD_ShowChar(98, 100, random_seed + '0', RED, WHITE);
                    
                    Delay(20000000); // Wait for player to see
                    current_state = COUNTDOWN;
                }
                break;
                
            case COUNTDOWN:
            case PLAYING:
            case GAMEOVER:
                // Placeholder for Phase 3
                break;
        }
    }
}

// Interrupt Handlers

void USART1_IRQHandler(void) {
    EIE3810_Debug_Printf(10, 10, BLACK, WHITE, "USART1_IRQHandler");
    // Check if RXNE (Read Data Register Not Empty) flag is set
    if (USART1->SR & (1 << 5)) {
        // Read data (this clears the RXNE bit)
        random_seed = USART1->DR;
        seed_received = 1;
    }
}
