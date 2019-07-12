//
// Created by Markus Thill on 2019-03-23
// A balanced binary tree implementation for shared memories. Since no pointers with absolute addresses are stored in
// the data structure, the tree can be copied to arbritrary memory segments without problems and can also be used for
// resizable shared memory implementations, where the base-address might change after resizing.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TreeMap.h"

int tm_poolExhausted(TreeMap *tm) {
    // the first node is always used as entry point into the Tree-Node pool (right) and root node of the actual tree (left)
    return tm->treeNodePool[0].right == 0;
}

/*
 * Estimate the number of bytes required, to initialize a binary balanced tree with a capacity of numNodes. Since the
 * user nodes are wrapped into some internal nodes, we need slightly more bytes than one might expect.
 */
size_t tm_estimateRequiredBytes(size_t nodeSize, int numNodes) {
    // +1, because the first node is misused as a pointer to the Node-pool and the actual binary tree
    return (sizeof(TreeNode) - sizeof(char) + nodeSize) * (numNodes +1);
}

void tm_initTreeNodePool(TreeMap *tm, void *ptr, size_t size, size_t sizeSingleNode) {
    tm->treeNodePool = ptr;
    tm->value_size = sizeSingleNode;
    memset(tm->treeNodePool, 0, size); // zero everyting

    tm->size_treeNodePool = (unsigned int) (size / sizeOfNode(tm));
    printf("The tree-node pool has %d elements.", tm->size_treeNodePool-1); // -1, because the first node cannot be really used

    // Initially, just build a linked list out of all elements in the array
    // If we need a node for the tree, just extract this node from the linked list
    // Note that the address stored in the left,right variables are relative addresses (index in the array)
    // The first node always shows on the first free usable node and is not used for other purposes
    for (unsigned int i = 0; i < tm->size_treeNodePool - 1; i++) {
        ab(tm, i)->right = i + 1;
    }
}

/*
 * If some other process resized the SHM, we only have to change the elements of tm
 */
/*void tm_adaptResizedSHM(TreeMap *tm, void *new_ptr, size_t new_size) {
    if (new_ptr != tm->treeNodePool)
        logm(SL4C_INFO, "Address of the Tree-Node pool changed!");
    tm->treeNodePool = new_ptr;

    size_t old_size = tm->size_treeNodePool * sizeOfNode(tm);
    size_t diff = (new_size - old_size);
    if (diff <= 0) printErrorAndDie("Reducing the size of the shared memory not supported yet!"); //exits
    int num_new_nodes = (int) diff / sizeOfNode(tm);

    // This is the new size of our tree-node pool
    tm->size_treeNodePool += num_new_nodes;
    logm(SL4C_INFO, "The tree-node pool has now %d elements", tm->size_treeNodePool);
}*/

/*
 * Re-init is only necessary, if we are in the process which actually performed the resize of the SHM
 */
void tm_resizeTreeNodePool(TreeMap *tm, void *new_ptr, size_t new_size, int re_init) {
    if (new_ptr != tm->treeNodePool) {
        printf("Address of the Tree-Node pool changed!");
    }
    size_t old_size = tm->size_treeNodePool * sizeOfNode(tm);
    size_t diff = (new_size - old_size);
    if (diff <= 0) {
        printf("Reducing the size of the shared memory not supported yet!");
        return;
    }

    // Number of new nodes added to the pool
    int num_new_nodes = (int) diff / sizeOfNode(tm);


    // link the new nodes to each other
    tm->treeNodePool = new_ptr;

    if(re_init) {
        // First Null the new memory area
        memset(new_ptr + old_size, 0, diff);
        for (unsigned int i = tm->size_treeNodePool; i < tm->size_treeNodePool + num_new_nodes - 1; i++)
            ab(tm, i)->right = i + 1;

        // Now put these new nodes into the pool...
        // Cut the list in the front and squeeze the new list into the gap
        // First, append the old pool list to the end of the new list
        ab(tm, tm->size_treeNodePool + num_new_nodes - 1)->right = tm->treeNodePool[0].right;

        // Then, add the list to the front of the pool
        tm->treeNodePool[0].right = tm->size_treeNodePool;
    }
    // This is the new size of our tree-node pool
    tm->size_treeNodePool += num_new_nodes;
    printf("The tree-node pool has now %d elements", tm->size_treeNodePool);
}

/*
 * Find a key in the tree and return the corresponding value. If not found, return NULL.
 */
void *tm_getValue(TreeMap *tm, char *key, void *value) {
    UL *root = &tm->treeNodePool[0].left;
    return IgetTreeNodeValue(tm, *root, key, value);
}

void tm_insert(TreeMap *tm, char *key, void *value) {
    // the first node in the pool contains the root node on the left branch
    // (The right branch contains the currently available pool of free nodes
    if (tm->treeNodePool[0].left == 0) { // if the tree is empty
        tm->treeNodePool[0].left = InewTreeNode(tm, key, value);
    } else {
        UL *root = &tm->treeNodePool[0].left;
        *root = IinsertTreeNode(tm, *root, key, value);
    }
}

void tm_delete(TreeMap *tm, char *key) {
    UL *root = &tm->treeNodePool[0].left;
    *root = IdeleteTreeNode(tm, *root, key);
}

int tm_getHeight(TreeMap *tm) {
    UL *root = &tm->treeNodePool[0].left;
    if(*root == 0) return 0;
    return IgetTreeNodeHeight(tm, *root);
}

int tm_countNodes(TreeMap *tm) {
    UL *root = &tm->treeNodePool[0].left;
    if(*root == 0) return 0;
    return IcountTreeNodes(tm, *root);
}

int tm_getKeys(TreeMap *tm, char (*keys)[MAX_KEYLENGTH]) {
    int i = 0;
    UL *root = &tm->treeNodePool[0].left;
    return IgetKeys(tm, *root, keys, i);
}



// ----------------------------------------------------------------------------------------------------------------
// Internal stuff
// ----------------------------------------------------------------------------------------------------------------

static size_t sizeOfNode(TreeMap *tm) {
    return sizeof(TreeNode) - sizeof(char) + tm->value_size;
}

/*
 * Convert a relative address (e.g., in a SHM) into a real memory address
 */
static TreeNode *ab(TreeMap *tm, UL rel) {
    return (rel >= 0 ? ((void *) tm->treeNodePool) + sizeOfNode(tm) * rel : NULL);
}

static UL IgetNodeFromPool(TreeMap *tm) {
    if (tm->treeNodePool[0].right == 0) {
        printf( "Node Pool exhausted");
        return 0;
    }

    UL ret = tm->treeNodePool[0].right;

    // Set relative pointer to next free node in the pool
    tm->treeNodePool[0].right = ab(tm, ret)->right;
    return ret;
}

static void Ifree_node(TreeMap *tm, UL n) {
    // Add this node to the pool again
    memset(ab(tm, n), 0, sizeOfNode(tm)); // Not necessary, but nicer

    // Put this node between treeNodePool[0] and the next free node in the pool
    ab(tm, n)->right = tm->treeNodePool[0].right;
    tm->treeNodePool[0].right = n;
}

static UL InewTreeNode(TreeMap *tm, char *key, void *value) {
    if (strlen(key) > MAX_KEYLENGTH) return 0;
    //TreeNode *node = (TreeNode *) malloc(sizeof(TreeNode)); // careful in shm
    UL node = IgetNodeFromPool(tm);
    if (node == 0) {
        fprintf(stderr, "Could not create new Node: %s, line %d\n", __FILE__, __LINE__);
        return 0;
    }
    ab(tm, node)->left = 0;
    ab(tm, node)->right = 0;
    ab(tm, node)->height = 1;
    memcpy(&(ab(tm, node)->value), value, tm->value_size);
    strcpy(ab(tm, node)->key, key);
    return (node);
}

static int Iheight(TreeMap *tm, UL n) {
    if (n != 0)
        return ab(tm, n)->height;
    return 0;
}

static int IbalanceFactor(TreeMap *tm, UL n) {
    return Iheight(tm, ab(tm, n)->right) - Iheight(tm, ab(tm, n)->left);
}

static int max(int a, int b) {
    return (a > b) ? a : b;
}

/*
 * returns the pointer value on success, otherwise NULL
 */
static void *IgetTreeNodeValue(TreeMap *tm, UL n, char *key, void *value) {
    while (n != 0) {
        int cmp = strcmp(key, ab(tm, n)->key);
        if (cmp == 0) {
            memcpy(value, &(ab(tm, n)->value), tm->value_size);
            return value;
        } else if (cmp < 0)
            n = ab(tm, n)->left;
        else
            n = ab(tm, n)->right;
    }
    return NULL; // Not found, so return NULL
}

static int IgetKeys(TreeMap *tm, UL n, char (*keys)[MAX_KEYLENGTH], int i) {
    // TODO: check size of keys...
    if (n == 0)
        return i;
    strcpy(keys[i++], ab(tm, n)->key);
    if (ab(tm, n)->left != 0)
        i = IgetKeys(tm, ab(tm, n)->left, keys, i);
    if (ab(tm, n)->right != 0)
        i = IgetKeys(tm, ab(tm, n)->right, keys, i);
    return i;
}

static int IcountTreeNodes(TreeMap *tm, UL n) {
    int count = 0;
    if (ab(tm, n)->left != 0)
        count += IcountTreeNodes(tm, ab(tm, n)->left);
    if (ab(tm, n)->right != 0)
        count += IcountTreeNodes(tm, ab(tm, n)->right);
    return count + 1; // +1 for the own node
}

static int IgetTreeNodeHeight(TreeMap *tm, UL n) {
    return max(Iheight(tm, ab(tm, n)->left), Iheight(tm, ab(tm, n)->right)) + 1;
}

static UL IrotateRight(TreeMap *tm, UL oldRoot) {
    UL newRoot = ab(tm, oldRoot)->left;
    UL cutOff = ab(tm, newRoot)->right;

    ab(tm, newRoot)->right = oldRoot;
    ab(tm, oldRoot)->left = cutOff;

    // Heights of oldRoot and newRoot change
    ab(tm, oldRoot)->height = IgetTreeNodeHeight(tm, oldRoot);
    ab(tm, newRoot)->height = IgetTreeNodeHeight(tm, newRoot);

    return newRoot;
}

static UL IrotateLeft(TreeMap *tm, UL oldRoot) {
    UL newRoot = ab(tm, oldRoot)->right;
    UL cutOff = ab(tm, newRoot)->left;

    ab(tm, newRoot)->left = oldRoot;
    ab(tm, oldRoot)->right = cutOff;

    // Heights of oldRoot and newRoot change
    ab(tm, oldRoot)->height = IgetTreeNodeHeight(tm, oldRoot);
    ab(tm, newRoot)->height = IgetTreeNodeHeight(tm, newRoot);

    return newRoot;
}

static UL IbalanceTree(TreeMap *tm, UL n) {
    int bf = IbalanceFactor(tm, n);

    // Left-Left
    if (bf < -1 && IbalanceFactor(tm, ab(tm, n)->left) <= 0)
        return IrotateRight(tm, n);

    // Left-Right
    if (bf < -1 && IbalanceFactor(tm, ab(tm, n)->left) > 0) {
        ab(tm, n)->left = IrotateLeft(tm, ab(tm, n)->left);
        return IrotateRight(tm, n);
    }

    // Right-Right
    if (bf > 1 && IbalanceFactor(tm, ab(tm, n)->right) >= 0)
        return IrotateLeft(tm, n);

    // Right-Left
    if (bf > 1 && IbalanceFactor(tm, ab(tm, n)->right) < 0) {
        ab(tm, n)->right = IrotateRight(tm, ab(tm, n)->right);
        return IrotateLeft(tm, n);
    }
    return n;
}


static UL IinsertTreeNode(TreeMap *tm, UL n, char *key, void *value) {
    if (n == 0) {
        return InewTreeNode(tm, key, value);
    }
    int cmp = strcmp(key, ab(tm, n)->key);
    if (cmp < 0) { // Go to left child
        ab(tm, n)->left = IinsertTreeNode(tm, ab(tm, n)->left, key, value);
    } else if (cmp > 0) { // Go to right child
        ab(tm, n)->right = IinsertTreeNode(tm, ab(tm, n)->right, key, value);
    } else {
        // We are at the correct node already
        // Replace value of the node
        // If value is too long, then don't do anything
        //if (strlen(value) > MAX_VALUELENGTH) return n;
        memcpy(&(ab(tm, n)->value), value, tm->value_size);
    }

    // Compute the Iheight of this sub-tree
    ab(tm, n)->height = IgetTreeNodeHeight(tm, n);
    return IbalanceTree(tm, n);
}


static UL IminValueNode(TreeMap *tm, UL n) {
    // Find the smallest key in this (sub-) tree
    UL i = n;
    while (ab(tm, i)->left != 0) i = ab(tm, i)->left;
    return i;
}


static UL IdeleteTreeNode(TreeMap *tm, UL n, char *key) {
    if (n == 0)
        return 0;

    int cmp = strcmp(key, ab(tm, n)->key);
    if (cmp < 0)
        ab(tm, n)->left = IdeleteTreeNode(tm, ab(tm, n)->left, key);
    else if (cmp > 0)
        ab(tm, n)->right = IdeleteTreeNode(tm, ab(tm, n)->right, key);
    else { // cmp == 0
        // Check, if at least one of the child nodes is NULL
        if (ab(tm, n)->left == 0 || ab(tm, n)->right == 0) {
            UL tmp = ab(tm, n)->left ? ab(tm, n)->left : ab(tm, n)->right;

            // Both child nodes are null (not existent)
            if (tmp == 0) {
                tmp = n;
                n = 0;
            } else // Or we have one child:
                // Overwrite our struct n completely with struct tmp
                //*ab(tm, n) = *ab(tm, tmp); // this line does not copy the value field, since it is not part of TreeNode
                memcpy(ab(tm, n), ab(tm, tmp), sizeOfNode(tm));
            // Either delete the child (because we copied it to n) or delete the current node (if it didnt have any children)
            Ifree_node(tm, tmp);
        } else { // The node has 2 children
            // Get the smallest node of the right sub-tree
            UL min = IminValueNode(tm, ab(tm, n)->right);

            // min is larger than n, but also larger than n->left and smaller than n->right
            // So replace the current node with min
            strcpy(ab(tm, n)->key, ab(tm, min)->key);
            memcpy(&(ab(tm, n)->value), &(ab(tm, min)->value), tm->value_size);

            // Now we have to find min in the right sub-tree again and delete it. We will end up in the if-clause above this otherwise...
            ab(tm, n)->right = IdeleteTreeNode(tm, ab(tm, n)->right, ab(tm, min)->key);
        }
    }
    // If we deleted the last node of the tree, we cannot do anything any longer
    if (n == 0) return 0;
    ab(tm, n)->height = IgetTreeNodeHeight(tm, n);

    return IbalanceTree(tm, n);
}


/*
void testTreeMap() {
    char shm_name[] = "ServerRasp-SHM";
    int shm_fd;
    off_t pageSize = sysconf(_SC_PAGE_SIZE); // Smallest possible initial SHM size
    void *ptr = dynshm_create(2000 * pageSize, shm_name, &shm_fd);
    //void *new_ptr = dynshm_resize(2, ptr);
    int j = 200;

    TreeMap tm;
    tm.value_size = sizeof(ClientInfo);

    tm_initTreeNodePool(&tm, ptr, j * pageSize);

    srand(time(NULL));   // Initialization, should only be called once.
    fflush(stdout);
    // Tests with TreeMaps
    //initTreeNodePool_heap();

    ClientInfo template;
    UL root = InewTreeNode(&tm, "1", &template);
    IinsertTreeNode(&tm, root, "2", &template);
    char buf[20];

    for (int i = 0; i < 2000; i++) {
        int r = rand() % 10000 - 5000;      // Returns a pseudo-random integer between 0 and RAND_MAX.
        sprintf(buf, "%d", r);
        sprintf(template.ip, "%d", rand());
        if (tm_poolExhausted(&tm)) {
            logm(SL4C_INFO, "Tree-Nodes in Pool exhausted. Resizing the pool.");
            tm_resizeTreeNodePool(&tm, ptr, (j += 200) * pageSize, 1);
        }
        root = IinsertTreeNode(&tm, root, buf, &template);
        if (r == 50)
            printf("%d : %s\n", r, template.ip);

        r = rand() % 10000 - 5000;      // Returns a pseudo-random integer between 0 and RAND_MAX.
        sprintf(buf, "%d", r);
        if (r != 50)
            root = IdeleteTreeNode(&tm, root, buf);
    }

    if (IgetTreeNodeValue(&tm, root, "50", &template) != NULL) {
        printf("Value for key 50: %s\n", template.ip);
    }

    int h = IgetTreeNodeHeight(&tm, root);
    printf("Height of tree: %d\n", h);

    printf("Number of Nodes in the Tree: %d", IcountTreeNodes(&tm, root));
    printf("\n\n\n");
    fflush(stdout);
    // --------------------------------------------------

}

void testTreeMap2() {
    char shm_name[] = "ServerRasp-SHM";
    int shm_fd;

    TreeMap tmp;
    tmp.value_size = sizeof(ClientInfo);
    TreeMap *tm = &tmp;

    off_t pageSize = sysconf(_SC_PAGE_SIZE); // Smallest possible initial SHM size
    void *ptr = dynshm_create(pageSize, shm_name, &shm_fd);
    void *new_ptr;
    tm_initTreeNodePool(tm, ptr, (size_t) pageSize);

    fflush(stdout);

    ClientInfo template;
    char buf[20];

    int values[10000];

    for (int i = 0; i < 10000; i++) {
        //if(i == 7117)
        //    i = i;
        //printf("%d\n", i);
        int r = rand() % 10000;      // Returns a pseudo-random integer between 0 and RAND_MAX.
        sprintf(buf, "%d", i);
        template.connection_fd = r;
        if (tm_poolExhausted(tm)) {
            logm(SL4C_INFO, "Tree-Nodes in Pool exhausted. Resizing the pool.");
            //printf("j:%d", j);
            new_ptr = dynshm_resize(2.0, ptr, shm_fd);

            off_t new_size = dynshm_getsize(shm_fd);
            tm_resizeTreeNodePool(tm, new_ptr, new_size, 1);
            // We have to adjust the ptr root into the SHM
            //long diff = new_ptr - ptr;
            ptr = new_ptr;
        }
        tm_insert(tm, buf, &template);
        //root = IinsertTreeNode(tm, root, buf, &template);
        values[i] = r;
        if (i % 2 == 0) {
            r = rand() % 10000;      // Returns a pseudo-random integer between 0 and RAND_MAX.
            sprintf(buf, "%d", r);
            //root = IdeleteTreeNode(tm, root, buf);
            tm_delete(tm, buf);
            values[r] = -1;
        }

    }

    // Now test if the values are correct
    for (int i = 0; i < 10000; i++) {
        sprintf(buf, "%d", i);
        // ClientInfo *ret = IgetTreeNodeValue(tm, root, buf, &template);
        ClientInfo *ret = tm_getValue(tm, buf, &template);
        if (ret == NULL) {
            if (values[i] != -1)
                printf("Found error: TN=NULL and value=%d", values[i]);
        } else if (template.connection_fd != values[i]) {
            printf("Error: TN=%d and value=%d", template.connection_fd, values[i]);
        }
    }

    if (tm_getValue(tm, buf, &template) != NULL) {
        printf("Value for key 50: %d\n", template.connection_fd);
    }

    int h = tm_getHeight(tm);
    printf("Height of tree: %d\n", h);

    printf("Number of Nodes in the Tree: %d", tm_countNodes(tm));
    printf("\n\n\n");
    fflush(stdout);
    // --------------------------------------------------

    dynshm_release(ptr, shm_name, shm_fd);
}
 */