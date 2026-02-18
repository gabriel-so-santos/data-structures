//
// Created by Gabriel Souza on 17/02/2026.
//

#ifndef DATA_STRUCTURES_INTERNAL_NODE_H
#define DATA_STRUCTURES_INTERNAL_NODE_H

typedef struct Node {
    struct Node *next;
    char data[];  // Flexible array member
} Node;

#endif //DATA_STRUCTURES_INTERNAL_NODE_H