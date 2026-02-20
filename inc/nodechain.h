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

typedef struct
{
    Node *head;
    Node *tail;
    size_t length;
    size_t value_size;
    Destructor destructor_func;
} NodeChain;

void nodechain_init(NodeChain *nodechain_ptr, size_t value_size, Destructor destructor_func);

Node *nodechain_alloc(const void *value_ptr, size_t value_size);

void nodechain_free(NodeChain *nodechain_ptr);

void nodechain_push_front(NodeChain *nodechain_ptr, const void *value_ptr);

void nodechain_push_back(NodeChain *nodechain_ptr, const void *value_ptr);

void nodechain_push_at(NodeChain *nodechain_ptr, const void *value_ptr, ptrdiff_t index);

#endif //DATA_STRUCTURES_NODECHAIN_H