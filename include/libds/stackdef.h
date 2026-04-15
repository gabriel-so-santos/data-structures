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
#include "impl/contdef.h"

#define LIBDS_DEF_STACK(Type, StackType, Prefix, CopyFunc, DestroyFunc)         \
                                                                                \
    LIBDS_DEF_CONTAINER(Type, StackType, Prefix, CopyFunc, DestroyFunc)         \
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
                                                                                \
                LIBDS_HANDLE_ERR(                                               \
                    DS_ERR_COPY_FAILED,                                         \
                    LIBDS_STRINGIFY(stack.copy(data, &value)),                  \
                    __FILE__, __LINE__, __func__                                \
                );                                                              \
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