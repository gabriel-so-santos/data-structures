//
// Created by Gabriel Souza on 16/02/2026.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "../inc/linkedlist.h"

void list_init(ListNode **head)
{
    if (head != NULL)
        *head = NULL;
}


void list_free(ListNode **head)
{
    if (head == NULL || *head == NULL) return;

    while (*head != NULL)
    {
        ListNode *node = *head;
        *head = (*head)->next;
        free(node);
    }
}


void list_append_value(ListNode **head, const int data)
{
    if (head == NULL) return;

    ListNode *new_node = malloc(sizeof(ListNode));
    if(new_node == NULL) return;

    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL)
    {
        *head = new_node;
        return;
    }

    ListNode *prev_node = *head;
    while(prev_node->next != NULL)
        prev_node = prev_node->next;

    prev_node->next = new_node;
}

/**
 *
 */
void list_prepend_value(ListNode **head, const int data)
{
    if (head == NULL) return;

    ListNode *new_node = malloc(sizeof(ListNode));
    if(new_node == NULL) return;

    new_node->data = data;
    new_node->next = *head;

    *head = new_node;
}

// Restruct project's files and add new functions

void list_insert_value(ListNode **head, const int data, int index)
{
    if (head == NULL) return;

    if (index < 0)
    {
        index = list_length(*head) + index;
        if (index < 0) return;
    }

    ListNode *new_node = malloc(sizeof(ListNode));
    if (new_node == NULL) return;

    new_node->data = data;

    if (index == 0)
    {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    ListNode *prev_node = *head;
    int i = 0;

    while (i < index - 1 && prev_node != NULL)
    {
        prev_node = prev_node->next;
        i++;
    }

    if (prev_node == NULL)
    {
        free(new_node);
        return;  // Index out of bounds
    }

    new_node->next = prev_node->next;
    prev_node->next = new_node;
}


void list_remove_value(ListNode **head, const int data)
{

}

void list_remove_node(ListNode **head, const int index)
{

}

int list_contains_value(const ListNode *head, const int data)
{
    return 0;
}

int list_length(const ListNode *head)
{
    if (head == NULL) return 0;

    int length = 0;
    while (head != NULL)
    {
        head = head->next;
        length++;
    }

    return length;
}


void list_print(const ListNode *head)
{
    printf("[");

    if (head != NULL)
    {
        printf("%d", head->data);
        head = head->next;

        while (head != NULL)
        {
            printf(", %d", head->data);
            head = head->next;
        }
    }

    printf("]");
}