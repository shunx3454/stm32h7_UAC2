#include "lcd.h"
#include "bord_config.h"
#include "lcd_fonts.h"
#include "periph.h"

#define LCD_WIDTH 800U
#define LCD_HEIGHT 480U
#define LCD_CLR_FMT LTDC_PIXEL_FORMAT_RGB565
#if LCD_CLR_FMT == LTDC_PIXEL_FORMAT_ARGB8888
#define LCD_PIXBYTES 4
#elif LCD_CLR_FMT == LTDC_PIXEL_FORMAT_RGB888
#define LCD_PIXBYTES 3
#else
#define LCD_PIXBYTES 2
#endif

struct _LCD lcd;
static uint8_t lcd_gram[LCD_WIDTH * LCD_HEIGHT * LCD_PIXBYTES] __attribute__((section(".lcd_gdram")));

void lcd_init(void) {
  lcd.dir = 1;
  lcd.width = LCD_WIDTH;
  lcd.height = LCD_HEIGHT;
  lcd.gram_addr = (uint32_t)lcd_gram;
  lcd.clr_fmt = LCD_CLR_FMT;
  lcd.bg_clr = 0xffff;
  lcd.color = 0xff00;
  lcd.Font = &Font32;
  lcd.pixbytes = LCD_PIXBYTES;

  ldtc_init(lcd.clr_fmt, LTDC_GRAM_ADDR);
  dma2d_init();
  lcd_bg(1);
}

void lcd_bg(uint8_t bg) {
  bg ? HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET)
     : HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
}

void lcd_refresh(uint32_t clr) {
  if (lcd.dir == 1) {
    lcd_fill_rect(0, 0, 800, 480, clr);
  } else if (lcd.dir == 2) {
    lcd_fill_rect(0, 0, 480, 800, clr);
  }
}

void lcd_draw_point(uint16_t x, uint16_t y, uint32_t clr) {
  uint16_t DIR0_X;
  uint16_t DIR0_Y;
  __IO uint8_t *gram_ptr_8b = (uint8_t *)lcd.gram_addr;
  __IO uint16_t *gram_ptr_16b = (uint16_t *)lcd.gram_addr;
  __IO uint32_t *gram_ptr_32b = (uint32_t *)lcd.gram_addr;

  if (lcd.dir == 1) {
    DIR0_X = x;
    DIR0_Y = y;
  } else if (lcd.dir == 2) {
    DIR0_X = y;
    DIR0_Y = lcd.height - 1 - x;
  } else
    return;

  /* 边界判断 */
  if (DIR0_X >= lcd.width || DIR0_Y >= lcd.height) {
    return;
  }

  if (lcd.pixbytes == 2) {
    gram_ptr_16b += DIR0_Y * lcd.width + DIR0_X;
    *gram_ptr_16b = (uint16_t)clr;
  } else if (lcd.pixbytes == 3) {
    gram_ptr_8b += (DIR0_Y * lcd.width + DIR0_X) * 3;
    // uint32 clr : [00] [R8] [G8] [B8]
    // addr: [B8] [G8] [R8]
    *gram_ptr_8b++ = clr & 0xff;        // R8
    *gram_ptr_8b++ = (clr >> 8) & 0xff; // G8
    *gram_ptr_8b++ = (clr >> 16);       // B8
  } else if (lcd.pixbytes == 4) {
    gram_ptr_32b += DIR0_Y * lcd.width + DIR0_X;
    *gram_ptr_32b = clr;
  }
}

/***************************************************************************************************************
 *	函 数 名:	LCD_DisplayChar
 *
 *	入口参数:	x - 起始水平坐标，取值范围0~799
 *					y - 起始垂直坐标，取值范围0~479
 *					c  - ASCII字符
 *
 *	函数功能:	在指定坐标显示指定的字符
 *
 *	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetFont(&Font24) 设置为 2412的ASCII字体
 *					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
 *					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
 *					4. 使用示例 LCD_DisplayChar( 10, 10, 'a') ，在坐标(10,10)显示字符 'a'
 *
 ***************************************************************************************************************/

void LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t c) {
  uint16_t index = 0, counter = 0; // 计数变量
  uint8_t disChar;                 // 存储字符的地址
  uint16_t Xaddress = x;           // 水平坐标

  c = c - 32; // 计算ASCII字符的偏移

  for (index = 0; index < lcd.Font->Sizes; index++) {
    disChar = lcd.Font->pTable[c * lcd.Font->Sizes + index]; // 获取字符的模值
    for (counter = 0; counter < 8; counter++) {
      if (disChar & 0x01) {
        lcd_draw_point(Xaddress, y, lcd.color); // 当前模值不为0时，使用画笔色绘点
      } else {
        lcd_draw_point(Xaddress, y, lcd.bg_clr); // 否则使用背景色绘制点
      }
      disChar >>= 1;
      Xaddress++; // 水平坐标自加

      if ((Xaddress - x) == lcd.Font->Width) // 如果水平坐标达到了字符宽度，则退出当前循环
      {                                      // 进入下一行的绘制
        Xaddress = x;
        y++;
        break;
      }
    }
  }
}

/***************************************************************************************************************
 *	函 数 名:	LCD_DisplayString
 *
 *	入口参数:	x - 起始水平坐标，取值范围0~799
 *					y - 起始垂直坐标，取值范围0~479
 *					p - ASCII字符串的首地址
 *
 *	函数功能:	在指定坐标显示指定的字符串
 *
 *	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetFont(&Font24) 设置为 2412的ASCII字体
 *					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
 *					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
 *					4. 使用示例 LCD_DisplayString( 10, 10, "FANKE")
 *，在起始坐标为(10,10)的地方显示字符串"FANKE"
 *
 ***************************************************************************************************************/

void LCD_DisplayString(uint16_t x, uint16_t y, const char *p) {
  while ((x < lcd.width) && (*p != 0)) // 判断显示坐标是否超出显示区域并且字符是否为空字符
  {
    LCD_DisplayChar(x, y, *p);
    x += lcd.Font->Width; // 显示下一个字符
    p++;                  // 取下一个字符地址
  }
}

/*
    区域包含sx,sy坐标点，不包含ex,ey坐标点
 */
void lcd_fill_rect(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t clr) {
  /* 相对方向0也就是横屏的起点坐标和尺寸 */
  uint16_t DIR0_SX;
  uint16_t DIR0_SY;
  uint16_t DIR0_width;
  uint16_t DIR0_lines;

  if (sx >= ex || sy >= ey)
    return;

  if (lcd.dir == 1) {
    /* 超出区域 */
    if (ex > lcd.width || ey > lcd.height) {
      return;
    }
    DIR0_SX = sx;
    DIR0_SY = sy;
    DIR0_width = ex - sx;
    DIR0_lines = ey - sy;
  } else if (lcd.dir == 2) {
    /* 超出屏幕区域 */
    if (ex > lcd.height || ey > lcd.width) {
      return;
    }
    DIR0_SX = sy;
    DIR0_SY = lcd.height - ex;
    DIR0_width = ey - sy;
    DIR0_lines = ex - sx;
  } else
    return;

  /* 停止DMA */
  DMA2D->CR &= ~DMA2D_CR_START;
  /* 单色寄存器到存储器 */
  DMA2D->CR = DMA2D_R2M;
  /* 输出颜色格式 */
  DMA2D->OPFCCR = lcd.clr_fmt;
  /* 输出存储器起始地址 */
  DMA2D->OMAR = lcd.gram_addr + (DIR0_SY * lcd.width + DIR0_SX) * lcd.pixbytes;
  /* 输出一行后偏移像数 */
  DMA2D->OOR = lcd.width - DIR0_width;
  /* 输出窗口大小 */
  DMA2D->NLR = (DIR0_width << 16) | DIR0_lines;
  /* 单色填充寄存器 */
  DMA2D->OCOLR = clr;
  /* 启动 */
  DMA2D->CR |= DMA2D_CR_START;
  /* 等待标志 */
  while ((DMA2D->ISR & DMA2D_FLAG_TC) == 0) {
  }
  /* 清除标志 */
  DMA2D->IFCR |= DMA2D_FLAG_TC;
}

void lcd_fill_color(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t *clr) {
  // 必须加上，否则花屏
  // 把Cache数据同步到主存，才能交给DMA传输
  // SCB_CleanInvalidateDCache();
  SCB_CleanDCache();    

  /* 存储器到存储器 */
  DMA2D->CR = 0x00000000UL;
  /* 输入、输出存储器起始地址 */
  DMA2D->FGMAR = (uint32_t)clr;
  DMA2D->OMAR = lcd.gram_addr + (sy * lcd.width + sx) * lcd.pixbytes;
  /* 输入、输出一行后偏移像数 */
  DMA2D->FGOR = 0;
  DMA2D->OOR = lcd.width - (ex - sx);
  /* 输入、输出颜色格式 */
  DMA2D->FGPFCCR = lcd.clr_fmt;
  DMA2D->OPFCCR = lcd.clr_fmt;
  /* 输出窗口大小 */
  DMA2D->NLR = (uint32_t)((ex - sx) << 16) | (ey - sy);
  /* 启动 */
  DMA2D->CR |= DMA2D_CR_START;
  /* 等待标志 */
  while (DMA2D->CR & DMA2D_CR_START) {
  }
}

void lcd_show_charCN(uint16_t x, uint16_t y, uint16_t cn_index) {
  int sizeOfBytes = 4 * 32;
  uint8_t cnByte;
  uint16_t Yaddress = y;

  for (int i = 0; i < sizeOfBytes; i++) {
    cnByte = CN_3232_Table[cn_index * sizeOfBytes + i];
    for (int j = 0; j < 8; j++) {
      if (cnByte & 0x01) {
        lcd_draw_point(x, Yaddress, lcd.color);
      } else {
        lcd_draw_point(x, Yaddress, lcd.bg_clr);
      }
      cnByte >>= 1;
      Yaddress++;
    }
    if (Yaddress - y >= 32) {
      Yaddress = y;
      x++;
    }
  }
}