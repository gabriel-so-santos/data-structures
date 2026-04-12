//
// Created by Gabriel Souza on 05/04/2026.
//

#include "libds/core.h"

#ifndef LIBDS_ENABLE_ERROR_PRINT
#define LIBDS_ENABLE_ERROR_PRINT 1
#endif

#ifndef LIBDS_ENABLE_EXIT_ON_FAIL
#define LIBDS_ENABLE_EXIT_ON_FAIL 0
#endif

#ifdef LIBDS_ENABLE_ERROR_PRINT
#include <stdio.h>
#endif

#ifdef LIBDS_ENABLE_EXIT_ON_FAIL
#include <stdlib.h>
#endif

const char *
ds_err_to_string(const enum ds_error err)
{
    switch (err)
    {
        case DS_ERR_NONE:
            return "No error";

        case DS_ERR_ALLOCATION_FAILED:
            return "Memory allocation failed (insufficient memory or invalid size)";

        case DS_ERR_NULL_POINTER:
            return "Null pointer provided (expected valid pointer to structure or buffer)";

        case DS_ERR_INDEX_OUT_OF_BOUNDS:
            return "Index out of bounds (index exceeds structure size)";

        case DS_ERR_EMPTY_STRUCTURE:
            return "Operation on empty structure (no elements to access)";

        default:
            return "Unknown error";
    }
}

enum ds_error
ds_handle_err(const enum ds_error err, const char *expr, const char *file, const int line, const char *func)
{
    if (err == DS_ERR_NONE)
        return err;

    #if LIBDS_ENABLE_ERROR_PRINT
    fprintf(
        stderr,
        "\n[LIBDS ERROR] %s (Code %d)\n"
        " --> %s:%d\n"
        "  | Function   : %s\n"
        "  | Expression : %s\n",
        ds_err_to_string(err), err, file, line, func, expr
    );
    #endif //LIBDS_ENABLE_ERROR_PRINT

    #if LIBDS_ENABLE_EXIT_ON_FAIL
    exit(EXIT_FAILURE);
    #endif //LIBDS_ENABLE_EXIT_ON_FAIL

    return err;
}
