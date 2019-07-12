//
// Created by Markus Thill on 2019-03-23.
//
#ifndef BS1_TREEMAP_H
#define BS1_TREEMAP_H

#include <stdlib.h>

#define MAX_KEYLENGTH 20

typedef unsigned long UL;

//typedef struct TreeNode TreeNode;
typedef struct TreeNode {
    char key[MAX_KEYLENGTH + 1];
    UL left;
    UL right;
    int height;
    char value; // This has to be specified properly in the User-defined struct
} TreeNode;

typedef struct TreeMap {
    TreeNode *treeNodePool;
    unsigned int size_treeNodePool; // Initial Number of Tree-Nodes. INITIAL_POOL_SIZE
    size_t value_size;
} TreeMap;

/*
 * These functions starting with tm_ should be used to manipulate/access the tree
 *
 */
int tm_getHeight(TreeMap *tm);

void *tm_getValue(TreeMap *tm, char *key, void *value);

void tm_insert(TreeMap *tm, char *key, void *value);

void tm_delete(TreeMap *tm, char *key);

size_t tm_estimateRequiredBytes(size_t nodeSize, int numNodes);

void tm_initTreeNodePool(TreeMap *tm, void *ptr, size_t size, size_t sizeSingleNode);

int tm_poolExhausted(TreeMap *tm);

void tm_resizeTreeNodePool(TreeMap *tm, void *new_ptr, size_t new_size, int re_init);

int tm_getKeys(TreeMap *tm, char (*keys)[MAX_KEYLENGTH]);

int tm_countNodes(TreeMap *tm);

static size_t sizeOfNode(TreeMap *tm);

static TreeNode *ab(TreeMap *tm, UL rel);

static UL IbalanceTree(TreeMap *tm, UL n);

static UL InewTreeNode(TreeMap *tm, char *key, void *value);

static int Iheight(TreeMap *tm, UL n);

static int IbalanceFactor(TreeMap *tm, unsigned long n);

static int max(int a, int b);

static int IgetTreeNodeHeight(TreeMap *tm, UL n);

static UL IrotateRight(TreeMap *tm, UL oldRoot);

static UL IrotateLeft(TreeMap *tm, UL oldRoot);

static UL IminValueNode(TreeMap *tm, UL n);

static int IgetKeys(TreeMap *tm, UL n, char (*keys)[MAX_KEYLENGTH], int i);

static void *IgetTreeNodeValue(TreeMap *tm, UL n, char *key, void *value);

static UL IinsertTreeNode(TreeMap *tm, UL n, char *key, void *value);

static UL IdeleteTreeNode(TreeMap *tm, UL n, char *key);

static int IcountTreeNodes(TreeMap *tm, UL n);

#endif //BS1_TREEMAP_H
