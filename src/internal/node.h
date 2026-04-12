/**
 * @file    internal/node.h
 * @brief   Core memory management for node-based data structures
 *
 * This module provides a custom memory allocator with node recycling capabilities.
 * It maintains pools of fixed-size nodes and utilizes dynamic batch allocation
 * with geometric growth to ensure memory footprint scales efficiently with data
 * structure size.
 *
 * @warning This implementation does NOT PROVIDE THREAD-SAFETY.
 *
 * @author  Gabriel Souza
 * @date    2026-04-09
 */

#ifndef LIBDS_INTERNAL_NODE_H
#define LIBDS_INTERNAL_NODE_H

#include <stddef.h>
#include <stdlib.h>
#include "libds/core.h"
#include "utils.h"

#ifndef LIBDS_NC_MIN_BATCH_SIZE
#define LIBDS_NC_MIN_BATCH_SIZE 8
#endif

#ifndef LIBDS_NC_GROWTH_FACTOR
#define LIBDS_NC_GROWTH_FACTOR 0.125f
#endif

/**
 * @var     MIN_BATCH_SIZE
 * @brief   Typed constant for the minimum batch capacity.
 * @see     LIBDS_NC_MIN_BATCH_SIZE
 */
static const size_t MIN_BATCH_SIZE = (LIBDS_NC_MIN_BATCH_SIZE);

/**
 * @var     GROWTH_FACTOR
 * @brief   Typed constant for the dynamic batch scaling multiplier.
 * @see     LIBDS_NC_GROWTH_FACTOR
 */
static const float GROWTH_FACTOR = (LIBDS_NC_GROWTH_FACTOR);

/**
 * @struct  node
 * @brief   Intrusive memory header for singly-linked elements.
 *
 * Acts as a transparent header preceding the actual user data. The total
 * memory allocated per slot is:
 *      [Node Header]
 *      [  Padding  ]
 *      [ User Data ]
 *
 * @note Type alignment is handled by the node chain structure.
 */
struct node
{
    struct node *next;  /**< Pointer to the next available or active node */
};
typedef struct node Node;


/**
 * @struct  ds_node_chain
 * @brief   State controller for node-based data structures.
 *
 * Manages the lifecycle, tracking, and geometric scaling of memory blocks.
 * It enforces strong encapsulation, tracking active state, recycled nodes,
 * and raw heap footprints to guarantee zero memory leaks upon destruction.
 */
struct ds_node_chain
{
    Node *head;        /**< First active node (NULL if empty) */
    Node *tail;        /**< Last active node (NULL if empty) */

    Node *buffer;      /**< Linked list of raw memory chunks to be freed upon destruction */
    Node *node_stack;  /**< Stack of recycled nodes ready for immediate O(1) use */
    size_t stack_size; /**< Total count of available nodes resting in the node_stack */

    size_t offset;     /**< Byte padding required to reach user data from the Node header */
    size_t stride;     /**< Total physical size of a single slot (Header + Padding + Data) */
    size_t length;     /**< Total count of active nodes currently holding valid data */
};
typedef struct ds_node_chain NodeChain;


/**
 * @brief   Retrieves the memory address of the user payload.
 *
 * @param   chain  Pointer to the active node chain.
 * @param   node   Pointer to the node header.
 *
 * @return  A void pointer targeting the start of the user data segment.
 *
 * @note    Uses byte pointer arithmetic with @p chain->offset.
 *
 * @warning Assumes @p chain and @p node are valid.
 */
static inline void *
get_data(const NodeChain *chain, const Node *node)
{
    return (byte *)node + chain->offset;
}


/**
 * @brief   Acquires a memory slot from the engine pool.
 * @param   chain  Pointer to the active node chain.
 * @return  Pointer to the allocated Node header, or NULL if allocation fails.
 *
 * @details Guarantees amortized O(1) allocation time.
 * - Pops from @p chain->node_stack, if recycled slots exist.
 * - If empty, triggers a geometric heap allocation: O(log N) frequency.
 * - Slices the new chunk into distinct nodes of @p chain->stride size.
 * - Pushes the unused slices to the stack and returns the first one.
 *
 * @warning This function assumes that the fields within @p chain
 *          contain valid, properly aligned values.
 *
 * @note    Increments @p chain->length and
 *          decrements @p chain->stack_size.
 *
 */
static Node *
alloc_node(NodeChain *chain)
{
    if (!chain->node_stack)
    {
        // dynamic batch sizing: geometric growth based of current length
        const size_t batch_size = max(MIN_BATCH_SIZE, chain->length * GROWTH_FACTOR);
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


/**
 * @brief   Releases a node back into the recycling pool.
 *
 * @param   chain    Pointer to the active node chain.
 * @param   node     Pointer to the node being retired.
 * @param   destroy  Optional destructor function to clean up payload memory (may be NUL).
 *
 * @details The memory slot itself is not returned to the OS here. Instead, it
 * is pushed onto the internal @p chain->node_stack for immediate future reuse,
 * preventing memory fragmentation.
 *
 * @note    Decrements @p chain->length and
 *          Increments @p chain->stack_size.
 *
 * @warning The memory at @p node becomes invalid for the user after this call.
 */
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

#endif //LIBDS_INTERNAL_NODE_H