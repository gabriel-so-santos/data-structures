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
        const size_t chunk_bytes = batch_size * chain->stride;

        Node *new_chunk = (Node *) malloc(sizeof(Node) + chunk_bytes);
        if (!new_chunk) return NULL;

        // push to liked chunks
        new_chunk->next = chain->chunk_head;
        chain->chunk_head = new_chunk;

        byte *memory_chunk = (byte *)(new_chunk + 1);

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