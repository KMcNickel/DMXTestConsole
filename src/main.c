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

uint8_t __attribute__ ((coherent)) channelBufferA [513];
uint8_t __attribute__ ((coherent)) channelBufferB [513];

enum DMXFrameStep TXStep = BREAK;
enum DMXFrameStep RXStep = BREAK;
void *TXSrcAddr = (uint8_t *) channelBufferA;
void *RXDestAddr = (uint8_t *) channelBufferB;
size_t size = 513;
uint8_t dummyData;

void ResetTimer1(uint16_t period)
{
    TMR1 = 0x0;
    TMR1_PeriodSet(period);
    TMR1_Start();
}

void StartBreak()
{
    TMR1_Stop();
    ResetTimer1(1150);
    P1_BREAK_CTRL_OutputEnable();
    P1_BREAK_CTRL_Clear();
}

static void TMR1Callback (uint32_t status, uintptr_t context)
{
    switch(TXStep)
    {
        case BREAK: //92uS
            TMR1_Stop();
            ResetTimer1(150);
            TXStep = MAB;
            P1_BREAK_CTRL_Set();
            P1_BREAK_CTRL_InputEnable();
            break;
        case MAB:   //12uS
            TMR1_Stop();
            IEC3SET = _IEC3_U1TXIE_MASK;    //Enable UART Interrupt
            DMAC_ChannelTransfer(DMAC_CHANNEL_0, TXSrcAddr, size, (uint8_t *) &U1TXREG, 1, 1);
            TXStep = CHANNELS;
            break;
        default:
            break;
    }
}

void DMAC0_Callback (DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if(event == DMAC_TRANSFER_EVENT_ERROR ) //On error, set the LED and re-try sending the frame
    {
        TXStep = BREAK;
        StartBreak();
        LED1_Set();
        return;
    }
    TXStep = DMACOMPLETE;
}

void UART1_Callback()
{
    IFS3CLR = _IFS3_U1TXIF_MASK;    //Clear the Interrupt Flag
    if(UART1_ErrorGet() != UART_ERROR_NONE) //On error, set the LED and re-try sending the frame
    {
        TXStep = BREAK;
        StartBreak();
        LED1_Set();
        return;
    }
    if(TXStep == DMACOMPLETE && (U1STA & _U1STA_TRMT_MASK)) 
    {
        IEC3CLR = _IEC3_U1TXIE_MASK;    //Disable UART Interrupt
        TXStep = BREAK;
        StartBreak();
    }
}

void TranslateAndFlipBuffers(uint16_t channels)
{
    int i;
    //Translate Ch 512 to all channels
    for(i = 1; i < channels; i++) ((uint8_t *)RXDestAddr)[i] = ((uint8_t *)RXDestAddr)[512];
    void *BufA = RXDestAddr;       //Flip the buffers
    RXDestAddr = TXSrcAddr;
    TXSrcAddr = BufA;
}

void DMAC1_Callback (DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    TranslateAndFlipBuffers(512);
}

void UART2RXCallback()
{
    if(U2STA & _U2STA_FERR_MASK)
    {
        dummyData = (uint8_t)(U2RXREG);
        DMAC_ChannelTransfer(DMAC_CHANNEL_1, (void *) &U2RXREG, 1, (uint8_t *) RXDestAddr, 513, 1);
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
    
    P1_DIR_CTRL_Set();
    P2_DIR_CTRL_Clear();

    P1_BREAK_CTRL_OutputEnable();
    P1_BREAK_CTRL_Clear();
    TXStep = BREAK;
    
    TMR1_CallbackRegister(TMR1Callback, (uintptr_t)NULL);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, DMAC0_Callback, (uintptr_t)NULL);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_1, DMAC1_Callback, (uintptr_t)NULL);
    //UART Callback is registered in the interrupts file
    
    //UART2 Interrupt Enables
    IEC4SET = _IEC4_U2EIE_MASK;
    IEC4SET = _IEC4_U2RXIE_MASK;
    
    //Don't overwrite buffer[0] as it is the NULL start code
//    uint16_t i;
//    for(i = 1; i < 513; i++) channelBufferA[i] = 0x55;
//    for(i = 1; i < 513; i++) channelBufferB[i] = 0xAA;
    
    TMR1_Start();
    
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

