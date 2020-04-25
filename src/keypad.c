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
#include "keypad.h"
#include "cli.h"
#include "UI.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

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

enum ButtonState
{
    KEYPAD_BUTTON_RELEASED,
    KEYPAD_BUTTON_PRESSED,
    KEYPAD_BUTTON_ACTIVATED,
    KEYPAD_BUTTON_PROCESSED
};

enum ButtonState buttonStates[24];
uint8_t buttonCounter[24];
uint32_t rawKeypadData;

uint16_t keypadMapping[] = 
{
    8, 5, 2, 0, Clear, 1, 4, 7,                             // 1A - H
    Thru, At, Full, Enter, Bksp, 3, 6, 9,                   // 2A - H
    Preset, Time, Next, Plus, Minus, Last, Offset, Record   // 3A - H
};


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

static void TMR2Callback (uint32_t status, uintptr_t context)
{
    TMR2_Stop();
    while(!UI_SetActiveDevice(UI_DEVICE_KEYPAD));
    KEYPAD_PL_Set();
    SPI2_Read(&rawKeypadData, 3);
}

void Keypad_SPICallback (uintptr_t context)
{
    uint8_t i;

    KEYPAD_PL_Clear();

    for(i = 0; i < 24; i++)
    {
        if(((rawKeypadData >> i) & 1) == 0 && 
                buttonStates[i] != KEYPAD_BUTTON_ACTIVATED &&
                buttonStates[i] != KEYPAD_BUTTON_PROCESSED)
        {
            buttonStates[i] = KEYPAD_BUTTON_PRESSED;
            buttonCounter[i]++;
        }
        else if(((rawKeypadData >> i) & 1) == 1)
        {
            buttonStates[i] = KEYPAD_BUTTON_RELEASED;
            buttonCounter[i] = 0;
        }
        if(buttonCounter[i] == DEBOUNCE_COUNT)
        {
            buttonStates[i] = KEYPAD_BUTTON_ACTIVATED;
            buttonCounter[i] = 0;
        }
    }
    
    TMR2_Start();
}

void Keypad_ProcessButtonPress()
{
    uint8_t i;
    for(i = 0; i < 24; i++)
    {
        if(buttonStates[i] == KEYPAD_BUTTON_ACTIVATED)
        {
            CLI_AddToCommand(keypadMapping[i]);
            buttonStates[i] = KEYPAD_BUTTON_PROCESSED;
            break;
        }
    }
}

void Keypad_Init()
{
    TMR2_CallbackRegister(TMR2Callback, (uintptr_t)NULL);
    
    uint8_t i;
    for(i = 0; i < 24; i++)
    {
        buttonStates[i] = KEYPAD_BUTTON_RELEASED;
        buttonCounter[i] = 0;
    }
    
    KEYPAD_PL_Clear();
    TMR2_Start();
}


/* *****************************************************************************
 End of File
 */
