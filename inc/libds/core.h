//
// Created by Gabriel Souza on 21/02/2026.
//

#ifndef DATA_STRUCTURES_CORE_H
#define DATA_STRUCTURES_CORE_H

#include <stddef.h>
#include <stdbool.h>

#ifndef LIBDS_ENABLE_ERROR_PRINT
#define LIBDS_ENABLE_ERROR_PRINT 1
#endif

#ifndef LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_ENABLE_EXIT_ON_FAIL 0
#endif

#if LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(expr) ds_handle_err((expr), #expr, __FILE__, __LINE__, __func__)
#else //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_CHECK(expr) (expr)
#endif //LIBDS_ENABLE_ERROR_PRINT || LIBDS_ENABLE_EXIT_ON_FAIL #else

/* Debug helpers */
#define LIBDS_STRINGIFY(Name) #Name
#define LIBDS_TOSTRING(Name) LIBDS_STRINGIFY(Name)

struct ds_node_chain;

typedef enum ds_error
{
    DS_ERR_NONE = 0,            /* No error */
    DS_ERR_ALLOCATION_FAILED,   /* Memory allocation failed */
    DS_ERR_NULL_POINTER,        /* Invalid null pointer provided */
    DS_ERR_INDEX_OUT_OF_BOUNDS, /* Index out of bounds */
    DS_ERR_EMPTY_STRUCTURE,     /* Invalid operation on an empty structure */
} ds_error_t;

typedef void (*ds_destructor_t)(void *);
typedef ds_error_t (*ds_copier_t)(void *dst_value, const void *src_value);

const char *
ds_err_to_string(ds_error_t err);

ds_error_t
ds_handle_err(ds_error_t err, const char *expr, const char *file, int line, const char *func);

#endif //DATA_STRUCTURES_CORE_H
