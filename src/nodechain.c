//
// Created by Gabriel Souza on 19/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "nodechain.h"


Node *nodechain_alloc(const void *value_ptr, const size_t value_size)
{
    Node *new_node = malloc(sizeof(Node) + value_size);
    if (!new_node) return NULL;

    memcpy(new_node->value, value_ptr, value_size);
    new_node->next = NULL;
    return new_node;
}


DSErrCode nodechain_free(NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return DS_ERR_NULL_POINTER;

    Node *node = nodechain_ptr->head;

    while (node != NULL)
    {
        Node *next = node->next;

        if (nodechain_ptr->destructor_func != NULL)
            nodechain_ptr->destructor_func(node->value);

        free(node);
        node = next;
    }

    nodechain_ptr->head = NULL;
    nodechain_ptr->tail = NULL;
    nodechain_ptr->length = 0;

    return DS_SUCCESS;
}


DSErrCode nodechain_push_front(NodeChain *nodechain_ptr, const void *value_ptr)
{
    if (!nodechain_ptr) return DS_ERR_NULL_POINTER;

    Node *new_node = nodechain_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    new_node->next = nodechain_ptr->head;

    if (!nodechain_ptr->head)
        nodechain_ptr->tail = new_node;

    nodechain_ptr->head = new_node;
    nodechain_ptr->length++;

    return DS_SUCCESS;
}


DSErrCode nodechain_push_back(NodeChain *nodechain_ptr, const void *value_ptr)
{
    if (!nodechain_ptr) return DS_ERR_NULL_POINTER;

    Node *new_node = nodechain_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

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

    return DS_SUCCESS;
}


DSErrCode nodechain_push_at(NodeChain *nodechain_ptr, const void *value_ptr, ptrdiff_t index)
{
    if (!nodechain_ptr) return DS_ERR_NULL_POINTER;

    const size_t length = nodechain_ptr->length;

    // Negative index counts from the tail
    if (index < 0)
        index += (ptrdiff_t) length;

    if (index < 0 || (size_t) index > length)
        return DS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    // Insert at beginning
    if (unsigned_index == 0)
        return nodechain_push_front(nodechain_ptr, value_ptr);

    // Insert at end
    if (unsigned_index == length)
        return nodechain_push_back(nodechain_ptr, value_ptr);

    // Insert in middle
    Node *prev_node = nodechain_ptr->head;
    for (size_t i = 0; i < unsigned_index - 1; i++)
        prev_node = prev_node->next;

    Node *new_node = nodechain_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    new_node->next = prev_node->next;
    prev_node->next = new_node;
    nodechain_ptr->length++;

    return DS_SUCCESS;
}
