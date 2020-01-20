/*
 * SUMMARY: dictionary.c
 * This file contains functions for a map data structure used for 
 * advanced systems programming assignment 1. The key is a 15 character
 * long string and the value contains the current total weight of the 
 * key.
 */

#include "dictionary.h"

/*
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 *                           FUNCTIONS
 * +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
 */

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: dict
 * Constructor for the dictionary data structure. This function
 * initializes a node but does not fill it with value keys/values.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
node_t* dict()
{
    node_t* map = malloc(sizeof(node_t));
    
    // fill the initial map values to invalid data
    map->valid = 0;
    map->value = 0;
    map->next = NULL;
    for (uint16_t i = 0; i < LEN_TOPIC; i++)
        map->key[i] = 'X';

    return map;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: dictAddToValue
 * This function adds input data to the currently stored key value.
 * If the key does not exist yet, it will be added and the value will
 * be initialized to 0. 
 * 
 * NOTE: NULL is returned if new data cannot be allocated.
 * RETURN: pointer to the node that was changed.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
node_t* dictAddToValue(node_t* map, char* key, int32_t value)
{
    node_t * iter = map; // iteration pointer

    // check if the intial node is valid. If not, create the first node 
    // values.
    if (!map->valid)
    {
        map->valid = 1; // make sure the map is valid for future accesses
        map->value = value;
        for (uint8_t i = 0; i < LEN_TOPIC; i++)
            map->key[i] = key[i];
        return map;
    }

    // search for keys that are equal to the requested key.
    while (1)
    {
        uint8_t stringsAreEqual = dictCompareStrings(key, iter->key);

        // If strings are equal, add the new value to the original value
        // and return a pointer to the node.
        if (stringsAreEqual)
        {
            iter->value += value;
            return iter;
        }

        // continue searching until the final value of the linked list
        // is found.. save this iteration's pointer as the final node
        // pointer for the next section.
        if (iter->next != NULL)
            iter = iter->next;
        else
            break;
    }

    // the key was never found, so the new node should be added.
    node_t * newNode = (node_t*)malloc(sizeof(node_t));
    
    if (newNode == NULL) // Memory allocation failed.. return error.
    {
        printf("ERROR: No dynamic memory to allocate, line 82");
        return NULL;
    }
    else
    {
        // initialize new node..
        newNode->valid = 1;
        newNode->value = value;
        newNode->next = NULL;
        for (uint16_t i = 0; i < LEN_TOPIC; i++)
            newNode->key[i] = key[i];
        
        // point the last node at the new node
        iter->next = newNode;
        
        return newNode;
    }
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: dictFreeNodes
 * Frees all dynamically allocated memory for the hash table.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void dictFreeNodes(node_t * firstNode)
{
    // exit if passed a null pointer
    if (firstNode == NULL)
        return;

    do
    {
        // first node always points at the next node so that the 
        // node to erase can be deleted.
        node_t * nodeToErase = firstNode;
        firstNode = firstNode->next;

#if DICTIONARY_DEBUG == 1
        printf("********** DELETING KEY ************\n");
        char string[LEN_TOPIC + 10]; // enough space to store topic string and large number
        char tempStr[LEN_TOPIC];     // buffer holds initial string
        snprintf(tempStr, LEN_TOPIC, "%s", nodeToErase->key);
        sprintf(string, "%s: %d", tempStr, nodeToErase->value);
        printf("%s\n\n", string);
#endif
        
        // free the current node..
        free((void*)nodeToErase);

    } while(firstNode != NULL);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: dictDisplayContents
 * Displays keys and values to the terminal.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void dictDisplayContents(node_t* node)
{
    printf("+-------------------------------------------+\n");
    printf("|            DICTIONARY CONTENTS            |\n");
    printf("+-------------------------------------------+\n");

    // continue displaying the map contents until the end of the
    // linked list is found.
    while (node != NULL)
    {
        char string[LEN_TOPIC + 10]; // enough space to store topic string and large number
        char tempStr[LEN_TOPIC];     // buffer holds initial string

        snprintf(tempStr, LEN_TOPIC, "%s", node->key);
        sprintf(string, "%s: %d", tempStr, node->value);
        printf("%s\n", string);
        node = node->next;
    }
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: dictCompareStrings
 * Compare two strings to see if they are matching topics.
 * Returns 1 for true, 0 for false
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
uint8_t dictCompareStrings(char* a, char* b)
{
    uint8_t result = 1;
    for (uint8_t i = 0; i < LEN_TOPIC; i++)
    {
        if (a[i] != b[i])
        {
            result = 0;
            break;
        }
    }

    return result;
}