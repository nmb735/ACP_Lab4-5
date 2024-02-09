/*The code uses a high-level C language code, which attempts to control the turning on, off, and alternation of LED lights through input and output units of our board.*/

#include "derivative.h" /* Include peripheral declarations */
#include "MKL25Z4.h"    /* Include board library */

/* Define constants for colors */
static const int RED = 0;
static const int GREEN = 1;

void wait()
{
    int i;
    for (i = 0; i < 100; i++)
    {
        asm("nop");
    }
}

int main(void)
{
    /* STEP 0: TEST. Turn on and off blue LED */
    SIM_BASE_PTR->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    SIM_BASE_PTR->SCGC5 |= SIM_SCGC5_TSI_MASK;

    PORTB_BASE_PTR->PCR[18] = (PORTB_BASE_PTR->PCR[18] | 0x000000100) & 0xFFFFF9FF;
    PORTB_BASE_PTR->PCR[19] = (PORTB_BASE_PTR->PCR[19] | 0x000000100) & 0xFFFFF9FF;

    PTB_BASE_PTR->PDDR |= (0x40000); // Turn on red LED
    PTB_BASE_PTR->PSOR |= (0x40000); // Turn off red LED

    PTB_BASE_PTR->PDDR |= (0x80000); // Turn on green LED
    PTB_BASE_PTR->PSOR |= (0x80000); // Turn off green LED

    /* Activate touch clock gating */
    SIM_SCGC5 |= (SIM_SCGC5_TSI_MASK);
    /* Configuration of voltages and frequencies */
    TSI0_GENCS |= (TSI_GENCS_MODE(0) | TSI_GENCS_REFCHRG(4) | TSI_GENCS_DVOLT(0) | TSI_GENCS_EXTCHRG(7) | TSI_GENCS_PS(4) | TSI_GENCS_NSCN(11) | TSI_GENCS_STPE_MASK);

    TSI0_BASE_PTR->GENCS |= (1 << 7);

    /* STEP 6: Declare that pins 16 and 17 will not be used by GPIO */
    PORTB_BASE_PTR->PCR[16] &= 0b11111111111111111111100011111111;
    PORTB_BASE_PTR->PCR[17] &= 0b11111111111111111111100011111111;

    /* Turn on green LED as initial state */
    PTB_BASE_PTR->PDDR = 1 << 19;

    int colorAnterior = GREEN;

    /* STEP 7: Capture time taken by touch for initial scan */
    while (!(TSI0_BASE_PTR->GENCS & 4))
        ;

    uint16_t timeScan = (uint16_t)TSI0_BASE_PTR->DATA;

    TSI0_BASE_PTR->GENCS |= (0x4);

    uint16_t anterior = timeScan;

    for (;;)
    {
        /* Start scan of channel 9 of touch */
        TSI0_BASE_PTR->DATA = (TSI0_BASE_PTR->DATA | 0x90000000);
        TSI0_BASE_PTR->DATA = (TSI0_BASE_PTR->DATA | 0x00400000);

        while (!(TSI0_BASE_PTR->GENCS & 4))
            ;

        uint16_t timeScan2 = (uint16_t)TSI0_BASE_PTR->DATA;
        uint16_t d = timeScan2 - timeScan;

        if (timeScan2 > timeScan + 50)
        {
            if (d > anterior)
            { // right
                if (colorAnterior == GREEN)
                {
                    PTB_BASE_PTR->PSOR = 1 << 19; // turn off previous
                }
                else if (colorAnterior == RED)
                {
                    PTB_BASE_PTR->PSOR = 1 << 18; // turn off previous
                }
                PTB_BASE_PTR->PCOR = 1 << 18; // turn on
                colorAnterior = RED;
            }
            else
            { // left
                if (colorAnterior == GREEN)
                {
                    PTB_BASE_PTR->PSOR = 1 << 19; // turn off previous
                }
                else if (colorAnterior == RED)
                {
                    PTB_BASE_PTR->PSOR = 1 << 18; // turn off previous
                }
                PTB_BASE_PTR->PCOR = 1 << 19; // turn on
                colorAnterior = GREEN;
            }
        }
        else
        {
            if (colorAnterior == RED)
            {
                PTB_BASE_PTR->PSOR = 1 << 18; // turn off previous
                PTB_BASE_PTR->PCOR = 1 << 18; // turn on
                colorAnterior = RED;
            }
            else if (colorAnterior == GREEN)
            {
                PTB_BASE_PTR->PSOR = 1 << 19; // turn off previous
                PTB_BASE_PTR->PCOR = 1 << 19; // turn on
                colorAnterior = GREEN;
            }
        }
        anterior = d;
        TSI0_BASE_PTR->GENCS |= 0x00000004; // Clear EOSF GENCS
        wait();                             // Time interval
    }

    return 0;
}
