#ifndef _REDUCER_SRC_HEADER_
#define _REDUCER_SRC_HEADER_

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
typedef struct rTupleIn 
{
    int8_t error;       // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];     // USERID - 4 digit number
    char topic[15];     // TOPIC - Pad this with space if unused.
    int32_t weight;     // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.

    struct rTupleIn* next; // FIFO implementation in channel.c
    struct rTupleIn* prev; // FIFO implementation in channel.c

} rTupleIn_t;

typedef enum state
{
    RE_LEFT_BRACKET,
    RE_USER_ID,
    RE_TOPIC,
    RE_WEIGHT,
    RE_RIGHT_BRACKET
} state_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int32_t r_console_tuple_read(rTupleIn_t * tuple);
void r_console_tuple_write(char* userId, node_t * dictionary);
void reduce(node_t * dictionary, rTupleIn_t * in);
int32_t compareUserId(char* a, char* b);
void copyUserId(char* copy, char* orig);

#endif