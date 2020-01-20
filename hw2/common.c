#include "common.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                            GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

// The RULE_ACTIONS and RULE_WEIGHTS below define how each action is mapped
// to the appropriate weight.
const char RULE_ACTION[MAPPING_COUNT]    = {'P',     'L',    'D',    'C',    'S'};
const int32_t RULE_WEIGHT[MAPPING_COUNT] = {50,      20,     -10,    30,     40};

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_string_read
 * This function reads in characters from the terminal until
 * it overflows the MAX_INPUT_STRING_LENGTH (returning an error)
 * or it reaches an CR character from the space bar.
 * 
 * RETURN: length of the string before reaching CR. Otherwise, -1.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int32_t console_string_read(char* store_string_location)
{
    char nextCharacter = 'X';
    uint32_t stringLength = 0;

    for (uint32_t i = 0; i < MAX_INPUT_STRING_LENGTH; i++)
    {
        nextCharacter = getchar();
        store_string_location[i] = nextCharacter;
        stringLength++;

        if (nextCharacter == (char)ENTER) 
            return stringLength;
    }

    return -1;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_string_write
 * This function writes characters to the terminal until reaching
 * the string length paramter.
 * 
 * NOTE: This function doesn't require placing a NULL character
 * at the end of the string to work.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void console_string_write(char* string, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
        putchar(string[i]);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: debugger
 * This function 
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
uint16_t debugger(char* string, uint16_t debugMode)
{
    if (debugMode) 
    {
        printf("**DEBUGGER**: %s\n", string);
    }
    return 0;
}