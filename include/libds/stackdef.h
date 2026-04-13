/**
 * @file    stackdef.h
 * @brief   Type-safe singly-linked stack generator macro.
 *
 * @author  Gabriel Souza
 * @date    2026-04-13
 */

#ifndef LIBDS_STACKDEF_H
#define LIBDS_STACKDEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"

#define LIBDS_DEFINE_STACK(Type, StackType, Prefix, CopyFunc, DestroyFunc)      \
                                                                                \
    typedef struct StackType                                                    \
    {                                                                           \
        const ds_copier_fn      copy;                                           \
        const ds_destructor_fn  destroy;                                        \
        struct ds_node_chain    *_nodes; /* must NOT be modified directly */    \
    } StackType;                                                                \
                                                                                \
    static inline StackType                                                     \
    Prefix##_create(void)                                                       \
    {                                                                           \
        size_t value_size  = sizeof(Type);                                      \
        size_t value_align = alignof(Type);                                     \
                                                                                \
        StackType stack = {                                                     \
            .copy    = (CopyFunc),                                              \
            .destroy = (DestroyFunc),                                           \
            ._nodes  = ds_nc_alloc(value_size, value_align)                     \
        };                                                                      \
        return stack;                                                           \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_delete(StackType *stack)                                           \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_free(&stack->_nodes, stack->destroy)                          \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_clear(StackType stack)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(stack._nodes, stack.destroy, false)                     \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_deep_clear(StackType stack)                                        \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_clear(stack._nodes, stack.destroy, true)                      \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_copy(StackType dst_stack, const StackType src_stack)               \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_copy(                                                         \
                dst_stack._nodes,                                               \
                src_stack._nodes,                                               \
                sizeof(Type),                                                   \
                dst_stack.copy,                                                 \
                dst_stack.destroy                                               \
            )                                                                   \
        );                                                                      \
    }                                                                           \
                                                                                \
    /* support of both `#_length` and `#_size` */                               \
    static inline size_t                                                        \
    Prefix##_length(const StackType stack)                                        \
    {                                                                           \
        return ds_nc_length(stack._nodes);                                       \
    }                                                                           \
    static inline size_t                                                        \
    Prefix##_size(const StackType stack)                                          \
    {                                                                           \
        return Prefix##_length(stack);                                           \
    }                                                                           \
                                                                                \
    static inline size_t                                                        \
    Prefix##_bytes(const StackType stack)                                         \
    {                                                                           \
        return ds_nc_bytes(stack._nodes);                                        \
    }                                                                           \
                                                                                \
    static inline bool                                                          \
    Prefix##_is_empty(const StackType stack)                                      \
    {                                                                           \
        return ds_nc_is_empty(stack._nodes);                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_front(StackType stack, Type *out)                              \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_front(stack._nodes, &data)                                \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_back(StackType stack, Type *out)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_back(stack._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_get_at(StackType stack, Type *out, const size_t index)             \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_at(stack._nodes, &data, index)                            \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push(StackType stack, Type value)                                  \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_front(stack._nodes, &data)                               \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!stack.copy)                                                        \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !stack.copy(data, &value) )                                    \
            {                                                                   \
                ds_nc_pop_front(stack._nodes, NULL, NULL);                      \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_pop(StackType stack, Type *out)                                    \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_front(stack._nodes, &data, stack.destroy)                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */

#endif //LIBDS_STACKDEF_H