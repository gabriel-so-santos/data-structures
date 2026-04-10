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
ds_nc_free(struct ds_node_chain **chain_ref, ds_destructor_fn destroy);

enum ds_error
ds_nc_clear(struct ds_node_chain *chain, ds_destructor_fn destroy);

enum ds_error
ds_nc_copy(struct ds_node_chain *dst_chain, const struct ds_node_chain *src_chain,
    size_t value_size, ds_copier_fn copy, ds_destructor_fn destroy);

enum ds_error
ds_nc_reverse(struct ds_node_chain *chain);


size_t
ds_nc_length(const struct ds_node_chain *chain);

size_t
ds_nc_bytes(const struct ds_node_chain *chain);

bool
ds_nc_is_empty(const struct ds_node_chain *chain);


enum ds_error
ds_nc_get_front(const struct ds_node_chain *chain, void **out);

enum ds_error
ds_nc_get_back(const struct ds_node_chain *chain, void **out);

enum ds_error
ds_nc_get_at(const struct ds_node_chain *chain, void **out, long long index);


enum ds_error
ds_nc_push_front(struct ds_node_chain *chain, void **out);

enum ds_error
ds_nc_push_back(struct ds_node_chain *chain, void **out);

enum ds_error
ds_nc_push_at(struct ds_node_chain *chain, void **out, long long index);


enum ds_error
ds_nc_pop_front(struct ds_node_chain *chain, ds_destructor_fn destroy, void **out);

enum ds_error
ds_nc_pop_back(struct ds_node_chain *chain, ds_destructor_fn destroy, void **out);

enum ds_error
ds_nc_pop_at(struct ds_node_chain *chain, ds_destructor_fn destroy, void **out, long long index);

#endif //DATA_STRUCTURES_NODECHAIN_H
