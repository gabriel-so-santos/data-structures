//
// Created by Gabriel Souza on 19/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "nodechain.h"

void nodechain_init(NodeChain *node_chain, const size_t element_size, const Destructor destructor_func)
{
    node_chain->head = NULL;
    node_chain->tail = NULL;
    node_chain->length = 0;
    node_chain->element_size = element_size;
    node_chain->destructor_func = destructor_func;
}

Node *nodechain_alloc(const void *data, const size_t element_size)
{
    Node *node = malloc(sizeof(Node) + element_size);
    if (node == NULL) return NULL;

    memcpy(node->data, data, element_size);
    node->next = NULL;
    return node;
}

void nodechain_free(NodeChain *node_chain)
{
    if (node_chain == NULL) return;

    Node *node = node_chain->head;

    while (node != NULL)
    {
        Node *next = node->next;

        if (node_chain->destructor_func != NULL)
            node_chain->destructor_func(node->data);

        free(node);
        node = next;
    }

    node_chain->head = NULL;
    node_chain->tail = NULL;
    node_chain->length = 0;
}

/**
 * Inserts a new node at the beginning of the chain.
 *
 * @param node_chain The address of the node chain structure.
 * @param data The data to be inserted into the list.
 */
void nodechain_push_front(NodeChain *node_chain, const void *data)
{
    if (node_chain == NULL) return;

    Node *new_node = nodechain_alloc(data, node_chain->element_size);
    if (new_node == NULL) return;
    new_node->next = node_chain->head;

    if (node_chain->head == NULL)
        node_chain->tail = new_node;

    node_chain->head = new_node;

    node_chain->length++;
}

/**
 * @brief Inserts a new node at end of the chain.
 *
 * @param node_chain The address of the node chain structure.
 * @param data The data to be inserted into the list.
 */
void nodechain_push_back(NodeChain *node_chain, const void *data)
{
    if (node_chain == NULL) return;

    Node *new_node = nodechain_alloc(data, node_chain->element_size);
    if (new_node == NULL) return;

    if (node_chain->head == NULL)
    {
        node_chain->head = new_node;
        node_chain->tail = new_node;
    }
    else
    {
        node_chain->tail->next = new_node;
        node_chain->tail = new_node;
    }

    node_chain->length++;
}

/**
 * @brief Inserts a new node at the given index.
 *
 * @param node_chain The address of the node chain structure.
 * @param data The data to be inserted into the list.
 * @param index The index where the new node will be placed (0-based).
 *              Can be negative for from-end insertions.
 */
void nodechain_push_at(NodeChain *node_chain, const void *data, ptrdiff_t index)
{
    if (node_chain == NULL) return;

    // Negative index counts from the tail
    if (index < 0)
        index = (ptrdiff_t) node_chain->length + index;

    if (index < 0 || index > node_chain->length) return;


    // Insert at beginning
    if (index == 0)
        nodechain_push_front(node_chain, data);

    // Insert at end
    else if (index == node_chain->length)
        nodechain_push_back(node_chain, data);

    // Insert in middle
    else
    {
        Node *prev_node = node_chain->head;
        for (long long i = 0; i < index - 1; i++)
            prev_node = prev_node->next;

        Node *new_node = nodechain_alloc(data, node_chain->element_size);
        if (new_node == NULL) return;

        new_node->next = prev_node->next;
        prev_node->next = new_node;

        node_chain->length++;
    }
}
