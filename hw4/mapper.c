#include "mapper.h"

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
    return NULL;
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
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
reducer_tuple_in_t* map(mapper_tuple_in_t* in)
{
    return NULL;
}