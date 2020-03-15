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
static void* areaChmap;

static reducer_tuple_in_t* tupleMatrix;
static pthread_mutex_t* mutexArray;
static reducer_tuple_fifo_t* fifo;
static channel_map_t* fifoChmap;

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


  // Instantiate area for user id / channel mapping structure
  areaChmap = mmap( NULL,
                    num_channels*sizeof(channel_map_t),
                    PROT_AREA, MAP_AREA,
                    -1, 0);
  fifoChmap = (channel_map_t*)areaChmap;

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
  fifo->_chmap = fifoChmap;

  // Connect FIFO functions
  fifo->read = &fifo_read;
  fifo->write = &fifo_write;
  fifo->getUserChannel = &fifo_get_user_channel;
  fifo->readUser = &fifo_read_user_id;
  fifo->writeUser = &fifo_write_user_id;

  // initialize fifo parameters
  fifo->num_channels = num_channels;
  fifo->num_channels_used = 0;
  for (int i = 0; i < num_channels; i++)
  {
    fifo->_wrindex[i] = 0;
    fifo->_rdindex[i] = 0;
    fifo->_depth[i] = buf_depth;
    fifo->_size[i] = 0;
  }

  // initialize dictionary so that it is not mapping any user ids
  // to channels.
  for (int i = 0; i < num_channels; i++)
  {
    strncpy(fifo->_chmap[i].userid, "\0\0\0\0", LEN_USER_ID);
    fifo->_chmap[i].channel = -1;
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

  if (pthread_mutex_init(&fifo->_mutex_chmap, NULL) != 0)
  {
    printf("ERROR (fifo.c): initializing chmap mutex\n");
    exit(0);
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
    pthread_mutex_unlock(&fifo->_mutex[ch]);
    free(copy);
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

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: fifo_get_user_channel
 * Search for the channel of the user id passed. If one is not
 * found, this function will map the userid to the next available
 * channel.
 *
 * RETURN: 0+ = channel number, -1 error
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
int fifo_get_user_channel(char* userid)
{
  int ch = 0;

  pthread_mutex_lock(&fifo->_mutex_chmap);

  // scan through channel map to see if the user id exists there.
  for (int i = 0; i < fifo->num_channels; i++)
  {
    // This portion of the array has not been defined yet. The user 
    // does not exist yet, so add it to the end.
    if (strncmp(fifo->_chmap[i].userid, "\0\0\0\0", LEN_USER_ID) == 0)
    {
      // check if there is still enough room to add another channel.
      if (fifo->num_channels_used == fifo->num_channels)
      {
        printf("ERROR: adding new user when max number of channels in use.\n");
        pthread_mutex_unlock(&fifo->_mutex_chmap);
        return -1;
      }

      strncpy(fifo->_chmap[i].userid, userid, LEN_USER_ID);
      fifo->_chmap[i].channel = i;
      fifo->num_channels_used++;
      ch = i;
      break;
    }

    // If a user id match is found, return the channel number.
    else if (strncmp(fifo->_chmap[i].userid, userid, LEN_USER_ID) == 0)
    {
      ch = i;
      break;
    }
  }

  pthread_mutex_unlock(&fifo->_mutex_chmap);
  return ch;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: fifo_read_user_id / fifo_write_user_id
 * Abstraction over fifo_read/write so user application can just
 * pass in the user id instead of finding the appropriate 
 * channel number for the user.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
reducer_tuple_in_t* fifo_read_user_id(char* userid)
{
  int ch;
  if ((ch = fifo_get_user_channel(userid)) == -1)
  {
    printf("ERROR: cannot find or add user id\n");
    return NULL;
  }
  return fifo_read(fifo, ch);
}

int fifo_write_user_id(char* userid, reducer_tuple_in_t* tuple)
{
  int ch;
  if ((ch = fifo_get_user_channel(userid)) == -1)
  {
    printf("ERROR: Cannot find or add user id\n");
    return -1;
  }
  return fifo_write(fifo, ch, tuple);
}