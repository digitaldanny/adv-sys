#include <stdint.h>
#include <stdio.h>

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                            DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

#define MAX_INPUT_STRING_LENGTH 50
#define ENTER                   10

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

typedef struct tupleIn 
{
    uint8_t error;      // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    uint32_t userid;    // USERID - 4 digit number
    char action;        // ACTION - Character to map using the rules defined in the 'main' summary.
    char topic[15];     // TOPIC - Pad this with space if unused.

} tupleIn_t;

typedef struct tupleOut 
{
    uint8_t error;      // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    uint32_t userid;    // USERID - 4 digit number
    char topic[15];     // TOPIC - Pad this with space if unused.
    uint32_t weight;    // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.

} tupleOut_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

uint32_t console_string_read(char* store_string_location);
void console_string_write(char* string, uint32_t len);

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
uint32_t console_string_read(char* store_string_location)
{
    char nextCharacter = 'X';
    uint32_t stringLength = 0;

    for (uint32_t i = 0; i < MAX_INPUT_STRING_LENGTH; i++)
    {
        nextCharacter = getchar();
        store_string_location[i] = nextCharacter;
        stringLength++;

        if (nextCharacter == (char)ENTER) 
            break;
    }

    return stringLength;
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
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: main
 * This function maps an input tuple into a different tuple
 * using the rules defined below. The generated tuple is immediately
 * output to the terminal.
 * 
 * RULES:
 * P = 50, L = 20, D = -10, C = 30, S = 40
 * 
 * EXAMPlE INPUT:   (1111,P,history)
 * EXAMPLE OUTPUT:  (1111,history,50)
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
void main (void)
{
    char string[MAX_INPUT_STRING_LENGTH]; // contains all characters input from the user
    uint32_t length;

    while (1)
    {
        printf("Program start.. type your message:\n");

        length = console_string_read((char*)&string);
        printf("Your message is below: \n");
        console_string_write((char*)&string, length);

        printf("\nProgram end..\n");
        break; // forces one iteration for debug
    }  
}