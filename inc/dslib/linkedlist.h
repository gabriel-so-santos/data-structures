//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>
#include "dslib/core.h"
#include "dslib/err.h"

#define DEFINE_TYPED_LINKEDLIST(Type, ListName, destructor_func) \
    typedef _NodeChain * const ListName; \
    \
    static inline ListName new_##ListName(void) \
    { return _dslib_nc_alloc(sizeof(Type), destructor_func); } \
    \
    static inline dslib_err_t ListName##_free(ListName list) \
    { return _dslib_nc_free(list); } \
    \
    static inline dslib_err_t ListName##_first(ListName list, Type *output_ptr) \
    { return _dslib_nc_get_front(list, output_ptr); } \
    \
    static inline dslib_err_t ListName##_last(ListName list, Type *output_ptr) \
    { return _dslib_nc_get_back(list, output_ptr); } \
    \
    static inline dslib_err_t ListName##_append(ListName list, Type value) \
    { return _dslib_nc_push_back(list, &value); } \
    \
    static inline dslib_err_t ListName##_prepend(ListName list, Type value) \
    { return _dslib_nc_push_front(list, &value); } \
    \
    static inline dslib_err_t ListName##_insert(ListName list, Type value, ptrdiff_t index) \
    { return _dslib_nc_push_at(list, &value, index); }

#endif //DATA_STRUCTURES_LINKEDLIST_H