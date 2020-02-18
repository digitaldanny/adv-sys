/*
 * SUMMARY: accountSearchTree.c
 * This file contains functions for the tree data structure that will hold all
 * bank account information. 
 *
 * NOTE: All functions use the root account pointer defined in the "globals" section
 *      so a tree pointer does not have to be passed in.
 *
 * NOTE: The tree is set up as an unbalanced search tree.
 */

#include "accountSearchTree.h"

/*
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*                             GLOBALS
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

account_t* root;
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
void initAccountTree()
{
  root = NULL;
  mutexTransfer = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
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
  node->left_node = NULL;
  node->right_node = NULL;
  node->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

  // Base case.. initializing the root.
  if (root == NULL)
  {
    printf("Initializing root.\n");
    root = node;
    return 0;
  }

  // Checking for position to add account to (left branch, right branch search)
  else
  {
    _insertAccountInTree(root, node);
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
  account_t* srcNode = _searchTree(root, src_account);
  account_t* destNode = _searchTree(root, dst_account);
  if (srcNode == NULL || destNode == NULL)
  {
    //printf("ERROR: Source (%d) OR destination (%d) could not be found.\n", src_account, dst_account);
    return -1;
  }

  // atomically check if both locks are available. If yes, claim both..
  // If not, don't claim either and return.
  // Do this section atomically so to avoid deadlocks. 
  pthread_mutex_lock(&mutexTransfer);
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
  printf("**DEBUG** PRINTING IN-ORDER CONTENTS\n");
  _printInOrderContents(root);
}

void destroyAccountTree()
{
  _destroyTree(root);
}

/*
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*                        PRIVATE FUNCTIONS
* +=====+=====+=====+=====+=====+=====+=====+=====+=====+=====+
*/

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: _insertAccountInTree
 * This function searches for the appropriate location to place
 * the node.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void _insertAccountInTree(account_t* root, account_t* node)
{
  // right branch search (>)
  if (node->account_number > root->account_number)
  {
    if (root->right_node == NULL)
      root->right_node = node;
    else
      _insertAccountInTree(root->right_node, node);
  }

  // left branch search (<=)
  else
  {
    if (root->left_node == NULL)
      root->left_node = node;
    else
      _insertAccountInTree(root->left_node, node);
  }
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: _destroyTree
 * This function recursively destroys the tree starting from the
 * left and working to the right.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void _destroyTree(account_t* root)
{
  if (root == NULL)
    return;

  _destroyTree(root->left_node);
  _destroyTree(root->right_node);
  free(root);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: _printInOrderContents
 * This function recursively prints the in-order contents of the
 * accounts in the tree.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
void _printInOrderContents(account_t* root)
{
  if (root == NULL)
    return;

  _printInOrderContents(root->left_node);
  printf("**DEBUG** AccountNo: %d, Balance: %d\n", root->account_number, root->balance);
  _printInOrderContents(root->right_node);
}

/*
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * SUMMARY: _searchTree
 * This function recursively searches for a node with a provided
 * key. If the key is not found, it will return NULL.
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 */
account_t* _searchTree(account_t* root, int account)
{
  // key was not found.
  if (root == NULL)
    return NULL;

  // key was found.
  if (root->account_number == account)
    return root;

  // BST - if key is greater, search right. Else search left.
  else if (account > root->account_number)
    return _searchTree(root->right_node, account);
  else
    return _searchTree(root->left_node, account);
}
