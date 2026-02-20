//
// Created by Gabriel Souza on 19/02/2026.
//

#ifndef DATA_STRUCTURES_NODECHAIN_H
#define DATA_STRUCTURES_NODECHAIN_H

#include <stddef.h>

typedef struct Node
{
    struct Node *next;
    unsigned char data[];  // Flexible array member
} Node;

typedef void (*Destructor)(void *);

typedef struct NodeChain
{
    Node *head;
    Node *tail;
    size_t length;
    size_t element_size;
    Destructor destructor_func;
} NodeChain;

void nodechain_init(NodeChain *node_chain, size_t element_size, Destructor destructor_func);

Node *nodechain_alloc(const void *data, size_t element_size);

void nodechain_free(NodeChain *node_chain);

void nodechain_push_front(NodeChain *node_chain, const void *data);

void nodechain_push_back(NodeChain *node_chain, const void *data);

void nodechain_push_at(NodeChain *node_chain, const void *data, ptrdiff_t index);

#endif //DATA_STRUCTURES_NODECHAIN_H