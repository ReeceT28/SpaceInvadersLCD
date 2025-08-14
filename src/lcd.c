#include "lcd.h"
#include <util/delay.h>

char* intToString(int num) 
{
    static char bum[5];  // Static buffer to hold the string, including null terminator

    bum[4] = '\0'; // Null-terminate the string
    for (int i = 3; i >= 0; i--) {
        bum[i] = (num % 10) + '0'; // Convert last digit to character
        num /= 10; // Remove last digit from num
    }

    return bum;
}

void LCD_create(LCD *lcd,
                volatile uint8_t *port,
                volatile uint8_t *ddr,
                uint8_t rs, uint8_t e,
                uint8_t db4, uint8_t db5, uint8_t db6, uint8_t db7)
{
    lcd->port = port;
    lcd->ddr = ddr;
    lcd->rs = rs;
    lcd->e = e;
    lcd->db4 = db4;
    lcd->db5 = db5;
    lcd->db6 = db6;
    lcd->db7 = db7;
    lcd->cols = LCD_DEFAULT_COLS;
}

void LCD_sendNibble(LCD *lcd, uint8_t data)
{
    if (data & 0x01) *(lcd->port) |= (1 << lcd->db4); else *(lcd->port) &= ~(1 << lcd->db4); //set D4
    if (data & 0x02) *(lcd->port) |= (1 << lcd->db5); else *(lcd->port) &= ~(1 << lcd->db5); //set D5
    if (data & 0x04) *(lcd->port) |= (1 << lcd->db6); else *(lcd->port) &= ~(1 << lcd->db6); //set D6
    if (data & 0x08) *(lcd->port) |= (1 << lcd->db7); else *(lcd->port) &= ~(1 << lcd->db7); //set D7

    *(lcd->port) |= (1 << lcd->e); //pulse E on
    _delay_us(50); // Short delay to latch the data
    *(lcd->port) &= ~(1 << lcd->e); //turn E off
    _delay_us(50); // Short delay after pulsing
}

void LCD_writeCmd(LCD *lcd, uint8_t cmd)
{
    // Set RS to 0 for cmd 
    *(lcd->port) &= ~(1 << lcd->rs);
    // Send the higher nibble
    LCD_sendNibble(lcd, cmd >> 4);
    // Send the lower nibble
    LCD_sendNibble(lcd, cmd);

}

void LCD_writeData(LCD *lcd, uint8_t data)
{
    // Set RS to 1 for data 
    *(lcd->port) |=  (1<<lcd->rs);
    // Send the higher nibble
    LCD_sendNibble(lcd, data >> 4);
    // Send the lower nibble
    LCD_sendNibble(lcd, data);
}

void LCD_writeString(LCD *lcd, const char *data_l)
{
    while (*data_l)
    {
        LCD_writeData(lcd, *data_l);
        data_l++;
    }
}

void LCD_init(LCD *lcd)
{

    *(lcd->ddr) |= (1<<lcd->db4) | (1<<lcd->db5) | (1<<lcd->db6) | (1<<lcd->db7) | (1<<lcd->e) | (1<<lcd->rs);
    _delay_ms(50); // Initial delay for LCD power-up

    LCD_writeCmd(lcd, 0x33); // Initialize for 4-bit mode (3 times as per spec)
    _delay_ms(5);
    LCD_writeCmd(lcd, 0x32); // Set to 4-bit mode
    _delay_ms(5);

    LCD_writeCmd(lcd, 0x28); // 2 lines, 5x7 matrix
    _delay_ms(2);

    LCD_writeCmd(lcd, 0x0C); // Display on, cursor off
    _delay_ms(2);

    LCD_writeCmd(lcd, 0x06); // Increment cursor
    _delay_ms(2);

    LCD_writeCmd(lcd, 0x01); // Clear display
    _delay_ms(2);
}

void LCD_update(LCD *lcd, int xValue, int yValue, int potValue)
{
    LCD_writeCmd(lcd, 0x82); // Move to position after X:
    LCD_writeString(lcd, intToString(xValue));

    LCD_writeCmd(lcd, 0x89); // position after Y:
    LCD_writeString(lcd, intToString(yValue));

    LCD_writeCmd(lcd, 0xC4); // position after Pot:
    LCD_writeString(lcd, intToString(potValue));
}

void LCD_scrollRight(LCD *lcd)
{
    LCD_writeCmd(lcd, 0x1F);
}

void LCD_scrollLeft(LCD *lcd)
{
    LCD_writeCmd(lcd, 0x18);
}

void LCD_createCustomChar(LCD *lcd, uint8_t location, uint8_t bitmap[]) 
{
    // If any location other than 0-7 is used then not valid so essentially clamp in this range
    location &= 0x07; 
    // Left shift location by 3 to multiply the address by 8 as each character takes 8 bytes 
    LCD_writeCmd(lcd, 0x40 | (location<<3)); // set CGRAM address

    for (uint8_t i = 0; i < 8; i++) 
    {
        // write each row byte
        LCD_writeData(lcd, bitmap[i]);
    }
}

void LCD_setCursorPos(LCD *lcd, int xPos, int yPos)
{
    if(yPos == 0)
    {
        LCD_writeCmd(lcd, 0x80 | (xPos));
    }
    else
    {
        LCD_writeCmd(lcd, 0x80 | (0x40 + xPos));
    }
}
