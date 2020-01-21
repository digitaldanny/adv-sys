#include "reducer.h"

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
int32_t r_console_tuple_read(rTupleIn_t * tuple)
{
    uint32_t        exitLoop    = 0;
    int32_t         error       = 0;
    uint32_t        charCount   = 0;
    state_t         state       = RE_LEFT_BRACKET;
    char            nextChar    = getchar();
    char            tempWeight[3] = {'X', 'X', 'X'};

    tuple->error = -1;

    while(exitLoop != 1 && error != -1 && nextChar != ENTER)
    {

        switch(state)
        {
            // Wait for left bracket before reading into the tuple..
            case RE_LEFT_BRACKET:
                if (nextChar == LB)
                    state = RE_USER_ID;
                break;

            // Add new characters to the user ID until comma is found
            // OR exceeding max characters..
            case RE_USER_ID:

                // comma found.. store future characters in the next state
                if (nextChar == DELIMITER)
                {
                    state = RE_TOPIC;
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
            case RE_TOPIC:

                // comma found.. fill the rest of the characters with spaces
                if (nextChar == DELIMITER)
                {
                    for (; charCount < LEN_TOPIC; charCount++)
                        tuple->topic[charCount] = SPACE;

                    state = RE_WEIGHT;
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
            case RE_WEIGHT:

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
void r_console_tuple_write(char* userId, node_t * dictionary)
{
    debugger("\n================ PROGRAM OUTPUT ================\n", REDUCER_DEBUG_MODE);

    while (dictionary != NULL)
    {
        // convert integer weight into printable string
        char weightString[3];
        weightString[2] = ' '; // in case only 2 characters are required
        sprintf(weightString, "%d", dictionary->value);

        // print out the tuple in the expected format..
        putchar(LB);
        console_string_write(userId, LEN_USER_ID);
        putchar(DELIMITER);
        console_string_write(dictionary->key, LEN_TOPIC);
        putchar(DELIMITER);
        // console_string_write(weightString, sizeof(weightString));
        printf("%s", weightString);
        putchar(RB);
        printf("\n");

        // go to next node in the hash map
        dictionary = dictionary->next;
    }
  
    debugger(" ", REDUCER_DEBUG_MODE);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reduce
 * Add the input tuple's weight to the hash map using the input
 * topic. If the topic doesn't exist in the map yet, it will be
 * added.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void reduce(node_t * dictionary, rTupleIn_t * in)
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