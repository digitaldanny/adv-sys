#ifndef _MAPPER_SRC_HEADER_
#define _MAPPER_SRC_HEADER_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "reducer.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

typedef struct mapper_tuple_in 
{
    char userid[LEN_USER_ID];   // USERID - 4 digit number
    char action;                // ACTION - Character to map using the rules defined in the 'main' summary.
    char topic[LEN_TOPIC];      // TOPIC - Pad this with space if unused.
} mapper_tuple_in_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

mapper_tuple_in_t* mapper_read_tuple(void);
reducer_tuple_in_t* map(mapper_tuple_in_t* in);

#endif