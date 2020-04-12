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
#include "cli.h"
#include <stdio.h>
#include "oled.h"
/* TODO:  Include other files here if needed. */


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

enum CommandSectionCompleteStatus
{
    CLI_COMMAND_SECTION_NEW,
    CLI_COMMAND_SECTION_CHANNEL,
    CLI_COMMAND_SECTION_CHANNEL_ADD,
    CLI_COMMAND_SECTION_CHANNEL_REMOVE,
    CLI_COMMAND_SECTION_CHANNEL_THRU,
    CLI_COMMAND_SECTION_OFFSET,
    CLI_COMMAND_SECTION_OFFSET_ENTERED,
    CLI_COMMAND_SECTION_VALUE,
    CLI_COMMAND_SECTION_VALUE_ENTERED,
    CLI_COMMAND_SECTION_VALUE_INCREMENT,
    CLI_COMMAND_SECTION_VALUE_INCREMENT_ENTERED,
    CLI_COMMAND_SECTION_VALUE_DECREMENT,
    CLI_COMMAND_SECTION_VALUE_DECREMENT_ENTERED,
    CLI_COMMAND_SECTION_VALUE_THRU,
    CLI_COMMAND_SECTION_VALUE_THRU_ENTERED,
    CLI_COMMAND_SECTION_TIME,
    CLI_COMMAND_SECTION_TIME_ENTERED,
    CLI_COMMAND_SECTION_RECORD,
    CLI_COMMAND_SECTION_RECORD_ENTERED,
    CLI_COMMAND_SECTION_PLAYBACK,
    CLI_COMMAND_SECTION_PLAYBACK_ENTERED,
    CLI_COMMAND_SECTION_COMPLETE,
    CLI_COMMAND_COMPLETE,
    CLI_COMMAND_SECTION_ERROR
};

enum commandActionType
{
    CLI_ACTION_SET_CHANNEL_VALUES,
    CLI_ACTION_RECORD_PRESET,
    CLI_ACTION_PLAY_PRESET
};

struct CLI_Data {
    uint16_t command[CLI_MAX_ITEMS];
    uint8_t counter;
    uint16_t chLow;
    uint16_t chHigh;
    uint8_t* values;
};

struct CLI_Data cliData;

uint8_t presetData[CLI_PRESET_COUNT][512];
int16_t fadeCoefficient[512];
uint16_t fadeTracker[512];
uint16_t fadeWaitTicks;
uint16_t fadeWaitTracker;
//uint16_t concat[CLI_MAX_ITEMS];
//struct CommandSectionData csData[CLI_MAX_ITEMS];

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



/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */
    void CLI_AddItem(uint16_t function)
    {
        bool counterWasZero = cliData.counter == 0;
        if(function > CLI_COMMAND_START)
        {
            cliData.command[cliData.counter] = function;
            cliData.counter++;
        }
        else
        {
            if(counterWasZero)
                cliData.counter++;
            
            uint16_t result = (cliData.command[cliData.counter - 1] * 10)
                    + function;
            if(result > CLI_COMMAND_START)
            {
                cliData.command[cliData.counter] = function;
                cliData.counter++;
            }
            else
            {
                cliData.command[cliData.counter - 1] = result;
            }
        }
    }
    
    void CLI_RemoveLastItem()
    {
        if(cliData.counter == 0)
            return;
        if(cliData.command[cliData.counter - 1] > CLI_COMMAND_START)
        {
            cliData.command[cliData.counter - 1] = 0;
            cliData.counter--;
        }
        else
        {
            cliData.command[cliData.counter - 1] = 
                    cliData.command[cliData.counter - 1] / 10;
            if(cliData.command[cliData.counter - 1] == 0)
                cliData.counter--;
        }
    }
    
    void CLI_Clear()
    {
        for(cliData.counter = 0; 
                cliData.counter < CLI_MAX_ITEMS; cliData.counter++)
            cliData.command[cliData.counter] = 0;
        cliData.counter = 0;
    }
       
    bool CLI_ProcessCommand()
    {
        uint16_t i, j;
        bool secActiveChannels[513];
        bool activeChannels[513];
        uint8_t chVals[513];
        bool useTXActiveChannels = false;
        int16_t thruPrefix = CLI_COMMAND_START;
        uint8_t lowVal = 0;
        uint8_t highVal = 0;
        int16_t valIncDec = 0;
        uint16_t offset = 1;
        uint8_t time = 0;
        uint8_t presetNum = 0;
        enum commandActionType cmdAction = CLI_ACTION_SET_CHANNEL_VALUES;
        enum CommandSectionCompleteStatus cmdStatus = CLI_COMMAND_SECTION_NEW;
        
        for(i = 0; i < cliData.counter; i++)
        {
            switch(cmdStatus)
            {
                case CLI_COMMAND_SECTION_NEW:
                    for(j = 0; j < 513; j++)
                    {
                        secActiveChannels[j] = false;
                        activeChannels[j] = false;
                        chVals[j] = 0;
                    }
                        
                    lowVal = 0;
                    highVal = 0;
                    offset = 1;
                    time = 0;
                    presetNum = 0;
                    thruPrefix = CLI_COMMAND_START;
                    useTXActiveChannels = false;
                    valIncDec = 0;
                    cmdAction = CLI_ACTION_SET_CHANNEL_VALUES;
                    switch(cliData.command[i])
                    {
                        case Record:
                            cmdStatus = CLI_COMMAND_SECTION_RECORD;
                            break;
                        case Preset:
                            cmdStatus = CLI_COMMAND_SECTION_PLAYBACK;
                            break;
                        case Minus:
                            useTXActiveChannels = true;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_DECREMENT;
                            break;
                        case Plus:
                            useTXActiveChannels = true;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_INCREMENT;
                            break;
                        case SWITCH_VALID_CHANNELS:
                            thruPrefix = cliData.command[i];
                            secActiveChannels[cliData.command[i]] = true;
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                            break;
                        case Thru:
                            thruPrefix = 1;
                            secActiveChannels[cliData.command[i]] = true;
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_THRU;
                            break;
                        case At:
                            useTXActiveChannels = true;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE;
                            break;
                        case Full:
                            useTXActiveChannels = true;
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        case Enter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                            
                    }
                    break;
                case CLI_COMMAND_SECTION_CHANNEL:
                    switch(cliData.command[i])
                    {
                        case Offset:
                            cmdStatus = CLI_COMMAND_SECTION_OFFSET;
                            break;
                        case Minus:
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_REMOVE;
                            break;
                        case Plus:
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_ADD;
                            break;
                        case Thru:
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_THRU;
                            break;
                        case At:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE;
                            break;
                        case Full:
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_CHANNEL_ADD:
                    if(cliData.command[i] > 0 && cliData.command[i] < 513)
                    {
                        thruPrefix = cliData.command[i];
                        secActiveChannels[cliData.command[i]] = true;
                        cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                    }
                    else
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    break;
                case CLI_COMMAND_SECTION_CHANNEL_REMOVE:
                    if(cliData.command[i] > 0 && cliData.command[i] < 513)
                    {
                        thruPrefix = cliData.command[i] * -1;
                        secActiveChannels[cliData.command[i]] = false;
                        cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                    }
                    else
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    break;
                case CLI_COMMAND_SECTION_CHANNEL_THRU:
                    if(thruPrefix == CLI_COMMAND_START)
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    else
                    {
                        switch(cliData.command[i])
                        {
                            case Offset:
                                for(j = thruPrefix; j <= 512; j++)
                                {
                                    secActiveChannels[j] = true;
                                }
                                cmdStatus = CLI_COMMAND_SECTION_OFFSET;
                                break;
                            case SWITCH_VALID_CHANNELS:
                                if(thruPrefix > cliData.command[i])
                                    for(j = cliData.command[i]; j <= thruPrefix; j++)
                                    {
                                        secActiveChannels[j] = true;
                                    }
                                else
                                    for(j = thruPrefix; j <= cliData.command[i]; j++)
                                    {
                                        secActiveChannels[j] = true;
                                    }
                                cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                                break;
                            case At:
                                for(j = thruPrefix; j <= 512; j++)
                                {
                                    secActiveChannels[j] = true;
                                }
                                cmdStatus = CLI_COMMAND_SECTION_VALUE;
                                break;
                            case Full:
                                for(j = thruPrefix; j <= 512; j++)
                                {
                                    secActiveChannels[j] = true;
                                }
                                lowVal = 255;
                                highVal = 255;
                                cmdStatus = CLI_COMMAND_COMPLETE;
                                break;
                            default:
                                cmdStatus = CLI_COMMAND_SECTION_ERROR;
                                break;
                        }
                    }
                    break;
                case CLI_COMMAND_SECTION_OFFSET:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_CHANNELS:
                            offset = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_OFFSET_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_OFFSET_ENTERED:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    switch(cliData.command[i])
                    {
                        case At:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE;
                            break;
                        case Full:
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                            
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PERCENT_VALUES:
                            lowVal = highVal = cliData.command[i] * 2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_ENTERED;
                            break;
                        case Minus:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_DECREMENT;
                            break;
                        case Plus:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_INCREMENT;
                            break;
                        case Thru:
                            //LowVal will already be set
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_THRU;
                            break;
                        case Full:
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;                            
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_ENTERED:
                    switch(cliData.command[i])
                    {
                        case Time:
                            cmdStatus = CLI_COMMAND_SECTION_TIME;
                            break;
                        case Plus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case Thru:
                            //LowVal will already be set
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_THRU;
                            break;
                        case Enter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;                            
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_INCREMENT:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;                    
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PERCENT_VALUES:
                            valIncDec = cliData.command[i] * 2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_INCREMENT_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;   
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_INCREMENT_ENTERED:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;                    
                    switch(cliData.command[i])
                    {
                        case Plus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case Enter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;   
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_DECREMENT:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;                    
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PERCENT_VALUES:
                            valIncDec = cliData.command[i] * -2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_DECREMENT_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;   
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_DECREMENT_ENTERED:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;                    
                    switch(cliData.command[i])
                    {
                        case Plus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case Enter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;   
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_THRU:
                    switch(cliData.command[i])
                    {
                        case Time:
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_SECTION_TIME;
                            break;
                        case Plus:
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case SWITCH_VALID_PERCENT_VALUES:
                            if(cliData.command[i] > lowVal)
                                highVal = cliData.command[i] * 2.55;
                            else
                                lowVal = cliData.command[i] * 2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_THRU_ENTERED;
                            break;
                        case Full:
                        case Enter:
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_THRU_ENTERED:
                    switch(cliData.command[i])
                    {
                        case Time:
                            cmdStatus = CLI_COMMAND_SECTION_TIME;
                            break;
                        case Plus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case Enter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_TIME:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_TIME:
                            time = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_TIME_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_TIME_ENTERED:
                    switch(cliData.command[i])
                    {
                        case Enter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_RECORD:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PRESETS:
                            presetNum = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_RECORD_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_RECORD_ENTERED:
                    switch(cliData.command[i])
                    {
                        case Enter:
                            cmdAction = CLI_ACTION_RECORD_PRESET;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_PLAYBACK:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PRESETS:
                            presetNum = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_PLAYBACK_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_PLAYBACK_ENTERED:
                    switch(cliData.command[i])
                    {
                        case Enter:
                            cmdAction = CLI_ACTION_PLAY_PRESET;
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_COMPLETE:
                case CLI_COMMAND_COMPLETE:
                case CLI_COMMAND_SECTION_ERROR:
                    //All of this is done after the switch
                    break;    
            }
            
            if(cmdStatus == CLI_COMMAND_SECTION_COMPLETE || cmdStatus == CLI_COMMAND_COMPLETE)
            {
                if(cmdAction == CLI_ACTION_SET_CHANNEL_VALUES)
                {
                    uint32_t fanAmount;
                    uint16_t k = 0, l = offset, secActiveChannelCount = 0;
                    if((lowVal || highVal) && valIncDec)    //Setting literal values AND Inc/Decrementing
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    else
                    {
                        if(useTXActiveChannels)
                        {
                            for(j = 1; j < 513; j++)
                            {
                                if(*(cliData.values + j))
                                {
                                    secActiveChannels[j] = true;
                                    if(!activeChannels[j])
                                        chVals[j] = *(cliData.values + j);
                                }
                            }
                        }
                        for(j = 1; j < 513; j++)
                        {
                            if(secActiveChannels[j])
                            {
                                if(l == offset)
                                {
                                    activeChannels[j] = true;
                                    secActiveChannelCount++;
                                    l = 0;
                                }
                                l++;
                            }
                        }
                        if(valIncDec == 0)
                        {
                            if(secActiveChannelCount == 1 || secActiveChannelCount == 0)
                                fanAmount = 0;
                            else
                            {
                                fanAmount = ((highVal - lowVal) * 10000) / (secActiveChannelCount - 1);
                            }
                            for(j = 0; j < 513; j++)
                            {
                                if(secActiveChannels[j])
                                {
                                    chVals[j] = lowVal + ((fanAmount * k) / 10000);
                                    k++;
                                }
                            }
                        }
                        else
                        {
                            for(j = 0; j < 513; j++)
                            {
                                if(secActiveChannels[j])
                                {
                                    if(chVals[j] < valIncDec)
                                        chVals[j] = 0;
                                    chVals[j] = chVals[j] + valIncDec;
                                }
                            }
                        }
                    }
                }
                
                if(time); 
                //Currently unused variables, need to be acted upon
            }
            if(cmdStatus == CLI_COMMAND_COMPLETE || 
                    cmdStatus == CLI_COMMAND_SECTION_ERROR)
                break;
        }
        
        
        
        if(cmdStatus == CLI_COMMAND_SECTION_ERROR)
            return false;
        
        if(cmdStatus == CLI_COMMAND_COMPLETE)
        {
            TMR3_Stop();
            if(time == 0)
            {
                switch(cmdAction)
                {
                    case CLI_ACTION_SET_CHANNEL_VALUES:
                        for(j = 1; j < 513; j++)
                        {
                            if(activeChannels[j])
                                *(cliData.values + j) = chVals[j];
                        }
                        break;
                    case CLI_ACTION_PLAY_PRESET:
                        for(j = 1; j < 513; j++)
                        {
                            *(cliData.values + j) = presetData[presetNum][j];
                        }
                        break;
                    case CLI_ACTION_RECORD_PRESET:
                        for(j = 1; j < 513; j++)
                        {
                            presetData[presetNum][j] = *(cliData.values + j);
                        }
                        break;
                    default:
                        break;
                }
            }
            else
            {
                fadeWaitTracker = 0;
                fadeWaitTicks = time;
                switch(cmdAction)
                {
                    case CLI_ACTION_SET_CHANNEL_VALUES:
                        for(j = 1; j < 513; j++)
                        {
                            fadeTracker[j - 1] = *(cliData.values + j) * CLI_TICKS_PER_SECOND;
                            if(!activeChannels[j])
                                fadeCoefficient[j - 1] = 0;
                            else
                                fadeCoefficient[j - 1] = 
                                        chVals[j] - *(cliData.values + j);
                        }
                        break;
                    case CLI_ACTION_PLAY_PRESET:
                        for(j = 1; j < 513; j++)
                        {
                            fadeTracker[j - 1] = *(cliData.values + j) * CLI_TICKS_PER_SECOND;
                            fadeCoefficient[j - 1] = 
                                    presetData[presetNum][j] -  *(cliData.values + j);
                        }
                        break;
                    default:
                        break;
                }
                TMR3_Start();
            }
        }
        return true;
    }
    
    char* CLI_FunctionToString(uint16_t function, char* str)
    {
        switch(function)
        {
            case Thru:
                strcpy(str, "Thru");
                break;
            case At:
                strcpy(str, "At");
                break;
            case Full:
                strcpy(str, "Full *");
                break;
            case Enter:
                strcpy(str, "*");
                break;
            case Bksp:
                strcpy(str, "Bksp");
                break;
            case Clear:
                strcpy(str, "Clear");
                break;
            case Plus:
                strcpy(str, "Plus");
                break;
            case Minus:
                strcpy(str, "Minus");
                break;
            case Last:
                strcpy(str, "Last");
                break;
            case Next:
                strcpy(str, "Next");
                break;
            case Record:
                strcpy(str, "Record");
                break;
            case Preset:
                strcpy(str, "Preset");
                break;
            case Offset:
                strcpy(str, "Offset");
                break;
            case Time:
                strcpy(str, "Time");
                break;
            default:
                itoa(str, function, 10);
                break;
        }
        return str;
    }
    
    void CLI_PrintCommand()
    {
        char string[64] = "";
        uint8_t i, j;
        for(i = 0; i < cliData.counter; i++)
        {
            CLI_FunctionToString(cliData.command[i], string + strlen(string));
            j = strlen(string);
            string[j] = ' ';
            string[j + 1] = '\0';
        }
        OLED_ClearLine(1);
        OLED_ClearLine(2);
        OLED_ClearLine(3);
        OLED_String(string, strlen(string), 0, 1);
        OLED_DrawScreen();
    }
    
    void CLI_PrintError (uint16_t function)
    {
        OLED_ClearLine(3);
        char string[15] = "Invalid Key: \"";
        OLED_String(string, 14, 0, 3);
        CLI_FunctionToString(function, string);
        OLED_String(string, strlen(string), 84, 3);
        OLED_String("\"", 1, 84 + (strlen(string) * 6), 3);        
        OLED_DrawScreen();
    }
    
    void CLI_Timer_Callback (uint32_t status, uintptr_t context)
    {
        uint16_t j;
        fadeWaitTracker++;
        if(!(fadeWaitTracker % fadeWaitTicks))
        {
            for(j = 1; j < 513; j++)
            {
                fadeTracker[j - 1] += fadeCoefficient[j - 1];
                *(cliData.values + j) = fadeTracker[j - 1] >> 8;
            }
        }
        if(fadeWaitTracker == fadeWaitTicks * CLI_TICKS_PER_SECOND)
            TMR3_Stop();
    }
    


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
    void CLI_Init(uint8_t* dmxBuf)
    {
        for(cliData.counter = 0; cliData.counter < 5; cliData.counter++)
            cliData.command[cliData.counter] = 0;
        cliData.counter = 0;
        cliData.chLow = 1;
        cliData.chHigh = 512;
        cliData.values = dmxBuf;
        
        TMR3_CallbackRegister(CLI_Timer_Callback, (uintptr_t) NULL);
    }
    
    void CLI_AddToCommand(uint16_t function)
    {
        if(function == Clear)
        {
            CLI_Clear();
            CLI_PrintCommand();
        }
        else if(function == Bksp)
        {
            CLI_RemoveLastItem();
            CLI_PrintCommand();
        }
        else if(function == Full || function == Enter)
        {
            CLI_AddItem(function);
            if(CLI_ProcessCommand())
            {
                CLI_PrintCommand();
            }
            else
                CLI_PrintError(function);
            cliData.counter = 0;
            cliData.command[0] = 0;
        }
        else if(function == Last || function == Next)
        {
            //Add the last and next functions
        }
        else
        {
            if(cliData.counter != CLI_MAX_ITEMS)
            {
                CLI_AddItem(function);
                if(CLI_ProcessCommand())
                    CLI_PrintCommand();
                else
                {
                    CLI_RemoveLastItem();
                    CLI_PrintError(function);
                }
            }
            else
                CLI_PrintError(function);
        }
    }

    


/* *****************************************************************************
 End of File
 */
