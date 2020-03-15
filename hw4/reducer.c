#include "reducer.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                            GLOBALS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

static node_t* root;
static int numTuples;
static int process_was_used;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reducer_write_tuple
 * This function initializes the reduced_tuple global.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void reducer_tuple_init(void)
{
  numTuples = 0;
  process_was_used = 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reducer_write_tuple
 * This function creates an output string from a hash map.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void reducer_write_tuple(void)
{
  //printf("Entered reducer_write_tuple\n");
  node_t* node = root;
  reducer_tuple_out_t* t;

  if (process_was_used)
  {
    for (int i = 0; i < numTuples; i++)
    {
      t = &node->tuple;
      printf("(%.4s,%.15s,%d)\n", &t->userid[0], &t->topic[0], t->weight_total);
      node = node->next;
    }
  }
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: reduce
 * Add the input tuple's weight to the reduced tuple using the input
 * topic. If the topic doesn't exist in the map yet, it will be
 * added.
 *
 * NOTE: the input tuple will be freed by this function.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
*/
void reduce(reducer_tuple_in_t* tuple)
{
  process_was_used = 1;
  node_t* node = root;
  reducer_tuple_out_t* t;
  int found = 0;

  // if no tuples have been added yet, instantiate the root
  if (numTuples == 0)
  {
    // init the node
    root = (node_t*)malloc(sizeof(node_t));
    root->next = root;
    root->prev = root;
    node = root;
    numTuples++;

    // add in the contents of the previously found node's tuple
    t = &root->tuple;
    strncpy(&t->userid[0], tuple->userid, LEN_USER_ID);
    strncpy(&t->topic[0], tuple->topic, LEN_TOPIC);
    t->weight_total = tuple->weight;
    free(tuple);   
    return;
  }

  // Search for topic in the array to see if it exists yet.
  for (int i = 0; i < numTuples; i++)
  {
    t = &node->tuple;
    if (strncmp(&t->topic[0], &tuple->topic[0], LEN_TOPIC) == 0)
    {
      found = 1;
      break;
    }
    node = node->next;
  }

  // If the tuple does not exist yet, malloc another node and reconnect
  // the appropriate node pointers.
  if (found == 0)
  {
    node_t* new = (node_t*)malloc(sizeof(node_t));

    // init new node pointers
    new->next = root;
    new->prev = root->prev;

    // fix the old tail's pointers
    root->prev->next = new;

    // fix the root pointers
    root->prev = new;

    numTuples++;

    new->tuple.weight_total = 0; // make sure this isn't a random value for += later
    t = &new->tuple; // this is the one we want to copy the contents into
  }

  // add in the contents of the previously found node's tuple
  strncpy(&t->userid[0], tuple->userid, LEN_USER_ID);
  strncpy(&t->topic[0], tuple->topic, LEN_TOPIC);
  t->weight_total += tuple->weight;
  free(tuple);   
}

