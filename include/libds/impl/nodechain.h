//
// Created by Gabriel Souza on 19/02/2026.
//

#ifndef DATA_STRUCTURES_NODECHAIN_H
#define DATA_STRUCTURES_NODECHAIN_H

#include <stddef.h>
#include "libds/core.h"

struct ds_node_chain *
ds_nc_alloc(size_t value_size, size_t value_align);

enum ds_error
ds_nc_free(struct ds_node_chain **chain_dptr, ds_destructor_t destroy_fn);

enum ds_error
ds_nc_clear(struct ds_node_chain *chain_ptr, ds_destructor_t destroy_fn);

enum ds_error
ds_nc_copy(struct ds_node_chain *dst_chain, const struct ds_node_chain *src_chain,
    size_t value_size, ds_copier_t copy_fn, ds_destructor_t destroy_fn);

enum ds_error
ds_nc_reverse(struct ds_node_chain *chain_ptr);

size_t
ds_nc_length(const struct ds_node_chain *chain_ptr);

size_t
ds_nc_bytes(const struct ds_node_chain *chain_ptr);

bool
ds_nc_is_empty(const struct ds_node_chain *chain_ptr);

enum ds_error
ds_nc_push_front(struct ds_node_chain *chain_ptr, void **data_dptr);

enum ds_error
ds_nc_push_back(struct ds_node_chain *chain_ptr, void **data_dptr);

enum ds_error
ds_nc_push_at(struct ds_node_chain *chain_ptr, void **data_dptr, long long index);

enum ds_error
ds_nc_get_front(const struct ds_node_chain *chain_ptr, void **out_dptr);

enum ds_error
ds_nc_get_back(const struct ds_node_chain *chain_ptr, void **out_dptr);

enum ds_error
ds_nc_get_at(const struct ds_node_chain *chain_ptr, void **out_dptr, long long index);

enum ds_error
ds_nc_pop_front(struct ds_node_chain *chain_ptr, ds_destructor_t destroy_fn, void **out_dptr);

enum ds_error
ds_nc_pop_back(struct ds_node_chain *chain_ptr, ds_destructor_t destroy_fn, void **out_dptr);

enum ds_error
ds_nc_pop_at(struct ds_node_chain *chain_ptr, ds_destructor_t destroy_fn, void **out_dptr, long long index);

#endif //DATA_STRUCTURES_NODECHAIN_H
