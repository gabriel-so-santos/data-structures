//
// Created by Gabriel Souza on 05/04/2026.
//

#include "libds/err.h"

#ifdef DS_ENABLE_ERROR_PRINT
    #include <stdio.h>
#endif

#ifdef DS_ENABLE_EXIT_ON_FAIL
    #include <stdlib.h>
#endif

const char *
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

ds_err_t
ds__handle_err(const ds_err_t err, const char *expr, const char *file, const int line, const char *func)
{
    if (err == LIBDS_SUCCESS)
        return err;

    #if DS_ENABLE_ERROR_PRINT
    fprintf(
        stderr,
        "\n[LIBDS ERROR] %s (Code %d)\n"
        " --> %s:%d\n"
        "  | Function   : %s\n"
        "  | Expression : %s\n",
        ds_err_to_string(err), err, file, line, func, expr
    );
    #endif

    #if DS_ENABLE_EXIT_ON_FAIL
    exit(EXIT_FAILURE);
    #endif

    return err;
}
