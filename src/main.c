/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "main.h"

uint8_t __attribute__ ((coherent)) channelBuffer[513];

uint16_t channelCounter = 513;
uint8_t inc;
uint8_t dummyData;

void UART2RXCallback()
{
    if(U2STA & _U2STA_FERR_MASK)
    {
        dummyData = (uint8_t)(U2RXREG);
        channelCounter = 0;
        inc++;
        if(inc == 20) __builtin_software_breakpoint();
    }
    while(U2STA & _U2STA_URXDA_MASK)
    {
        if(channelCounter != 513)
            channelBuffer[channelCounter++] = (uint8_t)(U2RXREG);
        else dummyData = (uint8_t)(U2RXREG);
    }

    /* Clear UART2 RX Interrupt flag after reading data buffer */
    IFS4CLR = _IFS4_U2RXIF_MASK;
}

void UART2ErrorCallback()
{
    UART2RXCallback();
    UART2_ErrorGet();
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    P2_DIR_CTRL_Clear();

    //UART2 Interrupt Enables
    IEC4SET = _IEC4_U2EIE_MASK;
    IEC4SET = _IEC4_U2RXIE_MASK;
    
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

/*******************************************************************************
 End of File
*/

