//
// Created by Gabriel Souza on 19/02/2026.
//

#ifndef DATA_STRUCTURES_NODECHAIN_H
#define DATA_STRUCTURES_NODECHAIN_H

#include <stddef.h>

typedef struct Node
{
    struct Node *next;
    unsigned char value[];  // Flexible array member
} Node;

typedef void (*Destructor)(void *);

typedef struct NodeChain NodeChain;

typedef enum
{
    DS_SUCCESS = 0,
    DS_ERR_ALLOCATION_FAILED = -1,
    DS_ERR_NULL_POINTER = -2,
    DS_ERR_INDEX_OUT_OF_BOUNDS = -3,
    DS_ERR_EMPTY_STRUCTURE = -4,
} DSErrCode;

Node *nodechain_alloc(const void *value_ptr, size_t value_size);

NodeChain *nodechain_create(size_t value_size, Destructor destructor_func);

size_t nodechain_length(const NodeChain *nodechain_ptr);

int nodechain_is_empty(const NodeChain *nodechain_ptr);

DSErrCode nodechain_free(NodeChain *nodechain_ptr);

DSErrCode nodechain_push_front(NodeChain *nodechain_ptr, const void *value_ptr);

DSErrCode nodechain_push_back(NodeChain *nodechain_ptr, const void *value_ptr);

DSErrCode nodechain_push_at(NodeChain *nodechain_ptr, const void *value_ptr, ptrdiff_t index);

DSErrCode nodechain_get_front(const NodeChain *nodechain_ptr, void *output_ptr);

DSErrCode nodechain_get_back(const NodeChain *nodechain_ptr, void *output_ptr);

#endif //DATA_STRUCTURES_NODECHAIN_H