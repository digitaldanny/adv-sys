#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                            DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

#define MAPPING_COUNT           5
#define MAX_INPUT_STRING_LENGTH 50
#define ENTER                   10
#define LB                      '('
#define RB                      ')'
#define delimiter               ','

#define LEN_USER_ID          4
#define LEN_ACTION           1
#define LEN_TOPIC            15

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

typedef struct tupleIn 
{
    int8_t error;       // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];     // USERID - 4 digit number
    char action;        // ACTION - Character to map using the rules defined in the 'main' summary.
    char topic[15];     // TOPIC - Pad this with space if unused.

} tupleIn_t;

typedef struct tupleOut 
{
    int8_t error;       // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];     // USERID - 4 digit number
    char topic[15];     // TOPIC - Pad this with space if unused.
    int32_t weight;     // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.

} tupleOut_t;

typedef enum tupleItem
{
    E_LEFT_BRACKET,
    E_USER_ID,
    E_ACTION,
    E_TOPIC,
    E_RIGHT_BRACKET
} tupleItem_t;

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
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int32_t console_string_read(char* store_string_location);
void console_string_write(char* string, uint32_t len);
int32_t console_tuple_read(tupleIn_t * tuple_in);
int32_t console_tuple_write(tupleOut_t * tuple_out);
int32_t map(tupleIn_t * in, tupleOut_t * out);

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
 * SUMMARY: console_tuple_read
 * This function creates an input tuple from an input string.
 * 
 * RETURN: 0 for valid data, -1 for error
 * 
 * EXAMPLE INPUT: "(1111,P,history)"
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int32_t console_tuple_read(tupleIn_t * tuple)
{
    uint32_t        exitLoop    = 0;
    int32_t         error       = 0;
    uint32_t        charCount   = 0;
    tupleItem_t     state       = E_LEFT_BRACKET;
    char            nextChar    = getchar();

    while(exitLoop != 1 && error != -1 && nextChar != ENTER)
    {

        switch(state)
        {
            // Wait for left bracket before reading into the tuple..
            case E_LEFT_BRACKET:
                if (nextChar == LB)
                    
                    state = E_USER_ID;
                break;

            // Add new characters to the user ID until comma is found
            // OR exceeding max characters..
            case E_USER_ID:

                // comma found.. store future characters in the next state
                if (nextChar == delimiter)
                {
                    state = E_ACTION;
                    charCount = 0;
                }
                // comma not found and property length already used.. error on input.
                else if (charCount == LEN_USER_ID)
                {
                    error = -1;
                }
                // store next character in the current property..
                else
                {
                    tuple->userid[charCount] = nextChar;
                    charCount++;
                }

                break;

            // Add new characters to the action property until reaching 
            // max characters OR comma is found..
            case E_ACTION:

                // comma found.. store future characters in the next state
                if (nextChar == delimiter)
                {
                    state = E_TOPIC;
                    charCount = 0;
                }
                // comma not found and property length already used.. error on input.
                else if (charCount == LEN_ACTION)
                {
                    error = -1;
                }
                // store next character in the current property..
                else
                {
                    tuple->action = nextChar;
                    charCount++;
                }

                break;

            // Add new characters to the topic until comma is found
            // OR exceeding max characters..
            case E_TOPIC:

                // store next character in the current property..
                tuple->action = nextChar;
                charCount++;

                // full tuple read in.. ready to exit
                if (charCount == LEN_TOPIC)
                    state = E_RIGHT_BRACKET;

                break;

            // Wait for right bracket before exitting the loop..
            case E_RIGHT_BRACKET:
                if (nextChar == RB)
                    exitLoop = 1;
                break;

            default:
                printf("ERROR: Should not be entering this case..");  
                break;  
        } 

        nextChar = getchar();
    }

    return error;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_tuple_write
 * This function creates an output string from an output tuple.
 * 
 * RETURN: 0 for valid data, -1 for error
 * 
 * EXAMPLE OUTPUT: "(1111,history,50)"
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int32_t console_tuple_write(tupleOut_t * tuple)
{
    if (tuple->error == 0)
    {
        // convert integer weight into printable string
        char weightString[3];
        sprintf(weightString, "%d", tuple->weight);

        // print out the tuple in the expected format..
        putchar(LB);
        console_string_write(tuple->userid, sizeof(tuple->userid));
        putchar(delimiter);
        console_string_write(tuple->topic, sizeof(tuple->topic));
        putchar(delimiter);
        console_string_write(weightString, sizeof(weightString));
        putchar(RB);
        printf("\n");
        
        return 0;
    }
    else
        return -1;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_tuple_write
 * This function maps the input action to the output weight.
 * Additionally, it deep copies the data from the input tuple
 * to the output tuple.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int32_t map(tupleIn_t * in, tupleOut_t * out)
{
    uint8_t index;
    for (index = 0; index < MAPPING_COUNT; index++)
    {
        // action used was found.. mark as successful and 
        // create the mapping. 
        if (in->action == RULE_ACTION[index])
        {
            out->error = 0;
            out->weight = RULE_WEIGHT[index];
            break;
        }

        // if the action was not found, mark as  an error (hopefully temporarily)
        // and continue the search.
        else
        {
            out->error = -1;
        }
    }

    // if the action search was successful, perform a deep copy from the
    // inputs to the outputs on all other items.
    if (out->error == 0)
    {
        // copy TOPIC
        for (uint32_t i = 0; i < sizeof(in->topic); i++)
            out->topic[i] = in->topic[i];

        // copy USER ID
        for (uint32_t i = 0; i < sizeof(in->userid); i++)
            out->userid[i] = in->userid[i];

        return 0;
    }
    else
        return -1;
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
    tupleOut_t outputTuple;
    tupleIn_t inputTuple;

    while (1)
    {
        printf("Create a tuple..\n");

        // get data from the terminal
        int32_t error = console_tuple_read(&inputTuple);
        if (error == -1)
            printf("ERROR in tuple read\n");  
        else
            printf("successful tuple read\n");

        printf("INPUT TUPLE...\n");

        printf("USER ID\n");
        console_string_write(inputTuple.userid, sizeof(inputTuple.userid));

        printf("\nACTION\n");
        putchar(inputTuple.action);

        printf("\nTOPIC\n");
        console_string_write(inputTuple.topic, sizeof(inputTuple.topic));
        printf("\n\n");

        // map the data to the output tuple and output
        map(&inputTuple, &outputTuple);
        console_tuple_write(&outputTuple);
        break;
    }  
}