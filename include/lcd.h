#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <stdint.h>
#define LCD_DEFAULT_COLS 16

typedef struct 
{
    volatile uint8_t *port;
    volatile uint8_t *ddr;
    uint8_t db4, db5, db6, db7;
    uint8_t e, rs;
    uint8_t cols;
} LCD;

void LCD_create(LCD *lcd,
                volatile uint8_t *port,
                volatile uint8_t *ddr,
                uint8_t rs, uint8_t e,
                uint8_t db4, uint8_t db5, uint8_t db6, uint8_t db7);

void LCD_sendNibble(LCD *lcd, uint8_t nibble);
void LCD_writeCmd(LCD *lcd, uint8_t cmd);
void LCD_writeData(LCD *lcd, uint8_t data);
void LCD_writeString(LCD *lcd, const char *str);
void LCD_init(LCD *lcd);
void LCD_scrollRight(LCD *lcd);
void LCD_scrollLeft(LCD *lcd);
void LCD_createCustomChar(LCD *lcd, uint8_t location, const uint8_t bitmap[]);
void LCD_setCursorPos(LCD *lcd, const uint8_t xPos, const uint8_t yPos);

char* intToString(uint16_t num);

#endif
