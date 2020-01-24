#ifndef _CHANNEL_SRC_HEADER_
#define _CHANNEL_SRC_HEADER_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "mapper.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

// channel structure is set up as a dynamic FIFO for tuples..
typedef struct channelStruct
{
  // public parameters
  char userid[LEN_USER_ID]; // user id for the selected channel
  int max;                  // max number of tuples in the tuple array
  int count;                // current number of tuples in FIFO

  mTupleOut_t* head; // ends of the linked list FIFO
  mTupleOut_t* tail; // ends of the linked list FIFO
  
  // functions
  int (*set_userid)(struct channelStruct* ch, char* userid); // sets the user id parameter
  mTupleOut_t* (*read)(struct channelStruct* ch); // reads the next tuple and increments read pointer
  int (*write)(struct channelStruct* ch, mTupleOut_t* tuple); // write the next tuple into the tuple array

} channel_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

int channel(channel_t* ch, int size);
int channelSetUserId(channel_t* ch, char* userid);
mTupleOut_t* channelReadTuple(channel_t* ch);
int channelWriteTuple(channel_t* ch, mTupleOut_t*);

#endif