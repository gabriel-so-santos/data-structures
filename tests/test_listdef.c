/**
 * @file    test_listdef.c
 * @brief   List generator tests
 *
 * @author  Gabriel Souza
 * @date    2026-04-14
 */

#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIBDS_ENABLE_ERROR_PRINT 0
#include "libds/core.h"

#include "test_runner.h"
#include "libds/listdef.h"

// ============================================================================
// Test Fixture Types
// ============================================================================

typedef struct {
    long long id;
    char username[16];
    char password[16];
    char email[16];
    int8_t age;
} User;

// ============================================================================
// Test Helpers / Mock Allocator
// ============================================================================

static int fail_after = -1;      // Fail after N allocations (-1 = never fail)
static int alloc_count = 0;      // Current allocation counter
static int destroy_calls = 0;    // Tracks destroy function calls

static void* test_malloc(const size_t size)
{
    if (fail_after >= 0 && alloc_count++ >= fail_after) {
        return NULL;
    }
    return malloc(size);
}

static bool copy_string(void* dst, const void* src)
{
    if (!dst || !src) return false;

    const size_t len = strlen(*(const char**)src);
    char* new_str = test_malloc(len + 1);
    if (!new_str) return false;

    new_str[len] = '\0';
    strncpy(new_str, *(const char**)src, len);
    *(char**)dst = new_str;

    return true;
}

static void destroy_string(void* data)
{
    if (!data) return;
    free(*(char**)data);
    *(char**)data = NULL;
    destroy_calls++;
}

// ============================================================================
// List Type Definitions (Template Instantiations)
// ============================================================================

LIBDS_DEF_LIST(int,     ListInt,     li, null_copy, null_destroy)
LIBDS_DEF_LIST(User,    ListUser,    lu, null_copy, null_destroy)
LIBDS_DEF_LIST(char*,   ListString,  ls, copy_string, destroy_string)

// ============================================================================
// Test Helpers - Test Data Generators
// ============================================================================

/**
 * Generates a random integer array of given size.
 * Caller must free the returned pointer.
 */
static int* generate_random_int_array(size_t size)
{
    int* array = (int*)malloc(size * sizeof(int));
    assert(array != NULL);

    for (size_t i = 0; i < size; i++) {
        array[i] = rand();
    }

    return array;
}

/**
 * Creates a test user with sequential ID and formatted fields.
 */
static User create_test_user(size_t id)
{
    User u;
    u.id = (long long)id;
    u.age = (int8_t)(20 + (id % 40));
    snprintf(u.username, sizeof(u.username), "user_%zu", id);
    snprintf(u.password, sizeof(u.password), "pass_%zu", id);
    snprintf(u.email, sizeof(u.email), "u%zu@gmail.com", id);
    return u;
}

/**
 * Verifies two User structs are identical via memcmp.
 */
static bool users_equal(const User* a, const User* b)
{
    return memcmp(a, b, sizeof(User)) == 0;
}

// ============================================================================
// Test Cases: Integer List
// ============================================================================

static void test_list_int(void)
{
    printf("\n    %-30s", "test_list_int");

    const size_t ARRAY_SIZE = 1024;
    const size_t REMOVE_COUNT = 512;

    // Setup
    int* array = generate_random_int_array(ARRAY_SIZE);
    ListInt list = li_create();

    // Initial state assertions
    assert(list._nodes != NULL);
    assert(li_size(list) == 0);
    assert(li_is_empty(list) == true);

    const size_t initial_byte_size = li_bytes(list);

    // Append all elements
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        assert(li_append(list, array[i]) == DS_ERR_NONE);
    }

    assert(li_size(list) == ARRAY_SIZE);
    assert(li_is_empty(list) == false);
    assert(li_bytes(list) > initial_byte_size);

    // Verify all elements
    int value;
    for (size_t i = 0; i < li_size(list); i++) {
        li_get_at(list, i, &value);
        assert(value == array[i]);
    }

    // Random removals
    for (size_t i = 0; i < REMOVE_COUNT; i++) {
        size_t index = rand() % li_size(list);
        assert(li_pop_at(list, index, NULL) == DS_ERR_NONE);
    }
    assert(li_size(list) == ARRAY_SIZE - REMOVE_COUNT);

    // Copy verification
    ListInt other_list = li_create();
    assert(other_list._nodes != NULL);

    li_copy(other_list, list);
    assert(li_size(other_list) == li_size(list));

    int other_value;
    for (size_t i = 0; i < li_size(list); i++) {
        li_get_at(list, i, &value);
        li_get_at(other_list, i, &other_value);
        assert(value == other_value);
    }

    // Clear operations
    const size_t before_clear = li_bytes(list);
    assert(li_clear(list) == DS_ERR_NONE);
    assert(li_size(list) == 0);
    assert(li_is_empty(list) == true);
    assert(li_bytes(list) == before_clear);

    assert(li_deep_clear(list) == DS_ERR_NONE);
    assert(li_size(list) == 0);
    assert(li_is_empty(list) == true);
    assert(li_bytes(list) < before_clear);

    // Cleanup
    li_delete(&list);
    assert(list._nodes == NULL);

    li_delete(&other_list);
    assert(other_list._nodes == NULL);

    free(array);
    printf(" [PASSED]\n");
}

// ============================================================================
// Test Cases: Struct List (User)
// ============================================================================

static void test_list_struct_user(void)
{
    printf("\n    %-30s", "test_list_struct_user");

    const size_t NUM_USERS = 100;

    ListUser users = lu_create();
    assert(users._nodes != NULL);
    assert(lu_size(users) == 0);
    assert(lu_is_empty(users) == true);

    // Push all users
    for (size_t i = 0; i < NUM_USERS; i++) {
        User u_in = create_test_user(i);
        assert(lu_append(users, u_in) == DS_ERR_NONE);
    }

    assert(lu_size(users) == NUM_USERS);
    assert(lu_is_empty(users) == false);

    // Verify all users
    User u_out;
    for (size_t i = 0; i < NUM_USERS; i++) {
        assert(lu_get_at(users, i, &u_out) == DS_ERR_NONE);
        assert(u_out.id == (long long)i);
        assert(u_out.age == (int8_t)(20 + (i % 40)));

        char expected_user[16];
        snprintf(expected_user, sizeof(expected_user), "user_%zu", i);
        assert(strcmp(u_out.username, expected_user) == 0);
    }

    // Copy verification
    ListUser users_backup = lu_create();
    assert(lu_copy(users_backup, users) == DS_ERR_NONE);
    assert(lu_size(users_backup) == lu_size(users));

    User u_copy;
    for (size_t i = 0; i < NUM_USERS; i++) {
        lu_get_at(users, i, &u_out);
        lu_get_at(users_backup, i, &u_copy);
        assert(users_equal(&u_out, &u_copy));
    }

    // Pop operation
    User popped_user;
    assert(lu_pop_at(users, 0, &popped_user) == DS_ERR_NONE);
    assert(popped_user.id == 0);
    assert(lu_size(users) == NUM_USERS - 1);

    // Cleanup
    lu_delete(&users);
    assert(users._nodes == NULL);

    lu_delete(&users_backup);
    assert(users_backup._nodes == NULL);

    printf(" [PASSED]\n");
}

// ============================================================================
// Test Cases: Dynamic String List
// ============================================================================

static void test_list_dynamic_str(void)
{
    printf("\n    %-30s", "test_list_dynamic_str");

    const char* test_data[] = {
        "Ada Lovelace",
        "Alan Turing",
        "John von Neumann",
        "Grace Hopper"
    };
    const size_t DATA_LEN = sizeof(test_data) / sizeof(char*);

    ListString names = ls_create();
    assert(names._nodes != NULL);
    assert(ls_size(names) == 0);
    assert(ls_is_empty(names) == true);

    // Append all strings
    for (size_t i = 0; i < DATA_LEN; i++) {
        assert(ls_append(names, (char*)test_data[i]) == DS_ERR_NONE);
        assert(ls_size(names) == i + 1);
        assert(ls_is_empty(names) == false);
    }

    // Non-destructive gets
    char* out;
    for (size_t i = 0; i < DATA_LEN; i++) {
        assert(ls_get_at(names, i, &out) == DS_ERR_NONE);
        assert(out != NULL);
        assert(strcmp(out, test_data[i]) == 0);
    }

    // Pop front
    assert(ls_pop_at(names, 0, &out) == DS_ERR_NONE);
    assert(ls_size(names) == DATA_LEN - 1);
    assert(strcmp(out, "Ada Lovelace") == 0);
    free(out);  // Ownership transferred

    // Pop back
    assert(ls_pop_at(names, ls_size(names) - 1, &out) == DS_ERR_NONE);
    assert(ls_size(names) == DATA_LEN - 2);
    assert(strcmp(out, "Grace Hopper") == 0);
    free(out);

    // Pop middle
    assert(ls_pop_at(names, 1, &out) == DS_ERR_NONE);
    assert(ls_size(names) == 1);
    assert(strcmp(out, "John von Neumann") == 0);
    free(out);

    // Verify remaining
    assert(ls_get_front(names, &out) == DS_ERR_NONE);
    assert(strcmp(out, "Alan Turing") == 0);

    // Cleanup
    assert(ls_delete(&names) == DS_ERR_NONE);
    assert(names._nodes == NULL);

    printf(" [PASSED]\n");
}

// ============================================================================
// Test Cases: Ownership & Memory Management
// ============================================================================

static void test_ownership(void)
{
    printf("\n    %-30s", "test_ownership");

    destroy_calls = 0;
    char* out = NULL;

    ListString list = ls_create();

    // Pop with ownership transfer
    ls_append(list, "hello");
    assert(ls_pop_at(list, 0, &out) == DS_ERR_NONE);
    assert(destroy_calls == 0);  // Destructor should NOT run
    assert(strcmp(out, "hello") == 0);
    free(out);

    // Pop with discard (NULL output)
    ls_append(list, "world");
    assert(ls_pop_at(list, 0, NULL) == DS_ERR_NONE);
    assert(destroy_calls == 1);  // Destructor MUST run

    ls_delete(&list);

    printf(" [PASSED]\n");
}

static void test_memory_reuse(void)
{
    printf("\n    %-30s", "test_memory_reuse");

    const int OPERATIONS = 100;

    ListInt list = li_create();

    // Fill then drain
    for (int i = 0; i < OPERATIONS; i++) li_append(list, i);
    for (int i = 0; i < OPERATIONS; i++) li_drop_at(list, 0);

    const size_t memory_before = li_bytes(list);

    // Fill again
    for (int i = 0; i < OPERATIONS; i++) li_append(list, i);

    const size_t memory_after = li_bytes(list);
    assert(memory_after == memory_before);  // Should reuse memory, no growth

    li_delete(&list);

    printf(" [PASSED]\n");
}

// ============================================================================
// Test Cases: Error Handling & Edge Cases
// ============================================================================

static void test_list_copy_failure(void)
{
    printf("\n    %-30s", "test_list_copy_failure");

    const char* test_data[] = {"Albert Einstein", "Marie Curie", "Isaac Newton"};
    const size_t DATA_LEN = sizeof(test_data) / sizeof(char*);

    ListString src = ls_create();
    ListString dst = ls_create();

    // Populate source
    for (size_t i = 0; i < DATA_LEN; i++) {
        assert(ls_append(src, test_data[i]) == DS_ERR_NONE);
    }

    // Inject allocation failure
    fail_after = 1;
    alloc_count = 0;

    // Copy should fail
    assert(ls_copy(dst, src) != DS_ERR_NONE);

    // Destination must remain unchanged
    assert(ls_size(dst) == 0);
    assert(ls_is_empty(dst) == true);

    // Source must be intact
    char* out;
    for (size_t i = 0; i < DATA_LEN; i++) {
        ls_get_at(src, i, &out);
        assert(strcmp(out, test_data[i]) == 0);
    }

    // Reset allocator mock
    fail_after = -1;
    alloc_count = 0;

    ls_delete(&src);
    ls_delete(&dst);

    printf(" [PASSED]\n");
}

// ============================================================================
// Test Cases: Fuzz Testing
// ============================================================================

static void test_fuzz(void)
{
    printf("\n    %-30s", "test_fuzz");

    const int ITERATIONS = 5000;
    const int MAX_REF_SIZE = 1024;

    ListInt list = li_create();
    int reference_array[MAX_REF_SIZE];
    size_t ref_size = 0;

    for (int i = 0; i < ITERATIONS; i++) {
        int operation = rand() % 3;  // 0=append, 1=pop_front, 2=no-op

        if (operation == 0 && ref_size < MAX_REF_SIZE) {
            // Append operation
            int value = rand();
            li_append(list, value);
            reference_array[ref_size++] = value;
        }
        else if (operation == 1 && ref_size > 0) {
            // Pop front operation
            int actual_value, expected_value;
            li_pop_at(list, 0, &actual_value);
            expected_value = reference_array[0];

            // Shift reference array
            memmove(reference_array, reference_array + 1, (--ref_size) * sizeof(int));

            assert(actual_value == expected_value);
        }
    }

    li_delete(&list);

    printf(" [PASSED]\n");
}

// ============================================================================
// Test Suite Runner
// ============================================================================

void run_listdef_tests(void)
{
    printf("\n+------------------------------------------------------+");
    printf("\n|                  'listdef' Test Suite                |");
    printf("\n+------------------------------------------------------+");

    test_list_int();
    test_list_struct_user();
    test_list_dynamic_str();
    test_ownership();
    test_memory_reuse();
    test_list_copy_failure();
    test_fuzz();

    printf("\n+------------------------------------------------------+");
    printf("\n|                    ALL TESTS PASSED                  |");
    printf("\n+------------------------------------------------------+\n");
}