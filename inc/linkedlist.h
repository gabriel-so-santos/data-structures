//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_LINKEDLIST_H
#define DATA_STRUCTURES_LINKEDLIST_H

/**
 * @brief Represents a node in a singly linked list.
 *
 * Each node contains an integer data value and a pointer to the next node.
 * The last node in the list has its 'next' pointer set to NULL.
 * Must be created as a pointer.
 *
 * @warning After using the list, list_free() must be called to avoid memory leaks.
 *
 * @code
 * ListNode *list_example;
 * list_init(&list_example);
 * ...
 * list_free(&list_example);
 * @endcode
 */
typedef struct ListNode {
    int data;
    struct ListNode *next;
} ListNode;

/**
 * @brief Initializes an empty linked list.
 *
 * Sets the head pointer to NULL, indicating an empty list.
 * This function must be called before using the list with other operations.
 *
 * @param head Double pointer to the head of the list (list's address). Must not be NULL.
 *
 * @warning After using the list, list_free() must be called to avoid memory leaks.
 *
 * @note The initialization could also be done by set the variable to a null pointer at declaration.
 *
 * @code
 * ListNode *list_example;
 * list_init(&list_example);
 * ...
 * list_free(&list_example);
 * @endcode
 */
void list_init(ListNode **head);

/**
 * @brief Frees all memory allocated for the linked list.
 *
 * Iterates through the entire list and frees each node.
 * After calling this function, the head pointer is set to NULL.
 *
 * @param head Double pointer to the head of the list (list's address). Must not be NULL.
 *
 * @warning This function should only be called once per list.
 * @warning Undefined behavior if head somehow points to invalid memory.
 *
 * @note Time complexity: O(n), where n is the number of elements in the list.
 *
 * @code
 * ListNode *list_example;
 * list_init(&list_example);
 * ...
 * list_free(&list_example);
 * @endcode
 */
void list_free(ListNode **head);

/**
* @brief Inserts a new value at the end of the linked list.
*
* Creates a new node with the given data and appends it to the end of the list.
* If the list is empty, the new node becomes the head.
*
* @param head Double pointer to the head of the list (list's address). Must not be NULL.
* @param data The integer value to be stored in the new node.
*
* @note Time complexity: O(n), where n is the number of elements in the list.
*
* @code
* list_append_value(&list_example, 10);
* @endcode
*/
void list_append_value(ListNode **head, int data);

/**
 *
 */
void list_prepend_value(ListNode **head, int data);

/**
 * @brief Inserts a value at the specified index in the linked list.
 *
 * @param head Double pointer to the head of the (list's address).
 * @param data The integer value to be stored in the new node.
 * @param index Position where to insert (0-based). If index equals list size, appends to the end.
 *              If index > size, insertion fails.
 *
 * @note Time complexity: O(n+1), where n is the given index.
 */
void list_insert_value(ListNode **head, int data, int index);

void list_remove_value(ListNode **head, int data);

void list_remove_node(ListNode **head, int index);

int list_contains_value(const ListNode *head, int data);

/**
 * @brief Returns the number of elements in the linked list.
 *
 * Traverses the list and counts each node until reaching NULL.
 *
 * @param head Pointer to the first node of the list (can be NULL)
 * @return int Number of nodes in the list (0 if list is empty)
 *
 * @code
 * ListNode* list_example = NULL;
 * list_insert(&list_example, 10);
 * list_insert(&list_example, 20);
 * int len = list_length(list_example);  // Returns 2
 * @endcode
 */
int list_length(const ListNode *head);

/**
 * @brief Prints all elements of the linked list in a formatted way.
 *
 * Prints the list elements enclosed in square brackets, separated by commas.
 * Examples:
 *   - Empty list: []
 *   - List with one element: [10]
 *   - List with multiple elements: [10, 20, 30]
 *
 * @param head Pointer to the first node of the list.
 *
 * @code
 * ListNode* list_example = NULL;
 * list_insert(&list_example, 10);
 * list_insert(&list_example, 20);
 * list_print(list_example);  // Output: [10, 20]
 * @endcode
 */
void list_print(const ListNode *head);

#endif //DATA_STRUCTURES_LINKEDLIST_H