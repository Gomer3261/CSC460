#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

int main(void)
{
    DDRB = 1 << PB7;           /* make the LED pin an output */
    PORTB = 0;

    for(;;){
        _delay_ms(250);  /* max is 262.14 ms / F_CPU in MHz */
        PORTB ^= 1 << PB7;    /* toggle the LED */
    }
    return 0;               /* never reached */
}
