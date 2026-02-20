//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>

#include "nodechain.h"

#define DEFINE_TYPED_LINKEDLIST(ItemType, ListName, destructor_func) \
    typedef NodeChain ListName; \
    \
    static inline ListName ListName##_init(void) \
    { \
        return (ListName){NULL, NULL, 0, sizeof(ItemType), destructor_func}; \
    } \
    \
    static inline void ListName##_free(ListName *list_ptr) \
    { \
        nodechain_free(list_ptr); \
    } \
    \
    static inline void ListName##_append(ListName *list_ptr, ItemType value) \
    { \
        nodechain_push_back(list_ptr, &value); \
    } \
    \
    static inline void ListName##_prepend(ListName *list_ptr, ItemType value) \
    { \
        nodechain_push_front(list_ptr, &value); \
    } \
    \
    static inline void ListName##_insert(ListName *list_ptr, ItemType value, ptrdiff_t index) \
    { \
        nodechain_push_at(list_ptr, &value, index); \
    }
#endif //DATA_STRUCTURES_LINKEDLIST_H