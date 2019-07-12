# PFBalancedBinaryTree
A pointer-free balanced binary tree in C.

## Overview
This project realizes an implementation of a balanced binary tree in C which is completely pointer-free.
Such an approach has several implications (advantages):
- The binary tree can be easily moved from one memory area to another
- When using shared memory areas or shared binary files, several processes can easily operate on the same binary tree
- The binary tree can be written to (read from) the hard-drive without any further processing steps
- The tree can insert any user-specified structs (of one type) as nodes without any difficult adjustments by the programmer

### Structure
![Structure](https://github.com/MarkusThill/PFBalancedBinaryTree/blob/master/rsc/structure.png "Structure")

## Getting started
To run the example code, follow the following steps:

```
# Install cmake, if not done yet
sudo apt-get install cmake
 
cd # switch to home directory
git clone https://github.com/MarkusThill/PFBalancedBinaryTree.git
cd PFBalancedBinaryTree
mkdir bin
cd bin
cmake ..
make
```

## Examples
### Example 1
- Creating a binary tree and performing elementary operations on the tree

## Example 2
- Using a binary tree in a resizebale (POSIX) shared memory segment

## Example 3
- Moving a binary tree from one memory segment to another

## Data Types
`TreeMap`

## Functions
`int tm_getHeight(TreeMap *tm)`
- dd
---

`void *tm_getValue(TreeMap *tm, char *key, void *value)`
- ff
---

`void tm_insert(TreeMap *tm, char *key, void *value)` 
- ff
---

`void tm_delete(TreeMap *tm, char *key)`
- ff
---

`size_t tm_estimateRequiredBytes(size_t nodeSize, int numNodes)`
- ff
---

`void tm_initTreeNodePool(TreeMap *tm, void *ptr, size_t size, size_t sizeSingleNode)`
- ff
---

`int tm_poolExhausted(TreeMap *tm)`
- ff
---

`void tm_resizeTreeNodePool(TreeMap *tm, void *new_ptr, size_t new_size, int re_init)`
- ff
---

`int tm_getKeys(TreeMap *tm, char (*keys)[MAX_KEYLENGTH])`
- ff
---

`int tm_countNodes(TreeMap *tm);`
- ff
---
