#ifndef _SRC_ACCOUNT_SEARCH_TREE_SRC_
#define _SRC_ACCOUNT_SEARCH_TREE_SRC_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                  TYPEDEFS / STRUCTS
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

/*
 * SUMMARY: account_t
 * account_number - account number, which will be used as the struct's key.
 * balance - amount of money in the account.
 * mutex - lock used to make sure the resource is not accessed by multiple threads.
 * left_node - node of an account with a smaller account number.
 * right_node - node of an account with a larger account number.
 */
typedef struct account
{
  // account properties
  int account_number;    // multiple readers are allowed outside of "addAccount." (works for tree traversal)
  int balance;           // only one reader OR writer is allowed at a time.
  pthread_mutex_t mutex;

  // linked list properties
  struct account* next;
} account_t;

/*
 * SUMMARY: transfer_buffer_t
 * empty - this flag will be set to 1 whenever it can be written to.
 * src - source account number for the transfer.
 * dest - destination account number for the transfer.
 * amount - amount of money being transferred from source -> destination.
 */
typedef struct transfer_buffer
{
  int empty;
  int src;
  int dest;
  int amount;
} transfer_buffer_t;

 /*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                   GLOBALS / EXTERNS
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

extern account_t* root;
extern pthread_mutex_t mutexTransfer;

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 *                       PROTOTYPES
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */

// PUBLIC
void initAccountLinkedList();
int addAccount(int account_number, int starting_balance);
int accountTransaction(int src_account, int dst_account, int value);
void destroyAccountLinkedList();
void printAccountContents();

// PRIVATE
void _destroyLinkedList(account_t* root);
void _printInOrderContents(account_t* root);
account_t* _searchLinkedList(account_t* root, int account);

#endif