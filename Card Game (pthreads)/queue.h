#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>

typedef struct node {
    int denomination;  // use to store the denomination of a card
    struct node* next; // pointer to next node in linked list
} node_t;

void push(node_t** front, node_t** tail, int card) {
    node_t* c = malloc(sizeof(node_t)); // allocate memory for a node object

    if (*tail == NULL) {c->denomination = card; *front = c; *tail = c;}
    else
    {
        c->denomination = card;  // store value of denomination in node representing card
        c->next = NULL;          // last card in queue has no successor
        (*tail)->next = c;       // append card to the end of the queue or put on bottom of deck
        (*tail) = c;             // point tail pointer to last card in deck or bottom of deck
    }
}

int pop(node_t** front, node_t** tail) {
    if (*front == NULL) return -1; // queue is empty, no card to remove from front

    int denomination = (*front)->denomination;
    node_t* deallocate = *front;

    *front = (*front)->next;            // store address of next node in list in front
    if (*front == NULL) *tail = NULL; // last node removed from queue
    free(deallocate);               // deallocate memory allocate for former node at front of queue

    return denomination;            // return the card removed from the top of the deck of first node in queue
} 

void listDeck(node_t** front) {
    node_t* traverse = *front; // point to the top of the deck

    while (traverse != NULL)
    {
        printf("%d ", traverse->denomination);

        traverse = traverse->next;
    }

    printf("\n");
}

void clearDeck(node_t** front, node_t** bottom) {
    node_t* remove = *front; // point to the top of the deck

    while (*front != NULL)
    {
        *front = (*front)->next; // point front of queue to the next card in the deck or node in the list
         free(remove);           // deallocate memory allocated for the node in the queue representing the card
         remove = *front;        // store address of node in front of queue in pointer
    }

    *front = NULL;
    *bottom = NULL;
}

#endif
