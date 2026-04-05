//
// Created by Gabriel Souza on 21/02/2026.
//

#ifndef DATA_STRUCTURES_ERR_H
#define DATA_STRUCTURES_ERR_H

#include <stddef.h>

#ifndef DS_ENABLE_ERROR_PRINT
    #define DS_ENABLE_ERROR_PRINT 1
#endif

#ifndef DS_ENABLE_EXIT_ON_FAIL
    #define DS_ENABLE_EXIT_ON_FAIL 0
#endif

typedef enum
{
    LIBDS_SUCCESS = 0,
    LIBDS_ERR_ALLOCATION_FAILED,
    LIBDS_ERR_NULL_POINTER,
    LIBDS_ERR_INDEX_OUT_OF_BOUNDS,
    LIBDS_ERR_EMPTY_STRUCTURE,
} ds_err_t;

const char *
ds_err_to_string(ds_err_t err);

ds_err_t
ds__handle_err(ds_err_t err, const char *expr, const char *file, int line, const char *func);

#if DS_ENABLE_ERROR_PRINT || DS_ENABLE_EXIT_ON_FAIL
    #define DS_CHECK(expr) ds__handle_err((expr), #expr, __FILE__, __LINE__, __func__)
#else
    #define DS_CHECK(expr) (expr)
#endif

// Debug helpers
#define DS__STRINGIFY(x) #x
#define DS__TOSTRING(x) DS__STRINGIFY(x)

#endif //DATA_STRUCTURES_ERR_H
