//
// Created by Gabriel Souza on 19/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "libds/core.h"
#include "libds/impl/nodechain.h"
#include "internal/utils.h"
#include "internal/node.h"

#ifndef LIBDS_NC_MIN_BATCH_SIZE
#define LIBDS_NC_MIN_BATCH_SIZE 8
#endif


size_t
ds_nc_length(const NodeChain *chain)
{
    if (!chain) return 0;
    return chain->length;
}


bool
ds_nc_is_empty(const NodeChain *chain)
{
    if (!chain) return true;
    return chain->length == 0;
}


size_t
ds_nc_bytes(const NodeChain *chain)
{
    if (!chain) return 0;

    size_t chunk_count = 0;
    const Node *curr_chunk = chain->buffer;
    while (curr_chunk != NULL)
    {
        chunk_count++;
        curr_chunk = curr_chunk->next;
    }

    const size_t struct_size = sizeof(NodeChain);
    const size_t nodes_total_size = (chain->length + chain->stack_size) * chain->stride;
    const size_t buffer_headers_size = chunk_count * sizeof(Node);

    return struct_size + nodes_total_size + buffer_headers_size;
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


enum ds_error
ds_nc_free(NodeChain **chain_ref, const ds_destructor_fn destroy)
{
    if (!chain_ref || !*chain_ref) return DS_ERR_NULL_POINTER;

    if (destroy)
    {
        const Node *node = (*chain_ref)->head;
        while (node != NULL)
        {
            void *data = get_data(*chain_ref, node);
            destroy(data);
            node = node->next;
        }
    }

    Node *buffer = (*chain_ref)->buffer;
    while (buffer != NULL)
    {
        Node *next = buffer->next;
        free(buffer);
        buffer = next;
    }

    free(*chain_ref);
    *chain_ref = NULL;
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_clear(NodeChain *chain, const ds_destructor_fn destroy)
{
    if (!chain) return DS_ERR_NULL_POINTER;
    if (chain->length == 0) return DS_ERR_NONE;

    if (destroy)
    {
        const Node *node = chain->head;
        while (node != NULL)
        {
            void *data = get_data(chain, node);
            destroy(data);
            node = node->next;
        }
    }

    // push to stack of available nodes
    chain->tail->next = chain->node_stack;
    chain->node_stack = chain->head;
    chain->stack_size += chain->length;

    chain->head = NULL;
    chain->tail = NULL;
    chain->length = 0;
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_copy(NodeChain *dst_chain, const NodeChain *src_chain, const size_t value_size,
    const ds_copier_fn copy, const ds_destructor_fn destroy)
{
    if (!dst_chain || !src_chain) return DS_ERR_NULL_POINTER;
    if (dst_chain == src_chain) return DS_ERR_NONE;

    // detach original data to allow rollback on failure
    const Node *old_head = dst_chain->head;
    const Node *old_tail = dst_chain->tail;
    const size_t old_length = dst_chain->length;

    dst_chain->head = NULL;
    dst_chain->tail = NULL;
    dst_chain->length = 0;

    const Node *src_node = src_chain->head;
    while (src_node != NULL)
    {
        Node *new_node = alloc_node(dst_chain);
        if (!new_node)
        {
            // rollback
            ds_nc_clear(dst_chain, destroy);
            dst_chain->head = (Node *)old_head;
            dst_chain->tail = (Node *)old_tail;
            dst_chain->length = old_length;

            return DS_ERR_ALLOCATION_FAILED;
        }

        const void *src_value = get_data(src_chain, src_node);
        void *dst_value = get_data(dst_chain, new_node);

        if (!copy)
            memcpy(dst_value, src_value, value_size);
        else
        {
            if ( !copy(dst_value, src_value) )
            {
                // the current new_node is invalid, drop it without destroying
                free_node(dst_chain, new_node, NULL);

                // rollback
                ds_nc_clear(dst_chain, destroy);
                dst_chain->head = (Node *)old_head;
                dst_chain->tail = (Node *)old_tail;
                dst_chain->length = old_length;

                return DS_ERR_COPY_FAILED;
            }
        }

        // update head in the first iteration
        if (!dst_chain->head)
            dst_chain->head = new_node;
        else
            dst_chain->tail->next = new_node;

        dst_chain->tail = new_node;

        src_node = src_node->next;
    }

    Node *curr_node = (Node *)old_head;
    Node *next_node = NULL;
    while (curr_node != NULL)
    {
        if (destroy)
        {
            void *data_ptr = get_data(dst_chain, curr_node);
            destroy(data_ptr);
        }

        next_node = curr_node->next;

        curr_node->next = dst_chain->node_stack;
        dst_chain->node_stack = curr_node;
        dst_chain->stack_size++;

        curr_node = next_node;
    }

    return DS_ERR_NONE;
}


enum ds_error
ds_nc_reverse(NodeChain *chain)
{
    if (!chain) return DS_ERR_NULL_POINTER;
    if (chain->length <= 1) return DS_ERR_NONE;

    Node *prev_node = NULL;
    Node *next_node = NULL;
    Node *curr_node = chain->head;

    while (curr_node != NULL)
    {
        next_node = curr_node->next;
        curr_node->next = prev_node;

        // after the loop, prev_node points to the last processed node
        prev_node = curr_node;
        curr_node = next_node;
    }

    chain->tail = chain->head;
    chain->head = prev_node;
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_push_front(NodeChain *chain, void **out)
{
    if (!chain || !out) return DS_ERR_NULL_POINTER;

    Node *new_node = alloc_node(chain);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    new_node->next = chain->head;

    // empty list case, tail points to the new node
    if (!chain->head)
        chain->tail = new_node;

    chain->head = new_node;

    *out = get_data(chain, new_node);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_push_back(NodeChain *chain, void **out)
{
    if (!chain || !out) return DS_ERR_NULL_POINTER;

    Node *new_node = alloc_node(chain);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    // empty structure case
    if (!chain->head)
    {
        chain->head = new_node;
        chain->tail = new_node;
    }
    // filled structure case
    else
    {
        chain->tail->next = new_node;
        chain->tail = new_node;
    }

    *out = get_data(chain, new_node);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_push_at(NodeChain *chain, void **out, long long index)
{
    if (!chain || !out)
        return DS_ERR_NULL_POINTER;

    const size_t length = chain->length;

    // if negative, subtract index from length
    if (index < 0) index += (long long) length;

    if (index < 0 || (size_t) index > length)
        return DS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t uindex = (size_t) index;

    // insert the node at beginning
    if (uindex == 0) return ds_nc_push_front(chain, out);

    // insert the node at end
    if (uindex == length) return ds_nc_push_back(chain, out);

    // insert the node in between
    Node *prev_node = chain->head;
    for (size_t i = 0; i < uindex - 1; i++)
        prev_node = prev_node->next;

    Node *new_node = alloc_node(chain);
    if (!new_node) return DS_ERR_ALLOCATION_FAILED;

    new_node->next = prev_node->next;
    prev_node->next = new_node;

    *out = get_data(chain, new_node);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_get_front(const NodeChain *chain, void **out)
{
    if (!chain || !out) return DS_ERR_NULL_POINTER;
    if (!chain->head) return DS_ERR_EMPTY_STRUCTURE;

    *out = get_data(chain, chain->head);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_get_back(const NodeChain *chain, void **out)
{
    if (!chain || !out) return DS_ERR_NULL_POINTER;
    if (!chain->tail) return DS_ERR_EMPTY_STRUCTURE;

    *out = get_data(chain, chain->tail);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_get_at(const NodeChain *chain, void **out, long long index)
{
    if (!chain || !out) return DS_ERR_NULL_POINTER;

    const size_t length = chain->length;

    // if negative, subtract index from length
    if (index < 0) index += (long long) length;

    if (index < 0 || (size_t) index >= length)
        return DS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t uindex = (size_t) index;

    // get the first node
    if (uindex == 0) return ds_nc_get_front(chain, out);

    // get the last node
    if (uindex == length -1) return ds_nc_get_back(chain, out);

    // get a node in between
    const Node *node = chain->head;
    for (size_t i = 0; i < uindex; i++)
        node = node->next;

    *out = get_data(chain, node);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_pop_front(NodeChain *chain, const ds_destructor_fn destroy, void **out)
{
    if (!chain) return DS_ERR_NULL_POINTER;
    if (!chain->head) return DS_ERR_EMPTY_STRUCTURE;

    Node *old_head = chain->head;
    chain->head = old_head->next;

    // if the structure is now empty, tail must be set to NULL
    if (!chain->head) chain->tail = NULL;

    if (out) *out = get_data(chain, old_head);

    free_node(chain, old_head, destroy);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_pop_back(NodeChain *chain, const ds_destructor_fn destroy, void **out)
{
    if (!chain) return DS_ERR_NULL_POINTER;
    if (!chain->tail) return DS_ERR_EMPTY_STRUCTURE;

    Node *old_tail = chain->tail;

    // empty structure case
    if (chain->head == chain->tail)
    {
        chain->head = NULL;
        chain->tail = NULL;
    }
    // filled structure case
    else
    {
        Node *tail_prev = chain->head;

        while (tail_prev->next != old_tail)
            tail_prev = tail_prev->next;

        tail_prev->next = NULL;
        chain->tail = tail_prev;
    }

    if (out) *out = get_data(chain, old_tail);

    free_node(chain, old_tail, destroy);
    return DS_ERR_NONE;
}


enum ds_error
ds_nc_pop_at(NodeChain *chain, const ds_destructor_fn destroy,
    void **out, long long index)
{
    if (!chain) return DS_ERR_NULL_POINTER;
    if (!chain->head) return DS_ERR_EMPTY_STRUCTURE;

    const size_t length = chain->length;

    // if negative, subtract index from length
    if (index < 0) index += (long long) length;

    if (index < 0 || (size_t) index >= length)
        return DS_ERR_INDEX_OUT_OF_BOUNDS;

    const size_t uindex = (size_t) index;

    // pop the first node
    if (uindex == 0) return ds_nc_pop_front(chain, destroy, out);

    // pop the last node
    if (uindex == length - 1) return ds_nc_pop_back(chain, destroy, out);

    // pop a node in between
    Node *prev_node = chain->head;
    for (size_t i = 0; i < uindex -1; i++)
        prev_node = prev_node->next;

    Node *node = prev_node->next;
    prev_node->next = node->next;

    if (out) *out = get_data(chain, node);

    free_node(chain, node, destroy);
    return DS_ERR_NONE;
}
