#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "dictionary.h"

#define REDUCER_DEBUG_MODE 0

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

// NOTE: This structure is the same as the tupleOut structure from the mapper.c file
typedef struct tupleIn 
{
    int8_t error;       // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];     // USERID - 4 digit number
    char topic[15];     // TOPIC - Pad this with space if unused.
    int32_t weight;     // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.

} tupleIn_t;

typedef enum state
{
    E_LEFT_BRACKET,
    E_USER_ID,
    E_TOPIC,
    E_WEIGHT,
    E_RIGHT_BRACKET
} state_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int32_t console_tuple_read(tupleIn_t * tuple);
void console_tuple_write(char* userId, node_t * dictionary);
void reduce(node_t * dictionary, tupleIn_t * in);
int32_t compareUserId(char* a, char* b);
void copyUserId(char* copy, char* orig);

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_tuple_read
 * This function creates an input tuple from an input string.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int32_t console_tuple_read(tupleIn_t * tuple)
{
    uint32_t        exitLoop    = 0;
    int32_t         error       = 0;
    uint32_t        charCount   = 0;
    state_t         state       = E_LEFT_BRACKET;
    char            nextChar    = getchar();
    char            tempWeight[3] = {'X', 'X', 'X'};

    tuple->error = -1;

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
                if (nextChar == DELIMITER)
                {
                    state = E_TOPIC;
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

            // Add new characters to the topic property until reaching 
            // max characters OR comma is found..
            case E_TOPIC:

                // comma found.. fill the rest of the characters with spaces
                if (nextChar == DELIMITER)
                {
                    for (; charCount < LEN_TOPIC; charCount++)
                        tuple->topic[charCount] = SPACE;

                    state = E_WEIGHT;
                    charCount = 0;
                }
                // comma not found and property length already used.. error on input.
                else if (charCount == LEN_TOPIC)
                {
                    error = -1;
                }
                // store next character in the current property..
                else
                {
                    tuple->topic[charCount] = nextChar;
                    charCount++;
                }

                break;

            // Add new characters till the right bracket is found. Convert the
            // characters from string to integer.
            case E_WEIGHT:

                // Tuple is complete when the right bracket has been found. Convert
                // the temporary string to an integer and end the function.
                if (nextChar == RB)
                {
                    tuple->weight = atoi(tempWeight);
                    tuple->error = 0;
                    exitLoop = 1;
                }
                // reached max topic length.. throw an error.
                else if (charCount == LEN_WEIGHT)
                {
                    error = -1;
                }
                // new character to add to the tuple's topic property.
                else
                {
                    tempWeight[charCount] = nextChar;
                    charCount++;
                }
                break;

            default:
                error = -1;
                printf("ERROR: Should not be entering this case..");  
                break;  
        } 

        // if (!exitLoop)
        //     nextChar = getchar();
        nextChar = getchar();

        // exit the function if the EOF is found
        if (nextChar == EOF)
        {
            if (feof(stdin))
                error = -1;
        }
    }

    return error;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_tuple_write
 * This function creates an output string from a hash map.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void console_tuple_write(char* userId, node_t * dictionary)
{
#if REDUCER_DEBUG_MODE == 1   
    printf("\n================ PROGRAM OUTPUT ================\n\n");
#endif

    while (dictionary != NULL)
    {
        // convert integer weight into printable string
        char weightString[3];
        sprintf(weightString, "%d", dictionary->value);

        // print out the tuple in the expected format..
        putchar(LB);
        console_string_write(userId, LEN_USER_ID);
        putchar(DELIMITER);
        console_string_write(dictionary->key, LEN_TOPIC);
        putchar(DELIMITER);
        console_string_write(weightString, sizeof(weightString));
        putchar(RB);
        printf("\n");

        // go to next node in the hash map
        dictionary = dictionary->next;
    }

#if REDUCER_DEBUG_MODE == 1   
    printf("\n");
#endif
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reduce
 * Add the input tuple's weight to the hash map using the input
 * topic. If the topic doesn't exist in the map yet, it will be
 * added.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void reduce(node_t * dictionary, tupleIn_t * in)
{
    dictAddToValue(dictionary, in->topic, in->weight);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: compareUserId
 * Compare strings 'a' and 'b' and return 0 if equal. Otherwise
 * return -1.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int32_t compareUserId(char* a, char* b)
{
    int32_t error = 0;

    for (uint16_t i = 0; i < LEN_USER_ID; i++)
    {
        if (a[i] != b[i])
        {
            error = -1;
            break;
        }
    }

    return error;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: copyUserId
 * Copy contents of the original string to the copy string.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void copyUserId(char* copy, char* orig)
{
    for (uint16_t i = 0; i < LEN_USER_ID; i++)
        copy[i] = orig[i];
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                              MAIN
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int main (void)
{
    node_t * dictionary = NULL;

    // initialize the user ID
    char currId[LEN_USER_ID];
    char prevId[LEN_USER_ID];
    for (uint16_t i = 0; i < LEN_USER_ID; i++)
    {
        currId[i] = 'X';
        prevId[i] = 'Y';
    }
 
    setbuf(stdout, NULL); // no delay in buffer output]

    while(1)
    {
        // reinitialize every iteration so the array start off empty.
        tupleIn_t inputTuple;

        // read in tuples from standard input
        int32_t error = console_tuple_read(&inputTuple);

        // no error in tuple format and has not reached end of the file
        if (!error)
        {
            // update the current user id and check if it's still
            // equal to the previous user id.
            copyUserId(currId, inputTuple.userid);

            // check if this is a new id.. if yes, clear out the dictionary 
            // and reinitialize the data structure
            if (compareUserId(currId, prevId) == -1)
            {
                // display all contents of the hash map as a list of tuples
                console_tuple_write(prevId, dictionary);

                // deallocate heap memory for hash map
                dictFreeNodes(dictionary);

                // initialize a new hash map
                dictionary = dict();
            }

            // store the new tuple value in the hash map
            reduce(dictionary, &inputTuple);
        }

        // end of file or error found.. allow the heap memory to be deallocated
        else
        {
            printf("Reducer end of file found\n");
            break;
        }

        // update the previous user id with the new user id
        copyUserId(prevId, currId);

#if REDUCER_DEBUG_MODE == 1
        // display the contents of the dictionary to see if elements are being
        // added correctly.
        dictDisplayContents(dictionary);
#endif
    }

    // final check to make sure dictionary was deallocated before exit
    printf("Reducer exitting..\n");
    console_tuple_write(currId, dictionary);
    dictFreeNodes(dictionary);
    return 0;
}