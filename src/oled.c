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

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "main.h"
#include "oled.h"
#include "font_table.h"
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

enum OLED_States 
{
    OLED_STATE_Idle,
    OLED_STATE_InitList,
    OLED_STATE_DrawScreen,
    OLED_STATE_DispOn,
    OLED_STATE_Waiting
};

enum OLED_DrawStates
{
    OLED_STATE_DrawIdle,
    OLED_STATE_SetParams,
    OLED_STATE_Data
};

#define OLED_PixelCount 128*4
uint8_t OLED_Buffer[OLED_PixelCount];
enum OLED_States OLED_Sys_State;
enum OLED_DrawStates OLED_Draw_State;
bool initialized;
uint8_t page;
uint8_t column;

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




void OLED_CommandArray_Write(uint8_t * cmd, uint16_t size)
{
    //while(!UI_SetActiveDevice(UI_DEVICE_OLED));
//    OLED_DC_Clear();
//    OLED_CS_Clear();
//    SPI2_Write(&cmd, size);
    I2C2_Write(SSD1305_ADDRESS, cmd, size);
}

void OLED_Command_Write(uint8_t cmd)
{
//    OLED_CommandArray_Write(&cmd, 1);
    uint8_t dataOut[2] = {0x00, cmd};
    I2C2_Write(SSD1305_ADDRESS, dataOut, 2);
}

void OLED_DataArray_Write(uint8_t * data, uint16_t size)
{
    //while(!UI_SetActiveDevice(UI_DEVICE_OLED));
//    OLED_DC_Set();
//    OLED_CS_Clear();
//    SPI2_Write(&data, size);
    I2C2_Write(SSD1305_ADDRESS, data, size);
}

void OLED_Data_Write(uint8_t data)
{
//    OLED_DataArray_Write(&data, size);
    uint8_t dataOut[2] = {0x40, data};
    I2C2_Write(SSD1305_ADDRESS, dataOut, 2);
}

uint8_t initList[] = 
{
    0x00,
    SSD1305_DISPLAYOFF,
    SSD1305_SETDISPLAYCLOCKDIV,
    0x10,
    SSD1305_SETMULTIPLEX,
    0x1F,
    SSD1305_SETDISPLAYOFFSET,
    0x00,
    SSD1305_SETSTARTLINE | 0x0,
    SSD1305_MASTERCONFIG,
    0x8E,
    SSD1305_SETAREACOLOR,
    0x05,
    SSD1305_MEMORYMODE,
    0x02,
    SSD1305_SEGREMAP | 0x01,
    SSD1305_COMSCANDEC,
    SSD1305_SETCOMPINS,
    0x12,
    SSD1305_SETLUT,
    0x3F,
    0x3F,
    0x3F,
    0x3F,
    SSD1305_SETCONTRAST,
    0xBF,
    SSD1305_SETBRIGHTNESS,
    0xBF,
    SSD1305_SETPRECHARGE,
    0xD2,
    SSD1305_SETVCOMLEVEL,
    0x08,
    SSD1305_DISPLAYALLON_RESUME,
    SSD1305_NORMALDISPLAY
};

uint8_t drawList[] = 
{
    0x00,
    SSD1305_SETPAGESTART,
    SSD1305_SETLOWCOLUMN + 4,
    SSD1305_SETHIGHCOLUMN
};

void OLED_Checkerboard()
{
    uint16_t i;
    for(i = 0; i < OLED_PixelCount; i++)
    {
        if(i%2)
            OLED_Buffer[i] = 0x55;
        else
            OLED_Buffer[i] = 0xAA;
    }
}

uint8_t* GetCharFromASCII(uint8_t ascii)
{
    if(ascii >= 0x20 && ascii <= 0x7D)
        return font_table[ascii - 0x20];
    else return no_char;
}

void OLED_Char(uint8_t* character, uint8_t column, uint8_t page)
{
    uint8_t i;
    for(i = 0; i < 5; i++)
        OLED_Buffer[i + (column + (page * 128))] = *(character + i);
}

void OLED_CharASCII(char character, uint8_t column, uint8_t page)
{
    OLED_Char(GetCharFromASCII(character), column, page);
}

void OLED_String(char* str, uint8_t len, uint8_t column, uint8_t page)
{
    char* follower = str;
    while(follower != str + len)
    {
        OLED_CharASCII(*follower, column, page);
        column += 6;
        if(column > 122)
        {
            page++;
            column = 0;
            if(*(follower + 1) == ' ')
                follower++;
        }
        follower++;
    }
}

void OLED_Blank()
{
    uint16_t i;
    for(i = 0; i < OLED_PixelCount; i++)
        OLED_Buffer[i] = 0;
}

void OLED_ClearLine(uint8_t line)
{
    uint16_t i, lineStart = line * 128;
    for(i = lineStart; i < lineStart + 128; i++)
        OLED_Buffer[i] = 0;
}

void OLED_Fill()
{
    uint16_t i;
    for(i = 0; i < OLED_PixelCount; i++)
        OLED_Buffer[i] = 0xFF;
}

bool OLED_UpdateScreen()
{
    switch(OLED_Draw_State)
    {
        case OLED_STATE_DrawIdle:
            page = column = 0;
            OLED_Draw_State = OLED_STATE_SetParams;
        case OLED_STATE_SetParams:
            drawList[1] = SSD1305_SETPAGESTART | page;
            OLED_CommandArray_Write(drawList, sizeof(drawList));
            OLED_Draw_State = OLED_STATE_Data;
            break;
        case OLED_STATE_Data:
            //OLED_DataArray_Write(OLED_Buffer + (page * 128), 128);
            OLED_Data_Write(*(OLED_Buffer + (page * 128) + column));
            column++;
            if(column == 128)
            {
                page++;
                column = 0;
                if(page != 4)
                    OLED_Draw_State = OLED_STATE_SetParams;
                else
                {
                    OLED_Draw_State = OLED_STATE_DrawIdle;
                    return true;
                }
            }
    }
    return false;
}

bool OLED_DrawScreen()
{
    if(OLED_Sys_State != OLED_STATE_Idle)
        return false;
    OLED_Sys_State = OLED_STATE_DrawScreen;
    OLED_UpdateScreen();
    return true;
}

void OLED_SPICallback(uintptr_t context)
{
    OLED_CS_Set();
    switch(OLED_Sys_State)
    {
        case OLED_STATE_Idle:
            break;
        case OLED_STATE_InitList:
            OLED_Blank();
            OLED_Sys_State = OLED_STATE_DrawScreen;
        case OLED_STATE_DrawScreen:
            if(OLED_UpdateScreen())
            {
                if(initialized)
                    OLED_Sys_State = OLED_STATE_Idle;
                else
                    OLED_Sys_State = OLED_STATE_DispOn;
            }
            break;
        case OLED_STATE_DispOn:
            OLED_Command_Write(SSD1305_DISPLAYON);
            OLED_Sys_State = OLED_STATE_Waiting;
            break;
        case OLED_STATE_Waiting:
            initialized = true;
            OLED_Sys_State = OLED_STATE_Idle;
    }
}

void OLED_Init()
{
    OLED_RST_Clear();
    uint16_t i;
    for(i = 0; i != 65535; i++);
    OLED_RST_Set();
    
    OLED_Sys_State = OLED_STATE_InitList;
    
    //SPI2_CallbackRegister(OLED_SPICallback, (uintptr_t)NULL);
    I2C2_CallbackRegister(OLED_SPICallback, (uintptr_t)NULL);
    
    OLED_CommandArray_Write(initList, sizeof(initList));
}

bool OLED_IsReady()
{
    return initialized & (OLED_Sys_State == OLED_STATE_Idle);
}

/* *****************************************************************************
 End of File
 */