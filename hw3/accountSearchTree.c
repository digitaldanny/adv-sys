/*
 * SUMMARY: accountSearchTree.c
 * This file contains functions for the tree data structure that will hold all
 * bank account information. 
 *
 * NOTE: All functions use the root account pointer defined in the "globals" section
 *      so a tree pointer does not have to be passed in.
 *
 * NOTE: This class was originally set up as a binary search tree; however, it has 
 *    been updated to run as a linked list. This change was to assist in meeting the
 *    requirement to output accounts in the order they were read in.
 */

#include "accountSearchTree.h"

/*
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*                             GLOBALS
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

account_t* root;
account_t* tail;
pthread_mutex_t mutexTransfer;

/*
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*                        PUBLIC FUNCTIONS
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: initAccountTree
 * This function initializes the root of the tree to be NULL
 * so the "addAccount" function can work.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void initAccountLinkedList()
{
  pthread_mutex_lock(&mutexTransfer);
  root = NULL;
  tail = NULL;
  mutexTransfer = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_unlock(&mutexTransfer);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: addAccount
 * This function adds a new account to the unbalanced binary
 * search tree.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int addAccount(int account_number, int starting_balance)
{
  account_t* node = (account_t*)malloc(sizeof(account_t));
  node->account_number = account_number;
  node->balance = starting_balance;
  node->next = NULL;
  node->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&mutexTransfer);

  // Base case.. initializing the root.
  if (root == NULL)
  {
    root = node;
    tail = node;
    pthread_mutex_unlock(&mutexTransfer);
    return 0;
  }

  // add to the end of the linked list.
  else
  {
    tail->next = node;
    tail = node;
    pthread_mutex_unlock(&mutexTransfer);
    return 0;
  }

  return -1;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: accountTransaction
 * This function attempts to transfer the value from source
 * account to destination account. 
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
int accountTransaction(int src_account, int dst_account, int value)
{
  // find the account nodes that will be changed.
  pthread_mutex_lock(&mutexTransfer);
  account_t* srcNode = _searchLinkedList(root, src_account);
  account_t* destNode = _searchLinkedList(root, dst_account);
  if (srcNode == NULL || destNode == NULL)
  {
    printf("ERROR: Source (%d) OR destination (%d) could not be found.\n", src_account, dst_account);
    pthread_mutex_unlock(&mutexTransfer);
    return -1;
  }

  // atomically check if both locks are available. If yes, claim both..
  // If not, don't claim either and return.
  // Do this section atomically so to avoid deadlocks. 
  if (pthread_mutex_lock(&srcNode->mutex) > 0) // try to claim source
  {
    pthread_mutex_unlock(&mutexTransfer);
    return -1;
  }

  if (pthread_mutex_lock(&destNode->mutex) > 0) // try to claim destination
  {
    pthread_mutex_unlock(&mutexTransfer);
    return -1;
  }
  pthread_mutex_unlock(&mutexTransfer);

  // transfer balance from source to destination
  srcNode->balance -= value;
  destNode->balance += value;

  // unlock the src/dest nodes so other threads can complete.
  pthread_mutex_unlock(&srcNode->mutex);
  pthread_mutex_unlock(&destNode->mutex);

  printf("Transaction complete\n");
  return 0;
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: printAccountContents
 * This function prints the in-order contents of each account in
 * the tree.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void printAccountContents()
{
  pthread_mutex_lock(&mutexTransfer);
  _printInOrderContents(root);
  pthread_mutex_unlock(&mutexTransfer);
}

void destroyAccountLinkedList()
{
  pthread_mutex_lock(&mutexTransfer);
  _destroyLinkedList(root);
  pthread_mutex_unlock(&mutexTransfer);
}

/*
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*                        PRIVATE FUNCTIONS
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: _destroyLinkedList
 * This function recursively destroys the linked list.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void _destroyLinkedList(account_t* root)
{
  if (root == NULL)
    return;

  _destroyLinkedList(root->next);
  free(root);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: _printInOrderContents
 * This function recursively prints the in-order contents of the
 * accounts in the linked list.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void _printInOrderContents(account_t* root)
{
  if (root == NULL)
    return;

  printf("**DEBUG** AccountNo: %d, Balance: %d\n", root->account_number, root->balance);
  _printInOrderContents(root->next);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: _searchTree
 * This function recursively searches for a node with a provided
 * key. If the key is not found, it will return NULL.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
account_t* _searchLinkedList(account_t* root, int account)
{
  // key was not found.
  if (root == NULL)
    return NULL;

  // key was found.
  if (root->account_number == account)
    return root;
  else
    return _searchLinkedList(root->next, account);
}
