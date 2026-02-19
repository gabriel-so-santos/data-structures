//
// Created by Gabriel Souza on 18/02/2026.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "node.h"

Node *_node_alloc(const void *data, const size_t element_size)
{
    Node *node = malloc(sizeof(Node) + element_size);
    if (node == NULL) return NULL;

    memcpy(node->data, data, element_size);
    node->next = NULL;
    return node;
}

void _node_free_all(Node **head)
{
    while (*head != NULL)
    {
        Node *node = *head;
        *head = (*head)->next;

        free(node);
    }
}


