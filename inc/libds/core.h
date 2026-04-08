//
// Created by Gabriel Souza on 21/02/2026.
//

#ifndef DATA_STRUCTURES_CORE_H
#define DATA_STRUCTURES_CORE_H

#include <stddef.h>
#include <stdbool.h>
#include <internal/nodechain.h>

typedef struct TypeWrapper
{
    const size_t value_size;
    const size_t value_align;
    const Destructor destructor;
    const Copier copier;
    NodeChain *nodechain_ptr;
} TypeWrapper;

#endif //DATA_STRUCTURES_CORE_H