/**
 * @file    listdef.h
 * @brief   Type-safe singly-linked list generator macro.
 *
 * @author  Gabriel Souza
 * @date    2026-04-09
 */

#ifndef LIBDS_LISTDEF_H
#define LIBDS_LISTDEF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

#include "core.h"
#include "impl/nodechain.h"
#include "impl/contdef.h"

#define LIBDS_DEF_LIST(Type, ListType, Prefix, CopyFunc, DestroyFunc)           \
                                                                                \
    LIBDS_DEF_CONTAINER(Type, ListType, Prefix, CopyFunc, DestroyFunc)          \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_reverse(ListType list)                                             \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_reverse(list._nodes)                                          \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_set_front(ListType list, Type value)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_front(list._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *((Type *)data) = value;                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_set_back(ListType list, Type value)                                \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_back(list._nodes, &data)                                  \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *((Type *)data) = value;                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_set_at(ListType list, const size_t index, Type value)              \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_get_at(list._nodes, index, &data)                             \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        *((Type *)data) = value;                                                \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push_front(ListType list, Type value)                              \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_front(list._nodes, &data)                                \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !list.copy(data, &value) )                                     \
            {                                                                   \
                ds_nc_pop_front(list._nodes, NULL, NULL);                       \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    /* support of both `#_push_front` and `#_prepend` */                        \
    static inline enum ds_error                                                 \
    Prefix##_prepend(ListType list, Type value)                                 \
    {                                                                           \
        return Prefix##_push_front(list, value);                                \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push_back(ListType list, Type value)                               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_back(list._nodes, &data)                                 \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !list.copy(data, &value) )                                     \
            {                                                                   \
                ds_nc_pop_back(list._nodes, NULL, NULL);                        \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
    /* support of both `#_push_back` and `#_append` */                          \
    static inline enum ds_error                                                 \
    Prefix##_append(ListType list, Type value)                                  \
    {                                                                           \
        return Prefix##_push_back(list, value);                                 \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_push_at(ListType list, const size_t index, Type value)             \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_push_at(list._nodes, index, &data)                            \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (!list.copy)                                                         \
            *((Type *)data) = value;                                            \
        else                                                                    \
        {                                                                       \
            if ( !list.copy(data, &value) )                                     \
            {                                                                   \
                ds_nc_pop_at(list._nodes, index, NULL, NULL);                   \
                return DS_ERR_COPY_FAILED;                                      \
            }                                                                   \
        }                                                                       \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_drop_front(ListType list)                                          \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_front(list._nodes, NULL, list.destroy)                    \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_drop_back(ListType list)                                           \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_back(list._nodes, NULL, list.destroy)                     \
        );                                                                      \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_drop_at(ListType list, const size_t index)                         \
    {                                                                           \
        return LIBDS_CHECK(                                                     \
            ds_nc_pop_at(list._nodes, index, NULL, list.destroy)                \
        );                                                                      \
    }                                                                           \
    static inline enum ds_error                                                 \
    Prefix##_pop_front(ListType list, Type *out)                                \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_front(list._nodes, &data, list.destroy)                   \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_pop_back(ListType list, Type *out)                                 \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_back(list._nodes, &data, list.destroy)                    \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
                                                                                \
    static inline enum ds_error                                                 \
    Prefix##_pop_at(ListType list, const size_t index, Type *out)               \
    {                                                                           \
        void *data = NULL;                                                      \
        enum ds_error error = LIBDS_CHECK(                                      \
            ds_nc_pop_at(list._nodes, index, &data, list.destroy)               \
        );                                                                      \
        if (error) return error;                                                \
                                                                                \
        if (out) *out = *((Type *)data);                                        \
        return DS_ERR_NONE;                                                     \
    }                                                                           \
/* end of macro */

#endif //LIBDS_LISTDEF_H
