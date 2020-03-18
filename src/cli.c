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
/* TODO:  Include other files here if needed. */


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

struct CLI_Data cliData;

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
        cliData.command[cliData.counter] = function;
        cliData.counter++;
    }
    
    bool CLI_AddNumeric(uint16_t digit, bool channel)
    {
        uint16_t val = (cliData.command[cliData.counter] * 10) + digit;
        if((channel && val >= 1 && val <= 512) || 
           (!channel && val >= 0 && val <= 255))
        {
            cliData.command[cliData.counter] = val;
            cliData.counter++;
            return true;
        }
        return false;
    }
    
    void CLI_RemoveNumeric()
    {
        uint16_t val = (cliData.command[cliData.counter] / 10);
        cliData.command[cliData.counter] = val;
        if(val == 0) cliData.counter--;
    }
    
    void CLI_RemoveItem()
    {
        cliData.command[cliData.counter] = 0;
        cliData.counter--;
    }

    char* CLI_GetString(uint16_t value)
    {
        char* buf = "";
        switch(value)
        {
            case Thru:
                return "Thru";
            case At:
                return "At";
            case Full:
                return "Full";
            case Enter:
                return "*";
            default:
                sprintf(buf, "%d", value);
                return buf;
        }
    }
    
    void CLI_PrintCommand(bool processed)
    {
        char buf[6] = "";
        switch(cliData.counter)
        {
            case 0:
                sprintf(buf, "%s", CLI_GetString(cliData.command[0]));
                break;
            case 1:
                sprintf(buf, "%s %s", 
                        CLI_GetString(cliData.command[0]), 
                        CLI_GetString(cliData.command[1]));
                break;
            case 2:
                sprintf(buf, "%s %s %s", 
                        CLI_GetString(cliData.command[0]), 
                        CLI_GetString(cliData.command[1]), 
                        CLI_GetString(cliData.command[2]));
                break;
            case 3:
                sprintf(buf, "%s %s %s %s", 
                        CLI_GetString(cliData.command[0]), 
                        CLI_GetString(cliData.command[1]), 
                        CLI_GetString(cliData.command[2]), 
                        CLI_GetString(cliData.command[3]));
                break;
            case 4:
                if(processed)
                {
                    sprintf(buf, "%s %s %s %s %s *", 
                            CLI_GetString(cliData.command[0]), 
                            CLI_GetString(cliData.command[1]), 
                            CLI_GetString(cliData.command[2]), 
                            CLI_GetString(cliData.command[3]), 
                            CLI_GetString(cliData.command[4]));
                }
                else
                {
                    sprintf(buf, "%s %s %s %s %s", 
                            CLI_GetString(cliData.command[0]), 
                            CLI_GetString(cliData.command[1]), 
                            CLI_GetString(cliData.command[2]), 
                            CLI_GetString(cliData.command[3]), 
                            CLI_GetString(cliData.command[4]));
                }
                break;
            default:
                break;
        }
        //DO SOMETHING WITH buf
    }

    void CLI_PrintError (uint16_t function)
    {
        char* buf = "";
        sprintf(buf, "%s is an invalid key", CLI_GetString(function));
        //DO SOMETHING WITH buf
    }
    
    void CLI_ProcessCommand()
    {
        CLI_PrintCommand(true);
        if(cliData.command[2] < cliData.command[0])
        {
            uint16_t tmp = cliData.command[0];
            cliData.command[0] = cliData.command[2];
            cliData.command[2] = tmp;
        }
        cliData.chLow = cliData.command[0];
        cliData.chHigh = cliData.command[2];
        uint8_t* follower = cliData.values + cliData.command[0];
        uint8_t* tail = cliData.values + cliData.command[2];
        do
        {
            *follower = cliData.command[4];
            follower++;
        }
        while(follower != tail);
        for(cliData.counter = 0; cliData.counter < 5; cliData.counter++)
            cliData.command[cliData.counter] = 0;
        cliData.counter = 0;
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
    }
    
    void CLI_AddToCommand(uint16_t function)
    {
        bool accepted = false;
        switch(function)
        {
            case 0 ... 9:
                if(cliData.counter == 0)
                {
                    if(CLI_AddNumeric(function, true))
                        accepted = true;
                }
                else if(cliData.command[cliData.counter - 1] == At)
                {
                    if(CLI_AddNumeric(function, false))
                        accepted = true;
                }
                else
                {
                    if(CLI_AddNumeric(function, true))
                        accepted = true;
                }
            //"Thru" must be first or follow a channel 
            case Thru:
                if(cliData.counter == 0)
                {
                    CLI_AddItem(1);
                    CLI_AddItem(Thru);
                    accepted = true;
                }
                else if(cliData.command[cliData.counter - 1] >= 1 &&
                        cliData.command[cliData.counter - 1] <= 512)
                {
                    CLI_AddItem(Thru);
                    accepted = true;
                }
                break;
            //"At" must be first or follow a channel or "Thru"
            case At:
                if(cliData.counter == 0)
                {
                    CLI_AddItem(cliData.chLow);
                    CLI_AddItem(Thru);
                    CLI_AddItem(cliData.chHigh);
                    CLI_AddItem(At);
                    accepted = true;
                }
                else if(cliData.command[cliData.counter - 1] == Thru)
                {
                    CLI_AddItem(512);
                    CLI_AddItem(At);
                    accepted = true;
                }
                else if(cliData.command[cliData.counter - 1] >= 1 &&
                        cliData.command[cliData.counter - 1] <= 512)
                {
                    CLI_AddItem(At);
                    accepted = true;
                }
                break;
            //"Full" must follow "At", a channel, or "Thru"
            case Full:
                if(cliData.command[cliData.counter - 1] == Thru)
                {
                    CLI_AddItem(512);
                    CLI_AddItem(At);
                    CLI_AddItem(Full);
                    accepted = true;
                }
                else if(cliData.command[cliData.counter - 1] == At)
                {
                    CLI_AddItem(Full);
                    accepted = true;
                }
                else if(cliData.command[cliData.counter - 1] >= 1 &&
                        cliData.command[cliData.counter - 1] <= 512)
                {
                    CLI_AddItem(At);
                    CLI_AddItem(Full);
                    accepted = true;
                }
                break;
            //Enter is always accepted
            case Enter:
                accepted = true;
                break;
            case Clear:
                for(cliData.counter = 0; cliData.counter < 5; cliData.counter++)
                    cliData.command[cliData.counter] = 0;
                cliData.counter = 0;
                accepted = true;
                break;
            case Bksp:
                if(cliData.command[cliData.counter - 1] <= 512)
                    CLI_RemoveNumeric();
                else CLI_RemoveItem();
            //Anything else is rejected
            default:
                break;
        }
        if(accepted)
        {
            switch(function)
            {
                case Full:
                    cliData.command[cliData.counter] = function;
                    cliData.counter++;
                case Enter:
                    CLI_ProcessCommand(false);
                    break;
                case Bksp:
                case Clear:
                    CLI_PrintCommand(false);
                    break;
                default:
                    if(cliData.counter == 5) accepted = false;
                    else
                    {
                        CLI_PrintCommand(false);
                    }
                    break;
            }
        }
        else CLI_PrintError(function);
    }

    


/* *****************************************************************************
 End of File
 */
