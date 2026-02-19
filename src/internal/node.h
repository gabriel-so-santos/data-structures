//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_INTERNAL_NODE_H
#define DATA_STRUCTURES_INTERNAL_NODE_H

typedef struct Node {
    struct Node *next;
    unsigned char data[];  // Flexible array member
} Node;

/**
 * @brief Internal node allocation function
 *
 * Allocates a new node.
 * The data is set using memcpy().
 * node->next is set to NULL.
 *
 * @param data The data to insert in the node.
 * @param element_size The data size in bytes.
 *
 * @return The new allocated node.
 */
Node *_node_alloc(const void *data, size_t element_size);

/**
 * @brief Internal node memory free function.
 *
 * Iterates through the entire node chain, freeing both the node structures and
 * the data they contain.
 *
 * @warning If the stored data are pointers to dynamically allocated memory
 *          (e.g., char* strings), it must be handled separately.
 *
 * @param head The double pointer to the head of the node chain.
 */
void _node_free_all(Node **head);

#endif //DATA_STRUCTURES_INTERNAL_NODE_H