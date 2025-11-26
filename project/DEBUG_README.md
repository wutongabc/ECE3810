# EIE3810 Debug Module 使用说明

## 简介
这是一个用于在LCD屏幕上显示调试信息的模块，支持显示各种格式的数据。

## 文件结构
- `EIE3810_Debug.h` - 头文件，包含函数声明
- `EIE3810_Debug.c` - 实现文件，包含所有函数实现
- `debug_example.c` - 使用示例

## 依赖
需要先包含以下模块：
- `EIE3810_TFTLCD.h` - LCD驱动模块
- `stm32f10x.h` - STM32标准库

## 功能列表

### 1. 显示字符
```c
void EIE3810_Debug_ShowChar(u16 x, u16 y, char c, u16 color, u16 bgcolor);
```
**示例：**
```c
EIE3810_Debug_ShowChar(10, 10, 'A', BLACK, WHITE);
```

### 2. 显示字符串
```c
void EIE3810_Debug_ShowString(u16 x, u16 y, const char *str, u16 color, u16 bgcolor);
```
**示例：**
```c
EIE3810_Debug_ShowString(10, 10, "Hello World", BLACK, WHITE);
EIE3810_Debug_ShowString(10, 40, "Line1\nLine2", BLUE, WHITE); // 支持换行
```

### 3. 显示十进制数字
```c
void EIE3810_Debug_ShowNum(u16 x, u16 y, u32 num, u8 len, u16 color, u16 bgcolor);
```
**参数：**
- `num` - 要显示的数字
- `len` - 显示的位数（会自动补零）

**示例：**
```c
EIE3810_Debug_ShowNum(10, 10, 123, 5, BLACK, WHITE);    // 显示 "00123"
EIE3810_Debug_ShowNum(10, 40, 9999, 4, RED, WHITE);     // 显示 "9999"
```

### 4. 显示十六进制数字
```c
void EIE3810_Debug_ShowHex(u16 x, u16 y, u32 num, u8 len, u16 color, u16 bgcolor);
```
**示例：**
```c
EIE3810_Debug_ShowHex(10, 10, 0xFF, 2, BLACK, WHITE);      // 显示 "FF"
EIE3810_Debug_ShowHex(10, 40, 0xABCD, 4, RED, WHITE);      // 显示 "ABCD"
```

### 5. 显示二进制数字
```c
void EIE3810_Debug_ShowBinary(u16 x, u16 y, u32 num, u8 bits, u16 color, u16 bgcolor);
```
**示例：**
```c
EIE3810_Debug_ShowBinary(10, 10, 0b1010, 4, BLACK, WHITE);     // 显示 "1010"
EIE3810_Debug_ShowBinary(10, 40, 0xFF, 8, RED, WHITE);         // 显示 "11111111"
```

### 6. 显示浮点数
```c
void EIE3810_Debug_ShowFloat(u16 x, u16 y, float num, u8 precision, u16 color, u16 bgcolor);
```
**参数：**
- `precision` - 小数点后的位数

**示例：**
```c
EIE3810_Debug_ShowFloat(10, 10, 3.14159, 2, BLACK, WHITE);  // 显示 "3.14"
EIE3810_Debug_ShowFloat(10, 40, -25.678, 3, RED, WHITE);    // 显示 "-25.678"
```

### 7. Printf风格格式化输出
```c
void EIE3810_Debug_Printf(u16 x, u16 y, u16 color, u16 bgcolor, const char *format, ...);
```
**示例：**
```c
int count = 42;
float temp = 25.5;
EIE3810_Debug_Printf(10, 10, BLACK, WHITE, "Count: %d", count);
EIE3810_Debug_Printf(10, 40, BLUE, WHITE, "Temp: %.1f C", temp);
EIE3810_Debug_Printf(10, 70, RED, WHITE, "Hex: 0x%04X", 0xABCD);
```

### 8. 清除一行
```c
void EIE3810_Debug_Clear_Line(u16 y, u16 color);
```
**示例：**
```c
EIE3810_Debug_Clear_Line(100, WHITE);  // 清除Y=100这一行
```

### 9. 清除指定区域
```c
void EIE3810_Debug_Clear_Area(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
```
**示例：**
```c
EIE3810_Debug_Clear_Area(10, 10, 200, 50, WHITE);  // 清除矩形区域
```

## 颜色常量
可用的颜色（在 EIE3810_TFTLCD.h 中定义）：
- `BLACK` - 黑色
- `WHITE` - 白色
- `RED` - 红色
- `GREEN` - 绿色
- `BLUE` - 蓝色
- `YELLOW` - 黄色

## 完整使用示例

```c
#include "stm32f10x.h"
#include "EIE3810_TFTLCD.h"
#include "EIE3810_Debug.h"

int main(void)
{
    u32 counter = 0;
    
    // 初始化
    EIE3810_clock_tree_init();
    EIE3810_TFTLCD_Init();
    EIE3810_TFTLCD_Clear(WHITE);
    
    // 显示标题
    EIE3810_Debug_ShowString(10, 10, "Debug Demo", BLACK, WHITE);
    
    // 主循环
    while (1)
    {
        // 清除旧数据
        EIE3810_Debug_Clear_Area(10, 50, 200, 80, WHITE);
        
        // 显示计数器（十进制）
        EIE3810_Debug_Printf(10, 50, BLUE, WHITE, "Dec: %d", counter);
        
        // 显示计数器（十六进制）
        EIE3810_Debug_Printf(10, 70, RED, WHITE, "Hex: 0x%04X", counter);
        
        counter++;
        if (counter > 9999) counter = 0;
        
        Delay(1000000);
    }
}
```

## 在 exp5.c 中的使用

exp5.c 已经集成了 Debug 模块：

```c
#include "EIE3810_Debug.h"

// 在 JOYPAD_Display_Key 函数中使用
void Display_Hex(u16 x, u16 y, u8 value)
{
    EIE3810_Debug_ShowHex(x, y, value, 2, RED, WHITE);
}
```

## 注意事项

1. **坐标系统**：
   - x: 0-479 (水平，从左到右)
   - y: 0-799 (垂直，从上到下)

2. **字符间距**：
   - 每个字符宽度：8像素
   - 每个字符高度：16像素

3. **更新显示**：
   - 在更新数值前，建议先清除旧的显示区域
   - 使用 `EIE3810_Debug_Clear_Area()` 清除指定区域

4. **性能考虑**：
   - LCD写入相对较慢，避免在高频中断中频繁更新
   - 建议在主循环或低频中断中更新显示

## 编译配置

确保在你的工程中包含以下文件：
- `EIE3810_Debug.c`
- `EIE3810_Debug.h`
- `EIE3810_TFTLCD.c`
- `EIE3810_TFTLCD.h`

## 许可证
用于 EIE3810 课程教学使用
