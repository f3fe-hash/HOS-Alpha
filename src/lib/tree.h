#ifndef __TREE_H__
#define __TREE_H__

#include <string.h>

#include "memory.h"

typedef struct TreeData_
{
    void* data;
    int size;
} TreeData;

typedef struct TreeNode_
{
    struct TreeNode_** children;
    TreeData** values;
    int num_values;
    int num_children;
    int id;
} TreeNode;

typedef struct
{
    TreeNode* root;
    int size;
    unsigned int next_id;
} Tree;

Tree* init_tree();
void destroy_tree(Tree* tree);

void add_node(Tree* tree, TreeData* value, unsigned int parent_id);
void add_child(Tree* tree, TreeNode* node, unsigned int parent_id);
void remove_node(Tree* tree, unsigned int parent_id, int index);
void remove_subtree(Tree* tree, unsigned int node_id);

TreeNode* find_node(Tree* tree, unsigned int id);

#endif
