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

int accountTransaction(int src_account, int dst_account, int value)
{
  return -1;
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
