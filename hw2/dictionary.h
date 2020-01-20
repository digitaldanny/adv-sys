#ifndef _SRC_DICTIONARY_HEADER_
#define _SRC_DICTIONARY_HEADER_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

#define DICTIONARY_DEBUG 0

typedef struct node
{   
    // used for map initialization to determine if a valid key and
    // value has been provided to the dictionary yet.
    uint8_t valid;

    char key[LEN_TOPIC];
    int32_t value;   

    // setting this struct up as a linked list allows it to 
    // grow as needed without reallocating memory.
    struct node * next;
} node_t;

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           PROTOTYPES
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

node_t* dict();
node_t* dictAddToValue(node_t* map, char* key, int32_t value);
uint8_t dictCompareStrings(char* a, char* b);
void dictFreeNodes(node_t * firstNode);
void dictDisplayContents(node_t* firstNode);

#endif