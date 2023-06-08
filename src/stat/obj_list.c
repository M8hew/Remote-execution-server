#include <stdlib.h>

#include "obj_list.h"

static Node *head;
static Node *tail;

void init_list() {
    head = calloc(1, sizeof(*head));
    tail = head;
}

void add_node(long long time, int type) {
    Node *tmp = calloc(1, sizeof(*tmp));

    head->next = tmp;
    head->time = time;
    head->type = type;

    head = tmp;
}

Node *get_tail() {
    return tail;
}

void delete_tail() {
    Node *tmp = tail->next;
    if (tmp != NULL) {
        free(tail);
        tail = tmp;
    }
}
