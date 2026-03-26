//
// Created by Gabriel Souza on 21/02/2026.
//

#ifndef DATA_STRUCTURES_CORE_H
#define DATA_STRUCTURES_CORE_H

#include <stddef.h>
#include <internal/nodechain.h>

typedef struct TypeWrapper
{
    const size_t value_size;
    const Destructor destructor;
    NodeChain *nodechain_ptr;
} TypeWrapper;

size_t
ds__length_of(const NodeChain *nodechain_ptr);

int
ds__is_empty(const NodeChain *nodechain_ptr);

#endif //DATA_STRUCTURES_CORE_H