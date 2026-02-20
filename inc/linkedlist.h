//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>

#include "nodechain.h"

#define DEFINE_TYPED_LIST(Type, ListName) \
    typedef NodeChain ListName; \
    \
    static inline void ListName##_init(ListName *list, Destructor destructor_func) \
    { \
        nodechain_init(list, sizeof(Type), destructor_func); \
    } \
    \
    static inline void ListName##_free(ListName *list) \
    { \
        nodechain_free(list); \
    } \
    \
    static inline void ListName##_append(ListName *list, Type value) \
    { \
        nodechain_push_back(list, &value); \
    } \
    \
    static inline void ListName##_prepend(ListName *list, Type value) \
    { \
        nodechain_push_front(list, &value); \
    } \
    \
    static inline void ListName##_insert(ListName *list, Type value, ptrdiff_t index) \
    { \
        nodechain_push_at(list, &value, index); \
    }
#endif //DATA_STRUCTURES_LINKEDLIST_H