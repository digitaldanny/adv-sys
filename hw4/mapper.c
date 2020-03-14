#include "mapper.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

// The RULE_ACTIONS and RULE_WEIGHTS below define how each action is mapped
// to the appropriate weight.
static const char RULE_ACTION[MAPPING_COUNT] = {'P', 'L', 'D', 'C', 'S'};
static const int32_t RULE_WEIGHT[MAPPING_COUNT] = {50, 20, -10, 30, 40};

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
mapper_tuple_in_t* mapper_read_tuple(void)
{
  uint32_t        exitLoop    = 0;
  int32_t         error       = 0;
  uint32_t        charCount   = 0;
  tupleItem_t     state       = ME_LEFT_BRACKET;
  char            nextChar    = getchar();
  mapper_tuple_in_t* tuple    = (mapper_tuple_in_t*)malloc(sizeof(mapper_tuple_in_t));

  if (nextChar == -1)
  {
    if (feof(stdin))
    {
      error = -1;
      free(tuple);
      return NULL;
    }
  }

  while(exitLoop != 1 && error != -1)
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
        //free(tuple);
        return tuple;
      }
    }
  }

  return tuple;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: map
 * This function maps the input action to the output weight.
 * Additionally, it deep copies the data from the input tuple
 * to the output tuple.
 *
 * NOTE: The output pointer is memory allocated, so the user 
 * must free() the return pointer once it is done using it.
 *
 * NOTE: This function automatically deallocates the input tuple
 * once it is done using it.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
reducer_tuple_in_t* map(mapper_tuple_in_t* in)
{
  reducer_tuple_in_t* out = (reducer_tuple_in_t*)malloc(sizeof(reducer_tuple_in_t));
  uint8_t index;
  int error;

  for (index = 0; index < MAPPING_COUNT; index++)
  {
    // action used was found.. mark as successful and 
    // create the mapping. 
    if (in->action == RULE_ACTION[index])
    {
      error = 0;
      out->weight = RULE_WEIGHT[index];
      break;
    }

    // if the action was not found, mark as  an error (hopefully temporarily)
    // and continue the search.
    else
    {
      error = -1;
    }
  }

  // if the action search was successful, perform a deep copy from the
  // inputs to the outputs on all other items.
  if (error == 0)
  {
    // copy TOPIC and USERID
    strncpy(out->topic, in->topic, LEN_TOPIC*sizeof(char));
    strncpy(out->userid, in->userid, LEN_USER_ID*sizeof(char));
    free(in);
    return out;
  }
  else
  {
    free(out);
    return NULL;
  }
}