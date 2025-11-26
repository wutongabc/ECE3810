#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_Key.h"
#include "EIE3810_Joyboard.h"
#include "EIE3810_USART.h"
#include "EIE3810_LED.h"
#include "EIE3810_Buzzer.h"

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
    EIE3810_TFTLCD_Init();
    EIE3810_Key_init();      // Note: lowercase 'i' based on header
    JOYPAD_Init();
    EIE3810_LED_Init();
    EIE3810_Buzzer_init();   // Note: lowercase 'i' based on header
    
    // USART init will be done in Phase 2
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
    
    System_Init();
    
    while(1) {
        switch(current_state) {
            case WELCOME:
                Draw_Welcome();
                // Wait for approx 2 seconds
                // Adjust delay count based on clock speed (approximate)
                Delay(20000000); 
                
                // Auto jump to DIFFICULTY
                current_state = DIFFICULTY;
                // Initial draw for difficulty screen
                Draw_Difficulty(current_difficulty);
                break;
                
            case DIFFICULTY:
                // Read Joypad
                joy_val = JOYPAD_Read();
                
                // Simple debounce logic could be added here or inside read
                // For now, using state change detection to avoid rapid toggling
                
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
                        // Show "Enter Phase 2"
                        EIE3810_TFTLCD_ShowChar(50, 100, 'P', BLACK, WHITE);
                        EIE3810_TFTLCD_ShowChar(58, 100, 'H', BLACK, WHITE);
                        EIE3810_TFTLCD_ShowChar(66, 100, 'A', BLACK, WHITE);
                        EIE3810_TFTLCD_ShowChar(74, 100, 'S', BLACK, WHITE);
                        EIE3810_TFTLCD_ShowChar(82, 100, 'E', BLACK, WHITE);
                        EIE3810_TFTLCD_ShowChar(98, 100, '2', BLACK, WHITE);
                    }
                    
                    Delay(1000000); // Small debounce delay
                }
                prev_joy_val = joy_val;
                break;
                
            case WAIT_SEED:
                // Waiting for implementation in Phase 2
                // Just loop here for now
                Delay(100000);
                break;
                
            case COUNTDOWN:
            case PLAYING:
            case GAMEOVER:
                // Placeholder for future states
                break;
        }
    }
}

