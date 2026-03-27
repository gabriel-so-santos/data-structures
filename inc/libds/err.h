//
// Created by Gabriel Souza on 21/02/2026.
//

#ifndef DATA_STRUCTURES_ERR_H
#define DATA_STRUCTURES_ERR_H

#include <stdio.h>
#include  <stddef.h>


#ifndef DS_ENABLE_ERROR_PRINT
    #define DS_ENABLE_ERROR_PRINT 1
#endif //DS_ENABLE_ERROR_PRINT


typedef enum
{
    LIBDS_SUCCESS = 0,
    LIBDS_ERR_ALLOCATION_FAILED,
    LIBDS_ERR_NULL_POINTER,
    LIBDS_ERR_INDEX_OUT_OF_BOUNDS,
    LIBDS_ERR_EMPTY_STRUCTURE,
} ds_err_t;

static inline const char *
ds_err_to_string(const ds_err_t err)
{
    switch (err)
    {
        case LIBDS_SUCCESS:
            return "Success";

        case LIBDS_ERR_ALLOCATION_FAILED:
            return "Memory allocation failed (insufficient memory or invalid size)";

        case LIBDS_ERR_NULL_POINTER:
            return "Null pointer provided (expected valid pointer to structure or buffer)";

        case LIBDS_ERR_INDEX_OUT_OF_BOUNDS:
            return "Index out of bounds (index exceeds structure size)";

        case LIBDS_ERR_EMPTY_STRUCTURE:
            return "Operation on empty structure (no elements to access)";

        default:
            return "Unknown error";
    }
}

#define DS__CHECK_ERR(expr)                                                \
    do {                                                                   \
        ds_err_t err_code = (expr);                                        \
        if (err_code != LIBDS_SUCCESS && DS_ENABLE_ERROR_PRINT)            \
        {                                                                  \
            fprintf(                                                       \
                stderr,                                                    \
                "\n[LIBDS ERROR] %s (Code %d)\n"                           \
                " --> %s:%d\n"                                             \
                "  | Function   : %s\n"                                    \
                "  | Expression : %s\n",                                   \
                ds_err_to_string(err_code),                                \
                err_code,                                                  \
                __FILE__,                                                  \
                __LINE__,                                                  \
                __func__,                                                  \
                #expr                                                      \
            );                                                             \
        }                                                                  \
        return err_code;                                                   \
    } while (0)

// Debug helpers
#define DS__STRINGIFY(x) #x
#define DS__TOSTRING(x) DS__STRINGIFY(x)

#endif //DATA_STRUCTURES_ERR_H