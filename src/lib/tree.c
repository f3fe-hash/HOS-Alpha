#include "tree.h"

Tree* init_tree()
{
    Tree* tree = mp_alloc(mpool_, sizeof(Tree));
    if (!tree) return NULL;

    tree->root = NULL;
    tree->size = 0;
    tree->next_id = 1;
    return tree;
}

void destroy_node(TreeNode* node)
{
    if (!node) return;

    for (int i = 0; i < node->num_values; ++i)
        mp_free(mpool_, node->values[i]);
    mp_free(mpool_, node->values);

    for (int i = 0; i < node->num_children; ++i)
        destroy_node(node->children[i]);
    mp_free(mpool_, node->children);

    mp_free(mpool_, node);
}

void destroy_tree(Tree* tree)
{
    if (!tree) return;

    destroy_node(tree->root);
    mp_free(mpool_, tree);
}

void add_child(Tree* tree, TreeNode* child, unsigned int parent_id)
{
    if (!tree || !child) return;

    child->id = tree->next_id++;
    child->children = NULL;
    child->values = NULL;
    child->num_children = 0;
    child->num_values = 0;

    if (parent_id == 0 || tree->root == NULL)
    {
        tree->root = child;
    }
    else
    {
        TreeNode* parent = find_node(tree, parent_id);
        if (!parent) return;

        size_t new_size = (parent->num_children + 1) * sizeof(TreeNode*);
        parent->children = mp_realloc(mpool_, parent->children, new_size);
        parent->children[parent->num_children++] = child;
    }

    tree->size++;
}

void add_node(Tree* tree, TreeData* value, unsigned int node_id)
{
    if (!tree || !value) return;

    TreeNode* node = find_node(tree, node_id);
    if (!node) return;

    size_t new_size = (node->num_values + 1) * sizeof(TreeData*);
    node->values = mp_realloc(mpool_, node->values, new_size);
    node->values[node->num_values++] = value;
}

void remove_node(Tree* tree, unsigned int node_id, int index)
{
    if (!tree) return;

    TreeNode* node = find_node(tree, node_id);
    if (!node || index < 0 || index >= node->num_values)
        return;

    mp_free(mpool_, node->values[index]);

    for (int i = index; i < node->num_values - 1; ++i)
        node->values[i] = node->values[i + 1];

    node->num_values--;
    if (node->num_values > 0)
        node->values = mp_realloc(mpool_, node->values, node->num_values * sizeof(TreeData*));
    else
        mp_free(mpool_, node->values), node->values = NULL;
}

int remove_subtree_recursive(TreeNode* parent, unsigned int target_id)
{
    if (!parent) return 0;

    for (int i = 0; i < parent->num_children; ++i)
    {
        if (parent->children[i]->id == target_id)
        {
            destroy_node(parent->children[i]);

            for (int j = i; j < parent->num_children - 1; ++j)
                parent->children[j] = parent->children[j + 1];

            parent->num_children--;
            if (parent->num_children > 0)
                parent->children = mp_realloc(mpool_, parent->children, parent->num_children * sizeof(TreeNode*));
            else
                mp_free(mpool_, parent->children), parent->children = NULL;

            return 1;
        }
        else if (remove_subtree_recursive(parent->children[i], target_id))
        {
            return 1;
        }
    }
    return 0;
}

void remove_subtree(Tree* tree, unsigned int node_id)
{
    if (!tree || !tree->root) return;

    if (tree->root->id == node_id)
    {
        destroy_node(tree->root);
        tree->root = NULL;
        tree->size = 0;
    }
    else
    {
        if (remove_subtree_recursive(tree->root, node_id))
            tree->size--; // One less node
    }
}

TreeNode* find_node(Tree* tree, unsigned int id)
{
    if (!tree || !tree->root)
        return NULL;

    TreeNode** stack = mp_alloc(mpool_, sizeof(TreeNode*) * 128);
    int stack_size = 0;

    stack[stack_size++] = tree->root;

    while (stack_size > 0)
    {
        TreeNode* curr = stack[--stack_size];

        if (curr->id == id)
        {
            mp_free(mpool_, stack);
            return curr;
        }

        for (int i = 0; i < curr->num_children; i++)
            stack[stack_size++] = curr->children[i];
    }

    mp_free(mpool_, stack);
    return NULL;
}
