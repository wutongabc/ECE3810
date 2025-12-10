#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_Key.h"
#include "EIE3810_Joyboard.h"
#include "EIE3810_USART.h"
#include "EIE3810_LED.h"
#include "EIE3810_Buzzer.h"
#include <stdlib.h>

// Phase 3 Game Constants
#define MY_BAUD_RATE 4800
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 800
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 10
#define BALL_RADIUS 8
#define PADDLE_Y_TOP 50       // Player B
#define PADDLE_Y_BOTTOM 750   // Player A

// Missing Colors
#ifndef MAGENTA
#define MAGENTA 0xF81F
#endif
#ifndef CYAN
#define CYAN 0x07FF
#endif

// Fixed Point Math Scale
#define FIXED_SCALE 10

// Game States
typedef enum {
    WELCOME,
    DIFFICULTY,
    WAIT_SEED,
    SHOW_SEED, // New state
    COUNTDOWN,
    PLAYING,
    PAUSED, // New PAUSED state
    GAMEOVER
} GameState;

// Difficulty Levels
typedef enum {
    DIFF_EASY,
    DIFF_HARD
} Difficulty;

// Structures
typedef struct {
    s32 x, y;       // Fixed Point (x10)
    s32 vx, vy;     // Fixed Point (x10)
    s16 draw_x, draw_y; // Pixel Coordinate (for rendering)
} Ball;

typedef struct {
    s16 x;
    s16 last_x;     // Pixel Coordinate
} Paddle;

typedef struct {
    s16 x, y, w, h;
    u16 color;
} Obstacle;

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
u8 winner = 0; 
u32 bounce_count = 0;
u32 game_frames = 0; // To calculate time

// Obstacles Array
Obstacle obstacles[3] = {
    // Center (Green)
    { (SCREEN_WIDTH/2 - 25), (SCREEN_HEIGHT/2 - 10), 50, 20, GREEN },
    // Top Left (Magenta)
    { 100, 300, 40, 40, MAGENTA },
    // Bottom Right (Cyan)
    { 340, 500, 40, 40, CYAN }
};

// Helper Functions
void Delay(u32 count) {
    while(count--);
}

void DrawString(u16 x, u16 y, char *str, u16 color, u16 bgcolor) {
    while (*str) {
        EIE3810_TFTLCD_ShowChar(x, y, *str, color, bgcolor);
        x += 8; 
        str++;
    }
}

// System Initialization
void System_Init(void) {
    EIE3810_clock_tree_init();
    
    EIE3810_Key_Init();      
    GPIOE->ODR |= (1 << 2); // Pull-up PE2
    GPIOE->ODR |= (1 << 3); // Pull-up PE3
    GPIOE->ODR |= (1 << 4); // Pull-up PE4
    
    EIE3810_LED_Init();
    EIE3810_Buzzer_Init(); 
    EIE3810_USART1_init(72, MY_BAUD_RATE);
    JOYPAD_Init();
    EIE3810_TFTLCD_Init();
}

// Initialize Game Objects
void Game_Reset(void) {
    // Center Ball (Fixed Point)
    ball.x = (SCREEN_WIDTH / 2) * FIXED_SCALE;
    ball.y = (SCREEN_HEIGHT / 2) * FIXED_SCALE;
    
    // Initial draw position
    ball.draw_x = SCREEN_WIDTH / 2;
    ball.draw_y = SCREEN_HEIGHT / 2;
    
    // Set Speed (Fixed Point: 20 means 2.0 pixels/frame)
    if (current_difficulty == DIFF_EASY) {
        ball.vx = 20; 
        ball.vy = 20; 
    } else {
        ball.vx = 35; // 3.5 pixels
        ball.vy = 35;
    }
    
    // Center Paddles (Pixel Coordinates)
    paddle_top.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    paddle_top.last_x = paddle_top.x;
    
    paddle_bottom.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    paddle_bottom.last_x = paddle_bottom.x;
    
    // Reset Stats
    bounce_count = 0;
    game_frames = 0;
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

// Partial Redraw
void Draw_Game_Update(void) {
    // 1. Erase Old Objects
    
    // Erase Ball (at old draw_x, draw_y)
    EIE3810_TFTLCD_DrawCircle(ball.draw_x, ball.draw_y, BALL_RADIUS, 1, WHITE);
    
    // Erase Paddles
    if (paddle_top.x != paddle_top.last_x) {
        EIE3810_TFTLCD_FillRectangle(paddle_top.last_x, PADDLE_Y_TOP, paddle_top.last_x + PADDLE_WIDTH, PADDLE_Y_TOP + PADDLE_HEIGHT, WHITE);
    }
    if (paddle_bottom.x != paddle_bottom.last_x) {
        EIE3810_TFTLCD_FillRectangle(paddle_bottom.last_x, PADDLE_Y_BOTTOM, paddle_bottom.last_x + PADDLE_WIDTH, PADDLE_Y_BOTTOM + PADDLE_HEIGHT, WHITE);
    }

    // 2. Draw Static Obstacles (Always redraw to handle overlap erasure)
    for(int i=0; i<3; i++) {
        EIE3810_TFTLCD_FillRectangle(obstacles[i].x, obstacles[i].y, 
                                     obstacles[i].x + obstacles[i].w, 
                                     obstacles[i].y + obstacles[i].h, 
                                     obstacles[i].color);
    }

    // 3. Draw New Objects
    
    // Calculate new pixel coordinates
    s16 screen_x = ball.x / FIXED_SCALE;
    s16 screen_y = ball.y / FIXED_SCALE;
    
    EIE3810_TFTLCD_DrawCircle(screen_x, screen_y, BALL_RADIUS, 1, RED);
    
    EIE3810_TFTLCD_FillRectangle(paddle_top.x, PADDLE_Y_TOP, paddle_top.x + PADDLE_WIDTH, PADDLE_Y_TOP + PADDLE_HEIGHT, BLUE);
    EIE3810_TFTLCD_FillRectangle(paddle_bottom.x, PADDLE_Y_BOTTOM, paddle_bottom.x + PADDLE_WIDTH, PADDLE_Y_BOTTOM + PADDLE_HEIGHT, BLUE);

    // 4. Update History
    ball.draw_x = screen_x;
    ball.draw_y = screen_y;
    paddle_top.last_x = paddle_top.x;
    paddle_bottom.last_x = paddle_bottom.x;
}


int main(void) {
    u8 joy_val;
    u8 prev_joy_val = 0;
    u8 show_wait_msg = 1; 
    
    System_Init();
    
    // Temp variables for loop
    s16 ball_px = 0, ball_py = 0;
    u8 hi = 0, lo = 0;
    char c_hi = 0, c_lo = 0;
    u32 seed_counter = 0; // Free running counter for pseudo-random

    while(1) {
        seed_counter++; // Increment every loop iteration
        switch(current_state) {
            case WELCOME:
                Draw_Welcome();
                Delay(20000000); 
                current_state = DIFFICULTY;
                Draw_Difficulty(current_difficulty);
                break;
                
            case DIFFICULTY:
                joy_val = JOYPAD_Read();
                if (joy_val != prev_joy_val && joy_val != 0) {
                    if (joy_val & 0x10) { 
                        if (current_difficulty == DIFF_HARD) {
                            current_difficulty = DIFF_EASY;
                            Draw_Difficulty(current_difficulty);
                        }
                    }
                    else if (joy_val & 0x20) { 
                        if (current_difficulty == DIFF_EASY) {
                            current_difficulty = DIFF_HARD;
                            Draw_Difficulty(current_difficulty);
                        }
                    }
                    else if (joy_val & 0x04) { 
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
                if (USART1->SR & (1 << 5)) { 
                    random_seed = USART1->DR;
                    seed_received = 1;
                }
                if (seed_received) {
                    // Go to SHOW_SEED instead of COUNTDOWN
                    EIE3810_TFTLCD_Clear(WHITE);
                    current_state = SHOW_SEED;
                }
                break;
            
            case SHOW_SEED:
                // Generate Pseudo-Random Seed based on user timing
                random_seed = seed_counter % 8;

                DrawString(160, 350, "SEED RECEIVED:", BLACK, WHITE);
                
                // Display the seed value (single digit or hex)
                // Assuming seed is 0-7 as per handout, or just raw hex
                // Displaying raw hex for debug clarity
                hi = 0; // High nibble is always 0 for 0-7
                lo = random_seed;
                c_hi = '0';
                c_lo = (lo < 10) ? (lo + '0') : (lo - 10 + 'A');
                
                EIE3810_TFTLCD_ShowChar(280, 350, '0', RED, WHITE);
                EIE3810_TFTLCD_ShowChar(288, 350, 'x', RED, WHITE);
                EIE3810_TFTLCD_ShowChar(296, 350, c_hi, RED, WHITE);
                EIE3810_TFTLCD_ShowChar(304, 350, c_lo, RED, WHITE);
                
                Delay(40000000); // Wait 2-3 seconds for user to see
                
                EIE3810_TFTLCD_Clear(WHITE);
                current_state = COUNTDOWN;
                break;
                
            case COUNTDOWN:
                Game_Reset(); 
                DrawString(230, 400, "3", RED, WHITE); Delay(10000000);
                DrawString(230, 400, "2", RED, WHITE); Delay(10000000);
                DrawString(230, 400, "1", RED, WHITE); Delay(10000000);
                DrawString(230, 400, "GO!", RED, WHITE); Delay(10000000);
                EIE3810_TFTLCD_Clear(WHITE); 
                current_state = PLAYING;
                break;
                
            case PLAYING:
                // --- PAUSE CHECK (Key1 - PE3) ---
                if ((GPIOE->IDR & (1<<3)) == 0) { // Key1 Pressed
                    current_state = PAUSED;
                    // Debounce wait (wait for release)
                    while((GPIOE->IDR & (1<<3)) == 0); 
                    Delay(5000000); 
                    break; // Exit PLAYING case immediately
                }

                // --- INPUT HANDLING ---
                if ((GPIOE->IDR & (1<<2)) == 0) paddle_bottom.x -= 3;
                if ((GPIOE->IDR & (1<<4)) == 0) paddle_bottom.x += 3;
                
                joy_val = JOYPAD_Read();
                if (joy_val & 0x40) paddle_top.x -= 3;
                if (joy_val & 0x80) paddle_top.x += 3;
                
                // Clamp
                if (paddle_bottom.x < 0) paddle_bottom.x = 0;
                if (paddle_bottom.x > SCREEN_WIDTH - PADDLE_WIDTH) paddle_bottom.x = SCREEN_WIDTH - PADDLE_WIDTH;
                if (paddle_top.x < 0) paddle_top.x = 0;
                if (paddle_top.x > SCREEN_WIDTH - PADDLE_WIDTH) paddle_top.x = SCREEN_WIDTH - PADDLE_WIDTH;
                
                // --- PHYSICS UPDATE (Fixed Point) ---
                ball.x += ball.vx;
                ball.y += ball.vy;
                
                // Convert to Pixel Coords for Collision
                ball_px = ball.x / FIXED_SCALE;
                ball_py = ball.y / FIXED_SCALE;
                
                // Wall Collisions
                if (ball_px <= BALL_RADIUS || ball_px >= SCREEN_WIDTH - BALL_RADIUS) {
                    ball.vx = -ball.vx;
                    bounce_count++; // Count Wall Bounce
                    // Push out
                    if (ball_px <= BALL_RADIUS) ball.x = (BALL_RADIUS + 1) * FIXED_SCALE;
                    else ball.x = (SCREEN_WIDTH - BALL_RADIUS - 1) * FIXED_SCALE;
                }
                
                // Obstacle Collisions
                for(int i=0; i<3; i++) {
                    if (ball_px + BALL_RADIUS >= obstacles[i].x && ball_px - BALL_RADIUS <= obstacles[i].x + obstacles[i].w &&
                        ball_py + BALL_RADIUS >= obstacles[i].y && ball_py - BALL_RADIUS <= obstacles[i].y + obstacles[i].h) {
                        
                        // Bounce Y
                        ball.vy = -ball.vy; 
                        bounce_count++; // Count Obstacle Bounce
                        
                        // Push out (Simplified vertical push)
                        if (ball.vy > 0) ball.y = (obstacles[i].y + obstacles[i].h + BALL_RADIUS + 1) * FIXED_SCALE;
                        else ball.y = (obstacles[i].y - BALL_RADIUS - 1) * FIXED_SCALE;
                        
                        Delay(20000); 
                    }
                }

                // Paddle Collisions (Top)
                if (ball_py - BALL_RADIUS <= PADDLE_Y_TOP + PADDLE_HEIGHT && ball_py - BALL_RADIUS >= PADDLE_Y_TOP) {
                    if (ball_px >= paddle_top.x && ball_px <= paddle_top.x + PADDLE_WIDTH) {
                        bounce_count++; // Count Paddle Bounce
                        
                        // Speed Up (Add 2 = 0.2 pixels)
                        if (abs(ball.vy) < 120) { // Max 12.0
                            if (ball.vy > 0) ball.vy += 2;
                            else ball.vy -= 2;
                        }
                        ball.vy = abs(ball.vy); 
                        
                        // Simple Bounce (Keep horizontal speed)
                        // Optional: Add small random jitter to prevent infinite loops
                        // ball.vx += (random_seed % 3) - 1;

                        // Move out of collision
                        ball.y = (PADDLE_Y_TOP + PADDLE_HEIGHT + BALL_RADIUS + 1) * FIXED_SCALE; 
                        Delay(50000); 
                    }
                }
                
                // Paddle Collisions (Bottom)
                if (ball_py + BALL_RADIUS >= PADDLE_Y_BOTTOM && ball_py + BALL_RADIUS <= PADDLE_Y_BOTTOM + PADDLE_HEIGHT) {
                    if (ball_px >= paddle_bottom.x && ball_px <= paddle_bottom.x + PADDLE_WIDTH) {
                        bounce_count++; // Count Paddle Bounce
                        
                        // Speed Up
                        if (abs(ball.vy) < 120) {
                            if (ball.vy > 0) ball.vy += 2;
                            else ball.vy -= 2;
                        }
                        ball.vy = -abs(ball.vy);
                        
                        // Simple Bounce
                        
                        // Move out of collision
                        ball.y = (PADDLE_Y_BOTTOM - BALL_RADIUS - 1) * FIXED_SCALE;
                        Delay(50000); 
                    }
                }
                
                // Win/Loss
                if (ball_py < 0) { winner = 1; current_state = GAMEOVER; }
                if (ball_py > SCREEN_HEIGHT) { winner = 2; current_state = GAMEOVER; }
                
                Draw_Game_Update();
                game_frames++;
                Delay(50000); 
                break;
                
            case PAUSED:
                DrawString(200, 400, "PAUSED", RED, WHITE);
                
                // Wait for Key1 to resume
                if ((GPIOE->IDR & (1<<3)) == 0) { // Key1 Pressed
                    // Clear "PAUSED" text (overwrite with white)
                    DrawString(200, 400, "PAUSED", WHITE, WHITE); 
                    
                    // Force a full redraw of game objects to ensure clean state
                    // (Optional, but good practice if text overlapped objects)
                    Draw_Game_Update();
                    
                    current_state = PLAYING;
                    
                    // Debounce (wait for release)
                    while((GPIOE->IDR & (1<<3)) == 0);
                    Delay(5000000);
                }
                break;
                
            case GAMEOVER:
            {
                EIE3810_TFTLCD_Clear(WHITE);
                if (winner == 1) DrawString(140, 300, "PLAYER A (BOTTOM) WINS!", RED, WHITE);
                else DrawString(160, 300, "PLAYER B (TOP) WINS!", BLUE, WHITE);
                
                // Display Stats
                DrawString(160, 350, "BOUNCES:", BLACK, WHITE);
                u32 tmp = bounce_count;
                u16 dx = 240;
                if (tmp == 0) EIE3810_TFTLCD_ShowChar(dx, 350, '0', BLACK, WHITE);
                else {
                    char buf[10]; int k=0;
                    while(tmp > 0) { buf[k++] = (tmp%10)+'0'; tmp/=10; }
                    while(k > 0) { EIE3810_TFTLCD_ShowChar(dx, 350, buf[--k], BLACK, WHITE); dx+=8; }
                }

                DrawString(160, 380, "TIME(s):", BLACK, WHITE);
                tmp = game_frames / 40; // Approx seconds
                dx = 240;
                if (tmp == 0) EIE3810_TFTLCD_ShowChar(dx, 380, '0', BLACK, WHITE);
                else {
                    char buf[10]; int k=0;
                    while(tmp > 0) { buf[k++] = (tmp%10)+'0'; tmp/=10; }
                    while(k > 0) { EIE3810_TFTLCD_ShowChar(dx, 380, buf[--k], BLACK, WHITE); dx+=8; }
                }

                DrawString(160, 500, "PRESS KEY0 TO RESTART", BLACK, WHITE);
                Delay(10000000); 
                while(1) {
                    if ((GPIOE->IDR & (1<<4)) == 0) { current_state = WELCOME; break; }
                    if (JOYPAD_Read() & 0x04) { current_state = WELCOME; break; }
                }
                break;
            }
        }
    }
}
