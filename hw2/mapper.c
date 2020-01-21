#include "mapper.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_tuple_read
 * This function mallocates an input tuple from an input string.
 * Return is NULL when there is an error reading.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
mTupleIn_t* m_console_tuple_read(void)
{
    uint32_t        exitLoop    = 0;
    int32_t         error       = 0;
    uint32_t        charCount   = 0;
    tupleItem_t     state       = ME_LEFT_BRACKET;
    char            nextChar    = getchar();
    mTupleIn_t*     tuple       = (mTupleIn_t*)malloc(sizeof(mTupleIn_t));

    while(exitLoop != 1 && error != -1 && nextChar != ENTER)
    {

        switch(state)
        {
            // Wait for left bracket before reading into the tuple..
            case ME_LEFT_BRACKET:
                if (nextChar == LB)
                    state = ME_USER_ID;
                break;

            // Add new characters to the user ID until comma is found
            // OR exceeding max characters..
            case ME_USER_ID:

                // comma found.. store future characters in the next state
                if (nextChar == DELIMITER)
                {
                    state = ME_ACTION;
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
            case ME_ACTION:

                // comma found.. store future characters in the next state
                if (nextChar == DELIMITER)
                {
                    state = ME_TOPIC;
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

            // Add new characters to the topic until the right bracket is
            // found. Then fill in the remainder of the characters with null
            // characters and exit..
            case ME_TOPIC:

                // Tuple is complete when the right bracket has been found. Fill
                // in the empty values with spaces
                if (nextChar == RB)
                {
                    exitLoop = 1;
                    for (; charCount < LEN_TOPIC; charCount++)
                        tuple->topic[charCount] = SPACE;
                }
                // reached max topic length.. throw an error.
                else if (charCount == LEN_TOPIC)
                {
                    error = -1;
                }
                // new character to add to the tuple's topic property.
                else
                {
                    tuple->topic[charCount] = nextChar;
                    charCount++;
                }
                break;

            default:
                printf("ERROR: Should not be entering this case..");  
                break;  
        } 

        nextChar = getchar();

        // exit the function if the EOF is found, deallocate the tuple,
        // and return a NULL pointer.
        if (nextChar == -1)
        {
            if (feof(stdin))
            {
                error = -1;
                free(tuple);
                tuple = NULL;
            }
        }
    }

    return tuple;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: console_tuple_write
 * This function maps the input action to the output weight.
 * Additionally, it deep copies the data from the input tuple
 * to the output tuple.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int32_t map(mTupleIn_t * in, mTupleOut_t * out)
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