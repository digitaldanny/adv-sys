#ifndef _MAPPER_SRC_HEADER_
#define _MAPPER_SRC_HEADER_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

typedef struct mTupleIn 
{
    int8_t error;       // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];     // USERID - 4 digit number
    char action;        // ACTION - Character to map using the rules defined in the 'main' summary.
    char topic[15];     // TOPIC - Pad this with space if unused.

} mTupleIn_t;

typedef struct mTupleOut 
{
    int8_t error;       // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];     // USERID - 4 digit number
    char topic[15];     // TOPIC - Pad this with space if unused.
    int32_t weight;     // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.

} mTupleOut_t;

typedef enum tupleItem
{
    ME_LEFT_BRACKET,
    ME_USER_ID,
    ME_ACTION,
    ME_TOPIC,
    ME_RIGHT_BRACKET
} tupleItem_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

mTupleIn_t* m_console_tuple_read(void);
int32_t map(mTupleIn_t * in, mTupleOut_t * out);


#endif