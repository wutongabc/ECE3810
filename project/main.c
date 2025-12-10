#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_Key.h"
#include "EIE3810_Joyboard.h"
#include "EIE3810_USART.h"
#include "EIE3810_LED.h"
#include "EIE3810_Buzzer.h"
#include <stdlib.h> // For abs()

// Phase 3 Game Constants
#define MY_BAUD_RATE 4800
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 800
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 10
#define BALL_RADIUS 8
#define PADDLE_Y_TOP 50       // Player B
#define PADDLE_Y_BOTTOM 750   // Player A

// Obstacle Constants
#define OBSTACLE_WIDTH 50
#define OBSTACLE_HEIGHT 20
#define OBSTACLE_X (SCREEN_WIDTH / 2 - OBSTACLE_WIDTH / 2)
#define OBSTACLE_Y (SCREEN_HEIGHT / 2 - OBSTACLE_HEIGHT / 2)

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

// Structures
typedef struct {
    s16 x, y;
    s16 vx, vy;
    s16 last_x, last_y; // For partial redraw
} Ball;

typedef struct {
    s16 x;
    s16 last_x;         // For partial redraw
} Paddle;

// Global Variables
GameState current_state = WELCOME;
Difficulty current_difficulty = DIFF_EASY;

// Phase 2 Globals
volatile u8 random_seed = 0;
volatile u8 seed_received = 0;

// Phase 3 Game Objects
Ball ball;
Paddle paddle_top;    // Player B
Paddle paddle_bottom; // Player A
u8 winner = 0; // 0: None, 1: Player A (Bottom), 2: Player B (Top)


// Helper Functions

void Delay(u32 count) {
    while(count--);
}

// Draw a simple string (helper wrapper)
void DrawString(u16 x, u16 y, char *str, u16 color, u16 bgcolor) {
    while (*str) {
        EIE3810_TFTLCD_ShowChar(x, y, *str, color, bgcolor);
        x += 8; 
        str++;
    }
}

// System Initialization
void System_Init(void) {
    // Initialize System Clock to 72MHz
    EIE3810_clock_tree_init();
    
    // Initialize Peripherals
    EIE3810_Key_Init();      // Note: Uppercase 'I' standard
    
    // FORCE PULL-UP for Keys (Fix for buttons not working)
    GPIOE->ODR |= (1 << 2); // Pull-up for KEY2 (PE2)
    GPIOE->ODR |= (1 << 4); // Pull-up for KEY0 (PE4)
    
    EIE3810_LED_Init();
    EIE3810_Buzzer_Init();   // Note: Uppercase 'I' standard
    EIE3810_USART1_init(72, MY_BAUD_RATE);
    JOYPAD_Init();
    EIE3810_TFTLCD_Init();
}

// Initialize Game Objects for a new round
void Game_Reset(void) {
    // Center Ball
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.last_x = ball.x;
    ball.last_y = ball.y;
    
    // Set Speed based on Difficulty
    if (current_difficulty == DIFF_EASY) {
        ball.vx = 1;
        ball.vy = 1; // Initially moves down-right
    } else {
        ball.vx = 2;
        ball.vy = 2;
    }
    
    // Center Paddles
    paddle_top.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    paddle_top.last_x = paddle_top.x;
    
    paddle_bottom.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    paddle_bottom.last_x = paddle_bottom.x;
}

// Draw Functions
void Draw_Welcome(void) {
    EIE3810_TFTLCD_Clear(WHITE); 
    DrawString(180, 350, "WELCOME", BLACK, WHITE);
    DrawString(160, 380, "ECE3080 PROJECT", BLUE, WHITE);
}

void Draw_Difficulty(Difficulty diff) {
    EIE3810_TFTLCD_Clear(WHITE);
    DrawString(160, 200, "SELECT DIFFICULTY", BLACK, WHITE);
    
    u16 easy_bg = (diff == DIFF_EASY) ? YELLOW : WHITE;
    u16 hard_bg = (diff == DIFF_HARD) ? YELLOW : WHITE;
    
    EIE3810_TFTLCD_FillRectangle(140, 300, 340, 350, easy_bg);
    DrawString(220, 315, "EASY", BLACK, easy_bg);
    
    EIE3810_TFTLCD_FillRectangle(140, 400, 340, 450, hard_bg);
    DrawString(220, 415, "HARD", BLACK, hard_bg);
    
    DrawString(140, 600, "UP/DOWN TO MOVE", BLACK, WHITE);
    DrawString(140, 620, "SEL TO CONFIRM", BLACK, WHITE);
}

// Partial Redraw for Game Loop
void Draw_Game_Update(void) {
    // 1. Erase Old Objects (Draw with Background Color WHITE)
    
    // Erase Old Ball
    EIE3810_TFTLCD_DrawCircle(ball.last_x, ball.last_y, BALL_RADIUS, 1, WHITE);
    
    // Erase Old Paddles
    if (paddle_top.x != paddle_top.last_x) {
        EIE3810_TFTLCD_FillRectangle(paddle_top.last_x, PADDLE_Y_TOP, paddle_top.last_x + PADDLE_WIDTH, PADDLE_Y_TOP + PADDLE_HEIGHT, WHITE);
    }
    if (paddle_bottom.x != paddle_bottom.last_x) {
        EIE3810_TFTLCD_FillRectangle(paddle_bottom.last_x, PADDLE_Y_BOTTOM, paddle_bottom.last_x + PADDLE_WIDTH, PADDLE_Y_BOTTOM + PADDLE_HEIGHT, WHITE);
    }

    // 2. Draw Obstacle (Static, redrawing ensures it's not erased by ball passing through)
    EIE3810_TFTLCD_FillRectangle(OBSTACLE_X, OBSTACLE_Y, OBSTACLE_X + OBSTACLE_WIDTH, OBSTACLE_Y + OBSTACLE_HEIGHT, GREEN);

    // 3. Draw New Objects
    
    // Draw New Ball
    EIE3810_TFTLCD_DrawCircle(ball.x, ball.y, BALL_RADIUS, 1, RED);
    
    // Draw New Paddles
    EIE3810_TFTLCD_FillRectangle(paddle_top.x, PADDLE_Y_TOP, paddle_top.x + PADDLE_WIDTH, PADDLE_Y_TOP + PADDLE_HEIGHT, BLUE);
    EIE3810_TFTLCD_FillRectangle(paddle_bottom.x, PADDLE_Y_BOTTOM, paddle_bottom.x + PADDLE_WIDTH, PADDLE_Y_BOTTOM + PADDLE_HEIGHT, BLUE);

    // 4. Update Last Positions
    ball.last_x = ball.x;
    ball.last_y = ball.y;
    paddle_top.last_x = paddle_top.x;
    paddle_bottom.last_x = paddle_bottom.x;
}


int main(void) {
    u8 joy_val;
    u8 prev_joy_val = 0;
    u8 show_wait_msg = 1; 
    
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
                joy_val = JOYPAD_Read();
                if (joy_val != prev_joy_val && joy_val != 0) {
                    if (joy_val & 0x10) { // UP
                        if (current_difficulty == DIFF_HARD) {
                            current_difficulty = DIFF_EASY;
                            Draw_Difficulty(current_difficulty);
                        }
                    }
                    else if (joy_val & 0x20) { // DOWN
                        if (current_difficulty == DIFF_EASY) {
                            current_difficulty = DIFF_HARD;
                            Draw_Difficulty(current_difficulty);
                        }
                    }
                    else if (joy_val & 0x04) { // SELECT
                        current_state = WAIT_SEED;
                        EIE3810_TFTLCD_Clear(WHITE);
                        show_wait_msg = 1; 
                        seed_received = 0; 
                    }
                    Delay(1000000); 
                }
                prev_joy_val = joy_val;
                break;
                
            case WAIT_SEED:
                if (show_wait_msg) {
                    DrawString(180, 400, "WAITING FOR PC...", BLACK, WHITE);
                    show_wait_msg = 0;
                }
                // Polling Check
                if (USART1->SR & (1 << 5)) { 
                    random_seed = USART1->DR;
                    seed_received = 1;
                }
                
                if (seed_received) {
                    // Just clear and go, ignoring value display for speed
                    EIE3810_TFTLCD_Clear(WHITE);
                    current_state = COUNTDOWN;
                }
                break;
                
            case COUNTDOWN:
                Game_Reset(); // Init physics
                
                DrawString(230, 400, "3", RED, WHITE);
                Delay(10000000);
                DrawString(230, 400, "2", RED, WHITE);
                Delay(10000000);
                DrawString(230, 400, "1", RED, WHITE);
                Delay(10000000);
                DrawString(230, 400, "GO!", RED, WHITE);
                Delay(10000000);
                
                EIE3810_TFTLCD_Clear(WHITE); // Clean slate for game
                current_state = PLAYING;
                break;
                
            case PLAYING:
                // --- INPUT HANDLING ---
                
                // Player A (Bottom) - Board Keys
                
                if ((GPIOE->IDR & (1<<2)) == 0) { // Key2 Pressed (Left)
                    paddle_bottom.x -= 3;
                }
                if ((GPIOE->IDR & (1<<4)) == 0) { // Key0 Pressed (Right)
                    paddle_bottom.x += 3;
                }
                
                // Player B (Top) - Joypad
                joy_val = JOYPAD_Read();
                if (joy_val & 0x40) { // LEFT (Bit 6)
                    paddle_top.x -= 3;
                }
                if (joy_val & 0x80) { // RIGHT (Bit 7)
                    paddle_top.x += 3;
                }
                
                // Boundary Checks for Paddles
                if (paddle_bottom.x < 0) paddle_bottom.x = 0;
                if (paddle_bottom.x > SCREEN_WIDTH - PADDLE_WIDTH) paddle_bottom.x = SCREEN_WIDTH - PADDLE_WIDTH;
                
                if (paddle_top.x < 0) paddle_top.x = 0;
                if (paddle_top.x > SCREEN_WIDTH - PADDLE_WIDTH) paddle_top.x = SCREEN_WIDTH - PADDLE_WIDTH;
                
                // --- PHYSICS UPDATE ---
                
                ball.x += ball.vx;
                ball.y += ball.vy;
                
                // Wall Collisions (Left/Right)
                if (ball.x <= BALL_RADIUS || ball.x >= SCREEN_WIDTH - BALL_RADIUS) {
                    ball.vx = -ball.vx;
                }
                
                // --- OBSTACLE COLLISION ---
                if (ball.x + BALL_RADIUS >= OBSTACLE_X && ball.x - BALL_RADIUS <= OBSTACLE_X + OBSTACLE_WIDTH &&
                    ball.y + BALL_RADIUS >= OBSTACLE_Y && ball.y - BALL_RADIUS <= OBSTACLE_Y + OBSTACLE_HEIGHT) {
                    
                    // Simple collision: reverse Y (most hits will be vertical)
                    // For better physics, could check overlap amount
                    ball.vy = -ball.vy; 
                    
                    // Push out to prevent sticking
                    if (ball.vy > 0) ball.y = OBSTACLE_Y + OBSTACLE_HEIGHT + BALL_RADIUS + 1;
                    else ball.y = OBSTACLE_Y - BALL_RADIUS - 1;

                    // Optional: Buzzer
                    // EIE3810_Buzzer_On();
                    Delay(20000); 
                    // EIE3810_Buzzer_Off();
                }

                // --- PADDLE COLLISIONS (With Curve & Speed Up) ---
                
                // Top Paddle (Player B)
                if (ball.y - BALL_RADIUS <= PADDLE_Y_TOP + PADDLE_HEIGHT && ball.y - BALL_RADIUS >= PADDLE_Y_TOP) {
                    if (ball.x >= paddle_top.x && ball.x <= paddle_top.x + PADDLE_WIDTH) {
                        
                        // 1. Dynamic Speed Up
                        if (abs(ball.vy) < 12) { // Max speed limit
                            if (ball.vy > 0) ball.vy += 0.2;
                            else ball.vy--;
                        }
                        
                        ball.vy = abs(ball.vy); // Ensure moving DOWN (positive Y)
                        
                        // 2. Curve Ball (Change VX based on hit position)
                        s16 diff = ball.x - (paddle_top.x + PADDLE_WIDTH / 2);
                        ball.vx = diff / 4; // Center=0, Edge=+/-12 (Adjust divisor 4 for sensitivity)
                        
                        // Prevent stuck horizontal bounce
                        if (ball.vx == 0) ball.vx = (random_seed % 2 == 0) ? 1 : -1;

                        // Move out of collision
                        ball.y = PADDLE_Y_TOP + PADDLE_HEIGHT + BALL_RADIUS + 1; 
                        
                        // EIE3810_Buzzer_On();
                        Delay(50000); 
                        // EIE3810_Buzzer_Off();
                    }
                }
                
                // Bottom Paddle (Player A)
                if (ball.y + BALL_RADIUS >= PADDLE_Y_BOTTOM && ball.y + BALL_RADIUS <= PADDLE_Y_BOTTOM + PADDLE_HEIGHT) {
                    if (ball.x >= paddle_bottom.x && ball.x <= paddle_bottom.x + PADDLE_WIDTH) {
                        
                        // 1. Dynamic Speed Up
                        if (abs(ball.vy) < 12) {
                            if (ball.vy > 0) ball.vy += 0.2;
                            else ball.vy--;
                        }

                        ball.vy = -abs(ball.vy); // Ensure moving UP (negative Y)
                        
                        // 2. Curve Ball
                        s16 diff = ball.x - (paddle_bottom.x + PADDLE_WIDTH / 2);
                        ball.vx = diff / 4;

                        // Prevent stuck horizontal bounce
                        if (ball.vx == 0) ball.vx = (random_seed % 2 == 0) ? 1 : -1;

                        // Move out of collision
                        ball.y = PADDLE_Y_BOTTOM - BALL_RADIUS - 1;
                        
                        // EIE3810_Buzzer_On();
                        Delay(50000); 
                        // EIE3810_Buzzer_Off();
                    }
                }
                
                // Win/Loss Condition
                if (ball.y < 0) {
                    // Top boundary crossed -> Bottom Player A Wins
                    winner = 1;
                    current_state = GAMEOVER;
                }
                if (ball.y > SCREEN_HEIGHT) {
                    // Bottom boundary crossed -> Top Player B Wins
                    winner = 2;
                    current_state = GAMEOVER;
                }
                
                // --- RENDER ---
                Draw_Game_Update();
                
                // Frame Delay (adjust for smooth 30-60fps)
                Delay(50000); 
                break;
                
            case GAMEOVER:
                EIE3810_TFTLCD_Clear(WHITE);
                if (winner == 1) {
                    DrawString(180, 400, "PLAYER A (BOTTOM) WINS!", RED, WHITE);
                } else {
                    DrawString(180, 400, "PLAYER B (TOP) WINS!", BLUE, WHITE);
                }
                
                DrawString(180, 450, "PRESS KEY0 TO RESTART", BLACK, WHITE);
                
                // Wait for restart
                // Simple debounce wait first
                Delay(10000000); 
                
                while(1) {
                    // Poll Key0 (PE4)
                    if ((GPIOE->IDR & (1<<4)) == 0) {
                        current_state = WELCOME;
                        break;
                    }
                    // Or Joypad Select
                    if (JOYPAD_Read() & 0x04) {
                        current_state = WELCOME;
                        break;
                    }
                }
                break;
        }
    }
}
