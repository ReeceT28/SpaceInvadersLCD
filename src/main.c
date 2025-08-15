#include "lcd.h"
#include "delay.h"
#include <avr/io.h>
#define MAX_BULLETS 4
#define MAX_ENEMIES 8
#define FRAME_PERIOD 10
#define DB4IO DDB1
#define DB5IO DDB2
#define DB6IO DDB3
#define DB7IO DDB4
#define EIO   DDB5
#define RSIO  DDB0

// LCD instance
LCD lcd;

static const uint8_t basePlayer[8] = {
    0b00011000,
    0b00011100,
    0b00011100,
    0b00011110,
    0b00011100,
    0b00011100,
    0b00011000,
    0b00000000
};

static const uint8_t invaderLeft[8] = {
    0b00001000,
    0b00000111,
    0b00001111,
    0b00011011,
    0b00011111,
    0b00010100,
    0b00010010,
    0b00000000
};

static const uint8_t invaderRight[8] = {
    0b00000100,
    0b00011000,
    0b00011100,
    0b00010110,
    0b00011110,
    0b00001010,
    0b00010010,
    0b00000000
};


// height ranges from 0-9
static int8_t height = 0;
static uint16_t frameCount = 0;
static uint8_t bulletX[MAX_BULLETS];
static uint8_t bulletRow[MAX_BULLETS];
static uint8_t invaderX[MAX_ENEMIES];      
static uint8_t invaderRow[MAX_ENEMIES];    

static void spawnInvader()
{
    uint8_t i=0;
    while(i<MAX_ENEMIES)
    {
        if (invaderX[i] == 0xff) // find inactive slot
        {
            // start just to the right of the screen
            invaderX[i] = 17;   
            // Generate Psuedorandom row 
            invaderRow[i] = (frameCount + i * 7) & 0x01;  
            // Draw the invader on the screen
            LCD_setCursorPos(&lcd, invaderX[i], invaderRow[i]);
            LCD_writeData(&lcd, 0x06);
            LCD_setCursorPos(&lcd, invaderX[i]+1, invaderRow[i]);
            LCD_writeData(&lcd, 0x07);
            break; // spawn only one invader per call
        }
        i++;
    }
}

static void fireBullet() 
{
    static const uint8_t blank[8] = {0,0,0,0,0,0,0,0};
    uint8_t i = 0;
    while (i < MAX_BULLETS) 
    {
        if (bulletX[i] == 0xff) 
        {
            // This saves like 28 bytes of flash compared to initialising sprite as {0,0,0,0,0,0,0,0}, I'm not quite sure why would need to look into assembly probably 
            uint8_t sprite[8];
            for (uint8_t r = 0; r < 8; r++) sprite[r] = blank[r];
            // Start next to player
            bulletX[i] = 1;
            bulletRow[i] = 0;
            // Gun starts at pixel 3
            uint8_t pixelRow = 3 + height;
            // If pixelRow>7 on bottom cell so -8 from pixelRow
            if (pixelRow > 7) { bulletRow[i] = 1; pixelRow -= 8; }
            sprite[pixelRow] = 0xFF;
            // Bullets start at 0x02 in CGRAM 
            LCD_createCustomChar(&lcd, 2 + i, sprite);
            break;
        }
        i++;
    }
}

static void updateInvaders()
{
    for(uint8_t i=0 ; i < MAX_ENEMIES; i++)
    {
        // Remove old bullet
        LCD_clearPos(&lcd,invaderX[i],invaderRow[i]);
        LCD_clearPos(&lcd,invaderX[i]+1, invaderRow[i]);

        if(invaderX[i] >= 1 && invaderX[i] < 0xff)
        {
            invaderX[i] -= 1;
        }
        else
        {
            invaderX[i]=0xff;
        }
    }   
}

static void updateBullets()
{
    for(uint8_t i=0 ; i < MAX_BULLETS; i++)
    {
        // Remove old bullet
        LCD_clearPos(&lcd,bulletX[i],bulletRow[i]);

        if(bulletX[i] != 0xff)
        {
            bulletX[i] += 1;
        }
        if(bulletX[i] > 15)
        {
            bulletX[i]=0xff;
        }
    }
}

static void checkHit()
{
    for(uint8_t i=0; i<MAX_BULLETS ; i++)
    {
        if(bulletX[i] == 0xff) continue;
        for(uint8_t j =0 ; j <MAX_ENEMIES; j++)
        {
            uint8_t x = invaderX[j];
            if (x == 0xFF) continue; 
            if(((bulletX[i] == x) || (bulletX[i] == x+1)) && (bulletRow[i] == invaderRow[j]))
            {
                LCD_clearPos(&lcd,bulletX[i],bulletRow[i]);
                LCD_clearPos(&lcd, x, invaderRow[j]);
                LCD_clearPos(&lcd, x+1, invaderRow[j]);
                bulletX[i]=0xff;
                invaderX[j]= 0xff;
            }
        }
    }
}

static void drawBullets()
{
    for(uint8_t i=0 ; i < MAX_BULLETS ; i++)
    {
        if(bulletX[i] !=0xff)
        {
            LCD_setCursorPos(&lcd, bulletX[i],bulletRow[i]);
            LCD_writeData(&lcd, 2+ i);
        } 
    }
}

static void drawInvader()
{
    for(uint8_t i=0 ; i<MAX_ENEMIES; i++)
    {
        if(invaderX[i]!=0xff)
        {
            LCD_setCursorPos(&lcd, invaderX[i], invaderRow[i]);
            LCD_writeData(&lcd, 0x06);
            LCD_setCursorPos(&lcd, invaderX[i]+1, invaderRow[i]);
            LCD_writeData(&lcd, 0x07);
        }
    }
}



static void drawPlayer()
{
    // Clamp height to valid range
    if (height > 9) height = 9;
    else if (height < 0) height = 0;

    // Initialise player sprites
    uint8_t playerTop[8], playerBottom[8];

    for (uint8_t row = 0; row < 8; row++)
    {
        int8_t topRow = row - height;
        playerTop[row] = (topRow >= 0 && topRow < 8) ? basePlayer[topRow] : 0;

        int8_t bottomRow = row + 8 - height;
        playerBottom[row] = (bottomRow >= 0 && bottomRow < 8) ? basePlayer[bottomRow] : 0;
    }

    // Draw player sprites
    LCD_createCustomChar(&lcd, 0x00, playerTop);
    LCD_createCustomChar(&lcd, 0x01, playerBottom);
    LCD_setCursorPos(&lcd, 0,0); 
    LCD_writeData(&lcd, 0x00);  
    LCD_setCursorPos(&lcd,0,1);
    LCD_writeData(&lcd, 0x01);
}


// Initialises everything in the program
void setup() 
{
    uint8_t i = 0;
    while (i < MAX_ENEMIES || i < MAX_BULLETS)
    {
        if (i < MAX_ENEMIES) invaderX[i] = 0xff;
        if (i < MAX_BULLETS) bulletX[i] = 0xff;
        i++;
    }

    // Create the LCD object with the given pins
    LCD_create(&lcd, &PORTB, &DDRB, RSIO, EIO, DB4IO, DB5IO, DB6IO, DB7IO);
    LCD_init(&lcd);

    // Create invader characters
    LCD_createCustomChar(&lcd, 0x06, invaderLeft);
    LCD_createCustomChar(&lcd, 0x07, invaderRight);

    // Analog read setup
    ADMUX|=(1<<REFS0); //set ADC reference to AVcc (5V)
    ADCSRA|=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN); //ADPS bits set prescaler (128) ADEN enables the ADC
}

// Empty loop function (can be expanded if needed)
void loop() 
{
    ADMUX|=(1<<MUX2) | (1<<MUX0); // A5 set as ADC reading
    ADCSRA|=(1<<ADSC); // begin convertion
    while(ADCSRA & (1<<ADSC)); // wait 

    // technically should be divided by 1023 but can save some space via this trick and doesn't make any noticable difference in controls
    int16_t delta = (int16_t)(ADC) - 512;        // ensure signed
    delta = (delta * 2 + 512) >>10;           
    height += delta;


    // ====================================================== POWER OF 2 MODULUS TRICK ======================================================
    // 63 = 0b00111111 = 0x3F,  frameCount & 0x3F sets all higher bits to 0 and only keeps the lower bits, all higher bits are
    // 64 multiplied by some power of 2 so if (frameCount & 0x3F ==0) then frameCount is multiple of 64, essentially a fast modulus for powers of 2

    // every 32 frames
    if ((frameCount & 0x1F) == 0)
    {   
        fireBullet();   
        spawnInvader();      
    }

    // every 8 frames
    if ((frameCount & 0x07) == 0)                     
    {      
        updateBullets();
        updateInvaders();
    }

    checkHit();
    drawBullets();
    drawInvader();
    drawPlayer();
    delayMs(FRAME_PERIOD);
    frameCount++;
}

int main(void)
{
    setup(); 
    while (1)
    {
        loop(); 
    }
    return 0;
}