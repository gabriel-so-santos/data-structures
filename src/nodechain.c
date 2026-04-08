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

#ifndef LIBDS_NC_MIN_BATCH_SIZE
    #define LIBDS_NC_MIN_BATCH_SIZE 4
#endif

#define LIBDS_MIN(x, y) ((x) < (y) ? (x) : (y))
#define LIBDS_MAX(x, y) ((x) > (y) ? (x) : (y))
#define LIBDS_ALIGNUP(value, align) ( ((value) + (align) - 1) & ~((align) - 1) )

typedef unsigned char Byte;

typedef struct Node
{
    struct Node *next;
} Node;

struct NodeChain
{
    Node *head;
    Node *tail;
    Node *buffer;
    Node *nodecache; // Cached available nodes
    size_t nodecache_size;
    //size_t nodecache_limit;
    size_t payload_offset;
    size_t node_stride;
    size_t length;
};



static Node *
ds__node_alloc(NodeChain *nodechain_ptr, const void *value_ptr, const size_t value_size)
{
    if (!nodechain_ptr->nodecache)
    {
        // Dynamic batch sizing (Geometric growth: +12.5% of current length)
        const size_t dyn_size = nodechain_ptr->length >> 3;
        const size_t batch_size = LIBDS_MAX(LIBDS_NC_MIN_BATCH_SIZE, dyn_size);
        const size_t chunk_bytes = batch_size * nodechain_ptr->node_stride;

        Node *chunk_header = (Node *) malloc(sizeof(Node) + chunk_bytes);
        if (!chunk_header) return NULL;

        chunk_header->next = nodechain_ptr->buffer;
        nodechain_ptr->buffer = chunk_header;

        Byte *memory_chunk = (Byte *)(chunk_header + 1);

        for (size_t i = 0; i < batch_size; i++)
        {
            Node *cached_node = (Node *)(memory_chunk + (i * nodechain_ptr->node_stride));

            cached_node->next = nodechain_ptr->nodecache;
            nodechain_ptr->nodecache = cached_node;
            nodechain_ptr->nodecache_size++;
        }
    }

    // Pop from cache
    Node *new_node = nodechain_ptr->nodecache;
    nodechain_ptr->nodecache = nodechain_ptr->nodecache->next;
    nodechain_ptr->nodecache_size--;

    // Update fields
    void *node_value = (Byte *)new_node + nodechain_ptr->payload_offset;
    memcpy(node_value, value_ptr, value_size);
    new_node->next = NULL;
    nodechain_ptr->length++;

    return new_node;
}


static void
ds__node_free(NodeChain *nodechain, Node *node, const Destructor destructor)
{
    if (destructor)
    {
        void *node_value = (Byte *)node + nodechain->payload_offset;
        destructor(node_value);
    }

    // Push to cache
    node->next = nodechain->nodecache;
    nodechain->nodecache = node;
    nodechain->nodecache_size++;

    nodechain->length--;
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
ds__nc_alloc(const size_t value_size, const size_t value_align)
{
    NodeChain *new_nodechain = (NodeChain *) malloc(sizeof(NodeChain));
    if (!new_nodechain) return NULL;

    new_nodechain->head = NULL;
    new_nodechain->tail = NULL;
    new_nodechain->nodecache = NULL;
    new_nodechain->buffer = NULL;
    new_nodechain->nodecache_size = 0;
    new_nodechain->length = 0;

    const size_t max_align = LIBDS_MAX(sizeof(Node), value_align);
    const size_t payload_offset = LIBDS_ALIGNUP(sizeof(Node), value_align);

    new_nodechain->payload_offset = payload_offset;
    new_nodechain->node_stride = LIBDS_ALIGNUP(payload_offset + value_size, max_align);

    return new_nodechain;
}


ds_err_t
ds__nc_free(NodeChain **nodechain_dptr, const Destructor destructor)
{
    if (!nodechain_dptr || !*nodechain_dptr) return LIBDS_ERR_NULL_POINTER;

    if (destructor)
    {
        Node *node = (*nodechain_dptr)->head;
        while (node != NULL)
        {
            void *node_value = (Byte *)node + (*nodechain_dptr)->payload_offset;
            destructor(node_value);
            node = node->next;
        }
    }

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
    if (nodechain_ptr->length == 0) return LIBDS_SUCCESS;

    if (destructor)
    {
        Node *node = nodechain_ptr->head;
        while (node != NULL)
        {
            void *node_value = (Byte *)node + nodechain_ptr->payload_offset;
            destructor(node_value);
            node = node->next;
        }
    }

    // Push to cache
    nodechain_ptr->tail->next = nodechain_ptr->nodecache;
    nodechain_ptr->nodecache = nodechain_ptr->head;
    nodechain_ptr->nodecache_size += nodechain_ptr->length;

    nodechain_ptr->head = NULL;
    nodechain_ptr->tail = NULL;
    nodechain_ptr->length = 0;

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_assign(NodeChain *dst, const NodeChain *src,
    const size_t value_size, const size_t value_align,
    const Destructor destructor, const Copier copier)
{
    if (!dst || !src) return LIBDS_ERR_NULL_POINTER;
    if (dst == src) return LIBDS_SUCCESS;

    NodeChain *new_nodechain = ds__nc_alloc(value_size, value_align);
    if (!new_nodechain) return LIBDS_ERR_ALLOCATION_FAILED;

    const Node *src_node = src->head;
    while (src_node != NULL)
    {
        void *node_value = (Byte *)src_node + src->payload_offset;

        Node *new_node = ds__node_alloc(new_nodechain, node_value, value_size);
        if (!new_node)
        {
            // If allocation fails, clean up and abort.
            ds__nc_free(&new_nodechain, destructor);
            return LIBDS_ERR_ALLOCATION_FAILED;
        }

        if (!new_nodechain->head)
            new_nodechain->head = new_node;
        else
            new_nodechain->tail->next = new_node;

        new_nodechain->tail = new_node;

        if (copier)
        {
            void *new_node_value = (Byte *)new_node + new_nodechain->payload_offset;
            const ds_err_t err = copier(new_node_value, node_value);
            if (err != LIBDS_SUCCESS)
            {
                // If copy fails, clean up and abort
                ds__node_free(new_nodechain, new_node, destructor);
                ds__nc_free(&new_nodechain, destructor);
                return err;
            }
        }

        src_node = src_node->next;
    }

    ds__nc_clear(dst, destructor);

    dst->head = new_nodechain->head;
    dst->tail = new_nodechain->tail;
    dst->length = new_nodechain->length;

    if (new_nodechain->nodecache)
    {
        Node *cache_tail = new_nodechain->nodecache;
        while (cache_tail->next != NULL) {
            cache_tail = cache_tail->next;
        }
        cache_tail->next = dst->nodecache;
        dst->nodecache = new_nodechain->nodecache;
        dst->nodecache_size += new_nodechain->nodecache_size;
    }

    if (new_nodechain->buffer)
    {
        Node *buffer_tail = new_nodechain->buffer;
        while (buffer_tail->next != NULL) {
            buffer_tail = buffer_tail->next;
        }
        buffer_tail->next = dst->buffer;
        dst->buffer = new_nodechain->buffer;
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

    const void *head_value = (Byte *)nodechain_ptr->head + nodechain_ptr->payload_offset;
    memcpy(output_ptr, head_value, output_size);

    return LIBDS_SUCCESS;
}


ds_err_t
ds__nc_get_back(const NodeChain *nodechain_ptr, void *output_ptr, const size_t output_size)
{
    if (!nodechain_ptr || !output_ptr)
        return LIBDS_ERR_NULL_POINTER;

    if (!nodechain_ptr->tail)
        return LIBDS_ERR_EMPTY_STRUCTURE;

    const void *tail_value = (Byte *)nodechain_ptr->tail + nodechain_ptr->payload_offset;
    memcpy(output_ptr, tail_value, output_size);

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

    const void *node_value = (Byte *)node + nodechain_ptr->payload_offset;
    memcpy(output_ptr, node_value, output_size);

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
