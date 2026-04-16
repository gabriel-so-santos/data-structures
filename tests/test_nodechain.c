/**
 * @file    test_nodechain.c
 * @brief   Node Chain tests
 * @author  Gabriel Souza
 * @date    2026-04-13
 */

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "test_runner.h"
#include "libds/impl/nodechain.h"

/* Type aliases for clarity */
typedef struct ds_node_chain NodeChain;

/* Helper macros */
#define IS_ALIGNED(ptr, alignment) (((uintptr_t)(ptr) % (alignment)) == 0)
#define TEST_LOOP_ITERATIONS 128

/* Test data types */
typedef struct { max_align_t buffer[1024]; } Blob;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Verifies that a chain allocates properly aligned storage for a given type.
 * @param type_name     Name for test output
 * @param element_size  Size of each element
 * @param alignment     Required alignment for the type
 */
static void
test_alignment_for_type(const char* type_name, const size_t element_size, const size_t alignment)
{
    printf("        %-15s ", type_name);

    NodeChain* chain = ds_nc_alloc(element_size, alignment);
    assert(chain != NULL);

    void* data_ptr = NULL;
    for (int i = 0; i < TEST_LOOP_ITERATIONS; i++) {
        assert(ds_nc_push_back(chain, &data_ptr) == DS_ERR_NONE);
        assert(ds_nc_get_at(chain, i, &data_ptr) == DS_ERR_NONE);
        assert(IS_ALIGNED(data_ptr, alignment));
    }

    ds_nc_free(&chain, null_destroy);
    assert(chain == NULL);
    printf("[OK]\n");
}

// ============================================================================
// Test Cases
// ============================================================================

static void
test_primitive_types(void)
{
    printf("\n    %-30s\n", "test_primitive_types");

    test_alignment_for_type("char",     sizeof(char),     alignof(char));
    test_alignment_for_type("int",      sizeof(int),      alignof(int));
    test_alignment_for_type("float",    sizeof(float),    alignof(float));
    test_alignment_for_type("double",   sizeof(double),   alignof(double));
    test_alignment_for_type("void*",    sizeof(void*),    alignof(void*));

    printf(" [PASSED]\n");
}

static void
test_big_types(void)
{
    printf("\n    %-30s\n", "test_big_types");

    test_alignment_for_type("long long",     sizeof(long long),     alignof(long long));
    test_alignment_for_type("long double",   sizeof(long double),   alignof(long double));
    test_alignment_for_type("size_t",        sizeof(size_t),        alignof(size_t));
    test_alignment_for_type("max_align_t",   sizeof(max_align_t),   alignof(max_align_t));
    test_alignment_for_type("Blob[1024]",    sizeof(Blob),          alignof(Blob));

    printf(" [PASSED]\n");
}

static void
test_bad_types(void)
{
    printf("\n    %-30s", "test_bad_types");

    // Invalid size or alignment
    assert(ds_nc_alloc(0, alignof(int)) == NULL);
    assert(ds_nc_alloc(sizeof(int), 0) == NULL);

    // Alignment must be power of two
    assert(ds_nc_alloc(sizeof(int) * 3, 3) == NULL);

    // Size must be multiple of alignment
    assert(ds_nc_alloc(10, 8) == NULL);

    // Integer overflow protection
    size_t massive_size = (size_t)-1;           // SIZE_MAX
    massive_size -= (massive_size % 8);         // Align to 8 bytes
    assert(ds_nc_alloc(massive_size, 8) == NULL);

    printf(" [PASSED]\n");
}

static void
test_push_operations(void)
{
    printf("\n    %-30s", "test_push_operations");

    const int test_value = rand();
    void* data_ptr = NULL;

    NodeChain* chain = ds_nc_alloc(sizeof(int), alignof(int));
    assert(chain != NULL);

    // NULL pointer validation
    assert(ds_nc_push_front(NULL, &data_ptr) == DS_ERR_NULL_POINTER);
    assert(ds_nc_push_front(chain, NULL) == DS_ERR_NULL_POINTER);

    // Boundary checks
    assert(ds_nc_push_at(chain, 1, &data_ptr) == DS_ERR_INDEX_OUT_OF_BOUNDS);
    assert(ds_nc_push_at(chain, -1, &data_ptr) == DS_ERR_INDEX_OUT_OF_BOUNDS);

    // Push at position 0 (acts as push_front)
    assert(ds_nc_push_at(chain, 0, &data_ptr) == DS_ERR_NONE);
    assert(ds_nc_length(chain) == 1);

    // Verify data integrity
    *((int*)data_ptr) = test_value;
    assert(ds_nc_get_at(chain, 0, &data_ptr) == DS_ERR_NONE);
    assert(*((int*)data_ptr) == test_value);

    ds_nc_free(&chain, NULL);
    assert(chain == NULL);

    printf(" [PASSED]\n");
}

static void
test_pop_operations(void)
{
    printf("\n    %-30s", "test_pop_operations");

    NodeChain* chain = ds_nc_alloc(sizeof(int), alignof(int));
    void* data_ptr = NULL;

    // Pop from empty chain
    assert(ds_nc_pop_front(chain, &data_ptr, null_destroy) == DS_ERR_EMPTY_STRUCTURE);
    assert(ds_nc_pop_back(chain, &data_ptr, null_destroy) == DS_ERR_EMPTY_STRUCTURE);
    assert(ds_nc_pop_at(chain, 0, &data_ptr, null_destroy) == DS_ERR_EMPTY_STRUCTURE);

    // NULL validation
    assert(ds_nc_pop_front(NULL, &data_ptr, null_destroy) == DS_ERR_NULL_POINTER);

    // Pop with NULL output parameter
    ds_nc_push_front(chain, &data_ptr);
    assert(ds_nc_length(chain) == 1);
    assert(ds_nc_pop_front(chain, NULL, NULL) == DS_ERR_NONE);
    assert(ds_nc_length(chain) == 0);

    ds_nc_free(&chain, NULL);

    printf(" [PASSED]\n");
}

static void
test_clear_operation(void)
{
    printf("\n    %-30s", "test_clear_operation");

    NodeChain* chain = ds_nc_alloc(sizeof(float), alignof(float));

    // NULL validation
    assert(ds_nc_clear(NULL, NULL, false) == DS_ERR_NULL_POINTER);

    // Clear already empty chain (should be safe)
    assert(ds_nc_clear(chain, NULL, false) == DS_ERR_NONE);
    assert(ds_nc_clear(chain, NULL, true) == DS_ERR_NONE);

    ds_nc_free(&chain, NULL);

    printf(" [PASSED]\n");
}

// ============================================================================
// Test Suite Runner
// ============================================================================

void run_nodechain_tests(void)
{
    printf("\n+------------------------------------------------------+");
    printf("\n|                  'nodechain' Test Suite              |");
    printf("\n+------------------------------------------------------+");

    test_primitive_types();
    test_big_types();
    test_bad_types();
    test_push_operations();
    test_pop_operations();
    test_clear_operation();

    printf("\n+------------------------------------------------------+");
    printf("\n|                    ALL TESTS PASSED                  |");
    printf("\n+------------------------------------------------------+\n");
}