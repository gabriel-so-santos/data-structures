//
// Created by Gabriel Souza on 19/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "libds/core.h"
#include "internal/nodechain.h"

#ifndef LIBDS_MIN_NC_BUFFER_LIMIT
#define LIBDS_MIN_NC_BUFFER_LIMIT 16
#endif

#ifndef LIBDS_NC_MIN_BATCH_SIZE
#define LIBDS_NC_MIN_BATCH_SIZE 8
#endif

/* Alias to a single-byte type for pointer arithmetic and byte-level operations */
typedef unsigned char byte;

/* Alias for internal usage of node chain */
typedef struct ds_node_chain NodeChain;

typedef struct node
{
    struct node *next;
}Node;

struct ds_node_chain
{
    Node *head;        /* First active node in the list (NULL if empty) */
    Node *tail;        /* Last active node in the list (NULL if empty) */

    Node *buffer;      /* Linked list of allocated memory blocks (ownership retained for later free) */
    Node *node_stack;  /* Linked list of available (recycled) nodes ready for reuse */
    size_t stack_size;  /* Number of nodes currently stored in the stack of free nodes */

    size_t offset;     /* Byte offset from the node pointer to the stored value */
    size_t stride;     /* Total size of each node slot (Node + padding + value) */
    size_t length;     /* Quantity of active nodes in the structure */
};

static inline size_t
max(const size_t x, const size_t y)
{
    return x > y ? x : y;
}

static inline size_t
min(const size_t x, const size_t y)
{
    return x < y ? x : y;
}

static inline size_t
align_value(const size_t value, const size_t align)
{
    return (value + align -1) & ~(align -1);
}

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
        const size_t batch_size = max(LIBDS_NC_MIN_BATCH_SIZE, dyn_size);
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
free_node(NodeChain *chain_ptr, Node *node, const DSDestructor destroy_fn)
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


size_t
ds_nc_length(const NodeChain *chain_ptr)
{
    if (!chain_ptr) return 0;
    return chain_ptr->length;
}


bool
ds_nc_is_empty(const NodeChain *chain_ptr)
{
    if (!chain_ptr) return true;
    return chain_ptr->length == 0;
}


NodeChain *
ds_nc_alloc(const size_t value_size, const size_t value_align)
{
    NodeChain *new_chain = (NodeChain *) malloc(sizeof(NodeChain));
    if (!new_chain) return NULL;

    const size_t max_align = max(sizeof(Node), value_align);
    const size_t payload_offset = align_value(sizeof(Node), value_align);
    const size_t node_stride = align_value(payload_offset + value_size, max_align);

    new_chain->head = NULL;
    new_chain->tail = NULL;

    new_chain->buffer = NULL;
    new_chain->node_stack = NULL;
    new_chain->stack_size = 0;

    new_chain->offset = payload_offset;
    new_chain->stride = node_stride;
    new_chain->length = 0;

    return new_chain;
}


ds_error_t
ds_nc_free(NodeChain **chain_dptr, const ds_destructor_t destroy_fn)
{
    if (!chain_dptr || !*chain_dptr) return DS_ERR_NULL_POINTER;

    if (destroy_fn)
    {
        const Node *node = (*chain_dptr)->head;
        while (node != NULL)
        {
            void *data_ptr = get_data(*chain_dptr, node);
            destroy_fn(data_ptr);
            node = node->next;
        }
    }

    Node *buffer = (*chain_dptr)->buffer;
    while (buffer != NULL)
    {
        Node *next = buffer->next;
        free(buffer);
        buffer = next;
    }

    free(*chain_dptr);
    *chain_dptr = NULL;
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_clear(NodeChain *chain_ptr, const ds_destructor_t destroy_fn)
{
    if (!chain_ptr) return DS_ERR_NULL_POINTER;
    if (chain_ptr->length == 0) return DS_ERR_NONE;

    if (destroy_fn)
    {
        const Node *node = chain_ptr->head;
        while (node != NULL)
        {
            void *data_ptr = get_data(chain_ptr, node);
            destroy_fn(data_ptr);
            node = node->next;
        }
    }

    // push to stack (available nodes)
    chain_ptr->tail->next = chain_ptr->node_stack;
    chain_ptr->node_stack = chain_ptr->head;
    chain_ptr->stack_size += chain_ptr->length;

    chain_ptr->head = NULL;
    chain_ptr->tail = NULL;
    chain_ptr->length = 0;
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_copy(NodeChain *dst_chain, const NodeChain *src_chain,
    const size_t value_size, const size_t value_align,
    const ds_copier_t copy_fn, const ds_destructor_t destroy_fn)
{
    if (!dst_chain || !src_chain) return DS_ERR_NULL_POINTER;
    if (dst_chain == src_chain) return DS_ERR_NONE;

    NodeChain *new_chain = ds_nc_alloc(value_size, value_align);
    if (!new_chain) return DS_ERR_ALLOCATION_FAILED;

    const Node *src_node = src_chain->head;
    while (src_node != NULL)
    {
        Node *new_node = alloc_node(new_chain);
        if (!new_node)
        {
            // clean up and abort on allocation fail
            ds_nc_free(&new_chain, destroy_fn);
            return DS_ERR_ALLOCATION_FAILED;
        }

        const void *src_value = get_data(src_chain, src_node);
        void *data_ptr = get_data(new_chain, new_node);
        memcpy(data_ptr, src_value, value_size);

        // update head in the first iteration
        if (!new_chain->head)
            new_chain->head = new_node;
        else
            new_chain->tail->next = new_node;

        new_chain->tail = new_node;

        if (copy_fn)
        {
            const ds_error_t status = copy_fn(data_ptr, src_value);
            if (status != DS_ERR_NONE)
            {
                // clean up and abort on copy fail
                free_node(new_chain, new_node, destroy_fn);
                ds_nc_free(&new_chain, destroy_fn);
                return status;
            }
        }

        src_node = src_node->next;
    }

    ds_nc_clear(dst_chain, destroy_fn);

    dst_chain->head = new_chain->head;
    dst_chain->tail = new_chain->tail;
    dst_chain->length = new_chain->length;

    if (new_chain->node_stack)
    {
        Node *cache_tail = new_chain->node_stack;

        while (cache_tail->next != NULL)
            cache_tail = cache_tail->next;

        cache_tail->next = dst_chain->node_stack;
        dst_chain->node_stack = new_chain->node_stack;
        dst_chain->stack_size += new_chain->stack_size;
    }

    if (new_chain->buffer)
    {
        Node *buffer_tail = new_chain->buffer;

        while (buffer_tail->next != NULL)
            buffer_tail = buffer_tail->next;

        buffer_tail->next = dst_chain->buffer;
        dst_chain->buffer = new_chain->buffer;
    }

    free(new_chain);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_reverse(NodeChain *chain_ptr)
{
    if (!chain_ptr) return DS_ERR_NULL_POINTER;
    if (chain_ptr->length <= 1) return DS_ERR_NONE;

    Node *prev_node = NULL;
    Node *next_node = NULL;
    Node *curr_node = chain_ptr->head;

    while (curr_node != NULL)
    {
        next_node = curr_node->next;
        curr_node->next = prev_node;

        // after the loop, prev_node points to the last processed node
        prev_node = curr_node;
        curr_node = next_node;
    }

    chain_ptr->tail = chain_ptr->head;
    chain_ptr->head = prev_node;
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_push_front(NodeChain *chain_ptr, void **data_dptr)
{
    if (!chain_ptr || !data_dptr) return DS_ERR_NULL_POINTER;

    Node *new_node = alloc_node(chain_ptr);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    new_node->next = chain_ptr->head;

    // empty list case, tail points to the new node
    if (!chain_ptr->head)
        chain_ptr->tail = new_node;

    chain_ptr->head = new_node;

    *data_dptr = get_data(chain_ptr, new_node);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_push_back(NodeChain *chain_ptr, void **data_dptr)
{
    if (!chain_ptr || !data_dptr) return DS_ERR_NULL_POINTER;

    Node *new_node = alloc_node(chain_ptr);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    // empty structure case
    if (!chain_ptr->head)
    {
        chain_ptr->head = new_node;
        chain_ptr->tail = new_node;
    }
    // filled structure case
    else
    {
        chain_ptr->tail->next = new_node;
        chain_ptr->tail = new_node;
    }

    *data_dptr = get_data(chain_ptr, new_node);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_push_at(NodeChain *chain_ptr, void **data_dptr, long long index)
{
    if (!chain_ptr || !data_dptr)
        return DS_ERR_NULL_POINTER;

    const size_t length = chain_ptr->length;

    // if negative, subtract index from length
    if (index < 0) index += (long long) length;

    if (index < 0 || (size_t) index > length)
        return DS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    // insert the node at beginning
    if (unsigned_index == 0)
        return ds_nc_push_front(chain_ptr, data_dptr);

    // insert the node at end
    if (unsigned_index == length)
        return ds_nc_push_back(chain_ptr, data_dptr);

    // insert the node in between
    Node *prev_node = chain_ptr->head;
    for (size_t i = 0; i < unsigned_index - 1; i++)
        prev_node = prev_node->next;

    Node *new_node = alloc_node(chain_ptr);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    new_node->next = prev_node->next;
    prev_node->next = new_node;

    *data_dptr = get_data(chain_ptr, new_node);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_get_front(const NodeChain *chain_ptr, void **out_dptr)
{
    if (!chain_ptr || !out_dptr) return DS_ERR_NULL_POINTER;
    if (!chain_ptr->head) return DS_ERR_EMPTY_STRUCTURE;

    *out_dptr = get_data(chain_ptr, chain_ptr->head);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_get_back(const NodeChain *chain_ptr, void **out_dptr)
{
    if (!chain_ptr || !out_dptr) return DS_ERR_NULL_POINTER;
    if (!chain_ptr->tail) return DS_ERR_EMPTY_STRUCTURE;

    *out_dptr = get_data(chain_ptr, chain_ptr->tail);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_get_at(const NodeChain *chain_ptr, void **out_dptr, long long index)
{
    if (!chain_ptr || !out_dptr) return DS_ERR_NULL_POINTER;

    const size_t length = chain_ptr->length;

    // if negative, subtract index from length
    if (index < 0) index += (long long) length;

    if (index < 0 || (size_t) index >= length)
        return DS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    // get the first node
    if (unsigned_index == 0)
        return ds_nc_get_front(chain_ptr, out_dptr);

    // get the last node
    if (unsigned_index == length - 1)
        return ds_nc_get_back(chain_ptr, out_dptr);

    // get a node in between
    const Node *node = chain_ptr->head;
    for (size_t i = 0; i < unsigned_index; i++)
        node = node->next;

    *out_dptr = get_data(chain_ptr, node);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_drop_front(NodeChain *chain_ptr, const ds_destructor_t destroy_fn)
{
    if (!chain_ptr) return DS_ERR_NULL_POINTER;
    if (!chain_ptr->head) return DS_ERR_EMPTY_STRUCTURE;

    Node *old_head = chain_ptr->head;
    chain_ptr->head = old_head->next;

    // if the structure is now empty, tail must be set to NULL
    if (!chain_ptr->head)
        chain_ptr->tail = NULL;

    free_node(chain_ptr, old_head, destroy_fn);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_drop_back(NodeChain *chain_ptr, const ds_destructor_t destroy_fn)
{
    if (!chain_ptr) return DS_ERR_NULL_POINTER;
    if (!chain_ptr->tail) return DS_ERR_EMPTY_STRUCTURE;

    Node *old_tail = chain_ptr->tail;

    // empty structure case
    if (chain_ptr->head == chain_ptr->tail)
    {
        chain_ptr->head = NULL;
        chain_ptr->tail = NULL;
    }
    // filled structure case
    else
    {
        Node *tail_prev = chain_ptr->head;

        while (tail_prev->next != old_tail)
            tail_prev = tail_prev->next;

        tail_prev->next = NULL;
        chain_ptr->tail = tail_prev;
    }

    free_node(chain_ptr, old_tail, destroy_fn);
    return DS_ERR_NONE;
}


ds_error_t
ds_nc_drop_at(NodeChain *chain_ptr, const ds_destructor_t destroy_fn, long long index)
{
    if (!chain_ptr) return DS_ERR_NULL_POINTER;
    if (!chain_ptr->head) return DS_ERR_EMPTY_STRUCTURE;

    const size_t length = chain_ptr->length;

    // if negative, subtract index from length
    if (index < 0) index += (long long) length;

    if (index < 0 || (size_t) index >= length)
        return DS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t unsigned_index = (size_t) index;

    // drop the first node
    if (unsigned_index == 0)
        return ds_nc_drop_front(chain_ptr, destroy_fn);

    // drop the last node
    if (unsigned_index == length - 1)
        return ds_nc_drop_back(chain_ptr, destroy_fn);

    // drop a node in between
    Node *prev_node = chain_ptr->head;
    for (size_t i = 0; i < unsigned_index - 1; i++)
        prev_node = prev_node->next;

    Node *node = prev_node->next;
    prev_node->next = node->next;

    free_node(chain_ptr, node, destroy_fn);
    return DS_ERR_NONE;
}
