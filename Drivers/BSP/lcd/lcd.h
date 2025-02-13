#ifndef __LCD_H_
#define __LCD_H_

#include "main.h"
#include <stdint.h>
#include "lcd_fonts.h"

extern struct _LCD lcd;

struct _LCD
{
    uint8_t pixbytes;
    uint16_t width;
    uint16_t height;
    uint8_t dir;
    uint32_t gram_addr;
    uint32_t bg_clr;
    uint32_t color;
    uint32_t clr_fmt;
    pFONT *Font;
};

void lcd_init(void);
void lcd_bg(uint8_t bg);
void lcd_refresh(uint32_t clr);
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t clr);
void LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t c);
void LCD_DisplayString(uint16_t x, uint16_t y, const char *p);
void lcd_show_charCN(uint16_t x, uint16_t y, uint16_t cn_index);
void lcd_fill_rect(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t clr);
void lcd_fill_color(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t *clr);

#endif
