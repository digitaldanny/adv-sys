/*
 * SUMMARY: fifo.c
 * This file implements the fifo functionality and mutual exclusion
 * for the multi-channel tuple buffers to write tuple values from 
 * mapper process to reducer process.
 *
 * IMPORTANT NOTE:
 * This data structure is defined for a specific application where
 * only ONE multi-channel fifo is required. Because global pointers 
 * defined in this file are used to hold the data, only one instance
 * of this class can be used at a time. To instantiate multiple instances
 * of this class (Fifo), extensive changes to this source code will need
 * to be made.
 */

 #include "fifo.h"

/*
 * ##########################################################
 *                     GLOBALS / EXTERNS
 * ##########################################################
*/

static void* areaMatrix;
static void* areaMutex;
static void* areaFifo;
static void* areaDepth;
static void* areaSize;
static void* areaWrindex;
static void* areaRdindex;

static reducer_tuple_in_t* tupleMatrix;
static pthread_mutex_t* mutexArray;
static reducer_tuple_fifo_t* fifo;

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: Fifo
 * This function initializes a process safe fifo by doing the
 * following:
 * 1.) MMAP the tuple matrix of dimensions (num_channels x buf_depth).
 * 2.) MMAP mutexes for each channel.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
reducer_tuple_fifo_t* Fifo(int num_channels, int buf_depth)
{
  // Instantiate matrix to hold all tuple data.
  areaMatrix = mmap(  NULL, 
                      buf_depth*num_channels*sizeof(reducer_tuple_in_t), 
                      PROT_AREA, MAP_AREA, 
                      -1, 0);
  tupleMatrix = (reducer_tuple_in_t*)areaMatrix;


  // Instantiate mutex for each channel.
  areaMutex = mmap(   NULL,
                      num_channels*sizeof(pthread_mutex_t),
                      PROT_AREA, MAP_AREA,
                      -1, 0);
  mutexArray = (pthread_mutex_t*)areaMutex;


  // Instantiate FIFO structure for application.
  areaFifo = mmap(NULL,
                  sizeof(reducer_tuple_fifo_t),
                  PROT_AREA, MAP_AREA,
                  -1, 0);
  fifo = (reducer_tuple_fifo_t*)areaFifo;

  // Instantiate area for FIFO attributes
  areaDepth = mmap(NULL, num_channels*sizeof(int), PROT_AREA, MAP_AREA, -1, 0);
  areaSize = mmap(NULL, num_channels*sizeof(int), PROT_AREA, MAP_AREA, -1, 0);
  areaWrindex = mmap(NULL, num_channels*sizeof(int), PROT_AREA, MAP_AREA, -1, 0);
  areaRdindex = mmap(NULL, num_channels*sizeof(int), PROT_AREA, MAP_AREA, -1, 0);
  fifo->_depth = (int*)areaDepth;
  fifo->_size = (int*)areaSize;
  fifo->_rdindex = (int*)areaRdindex;
  fifo->_wrindex = (int*)areaWrindex;

  // Connect the FIFO parameters to the appropriate structures / functions
  // and initialize the values if necessary.
  fifo->_tuple = tupleMatrix;
  fifo->_mutex = mutexArray;
  fifo->read = &fifo_read;
  fifo->write = &fifo_write;

  for (int i = 0; i < num_channels; i++)
  {
    fifo->_wrindex[i] = 0;
    fifo->_rdindex[i] = 0;
    fifo->_depth[i] = buf_depth;
    fifo->_size[i] = 0;
  }

  // make sure all mutexes are unlocked
  for (int i = 0; i < num_channels; i++)
  {
    if (pthread_mutex_init(&fifo->_mutex[i], NULL) != 0)
    {
      printf("ERROR (fifo.c): initializing mutex\n");
      exit(0);
    }
  }

  return fifo;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: FifoDestruct
 * This function unmaps entire Fifo resources.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
int FifoDestruct(reducer_tuple_fifo_t* fifo)
{
  return -1;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: fifo_read
 * This function reads the next data from the fifo and increments
 * its read index.
 *
 * NOTE: The return value from this function is mallocated, so
 * the user application must perform a free() on the returned
 * pointer after it is used.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
reducer_tuple_in_t* fifo_read(reducer_tuple_fifo_t* fifo, int ch)
{
  reducer_tuple_in_t* copy = (reducer_tuple_in_t*)malloc(sizeof(reducer_tuple_in_t));

  pthread_mutex_lock(&fifo->_mutex[ch]);
  
  // only read from the buffer if it is not empty.
  if (fifo->_size[ch] > 0)
  {
    int base = ch*fifo->_depth[ch];
    int offset = fifo->_rdindex[ch];

    copy_reducer_tuple(copy, &fifo->_tuple[base+offset]);
    fifo->_size[ch]--;

    // handle wrap around when buffer is full
    if (fifo->_rdindex[ch]++ == fifo->_depth[ch])
      fifo->_rdindex[ch] = 0;
  }

  // if the buffer is empty, deallocate the return tuple and return NULL.
  else
  {
    free(copy);
    pthread_mutex_unlock(&fifo->_mutex[ch]);
    return NULL;
  }

  pthread_mutex_unlock(&fifo->_mutex[ch]);

  return copy;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: fifo_write
 * This function deep copies the passed tuple value into the fifo
 * and increments its write index.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
int fifo_write(reducer_tuple_fifo_t* fifo, int ch, reducer_tuple_in_t* val)
{
  pthread_mutex_lock(&fifo->_mutex[ch]);
  
  // only write to the buffer if it is not full yet.
  if (fifo->_size[ch] < fifo->_depth[ch])
  {
    int base = ch*fifo->_depth[ch];
    int offset = fifo->_wrindex[ch];

    copy_reducer_tuple(&fifo->_tuple[base+offset], val);
    fifo->_size[ch]++;

    // handle wrap around when buffer is full
    if (fifo->_wrindex[ch]++ == fifo->_depth[ch])
      fifo->_wrindex[ch] = 0;
  }

  // if the buffer is full, unlock the mutex and return an error.
  else
  {
    pthread_mutex_unlock(&fifo->_mutex[ch]);
    return -1;
  }

  pthread_mutex_unlock(&fifo->_mutex[ch]); 
  return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: copy_reducer_tuple
 * Deep copy contents of orig into copy.
 *
 * RETURN: 0 => success, -1 => failure
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
int copy_reducer_tuple(reducer_tuple_in_t* copy, reducer_tuple_in_t* orig)
{
  strncpy(copy->topic, orig->topic, LEN_TOPIC);
  strncpy(copy->userid, orig->userid, LEN_USER_ID);
  copy->weight = orig->weight;
  return 0;
}