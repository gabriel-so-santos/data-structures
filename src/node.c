/**
 * @file    node.c
 * @brief   Memory allocation logic for singly-nodes
 *
 * @author  Gabriel Souza
 * @date    2026-04-12
 */


#include <stdlib.h>
#include "internal/node.h"
#include "internal/utils.h"

Node *
alloc_node(NodeChain *chain)
{
    if (!chain->node_stack)
    {
        // dynamic batch sizing: geometric growth based of current length
        const size_t batch_size = max(MIN_BATCH_SIZE, chain->length * GROWTH_FACTOR);

        // one extra slot for the chunk header
        const size_t total_size = (batch_size +1) * chain->stride;

        Chunk *new_chunk = (Chunk *) malloc(total_size);
        if (!new_chunk) return NULL;

        // push to liked chunks
        new_chunk->next = chain->chunk_head;
        chain->chunk_head = new_chunk;

        // skip the header of the chunk
        byte *memory_chunk = (byte *)new_chunk + chain->stride;

        for (size_t i = 0; i < batch_size; i++)
        {
            // slice the chunk in `chain->stride` spaced slots
            Node *cached_node = (Node *)(memory_chunk + (i * chain->stride));

            // send node to stack
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

void
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