#include <stdio.h>
#include <stdlib.h>
#include "TreeMap.h"

typedef struct {
    char bookName[10];
    char bookDate[10];
    char bookAuthor[100];
} MyNode;

void example1();

int main() {
    printf("Running Example 1!\n");
    example1();
    return 0;
}

/*
 * This example creates a binary tree on the heap.
 * Then it writes 100 elements into the tree, checks if everything worked and finally
 * deletes all 100 elements again. This example should be a good starting point, if you have not used this library before.
 */
void example1() {
    /*
     * What should be the initial max. capacity of our Tree?
     * Lets start with 100 elements.
     */
    const int initialNumNodes = 100;

    /*
     * We need to specify the size of the nodes that we want to place in
     */
    size_t nodeSize = sizeof(MyNode);

    /*
     * Initially, we need some memory for the Tree-Map.
     * Let's get some from the heap.
     */
    int memSize = tm_estimateRequiredBytes(nodeSize, initialNumNodes);
    void *ptr = malloc(memSize);
    if (ptr == NULL)
        perror("Error in malloc"), exit(EXIT_FAILURE);

    /*
     * Now we create a struct of type TreeMap
     */
    TreeMap tm;

    /*
     * Now let us initialize our binary tree in the obtained memory area
     */
    tm_initTreeNodePool(&tm, ptr, memSize, nodeSize);

    /*
     * Initially, the tree should be empty
     */
    printf("\nSome stats on the empty tree:\n");
    printf("Height of the tree: %d\n", tm_getHeight(&tm));
    printf("Number of nodes in the tree: %d\n", tm_countNodes(&tm));


    /*
     * Lets just insert some nodes with the keys 0-99 into the tree
     */
    char key[MAX_KEYLENGTH];
    MyNode template;
    for (int i = 0; i < initialNumNodes + 3; i++) { // Let us try to write a few more than actually possible...
        // If the capacity of the binary tree is exhausted, then stop (this is the case after 100 entries)
        if (tm_poolExhausted(&tm))
            break;
        // The key has to be a string
        sprintf(key, "%d", i);
        sprintf(template.bookAuthor, "Markus %d", i);
        sprintf(template.bookDate, "%d.%d.19%d", i % 28, i % 12, i % 100);
        sprintf(template.bookName, "My Book Episode %d", i);
        tm_insert(&tm, key, &template);
    }

    /*
     * Print a few statistics about the binary tree now:
     */
    printf("\nHeight of the binary tree after inserting %d elements: %d\n", initialNumNodes, tm_getHeight(&tm));
    printf("Number of Nodes: %d\n", tm_countNodes(&tm));
    printf("All keys in the tree:");
    char keys[100][MAX_KEYLENGTH];
    tm_getKeys(&tm, keys);
    for (int i = 0; i < tm_countNodes(&tm); i++) {
        if (i % 20 == 0) printf("\n");
        printf("%s ", keys[i]);
    }

    /*
     * Our binary tree should now contain all keys from 0-99
     * Just check this here...
     */
    MyNode value;
    int errorFlag = 0;
    for (int i = 0; i < initialNumNodes; i++) {
        sprintf(key, "%d", i);
        if (tm_getValue(&tm, key, &value) == NULL) {
            fprintf(stderr, "ERROR. Should have found the key %s in the tree: %s, line %d\n", key, __FILE__, __LINE__);
            errorFlag = 1;
        }

    }
    if (errorFlag == 0)
        printf("\nSuccessfully found all previously written elements again in the tree!\n");

    /*
     * Finally, delete all the keys from the tree again...
     */
    for (int i = 0; i < initialNumNodes; i++) {
        sprintf(key, "%d", i);
        tm_delete(&tm, key);
    }
    printf("\nAfter deleting all elements again, the tree should be empty. Lets see...\n");
    printf("Height of the tree: %d\n", tm_getHeight(&tm));
    printf("Number of nodes in the tree: %d\n", tm_countNodes(&tm));
    if(tm_getValue(&tm, "1", &value) != NULL)
        fprintf(stderr, "The tree should be empty: %s, line %d\n", __FILE__, __LINE__);

    /*
     * Remember to free the memory, obtained with malloc()
     */
    free(ptr);
}