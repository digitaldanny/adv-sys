/*
 * SUMMARY: channel
 * This file contains functions to read/write/initialize a SINGLE channel
 * (NOT A CHANNEL ARRAY).
 */

#include "channel.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                      FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: channel
 * set the size of the tuple array + max + init pointers for the 
 * selected channel.
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
int channel(channel_t* ch, int size)
{
  // initialize user id to be null characters
  for (int i = 0; i < LEN_USER_ID; i++)
    ch->userid[i] = '\0';

  ch->max = size;
  ch->count = 0;

  ch->head = NULL;
  ch->tail = NULL;
  
  // connect functions
  ch->set_userid = &channelSetUserId;
  ch->write = &channelWriteTuple;
  ch->read = &channelReadTuple;
  return 0;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: channelReadTuple -> connects to channel.read
 * reads the next tuple and increments read pointer
 *
 * NOTE: Returns NULL if the buffer is EMPTY.
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
mTupleOut_t* channelReadTuple(channel_t* ch)
{
  // detach tail and relink list
  if (ch->count > 0)
  {
    mTupleOut_t* oldTail = ch->tail;
    mTupleOut_t* newTail = ch->tail->prev;

    // detatch the original tail before returning
    oldTail->prev = NULL;

    // link up the new tail correctly
    ch->tail = newTail;

    // decrement channel counter because original tail was read
    ch->count--;
    return oldTail;
  }
  else return NULL;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: channelWriteTuple -> conncets to channel.write
 * write the next tuple into the tuple list
 *
 * NOTE: Returns -1 if the buffer is FULL.
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
int channelWriteTuple(channel_t* ch, mTupleOut_t* tuple)
{
  // only add to the FIFO if the buffer is not full.
  if (ch->count < ch->max)
  {

    // handle if this is the first tuple being written
    if (ch->head == NULL || ch->tail == NULL)
    {
      tuple->next = NULL;
      tuple->prev = NULL;
      ch->head = tuple;
      ch->tail = tuple;
    }

    // make the input tuple the new head and relink all nodes
    else 
    {
      ch->head->prev = tuple;
      ch->head = tuple;
    }

    ch->count++;
    return 0;
  }

  // if the buffer is full, return an error
  else return -1;
}

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 * SUMMARY: channelSetUserId -> connects to channel.set_id
 * copy the character array into the channel's userid
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */
int channelSetUserId(channel_t* ch, char* userid)
{
  strncpy((char*)ch->userid, userid, LEN_USER_ID*sizeof(char));
  return 0;
}