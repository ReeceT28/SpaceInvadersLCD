#ifndef DELAY_H
#define DELAY_H

#ifndef F_CPU
#define F_CPU 16000000UL  // default 16 MHz
#endif

// Exact-cycle delay in microseconds
static inline void delayUs(uint16_t us)
{
    // Each loop below takes exactly 4 cycles:

    // 2 cycles for 'SBIW' (subtract immediate from word)
    // Works on 2 consecutive 8-bit registers treated as a 16-bit word. You can subtract 0 to 63 directly from that 16-bit word. The result is stored in the same register pair
    // "This instruction operates on the upper four register pairs" (this is important later)

    // 2 cycles for 'BRNE' (branch if not equal) if it branches or 1 cycle if it doesn't branch
    // BRNE checks if the Zero flag (Z) is set (result of last calculation=0), it branches if Z=0 meaning the result of the last calculation wasn't 0
    // It is like an inverse BRZ

    // F_CPU / 1000000UL is the amount of cycles per microsecond, multiply by number of microseconds, then divide by 4 as our loops take 4 cycles
    uint16_t loops = (F_CPU / 1000000UL) * us / 4;

    // Some extra stuff:
    // https://www.nongnu.org/avr-libc/user-manual/inline_asm.html
    // Go to "Input and Output Operands"
    // "=w" specifies the use of special upper register pairs the ones that SBIW can operate on
    // Constraint "0" tells the compiler, to use the same input register as for the first operand.

    // How it works:
    // %0 refers to the first operand listed in the output/input sections (the loops variable)
    // Subtracts 1 from loops, if this result isn't 0 then go back to the start (1b) else continue (so the loop ends)
    __asm__ __volatile__ (
        "1: sbiw  %0, 1" "\n\t" 
        "brne   1b"       "\n\t" 
        : "=w" (loops) 
        : "0"  (loops)
    );
}

static inline void delayMs(uint32_t ms)
{
    while(ms--)
    {
        delayUs(1000);
    }
}

#endif