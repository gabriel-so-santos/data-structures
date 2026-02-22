//
// Created by Gabriel Souza on 21/02/2026.
//

#ifndef DATA_STRUCTURES_CORE_H
#define DATA_STRUCTURES_CORE_H

#include <stddef.h>
#include "internal/nodechain.h"

size_t dslib_length_of(const NodeChain *nodechain_ptr);

int dslib_is_empty(const NodeChain *nodechain_ptr);

#endif //DATA_STRUCTURES_CORE_H