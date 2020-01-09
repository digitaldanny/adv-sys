#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "dictionary.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           TYPEDEFS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

// NOTE: This structure is the same as the tupleOut structure from the mapper.c file
typedef struct tupleIn 
{
    int8_t error;       // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];     // USERID - 4 digit number
    char topic[15];     // TOPIC - Pad this with space if unused.
    int32_t weight;     // WEIGHT - Value mapped from the input action as defined by the set of rules in 'main' summary.

} tupleIn_t;

typedef struct tupleOut 
{
    int8_t error;           // ERROR -  0 = tuple data is valid, 1 = There was an error while reading the tuple.
    char userid[4];         // USERID - 4 digit number
    char topic[15];         // TOPIC - Pad this with space if unused.
    int32_t total_weight;   // TOTAL WEIGHT - weights from all functions with the same user and topic are added together here..

} tupleOut_t;

typedef enum state
{
    E_LEFT_BRACKET,
    E_USER_ID,
    E_TOPIC,
    E_WEIGHT,
    E_RIGHT_BRACKET
} state_t;

void debugger(int32_t value);
void debugger(int32_t value)
{
    char string[15];
    sprintf(string, "%d", value);
    string[14] = '\0';
    printf("%s", string);
}

int main (void)
{
    // constructor
    node_t * dictionary = dict();

    // adding first value
    node_t * changedNode = dictAddToValue(dictionary, "Hello world!!!", 10);
    printf("Hello world: ");
    debugger(changedNode->value);
    printf("\n\n");

    // making a new entry
    changedNode = dictAddToValue(dictionary, "Mello Zorld???", 456);
    printf("Mello Zorld: ");
    debugger(changedNode->value);
    printf("\n\n");

    // adding to a previous entry
    changedNode = dictAddToValue(dictionary, "Hello world!!!", 5);
    printf("Hello world: ");
    debugger(changedNode->value);
    printf("\n\n");

    // adding to a previous entry
    changedNode = dictAddToValue(dictionary, "Mello Zorld???", 1000);
    printf("Mello Zorld: ");
    debugger(changedNode->value);
    printf("\n\n");

    // adding to a previous entry
    changedNode = dictAddToValue(dictionary, "Hello world!!!", 5);
    printf("Hello world: ");
    debugger(changedNode->value);
    printf("\n\n");

    // adding to a previous entry
    changedNode = dictAddToValue(dictionary, "Mello Zorld???", 20000);
    printf("Mello Zorld: ");
    debugger(changedNode->value);
    printf("\n\n");

    // add a new node
    changedNode = dictAddToValue(dictionary, "This isn't real", 12321);
    printf("This isn't real: ");
    debugger(changedNode->value);
    printf("\n\n");

    // add a new node
    changedNode = dictAddToValue(dictionary, "This isn't real", 12321);
    printf("This isn't real: ");
    debugger(changedNode->value);
    printf("\n\n");

    // free all the allocated data
    dictFreeNodes(dictionary);

    return 0;
}