/*
 * EIE3810_Debug 使用示例
 *
 * 这个文件展示了如何使用 Debug 模块在LCD上显示各种调试信息
 */

#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_Debug.h"
#include "EIE3810_LED.h"

void Delay(u32 nCount);

int main(void)
{
    u32 counter = 0;
    float temperature = 25.6;

    // 初始化系统
    EIE3810_clock_tree_init();
    EIE3810_LED_Init();
    EIE3810_TFTLCD_Init();
    EIE3810_TFTLCD_Clear(WHITE);

    // ===== 示例 1: 显示字符串 =====
    EIE3810_Debug_ShowString(10, 10, "Debug Test", BLACK, WHITE);

    // ===== 示例 2: 显示十进制数字 =====
    // 参数: (x, y, 数值, 显示位数, 字体颜色, 背景色)
    EIE3810_Debug_ShowString(10, 40, "Decimal:", BLACK, WHITE);
    EIE3810_Debug_ShowNum(90, 40, 12345, 5, BLUE, WHITE);

    // ===== 示例 3: 显示十六进制数字 =====
    EIE3810_Debug_ShowString(10, 70, "Hex:", BLACK, WHITE);
    EIE3810_Debug_ShowHex(60, 70, 0xABCD, 4, RED, WHITE);

    // ===== 示例 4: 显示二进制数字 =====
    EIE3810_Debug_ShowString(10, 100, "Binary:", BLACK, WHITE);
    EIE3810_Debug_ShowBinary(80, 100, 0b10101010, 8, GREEN, WHITE);

    // ===== 示例 5: 显示浮点数 =====
    EIE3810_Debug_ShowString(10, 130, "Float:", BLACK, WHITE);
    EIE3810_Debug_ShowFloat(70, 130, 3.14159, 3, BLUE, WHITE);

    // ===== 示例 6: 使用 Printf 格式化输出 =====
    EIE3810_Debug_Printf(10, 160, BLACK, WHITE, "Temp: %.1f C", temperature);

    // ===== 示例 7: 动态计数器 =====
    EIE3810_Debug_ShowString(10, 200, "Counter:", BLACK, WHITE);

    while (1)
    {
        // 清除旧的数值区域
        EIE3810_Debug_Clear_Area(90, 200, 200, 215, WHITE);

        // 显示新的计数值
        EIE3810_Debug_ShowNum(90, 200, counter, 6, RED, WHITE);

        // 也可以用 Printf 方式
        EIE3810_Debug_Clear_Area(10, 230, 150, 245, WHITE);
        EIE3810_Debug_Printf(10, 230, BLUE, WHITE, "0x%04X", counter);

        counter++;
        if (counter > 999999)
            counter = 0;

        // 延时
        Delay(1000000);
    }
}

void Delay(u32 nCount)
{
    for (; nCount != 0; nCount--)
        ;
}
