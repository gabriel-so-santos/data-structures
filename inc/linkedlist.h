//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

#include <stddef.h>

typedef struct Node Node;

/**
 * @brief A generic dynamic list structure that can hold elements of any type.
 *
 * This list implementation stores elements by copying them into dynamically
 * allocated nodes. Each element is stored in a node that contains a void pointer
 * to the element's data and a pointer to the next node.
 *
 * The list is generic - it can store elements of any data type by knowing the
 * size of each element. Elements are copied by value into the list, so the
 * original data can be safely modified or freed after insertion.
 *
 * Memory Layout:
 * @code
 * List structure:
 * +---------------------+----------------+----------+
 * |  head/tail (Node*)  |  element_size  |  length  |
 * +---------------------+----------------+----------+
 *        |
 *        v
 * Node structure:
 * +----------------+-------------------------+
 * |  next (Node*)  |  data (flexible array)  |
 * +----------------+-------------------------+
 *        |
 *        v
 * [Next node of the list]
 *
 * @endcode
 *
 * @note This structure should be initialized using list_of() macro:
 *       List int_list = list_of(int);
 *
 * @warning The list does not manage memory for pointer types automatically.
 *          If storing pointers (e.g., char* strings), you are responsible for
 *          freeing the referenced data when removing/destroying elements.
 *
 * @see list_of()
 * @see list_free()
 * @see list_append()
 * @see list_prepend()
 * @see list_insert()
 */
typedef struct List{
    Node *head;
    Node *tail;
    size_t element_size;
    size_t length;
} List;


void _list_free(List *list);

void _list_append(List *list, const void *data);

void _list_prepend(List *list, const void *data);

void _list_insert(List *list, const void *data, ptrdiff_t index);


/**
 * @brief Initializes an empty linked list.
 *
 * Sets the head and tail pointers to NULL, the element size to the size of the given type, and the length to 0.
 * This function must be called before using the list with other operations.
 *
 * @param type The type of the elements of the list.
 *
 * @warning After using the list, list_free() macro must be used to avoid memory leaks.
 *
 * @code
 * List list_example = list_of(int);
 * // list usage...
 * list_free(list_example);
 * @endcode
 */
#define list_of(type) \
    ((List){NULL, NULL, sizeof(type), 0})


/**
* @brief Frees all memory allocated for the list and its elements.
*
* Iterates through the entire list, freeing both the node structures and
* the data they contain. After calling this function, the list structure
* is reset to an empty state (head = NULL, tail = NULL, length = 0).
*
* @param list The List variable.
*
* @warning This function frees the data stored in the list. If you stored
*          pointers to dynamically allocated memory (e.g., char* strings),
*          you must set up a custom element destructor or handle that
*          separately before calling list_free().
*
* @note Time complexity: O(n), where n is the number of elements in the list.
* @note After calling list_free(), the list can be reused without reinitialization
*       (it will be empty with the same element_size).
*
* @code
* List list_example = list_of(int);
*
* // Add some elements
* list_append(list_example, 77);
*
* // Free the list
* list_free(list_example);
* // list_example.head and list_example.tail is now NULL, my_list.length is 0
* // list_example.element_size is unchanged (still sizeof(int))
* @endcode
*/
#define list_free(list) \
    _list_free(&(list))


/**
 * @brief Appends an element to the end of the list.
 *
 * Creates a new node with the given data and inserts it as the last element.
 * Sets the tail pointer to the appended node.
 * Updates the Length of the list.
 *
 * @param list The list variable.
 * @param value The data to be inserted into the list.
 *
 * @note Time complexity: O(1).
 *
 * @code
 * List numbers = list_of(int);
 * list_append(numbers, 42);     // [42]
 * list_append(numbers, 100);    // [42, 100]
 * @endcode
 */
#define list_append(list, data) \
    do { \
        __typeof__(data) _tmp = (data); \
        _list_append(&(list), &(_tmp)); \
    } while (0)


/**
 * @brief Prepends an element to the beginning of the list.
 *
 * Creates a new node with the given data and inserts it as the first element.
 * Sets the head pointer to the prepended node.
 * Updates the Length of the list.
 *
 * @param list The List variable.
 * @param data The data to be inserted into the list.
 *
 * @note Time complexity: O(1).
 *
 * @code
 * List numbers = list_of(int);
 * list_prepend(numbers, 30);  // [30]
 * list_prepend(numbers, 20);  // [20, 30]
 * list_prepend(numbers, 10);  // [10, 20, 30]
 * @endcode
 */
#define list_prepend(list, data) \
    do { \
        __typeof__(data) _tmp = (data); \
        _list_prepend(&(list), &(_tmp)); \
    } while (0)

/**
 * @brief Inserts an element at the specified position in the list.
 *
 * Creates a new node with the given data and inserts it at the specified index.
 * Elements after the insertion point are shifted to the right.
 * Updates the length of the list.
 *
 * @param list The list variable.
 * @param data The data to be inserted.

* @param index The index where the new node will be placed (0-based).
 *              Can be negative for from-end insertions.
 *              Valid range: -list.length to list.length.
 *
 * @note Time complexity: O(n) for index > 0;
 *                        O(1) for index == 0 or index == length.
 *
 * @code
 * List numbers = list_of(int);
 * list_append(numbers, 10);     // [10]
 * list_append(numbers, 30);     // [10, 30]
 * list_insert(numbers, 20, 1);  // [10, 20, 30]
 * @endcode
 */
#define list_insert(list, data, index) \
    do { \
        __typeof__(data) _tmp = (data); \
        _list_insert(&(list), &(_tmp), (ptrdiff_t)(index)); \
    } while (0)

#endif //DATA_STRUCTURES_LINKEDLIST_H