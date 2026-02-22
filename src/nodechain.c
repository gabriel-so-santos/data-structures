//
// Created by Gabriel Souza on 19/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "internal/nodechain.h"

typedef struct Node
{
    struct Node *next;
    unsigned char value[];  // Flexible array member
} Node;

struct NodeChain
{
    Node *head;
    Node *tail;
    size_t length;
    size_t value_size;
    Destructor destructor_func;
};

size_t dslib_length_of(const NodeChain *nodechain_ptr)
{
    return nodechain_ptr->length;
}

int dslib_is_empty(const NodeChain *nodechain_ptr)
{
    return nodechain_ptr->length == 0;
}


Node *_node_alloc(const void *value_ptr, const size_t value_size)
{
    Node *new_node = malloc(sizeof(Node) + value_size);
    if (!new_node) return NULL;

    memcpy(new_node->value, value_ptr, value_size);
    new_node->next = NULL;
    return new_node;
}


NodeChain *_dslib_nc_alloc(const size_t value_size, const Destructor destructor_func)
{
    NodeChain *new_structure = malloc(sizeof(NodeChain));
    if (!new_structure) return NULL;

    new_structure->head = NULL;
    new_structure->tail = NULL;
    new_structure->length = 0;
    new_structure->value_size = value_size;
    new_structure->destructor_func = destructor_func;

    return new_structure;
}


dslib_err_t _dslib_nc_free(NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return DSLIB_ERR_NULL_POINTER;

    Node *node = nodechain_ptr->head;

    while (node != NULL)
    {
        Node *next = node->next;

        if (nodechain_ptr->destructor_func != NULL)
            nodechain_ptr->destructor_func(node->value);

        free(node);
        node = next;
    }

    free(nodechain_ptr);

    return DSLIB_SUCCESS;
}


dslib_err_t _dslib_nc_push_front(NodeChain *nodechain_ptr, const void *value_ptr)
{
    if (!nodechain_ptr) return DSLIB_ERR_NULL_POINTER;

    Node *new_node = _node_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return DSLIB_ERR_ALLOCATION_FAILED;

    new_node->next = nodechain_ptr->head;

    if (!nodechain_ptr->head)
        nodechain_ptr->tail = new_node;

    nodechain_ptr->head = new_node;
    nodechain_ptr->length++;

    return DSLIB_SUCCESS;
}


dslib_err_t _dslib_nc_push_back(NodeChain *nodechain_ptr, const void *value_ptr)
{
    if (!nodechain_ptr) return DSLIB_ERR_NULL_POINTER;

    Node *new_node = _node_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return DSLIB_ERR_ALLOCATION_FAILED;

    if (!nodechain_ptr->head)
    {
        nodechain_ptr->head = new_node;
        nodechain_ptr->tail = new_node;
    }
    else
    {
        nodechain_ptr->tail->next = new_node;
        nodechain_ptr->tail = new_node;
    }

    nodechain_ptr->length++;

    return DSLIB_SUCCESS;
}


dslib_err_t _dslib_nc_push_at(NodeChain *nodechain_ptr, const void *value_ptr, ptrdiff_t index)
{
    if (!nodechain_ptr) return DSLIB_ERR_NULL_POINTER;

    const size_t length = nodechain_ptr->length;

    // Negative index counts from the tail
    if (index < 0)
        index += (ptrdiff_t) length;

    if (index < 0 || (size_t) index > length)
        return DSLIB_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    // Insert at beginning
    if (unsigned_index == 0)
        return _dslib_nc_push_front(nodechain_ptr, value_ptr);

    // Insert at end
    if (unsigned_index == length)
        return _dslib_nc_push_back(nodechain_ptr, value_ptr);

    // Insert in middle
    Node *prev_node = nodechain_ptr->head;
    for (size_t i = 0; i < unsigned_index - 1; i++)
        prev_node = prev_node->next;

    Node *new_node = _node_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return DSLIB_ERR_ALLOCATION_FAILED;

    new_node->next = prev_node->next;
    prev_node->next = new_node;
    nodechain_ptr->length++;

    return DSLIB_SUCCESS;
}


dslib_err_t _dslib_nc_get_front(const NodeChain *nodechain_ptr, void *output_ptr)
{
    if (!nodechain_ptr || !output_ptr)
        return DSLIB_ERR_NULL_POINTER;

    if (!nodechain_ptr->head)
        return DSLIB_ERR_EMPTY_STRUCTURE;

    memcpy(output_ptr, nodechain_ptr->head->value, nodechain_ptr->value_size);

    return DSLIB_SUCCESS;
}


dslib_err_t _dslib_nc_get_back(const NodeChain *nodechain_ptr, void *output_ptr)
{
    if (!nodechain_ptr || !output_ptr)
        return DSLIB_ERR_NULL_POINTER;

    if (!nodechain_ptr->tail)
        return DSLIB_ERR_EMPTY_STRUCTURE;

    memcpy(output_ptr, nodechain_ptr->tail->value, nodechain_ptr->value_size);

    return DSLIB_SUCCESS;
}