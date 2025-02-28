#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "f73-rncontrol-lib/uart.h"
#include "f73-rncontrol-lib/adc.h"
#include "f73-rncontrol-lib/button.h"
#include "f73-rncontrol-lib/led.h"
#include "f73-rncontrol-lib/counter.h"

#include "can.h"

volatile uint32_t timeStep;

#define ztime 100 // Zykluszeit

int main()
{

    can_t sendmsg;

    sendmsg.id = 0x0F;
    sendmsg.length = 2;
    sendmsg.data[0] = 0xff;
    sendmsg.data[1] = 0x11;

    uint32_t TimeStepOld = 0;
    uint32_t NextTimeStep = 0;

    ledInit();
    uartInit(57600, 8, 'N', 1);
    can_init(BITRATE_500_KBPS); // CAN init 500 kbit/s

    // Counter 0 CTC Mode 1ms
    counter0SetCompare(249);
    counter0EnableCompareMatchInterrupt();
    counter0EnableCTC; // set CTC mode
    counter0Start(ATMEGA32_COUNTER_0_PRESCALER_64);

    sei();

    for (;;)
    {

            while (TimeStepOld!=NextTimeStep)
            {

                cli();
                TimeStepOld = timeStep; // get copy in atomic operation (not interrupted by ISR)
                sei();
                /* code */ // to do Sleep
            }
            
        
            uint8_t status = can_send_message(&sendmsg);
            if (!status)
            { // did we send something?
                printf("Nachricht konnte nicht gesendet werden.\r\n");
            }
            else
            {
                printf("%8lu ms SendCanMsg: ID 0x%02x; Length %2d; Data: ", TimeStepOld, sendmsg.id, sendmsg.length);
                for (uint8_t i = 0; i < sendmsg.length; i++)
                {
                    printf("0x%02x ", sendmsg.data[i]);
                }

                printf("\r\n");

            }

            NextTimeStep=TimeStepOld+ztime;
        
    }
    return 0;
}

ISR(TIMER0_COMP_vect)
{
    timeStep++;
}
