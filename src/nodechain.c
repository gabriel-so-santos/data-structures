//
// Created by Gabriel Souza on 19/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include <internal/nodechain.h>

typedef struct Node
{
    struct Node *next;
    unsigned char value[];  // Flexible array member
} Node;

struct NodeChain
{
    Node *head;
    Node *tail;
    Node *tail_prev;
    size_t length;
};

size_t
ds__nc_length_of(const NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return 0;
    return nodechain_ptr->length;
}

bool
ds__nc_is_empty(const NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return true;
    return nodechain_ptr->length == 0;
}


Node *
ds__node_alloc(const void *value_ptr, const size_t value_size)
{
    Node *new_node = malloc(sizeof(Node) + value_size);
    if (!new_node) return NULL;

    memcpy(new_node->value, value_ptr, value_size);
    new_node->next = NULL;
    return new_node;
}


NodeChain *
ds__nc_alloc(void)
{
    NodeChain *new_structure = malloc(sizeof(NodeChain));
    if (!new_structure) return NULL;

    new_structure->head = NULL;
    new_structure->tail = NULL;
    new_structure->length = 0;

    return new_structure;
}


ds_err_t
ds__nc_free(NodeChain **nodechain_dptr, const Destructor destructor)
{
    if (!nodechain_dptr || !*nodechain_dptr) return LIBDS_ERR_NULL_POINTER;

    NodeChain *nodechain_ptr = *nodechain_dptr;

    Node *node = nodechain_ptr->head;

    while (node != NULL)
    {
        Node *next = node->next;

        if (destructor)
            destructor(node->value);

        free(node);
        node = next;
    }

    free(nodechain_ptr);
    *nodechain_dptr = NULL;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_push_front(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    Node *new_node = ds__node_alloc(value_ptr, value_size);
    if (!new_node) return LIBDS_ERR_ALLOCATION_FAILED;

    new_node->next = nodechain_ptr->head;

    if (!nodechain_ptr->head)
        nodechain_ptr->tail = new_node;

    nodechain_ptr->head = new_node;
    nodechain_ptr->length++;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_push_back(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    Node *new_node = ds__node_alloc(value_ptr, value_size);
    if (!new_node) return LIBDS_ERR_ALLOCATION_FAILED;

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

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_push_at(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size, long long index)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    const size_t length = nodechain_ptr->length;

    // Negative index counts from the tail
    if (index < 0)
        index += (long long) length;

    if (index < 0 || (size_t) index > length)
        return LIBDS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    // Insert at beginning
    if (unsigned_index == 0)
        return ds__nc_push_front(nodechain_ptr, value_ptr, value_size);

    // Insert at end
    if (unsigned_index == length)
        return ds__nc_push_back(nodechain_ptr, value_ptr, value_size);

    // Insert in middle
    Node *prev_node = nodechain_ptr->head;
    for (size_t i = 0; i < unsigned_index - 1; i++)
        prev_node = prev_node->next;

    Node *new_node = ds__node_alloc(value_ptr, value_size);
    if (!new_node) return LIBDS_ERR_ALLOCATION_FAILED;

    new_node->next = prev_node->next;
    prev_node->next = new_node;
    nodechain_ptr->length++;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_get_front(const NodeChain *nodechain_ptr, void *output_ptr, const size_t output_size)
{
    if (!nodechain_ptr || !output_ptr)
        return LIBDS_ERR_NULL_POINTER;

    if (!nodechain_ptr->head)
        return LIBDS_ERR_EMPTY_STRUCTURE;

    memcpy(output_ptr, nodechain_ptr->head->value, output_size);

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_get_back(const NodeChain *nodechain_ptr, void *output_ptr, const size_t output_size)
{
    if (!nodechain_ptr || !output_ptr)
        return LIBDS_ERR_NULL_POINTER;

    if (!nodechain_ptr->tail)
        return LIBDS_ERR_EMPTY_STRUCTURE;

    memcpy(output_ptr, nodechain_ptr->tail->value, output_size);

    return LIBDS_SUCCESS;
}

ds_err_t
ds__pop_front(NodeChain *nodechain_ptr, const Destructor destructor)
{
    if (!nodechain_ptr)
        return LIBDS_ERR_NULL_POINTER;

    if (!nodechain_ptr->head)
        return LIBDS_ERR_EMPTY_STRUCTURE;

    Node *old_head = nodechain_ptr->head;
    nodechain_ptr->head = old_head->next;

    // If it's now empty, tail must be NULL
    if (!nodechain_ptr->head)
        nodechain_ptr->tail = NULL;

    if (destructor)
        destructor(old_head->value);

    free(old_head);
    nodechain_ptr->length--;

    return LIBDS_SUCCESS;
}

ds_err_t
ds__pop_back(NodeChain *nodechain_ptr, const Destructor destructor)
{
    if (!nodechain_ptr)
        return LIBDS_ERR_NULL_POINTER;

    if (!nodechain_ptr->tail)
        return LIBDS_ERR_EMPTY_STRUCTURE;

    Node *old_tail = nodechain_ptr->tail;

    // One element case
    if (nodechain_ptr->head == nodechain_ptr->tail)
    {
        nodechain_ptr->head = NULL;
        nodechain_ptr->tail = NULL;
    }
    else
    {
        Node *tail_prev = nodechain_ptr->head;

        while (tail_prev->next != old_tail)
            tail_prev = tail_prev->next;

        tail_prev->next = NULL;
        nodechain_ptr->tail = tail_prev;
    }

    if (destructor)
        destructor(old_tail->value);

    free(old_tail);
    nodechain_ptr->length--;

    return LIBDS_SUCCESS;
}
