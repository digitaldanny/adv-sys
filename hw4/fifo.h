#ifndef _SRC_FIFO_
#define _SRC_FIFO_

/*
 * ##########################################################
 *                          INCLUDES
 * ##########################################################
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>

#include "reducer.h"

/*
 * ##########################################################
 *                          DEFINES
 * ##########################################################
*/

#define PROT_AREA     (PROT_READ | PROT_WRITE)
#define MAP_AREA      (MAP_SHARED | MAP_ANON)

// variable number of inputs for function pointer definitions
#define FUNC_PTR_1IN(name, in_type, out_type)                       out_type (*name)(in_type)
#define FUNC_PTR_2IN(name, in1_type, in2_type, out_type)            out_type (*name)(in1_type, in2_type)
#define FUNC_PTR_3IN(name, in1_type, in2_type, in3_type, out_type)  out_type (*name)(in1_type, in2_type, in3_type)           

/*
 * ##########################################################
 *                          STRUCTS
 * ##########################################################
*/

// This structure will map the user id's to the appropriate channel numbers.
typedef struct channel_map
{
  char userid[LEN_USER_ID];
  int channel;
} channel_map_t;

// This structure will wrap the matrix of reducer_tuple_in structures 
// to make reading and writing to the fifo easier.
typedef struct reducer_tuple_fifo
{

  // public functions
  FUNC_PTR_2IN(read, struct reducer_tuple_fifo*, int, reducer_tuple_in_t*);       // READ
  FUNC_PTR_3IN(write, struct reducer_tuple_fifo*, int, reducer_tuple_in_t*, int); // WRITE
  FUNC_PTR_1IN(getUserChannel, char*, int); // GET USER CHANNEL
  FUNC_PTR_1IN(readUser, char*, reducer_tuple_in_t*); // READ USER CHANNEL
  FUNC_PTR_2IN(writeUser, char*, reducer_tuple_in_t* tuple, int); // WRITE USER CHANNEL

  // private parameters
  int* _wrindex;
  int* _rdindex;
  int* _size;
  int* _depth;
  pthread_mutex_t* _mutex;
  reducer_tuple_in_t* _tuple;
  channel_map_t* _chmap;

  pthread_mutex_t _mutex_chmap;
  int num_channels;
  int num_channels_used;

} reducer_tuple_fifo_t;

/*
 * ##########################################################
 *                         PROTOTYPES
 * ##########################################################
*/

reducer_tuple_fifo_t* Fifo(int num_channels, int buffer_depth);
int FifoDestruct(reducer_tuple_fifo_t* fifo);
reducer_tuple_in_t* fifo_read(reducer_tuple_fifo_t* fifo, int ch);
int fifo_write(reducer_tuple_fifo_t* fifo, int ch, reducer_tuple_in_t* val);
int copy_reducer_tuple(reducer_tuple_in_t* copy, reducer_tuple_in_t* orig);

int fifo_get_user_channel(char* userid);
reducer_tuple_in_t* fifo_read_user_id(char* userid);
int fifo_write_user_id(char* userid, reducer_tuple_in_t* tuple);

#endif