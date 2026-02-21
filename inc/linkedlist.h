//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>

#include "nodechain.h"

#define DEFINE_TYPED_LINKEDLIST(Type, ListName, destructor_func) \
    typedef NodeChain * ListName; \
    \
    static inline ListName ListName##_create(void) \
    { \
        return nodechain_create(sizeof(Type), destructor_func); \
    } \
    \
    static inline DSErrCode ListName##_free(ListName list) \
    { \
        return nodechain_free(list); \
    } \
    \
    static inline size_t ListName##_length(ListName list) \
    { \
        return nodechain_length(list); \
    } \
    \
    static inline int ListName##_is_empty(ListName list) \
    { \
        return nodechain_is_empty(list); \
    } \
    \
    static inline DSErrCode ListName##_first(ListName list, Type *output_ptr) \
    { \
        return nodechain_get_front(list, output_ptr); \
    } \
    \
    static inline DSErrCode ListName##_last(ListName list, Type *output_ptr) \
    { \
        return nodechain_get_back(list, output_ptr); \
    } \
    \
    static inline DSErrCode ListName##_append(ListName list, Type value) \
    { \
        return nodechain_push_back(list, &value); \
    } \
    \
    static inline DSErrCode ListName##_prepend(ListName list, Type value) \
    { \
        return nodechain_push_front(list, &value); \
    } \
    \
    static inline DSErrCode ListName##_insert(ListName list, Type value, ptrdiff_t index) \
    { \
        return nodechain_push_at(list, &value, index); \
    }
#endif //DATA_STRUCTURES_LINKEDLIST_H