#include "lcd.h"
#include <util/delay.h>
#define MAX_BULLETS 4
#define MAX_ENEMIES 8
#define FRAME_PERIOD 120
// LCD instance
LCD lcd;

uint8_t basePlayer[8] = {
    0b00011000,
    0b00011100,
    0b00011100,
    0b00011110,
    0b00011100,
    0b00011100,
    0b00011000,
    0b00000000
};

uint8_t invaderLeft[8] = {
    0b00001000,
    0b00000111,
    0b00001111,
    0b00011011,
    0b00011111,
    0b00010100,
    0b00010010,
    0b00000000
};

uint8_t invaderRight[8] = {
    0b00000100,
    0b00011000,
    0b00011100,
    0b00010110,
    0b00011110,
    0b00001010,
    0b00010010,
    0b00000000
};

uint8_t invaderSpeed =1 ;

// height ranges from 0-9
uint8_t height = 0;
uint8_t bulletSpeed = 1; 
uint8_t frameCount= 0;
uint8_t lastFire  = 0;
uint8_t lastInvader =0;

int8_t bulletX[MAX_BULLETS] = {-1,-1,-1,-1}; // -1 = inactive
uint8_t bulletRow[MAX_BULLETS] = {0};

int8_t invaderX[MAX_ENEMIES];      // -1 = inactive
uint8_t invaderRow[MAX_ENEMIES];    // row of each invader

void spawnInvader()
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (invaderX[i] == -1) // find inactive slot
        {
            // Spawn position: top row, leftmost column
            invaderX[i] = 17;    // start at far right
            invaderRow[i] = (frameCount + i * 7) % 2;  // Generate Psuedorandom row

            // Draw the invader on the screen
            LCD_setCursorPos(&lcd, invaderX[i], invaderRow[i]);
            LCD_writeData(&lcd, 0x06);
            LCD_setCursorPos(&lcd, invaderX[i]+1, invaderRow[i]);
            LCD_writeData(&lcd, 0x07);

            break; // spawn only one invader per call
        }
    }
}

void updateInvaders()
{
    for(int i=0 ; i < MAX_ENEMIES; i++)
    {
        // Remove old bullet
        LCD_setCursorPos(&lcd, invaderX[i], invaderRow[i]);
        LCD_writeData(&lcd,0b00100000);
        LCD_setCursorPos(&lcd, invaderX[i]+1, invaderRow[i]);
        LCD_writeData(&lcd,0b00100000);
        if(invaderX[i] >= 1)
        {
            invaderX[i] -= invaderSpeed;
        }
        else
        {
            invaderX[i]=-1;
        }
    }   
}

void drawInvader()
{
    for(int i=0 ; i<MAX_ENEMIES; i++)
    {
        if(invaderX[i]!=-1)
        {
            LCD_setCursorPos(&lcd, invaderX[i], invaderRow[i]);
            LCD_writeData(&lcd, 0x06);
            LCD_setCursorPos(&lcd, invaderX[i]+1, invaderRow[i]);
            LCD_writeData(&lcd, 0x07);
        }
    }
}

// Bullets start at 0x02 in CGRAM 
void fireBullet() 
{
    for (int i = 0; i < MAX_BULLETS; i++) 
    {
        if (bulletX[i] == -1) 
        {
            uint8_t bulletSprite[8] = {
                    0b00000000,
                    0b00000000,
                    0b00000000,
                    0b00000000,
                    0b00000000,
                    0b00000000,
                    0b00000000,
                    0b00000000
            };

            int pixelRow;
            bulletX[i] = 1; // start next to the player
            pixelRow = 3 + height;
            bulletRow[i]=0;
            if (pixelRow > 7) 
            {
                bulletRow[i] = 1;       // bottom half
                pixelRow -= 8;          // adjust to 0–7 range
            }
            // Char position in memory = 2+i
            bulletSprite[pixelRow] = 0xff;
            LCD_createCustomChar(&lcd, 2 + i, bulletSprite);
            break; // fire only one bullet per call
        }
    }
}
void updateBullets()
{
    for(int i=0 ; i < MAX_BULLETS; i++)
    {
        // Remove old bullet
        LCD_setCursorPos(&lcd, bulletX[i], bulletRow[i]);
        LCD_writeData(&lcd,0b00100000);
        if(bulletX[i] != -1)
        {
            bulletX[i] += bulletSpeed;
        }
        if(bulletX[i] > 15)
        {
            bulletX[i]=-1;
        }
    }
}

void drawBullets()
{
    for(int i=0 ; i < MAX_BULLETS ; i++)
    {
        if(bulletX[i] !=-1)
        {
            LCD_setCursorPos(&lcd, bulletX[i],bulletRow[i]);
            LCD_writeData(&lcd, 2+ i);
        } 
    }
}

void checkHit()
{
    for(int i=0; i<MAX_BULLETS ; i++)
    {
        for(int j =0 ; j <MAX_ENEMIES; j++)
        {
            if(((bulletX[i] == invaderX[j]) || (bulletX[i] == invaderX[j]+1)) && (bulletRow[i] == invaderRow[j]))
            {
                LCD_setCursorPos(&lcd, bulletX[i], bulletRow[i]);
                LCD_writeData(&lcd,0b00100000);
                LCD_setCursorPos(&lcd, invaderX[i], invaderRow[i]);
                LCD_writeData(&lcd,0b00100000);
                LCD_setCursorPos(&lcd, invaderX[i]+1, invaderRow[i]);
                LCD_writeData(&lcd,0b00100000);
                bulletX[i]=-1;
                invaderX[j]= -1;
            }
        }
    }
}

void drawPlayer()
{
    if(height>9)
    {
        height = 9;
    }
    else if(height <0)
    {
        height = 0;
    }
    uint8_t playerTop[8] = {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000  
    };
    uint8_t playerBottom[8] = {
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000  
    };
    
    for (int row = 0; row < 8; row++)
    {
        int srcRow = row - height;  // calculate which base row this maps to
        playerTop[row] = (srcRow >= 0 && srcRow < 8) ? basePlayer[srcRow] : 0;
    }

    for (int row = 0; row < 8; row++)
    {
        int srcRow = row + 8 - height;  // offset for bottom half
        playerBottom[row] = (srcRow >= 0 && srcRow < 8) ? basePlayer[srcRow] : 0;
    }

    LCD_createCustomChar(&lcd, 0x00, playerTop);
    LCD_createCustomChar(&lcd, 0x01, playerBottom);
    LCD_setCursorPos(&lcd, 0,0); 
    LCD_writeData(&lcd, 0x00);  
    LCD_setCursorPos(&lcd,0,1);
    LCD_writeData(&lcd, 0x01);
}

// Setup function to initialize LCD
void setup() 
{

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        invaderX[i] = -1;
    }
    uint8_t DB4IO = DDB1;
    uint8_t DB5IO = DDB2;
    uint8_t DB6IO = DDB3;
    uint8_t DB7IO = DDB4;
    uint8_t EIO   = DDB5;
    uint8_t RSIO  = DDB0;

    // Create the LCD object with the given pins
    LCD_create(&lcd, &PORTB, &DDRB, RSIO, EIO, DB4IO, DB5IO, DB6IO, DB7IO);
    LCD_init(&lcd); // Initialize the LCD


    LCD_createCustomChar(&lcd, 0x00, basePlayer);
    LCD_createCustomChar(&lcd, 0x01, (uint8_t[8]){0,0,0,0,0,0,0,0});

    LCD_setCursorPos(&lcd, 0,0); 
    LCD_writeData(&lcd, 0x00);  
    LCD_setCursorPos(&lcd,0,1);
    LCD_writeData(&lcd, 0x01);

    LCD_createCustomChar(&lcd, 0x06, invaderLeft);
    LCD_createCustomChar(&lcd, 0x07, invaderRight);

    // Analog read setup
    ADMUX|=(1<<REFS0); //set ADC reference to AVcc (5V)
    ADCSRA|=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN); //ADPS bits set prescaler (128) ADEN enables the ADC
}

// Empty loop function (can be expanded if needed)
void loop() 
{
    ADMUX|=(1<<MUX2) | (1<<MUX0); // A5 set as ADC reading for X axis of joystick
    ADCSRA|=(1<<ADSC); // begin convertion
    while(ADCSRA & (1<<ADSC)); // wait 
    int xValue=ADC; 

    height = (xValue/1023.0f) * 9;

    if(frameCount - lastFire > 5)
    {
        lastFire = frameCount;
        fireBullet();
    }
    if(frameCount - lastInvader >10)
    {
        lastInvader = frameCount;
        spawnInvader();
    }
    updateBullets();
    updateInvaders();
    checkHit();
    drawBullets();
    drawInvader();

    drawPlayer();

    _delay_ms(FRAME_PERIOD);
    frameCount++;
}

// Main function
int main(void)
{
    setup();  // Initialize and display the message

    while (1)
    {
        loop(); // Run loop (currently empty)
    }

    return 0; // End of program
}
