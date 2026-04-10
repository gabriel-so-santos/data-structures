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

#ifndef LIBDS_NC_GROWTH_FACTOR
#define LIBDS_NC_GROWTH_FACTOR 1.25
#endif

struct node { struct node *next; };
typedef struct node Node;

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
get_data(const NodeChain *chain, const Node *node)
{
    return (byte *)node + chain->offset;
}

static Node *
alloc_node(NodeChain *chain)
{
    if (!chain->node_stack)
    {
        // dynamic batch sizing: geometric growth based of current length
        const size_t min_size = LIBDS_NC_MIN_BATCH_SIZE;
        const double growth_factor = LIBDS_NC_GROWTH_FACTOR;

        const size_t dyn_size = chain->length * growth_factor;
        const size_t batch_size = max(min_size, dyn_size);
        const size_t chunk_bytes = batch_size * chain->stride;

        Node *chunk_header = (Node *) malloc(sizeof(Node) + chunk_bytes);
        if (!chunk_header) return NULL;

        chunk_header->next = chain->buffer;
        chain->buffer = chunk_header;

        byte *memory_chunk = (byte *)(chunk_header + 1);

        for (size_t i = 0; i < batch_size; i++)
        {
            Node *cached_node = (Node *)(memory_chunk + (i * chain->stride));

            cached_node->next = chain->node_stack;
            chain->node_stack = cached_node;
            chain->stack_size++;
        }
    }

    // pop from stack of available nodes
    Node *new_node = chain->node_stack;
    chain->node_stack = chain->node_stack->next;
    new_node->next = NULL;

    chain->stack_size--;
    chain->length++;

    return new_node;
}


static void
free_node(NodeChain *chain, Node *node, const ds_destructor_fn destroy)
{
    if (destroy)
    {
        void *data = get_data(chain, node);
        destroy(data);
    }

    // push to stack of available nodes
    node->next = chain->node_stack;
    chain->node_stack = node;

    chain->stack_size++;
    chain->length--;
}

#endif //DATA_STRUCTURES_NODE_H
