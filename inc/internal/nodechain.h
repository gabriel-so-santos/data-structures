//
// Created by Gabriel Souza on 19/02/2026.
//

#ifndef DATA_STRUCTURES_NODECHAIN_H
#define DATA_STRUCTURES_NODECHAIN_H

#include <stddef.h>
#include "dslib/err.h"

typedef struct NodeChain NodeChain;

typedef void (*Destructor)(void *);

NodeChain *_dslib_nc_alloc(size_t value_size, Destructor destructor_func);

dslib_err_t _dslib_nc_free(NodeChain *nodechain_ptr);

dslib_err_t _dslib_nc_push_front(NodeChain *nodechain_ptr, const void *value_ptr);

dslib_err_t _dslib_nc_push_back(NodeChain *nodechain_ptr, const void *value_ptr);

dslib_err_t _dslib_nc_push_at(NodeChain *nodechain_ptr, const void *value_ptr, ptrdiff_t index);

dslib_err_t _dslib_nc_get_front(const NodeChain *nodechain_ptr, void *output_ptr);

dslib_err_t _dslib_nc_get_back(const NodeChain *nodechain_ptr, void *output_ptr);

#endif //DATA_STRUCTURES_NODECHAIN_H