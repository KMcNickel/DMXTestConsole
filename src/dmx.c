/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "dmx.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

enum DMXFrameStep {
    BREAK,
    MAB,
    CHANNELS,
    DMACOMPLETE,
    MBB
};

uint8_t __attribute__ ((coherent)) channelBufferA [513];
uint8_t __attribute__ ((coherent)) channelBufferB [513];

enum DMXFrameStep TXStep = BREAK;
enum DMXFrameStep RXStep = BREAK;
void *TXSrcAddr = (uint8_t *) channelBufferA;
void *RXDestAddr = (uint8_t *) channelBufferB;
size_t size = 513;
uint8_t dummyData;

/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */

/** 
  @Function
    int ExampleLocalFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Description
    Full description, explaining the purpose and usage of the function.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

  @Precondition
    List and describe any required preconditions. If there are no preconditions,
    enter "None."

  @Parameters
    @param param1 Describe the first parameter to the function.
    
    @param param2 Describe the second parameter to the function.

  @Returns
    List (if feasible) and describe the return values of the function.
    <ul>
      <li>1   Indicates an error occurred
      <li>0   Indicates an error did not occur
    </ul>

  @Remarks
    Describe any special behavior not described above.
    <p>
    Any additional remarks.

  @Example
    @code
    if(ExampleFunctionName(1, 2) == 0)
    {
        return 3;
    }
 */


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

// *****************************************************************************

/** 
  @Function
    int ExampleInterfaceFunctionName ( int param1, int param2 ) 

  @Summary
    Brief one-line description of the function.

  @Remarks
    Refer to the example_file.h interface header for function usage details.
 */

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
        return;
    }
    if(TXStep == DMACOMPLETE && (U1STA & _U1STA_TRMT_MASK)) 
    {
        IEC3CLR = _IEC3_U1TXIE_MASK;    //Disable UART Interrupt
        TXStep = BREAK;
        StartBreak();
    }
}

//void TranslateAndFlipBuffers(uint16_t channels)
//{
//    int i;
//    //Translate Ch 512 to all channels
//    for(i = 1; i < channels; i++) ((uint8_t *)RXDestAddr)[i] = ((uint8_t *)RXDestAddr)[512];
//    void *BufA = RXDestAddr;       //Flip the buffers
//    RXDestAddr = TXSrcAddr;
//    TXSrcAddr = BufA;
//}
//
//void DMAC1_Callback (DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
//{
//    TranslateAndFlipBuffers(512);
//}
//
//void UART2RXCallback()
//{
//    if(U2STA & _U2STA_FERR_MASK)
//    {
//        dummyData = (uint8_t)(U2RXREG);
//        DMAC_ChannelTransfer(DMAC_CHANNEL_1, (void *) &U2RXREG, 1, (uint8_t *) RXDestAddr, 513, 1);
//    }
//
//    /* Clear UART2 RX Interrupt flag after reading data buffer */
//    IFS4CLR = _IFS4_U2RXIF_MASK;
//}

//void UART2ErrorCallback()
//{
//    UART2RXCallback();
//    UART2_ErrorGet();
//}

void DMX_Init()
{
    P1_DIR_CTRL_Set();

    P1_BREAK_CTRL_Clear();
    TXStep = BREAK;
    
    //UART2 Interrupt Enables
//    IEC4SET = _IEC4_U2EIE_MASK;
//    IEC4SET = _IEC4_U2RXIE_MASK;
    
    TMR1_CallbackRegister(TMR1Callback, (uintptr_t)NULL);
    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, DMAC0_Callback, (uintptr_t)NULL);
//    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_1, DMAC1_Callback, (uintptr_t)NULL);
    //UART Callback is registered in the interrupts file
    
    TMR1_Start();
}

/* *****************************************************************************
 End of File
 */
