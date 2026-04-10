//
// Created by Gabriel Souza on 09/04/2026.
//

#ifndef DATA_STRUCTURES_NODE_H
#define DATA_STRUCTURES_NODE_H

#include <stddef.h>
#include "utils.h"

#ifndef LIBDS_NC_MIN_BATCH_SIZE
#define LIBDS_NC_MIN_BATCH_SIZE 8
#endif

typedef struct node
{
    struct node *next;
}Node;

/* Alias for internal usage of node chain */
typedef struct ds_node_chain NodeChain;

struct ds_node_chain
{
    Node *head;        /* First active node in the list (NULL if empty) */
    Node *tail;        /* Last active node in the list (NULL if empty) */

    Node *buffer;      /* Linked list of allocated memory blocks (ownership retained for later free) */
    Node *node_stack;  /* Linked list of available (recycled) nodes ready for reuse */
    size_t stack_size; /* Number of nodes currently stored in the stack of free nodes */

    size_t offset;     /* Byte offset from the node pointer to the stored value */
    size_t stride;     /* Total size of each node slot (Node + padding + value) */
    size_t length;     /* Quantity of active nodes in the structure */
};

static inline void *
get_data(const NodeChain *chain_ptr, const Node *node)
{
    void *data_ptr = (byte *)node + chain_ptr->offset;
    return data_ptr;
}

static Node *
alloc_node(NodeChain *chain_ptr)
{
    if (!chain_ptr->node_stack)
    {
        // dynamic batch sizing: geometric growth: +12.5% of current length
        const size_t dyn_size = chain_ptr->length >> 3;
        const size_t min_size = LIBDS_NC_MIN_BATCH_SIZE;

        const size_t batch_size = max(min_size, dyn_size);
        const size_t chunk_bytes = batch_size * chain_ptr->stride;

        Node *chunk_header = (Node *) malloc(sizeof(Node) + chunk_bytes);
        if (!chunk_header) return NULL;

        chunk_header->next = chain_ptr->buffer;
        chain_ptr->buffer = chunk_header;

        byte *memory_chunk = (byte *)(chunk_header + 1);

        for (size_t i = 0; i < batch_size; i++)
        {
            Node *cached_node = (Node *)(memory_chunk + (i * chain_ptr->stride));

            cached_node->next = chain_ptr->node_stack;
            chain_ptr->node_stack = cached_node;
            chain_ptr->stack_size++;
        }
    }

    // pop from stack (available nodes)
    Node *new_node = chain_ptr->node_stack;
    chain_ptr->node_stack = chain_ptr->node_stack->next;
    new_node->next = NULL;

    chain_ptr->stack_size--;
    chain_ptr->length++;

    return new_node;
}


static void
free_node(NodeChain *chain_ptr, Node *node, const ds_destructor_t destroy_fn)
{
    if (destroy_fn)
    {
        void *data_ptr = get_data(chain_ptr, node);
        destroy_fn(data_ptr);
    }

    // push to stack (available nodes)
    node->next = chain_ptr->node_stack;
    chain_ptr->node_stack = node;

    chain_ptr->stack_size++;
    chain_ptr->length--;
}

#endif //DATA_STRUCTURES_NODE_H
