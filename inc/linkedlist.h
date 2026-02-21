//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>

#include "nodechain.h"

#define DEFINE_TYPED_LINKEDLIST(ItemType, ListName, destructor_func) \
    typedef NodeChain* ListName; \
    \
    static inline ListName ListName##_create(void) \
    { \
        return nodechain_create(sizeof(ItemType), destructor_func); \
    } \
    \
    static inline DSErrCode ListName##_free(ListName list) \
    { \
        return nodechain_free(list); \
    } \
    \
    static inline DSErrCode ListName##_append(ListName list, ItemType value) \
    { \
        return nodechain_push_back(list, &value); \
    } \
    \
    static inline DSErrCode ListName##_prepend(ListName list, ItemType value) \
    { \
        return nodechain_push_front(list, &value); \
    } \
    \
    static inline DSErrCode ListName##_insert(ListName list, ItemType value, ptrdiff_t index) \
    { \
        return nodechain_push_at(list, &value, index); \
    }
#endif //DATA_STRUCTURES_LINKEDLIST_H