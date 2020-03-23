/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _DMX_CLI_H    /* Guard against multiple inclusion */
#define _DMX_CLI_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    /* ************************************************************************** */
    /** Descriptive Constant Name

      @Summary
        Brief one-line summary of the constant.
    
      @Description
        Full description, explaining the purpose and usage of the constant.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
      @Remarks
        Any additional remarks
     */
#define CLI_MAX_ITEMS 33
#define CLI_COMMAND_START 600
#define SWITCH_VALID_CHANNELS 1 ... 512
#define SWITCH_VALID_RAW_VALUES 0 ... 255
#define SWITCH_VALID_PERCENT_VALUES 0 ... 100
#define SWITCH_VALID_PRESETS 1 ... 20
#define SWITCH_VALID_TIME 0 ... 60

#define Thru 601
#define At 602
#define Full 603
#define Enter 604
#define Bksp 605
#define Clear 606
#define Plus 607
#define Minus 608
#define Last 609
#define Next 610
#define Record 611
#define Preset 612
#define Offset 613
#define Time 614
    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    // *****************************************************************************

    /** Descriptive Data Type Name

      @Summary
        Brief one-line summary of the data type.
    
      @Description
        Full description, explaining the purpose and usage of the data type.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Remarks
        Any additional remarks
        <p>
        Describe enumeration elements and structure and union members above each 
        element or member.
     */
    
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
    
    struct CLI_Data {
        uint16_t command[CLI_MAX_ITEMS];
        uint8_t counter;
        uint16_t chLow;
        uint16_t chHigh;
        uint8_t* values;
    };


    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    /*  A brief description of a section can be given directly below the section
        banner.
     */

    // *****************************************************************************
    /**
      @Function
        int ExampleFunctionName ( int param1, int param2 ) 

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
    void CLI_Init(uint8_t* dmxBuf);
    void CLI_AddToCommand(uint16_t function);


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
