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

//==============================================================================
// Error Handling
//==============================================================================

const char *
ds_err_to_string(const enum ds_error err)
{
    switch (err)
    {
        case DS_ERR_NONE:
            return "Success: Operation completed without errors";

        case DS_ERR_ALLOCATION_FAILED:
            return "Fatal: Memory allocation failed - system may be out of memory, "
                   "\nor requested size/alignment constraints cannot be satisfied";

        case DS_ERR_NULL_POINTER:
            return "Fatal: Null pointer encountered - expected a valid pointer to "
                   "\nan initialized structure or output buffer";

        case DS_ERR_INDEX_OUT_OF_BOUNDS:
            return "Error: Index out of bounds - index must be in range [0, length-1] "
                   "\nfor remove/read operations, or [0, length] for insert operations";

        case DS_ERR_EMPTY_STRUCTURE:
            return "Error: Operation requires non-empty structure - "
                   "\ncannot remove or read from an empty structure";

        case DS_ERR_COPY_FAILED:
            return "Error: Custom copy operation failed - the provided copier "
                   "\nfunction returned false, indicating copy could not complete";

        default:
            return "Unknown error: Unrecognized error code";
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
        "\n[LIBDS] %s (Code %d)\n"
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
