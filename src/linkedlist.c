//
// Created by Gabriel Souza on 16/02/2026.
//

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"
#include "internal/node.h"

/**
 * @brief Internal list free function.
 *
 * @param list The address of the list.
 */
void _list_free(List *list)
{
    if (list == NULL) return;

    Node *head = list->head;
    while (head != NULL)
    {
        Node *node = head;
        head = head->next;

        free(node);
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

/**
 * @brief Internal list append function.
 *
 * @param list The address of the list.
 * @param data The data to be inserted into the list.
 */
void _list_append(List *list, const void *data)
{
    if (list == NULL) return;

    Node *new_node = malloc(sizeof(Node) + list->element_size);
    if (new_node == NULL) return;

    memcpy(new_node->data, data, list->element_size);
    new_node->next = NULL;

    if (list->head == NULL)
    {
        list->head = new_node;
        list->tail = new_node;
    }
    else
    {
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->length++;
}

/**
 * @brief Internal list prepend function.
 *
 * @param list The address of the list.
 * @param data The data to be inserted into the list.
 */
void _list_prepend(List *list, const void *data)
{
    if (list == NULL) return;

    Node *new_node = malloc(sizeof(Node) + list->element_size);
    if(new_node == NULL) return;

    memcpy(new_node->data, data, list->element_size);
    new_node->next = list->head;

    if (list->head == NULL)
        list->tail = new_node;

    list->head = new_node;

    list->length++;
}

/**
 * @brief Internal list insertion function.
 *
 * @param list The address of the list.
 * @param data The data to be inserted into the list.
 * @param index The index where the new node will be placed (0-based).
 *              Can be negative for from-end insertions.
 */
void _list_insert(List *list, const void *data, ptrdiff_t index)
{
    if (list == NULL || data == NULL) return;

    // Negative index counts from the tail
    if (index < 0)
        index = (ptrdiff_t) list->length + index;

    if (index < 0 || index > list->length) return;


    // Insert at beginning
    if (index == 0)
        _list_prepend(list, data);

    // Insert at end
    else if (index == list->length)
        _list_append(list, data);

    // Insert in middle
    else
    {
        Node *prev_node = list->head;
        for (long long i = 0; i < index - 1; i++)
            prev_node = prev_node->next;

        Node *new_node = malloc(sizeof(Node) + list->element_size);
        if (new_node == NULL) return;

        memcpy(new_node->data, data, list->element_size);
        new_node->next = prev_node->next;
        prev_node->next = new_node;

        list->length++;
    }
}
