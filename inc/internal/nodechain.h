//
// Created by Gabriel Souza on 19/02/2026.
//

#ifndef DATA_STRUCTURES_NODECHAIN_H
#define DATA_STRUCTURES_NODECHAIN_H

#include <stddef.h>
#include <libds/err.h>

typedef struct NodeChain NodeChain;

typedef void (*Destructor)(void *);
typedef ds_err_t (*Copier)(void *dst_value, const void *src_value);

NodeChain *
ds__nc_alloc(void);

ds_err_t
ds__nc_free(NodeChain **nodechain_dptr, Destructor destructor);

ds_err_t
ds__nc_clear(NodeChain *nodechain_ptr, Destructor destructor);

ds_err_t
ds__nc_assign(NodeChain *dst, const NodeChain *src, size_t value_size, Destructor destructor, Copier copier);

ds_err_t
ds__nc_reverse(NodeChain *nodechain_ptr);

size_t
ds__nc_length(const NodeChain *nodechain_ptr);

bool
ds__nc_isempty(const NodeChain *nodechain_ptr);

ds_err_t
ds__nc_push_front(NodeChain *nodechain_ptr, const void *value_ptr, size_t value_size);

ds_err_t
ds__nc_push_back(NodeChain *nodechain_ptr, const void *value_ptr, size_t value_size);

ds_err_t
ds__nc_push_at(NodeChain *nodechain_ptr, const void *value_ptr, size_t value_size, long long index);

ds_err_t
ds__nc_get_front(const NodeChain *nodechain_ptr, void *output_ptr, size_t output_size);

ds_err_t
ds__nc_get_back(const NodeChain *nodechain_ptr, void *output_ptr, size_t output_size);

ds_err_t
ds__nc_get_at(const NodeChain *nodechain_ptr, void *output_ptr, size_t output_size, long long index);

ds_err_t
ds__nc_drop_front(NodeChain *nodechain_ptr, Destructor destructor);

ds_err_t
ds__nc_drop_back(NodeChain *nodechain_ptr, Destructor destructor);

ds_err_t
ds__nc_drop_at(NodeChain *nodechain_ptr, Destructor destructor, long long index);

#endif //DATA_STRUCTURES_NODECHAIN_H
