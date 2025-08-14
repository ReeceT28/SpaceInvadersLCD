#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#ifndef F_CPU
#define F_CPU 16000000UL  // default 16 MHz, override as needed
#endif

// Busy-wait delay for microseconds
static inline void delayUs(uint32_t us)
{
    // Each iteration of the inner loop takes roughly 4 CPU cycles
    // So we calculate the number of iterations needed:
    uint32_t cycles = (F_CPU / 1000000UL) * us / 4;
    while(cycles--) 
    {
        __asm__ volatile("nop");
    }
}

// Busy-wait delay for milliseconds
static inline void delayMs(uint32_t ms)
{
    while(ms--)
    {
        delayUs(1000);
    }
}

#endif