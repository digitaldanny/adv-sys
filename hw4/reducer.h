#ifndef _REDUCER_SRC_HEADER_
#define _REDUCER_SRC_HEADER_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           DEFINES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

#define REDUCER_DEBUG_MODE 0

#define LEN_USER_ID   4
#define LEN_TOPIC     15

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           STRUCTS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

// Create a matrix of shape (W x N) of this structure where W = number of
// workers (channels in the fifo) and N is the depth of each channel 
// (size of the buffer).
typedef struct reducer_tuple_in 
{
  char userid[LEN_USER_ID];   // USERID - 4 digit number
  char topic[LEN_TOPIC];      // TOPIC - Pad this with space if unused.
  int32_t weight;             // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.
} reducer_tuple_in_t;

typedef struct reducer_tuple_out
{
  char userid[LEN_USER_ID];
  char topic[LEN_TOPIC];
  int32_t weight_total;
} reducer_tuple_out_t;

typedef struct node
{
  struct node* prev;
  struct node* next;
  reducer_tuple_out_t tuple;
} node_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

void reducer_tuple_init(void);
void reduce(reducer_tuple_in_t* tuple);
void reducer_write_tuple(void);

#endif