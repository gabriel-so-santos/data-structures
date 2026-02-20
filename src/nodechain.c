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


void nodechain_free(NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return;

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
}


void nodechain_push_front(NodeChain *nodechain_ptr, const void *value_ptr)
{
    if (!nodechain_ptr) return;

    Node *new_node = nodechain_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return;
    new_node->next = nodechain_ptr->head;

    if (!nodechain_ptr->head)
        nodechain_ptr->tail = new_node;

    nodechain_ptr->head = new_node;

    nodechain_ptr->length++;
}


void nodechain_push_back(NodeChain *nodechain_ptr, const void *value_ptr)
{
    if (!nodechain_ptr) return;

    Node *new_node = nodechain_alloc(value_ptr, nodechain_ptr->value_size);
    if (!new_node) return;

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
}


void nodechain_push_at(NodeChain *nodechain_ptr, const void *value_ptr, const ptrdiff_t index)
{
    if (!nodechain_ptr) return;

    if ((size_t) -index > nodechain_ptr->length) return;

    size_t unsigned_index;

    // Negative index counts from the tail
    if (index < 0)
        unsigned_index = nodechain_ptr->length + index;
    else
        unsigned_index = index;

    if (unsigned_index > nodechain_ptr->length) return;


    // Insert at beginning
    if (unsigned_index == 0)
        nodechain_push_front(nodechain_ptr, value_ptr);

    // Insert at end
    else if (unsigned_index == nodechain_ptr->length)
        nodechain_push_back(nodechain_ptr, value_ptr);

    // Insert in middle
    else
    {
        Node *prev_node = nodechain_ptr->head;
        for (size_t i = 0; i < unsigned_index - 1; i++)
            prev_node = prev_node->next;

        Node *new_node = nodechain_alloc(value_ptr, nodechain_ptr->value_size);
        if (!new_node) return;

        new_node->next = prev_node->next;
        prev_node->next = new_node;

        nodechain_ptr->length++;
    }
}
