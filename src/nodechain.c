//
// Created by Gabriel Souza on 19/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "internal/nodechain.h"

#ifndef LIBDS_MIN_NC_BUFFER_LIMIT
    #define LIBDS_MIN_NC_BUFFER_LIMIT 16
#endif

#ifndef LIBDS_NC_BUFFER_BATCH_SIZE
    #define LIBDS_NC_BUFFER_BATCH_SIZE 4
#endif

#define LIBDS__MIN(x, y) ((x) < (y) ? (x) : (y))
#define LIBDS__MAX(x, y) ((x) > (y) ? (x) : (y))

typedef struct Node
{
    struct Node *next;
    unsigned char value[]; // Flexible array member
} Node;


struct NodeChain
{
    Node *head;
    Node *tail;
    Node *buffer; // Cached available nodes
    size_t buffer_size;
    size_t buffer_limit;
    size_t length;
};


static Node *
ds__node_alloc(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size)
{
    if (!nodechain_ptr->buffer)
    {
        for (size_t i = 0; i < LIBDS_NC_BUFFER_BATCH_SIZE; i++)
        {
            // If malloc fails, it keeps successfully allocated nodes
            Node *cached_node = malloc(sizeof(Node) + value_size);
            if (!cached_node) break;

            cached_node->next = nodechain_ptr->buffer;
            nodechain_ptr->buffer = cached_node;
            nodechain_ptr->buffer_size++;
        }

        if (!nodechain_ptr->buffer) return NULL;
    }

    // Pop from cache
    Node *new_node = nodechain_ptr->buffer;
    nodechain_ptr->buffer = nodechain_ptr->buffer->next;
    nodechain_ptr->buffer_size--;

    // Update node fields
    memcpy(new_node->value, value_ptr, value_size);
    new_node->next = NULL;

    // Update nodechain fields
    nodechain_ptr->length++;
    const size_t dynamic_limit = nodechain_ptr->length >> 2; // 25% of length
    nodechain_ptr->buffer_limit = LIBDS__MAX(dynamic_limit, LIBDS_MIN_NC_BUFFER_LIMIT);

    return new_node;
}


static void
ds__node_free(NodeChain *nodechain, Node *node, const Destructor destructor)
{
    if (destructor)
        destructor(node->value);

    if (nodechain->buffer_size < nodechain->buffer_limit)
    {
        // Push to cache
        node->next = nodechain->buffer;
        nodechain->buffer = node;
        nodechain->buffer_size++;
    }
    else free(node);

    // Update nodechain fields
    nodechain->length--;
    const size_t dynamic_limit = nodechain->length >> 2; // 25% of length
    nodechain->buffer_limit = LIBDS__MAX(dynamic_limit, LIBDS_MIN_NC_BUFFER_LIMIT);
}


size_t
ds__nc_length(const NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return 0;
    return nodechain_ptr->length;
}


bool
ds__nc_isempty(const NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return true;
    return nodechain_ptr->length == 0;
}


NodeChain *
ds__nc_alloc(void)
{
    NodeChain *new_structure = malloc(sizeof(NodeChain));
    if (!new_structure) return NULL;

    new_structure->head = NULL;
    new_structure->tail = NULL;
    new_structure->buffer = NULL;
    new_structure->buffer_size = 0;
    new_structure->buffer_limit = 0;
    new_structure->length = 0;

    return new_structure;
}


ds_err_t
ds__nc_free(NodeChain **nodechain_dptr, const Destructor destructor)
{
    if (!nodechain_dptr || !*nodechain_dptr) return LIBDS_ERR_NULL_POINTER;

    ds__nc_clear(*nodechain_dptr, destructor);

    Node *buffer = (*nodechain_dptr)->buffer;
    while (buffer != NULL)
    {
        Node *next = buffer->next;
        free(buffer);
        buffer = next;
    }

    free(*nodechain_dptr);
    *nodechain_dptr = NULL;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_clear(NodeChain *nodechain_ptr, const Destructor destructor)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    Node *node = nodechain_ptr->head;
    while (node != NULL)
    {
        Node *next = node->next;

        ds__node_free(nodechain_ptr, node, destructor);
        node = next;
    }

    nodechain_ptr->head = NULL;
    nodechain_ptr->tail = NULL;
    nodechain_ptr->length = 0;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_assign(NodeChain *dst, const NodeChain *src, const size_t value_size,
    const Destructor destructor, const Copier copier)
{
    if (dst == src) return LIBDS_SUCCESS;
    if (!dst || !src) return LIBDS_ERR_NULL_POINTER;

    NodeChain *new_nodechain = ds__nc_alloc();
    if (!new_nodechain) return LIBDS_ERR_ALLOCATION_FAILED;

    const Node *src_node = src->head;
    while (src_node != NULL)
    {
        Node *new_node = ds__node_alloc(new_nodechain, src_node->value, value_size);
        if (!new_node)
        {
            // If allocation fails, clean up and abort.
            ds__nc_free(&new_nodechain, destructor);
            return LIBDS_ERR_ALLOCATION_FAILED;
        }

        if (copier)
        {
            const ds_err_t err = copier(new_node->value, src_node->value);
            if (err != LIBDS_SUCCESS)
            {
                // If copy fails, clean up and abort
                ds__node_free(new_nodechain, new_node, destructor);
                ds__nc_free(&new_nodechain, destructor);
                return err;
            }
        }

        if (!new_nodechain->head)
            new_nodechain->head = new_node; // Empty nodechain case
        else
            new_nodechain->tail->next = new_node; // Append the new node

        new_nodechain->tail = new_node; // Update tail

        src_node = src_node->next;
    }

    ds__nc_clear(dst, destructor);

    dst->head = new_nodechain->head;
    dst->tail = new_nodechain->tail;
    dst->length = new_nodechain->length;

    const size_t dyn_limit = dst->length >> 2; // 25% of length
    dst->buffer_limit = LIBDS__MAX(dyn_limit, LIBDS_MIN_NC_BUFFER_LIMIT);

    Node *buffer = new_nodechain->buffer;
    while (buffer != NULL)
    {
        Node *next = buffer->next;
        if (dst->buffer_size < dst->buffer_limit)
        {
            buffer->next = dst->buffer;
            dst->buffer = buffer;
            dst->buffer_size++;
        }
        else free(buffer); // when dst->buffer is full

        buffer = next;
    }

    free(new_nodechain);

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_reverse(NodeChain *nodechain_ptr)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    if (nodechain_ptr->length <= 1) return LIBDS_SUCCESS;

    Node *prev_node = NULL;
    Node *next_node = NULL;
    Node *curr_node = nodechain_ptr->head;

    while (curr_node != NULL)
    {
        next_node = curr_node->next;
        curr_node->next = prev_node;

        // After the loop, 'prev_node' is pointing to the last processed node
        prev_node = curr_node;
        curr_node = next_node;
    }

    nodechain_ptr->tail = nodechain_ptr->head;
    nodechain_ptr->head = prev_node;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_push_front(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    Node *new_node = ds__node_alloc(nodechain_ptr, value_ptr, value_size);
    if (!new_node) return LIBDS_ERR_ALLOCATION_FAILED;

    new_node->next = nodechain_ptr->head;

    if (!nodechain_ptr->head)
        nodechain_ptr->tail = new_node;

    nodechain_ptr->head = new_node;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_push_back(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    Node *new_node = ds__node_alloc(nodechain_ptr, value_ptr, value_size);
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

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_push_at(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size, long long index)
{
    if (!nodechain_ptr) return LIBDS_ERR_NULL_POINTER;

    const size_t length = nodechain_ptr->length;

    // Negative index counts from the tail
    if (index < 0)
        index += (long long) length; // Subtract value from length

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

    Node *new_node = ds__node_alloc(nodechain_ptr, value_ptr, value_size);
    if (!new_node) return LIBDS_ERR_ALLOCATION_FAILED;

    new_node->next = prev_node->next;
    prev_node->next = new_node;

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
ds__nc_get_at(const NodeChain *nodechain_ptr, void *output_ptr, const size_t output_size, long long index)
{
    if (!nodechain_ptr || !output_ptr)
        return LIBDS_ERR_NULL_POINTER;

    const size_t length = nodechain_ptr->length;

    // Negative index counts from the tail
    if (index < 0)
        index += (long long) length; // Subtract value from length

    if (index < 0 || (size_t) index >= length)
        return LIBDS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    if (unsigned_index == 0)
        return ds__nc_get_front(nodechain_ptr, output_ptr, output_size);

    if (unsigned_index == length - 1)
        return ds__nc_get_back(nodechain_ptr, output_ptr, output_size);

    const Node *node = nodechain_ptr->head;
    for (size_t i = 0; i < unsigned_index; i++)
        node = node->next;

    memcpy(output_ptr, node->value, output_size);

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_drop_front(NodeChain *nodechain_ptr, const Destructor destructor)
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

    ds__node_free(nodechain_ptr, old_head, destructor);

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_drop_back(NodeChain *nodechain_ptr, const Destructor destructor)
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

    ds__node_free(nodechain_ptr, old_tail, destructor);

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_drop_at(NodeChain *nodechain_ptr, const Destructor destructor, long long index)
{
    if (!nodechain_ptr)
        return LIBDS_ERR_NULL_POINTER;

    if (!nodechain_ptr->head)
        return LIBDS_ERR_EMPTY_STRUCTURE;

    const size_t length = nodechain_ptr->length;

    // Negative index counts from the tail
    if (index < 0)
        index += (long long) length; // Subtract value from length

    if (index < 0 || (size_t) index >= length)
        return LIBDS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    if (unsigned_index == 0)
        return ds__nc_drop_front(nodechain_ptr, destructor);

    if (unsigned_index == length - 1)
        return ds__nc_drop_back(nodechain_ptr, destructor);

    Node *prev_node = nodechain_ptr->head;
    for (size_t i = 0; i < unsigned_index - 1; i++)
        prev_node = prev_node->next;

    Node *node = prev_node->next;
    prev_node->next = node->next;

    ds__node_free(nodechain_ptr, node, destructor);

    return LIBDS_SUCCESS;
}
