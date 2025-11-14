#include "list.h"
#include <stdlib.h>
#include <string.h>

List* list_create(void) {
    List* list = malloc(sizeof(List));
    if (!list) return NULL;
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void list_destroy(List* list) {
    if (!list) return;
    list_clear(list);
    free(list);
}

void list_clear(List* list) {
    if (!list) return;
    
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        free(current);
        current = next;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

int list_append(List* list, void* data) {
    if (!list) return 0;
    
    ListNode* new_node = malloc(sizeof(ListNode));
    if (!new_node) return 0;
    
    new_node->data = data;
    new_node->next = NULL;
    
    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }
    
    list->tail = new_node;
    list->size++;
    return 1;
}

int list_prepend(List* list, void* data) {
    if (!list) return 0;
    
    ListNode* new_node = malloc(sizeof(ListNode));
    if (!new_node) return 0;
    
    new_node->data = data;
    new_node->next = list->head;
    
    list->head = new_node;
    if (!list->tail) {
        list->tail = new_node;
    }
    
    list->size++;
    return 1;
}

int list_insert(List* list, int index, void* data) {
    if (!list || index > list->size) return 0;
    
    if (index == 0) return list_prepend(list, data);
    if (index == list->size) return list_append(list, data);
    
    ListNode* new_node = malloc(sizeof(ListNode));
    if (!new_node) return 0;
    
    ListNode* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }
    
    new_node->data = data;
    new_node->next = current->next;
    current->next = new_node;
    
    list->size++;
    return 1;
}

void* list_remove_first(List* list) {
    if (!list || !list->head) return NULL;
    
    ListNode* node = list->head;
    void* data = node->data;
    
    list->head = node->next;
    if (!list->head) {
        list->tail = NULL;
    }
    
    free(node);
    list->size--;
    return data;
}

void* list_remove_last(List* list) {
    if (!list || !list->head) return NULL;
    
    if (!list->head->next) {
        return list_remove_first(list);
    }
    
    ListNode* current = list->head;
    while (current->next != list->tail) {
        current = current->next;
    }
    
    void* data = list->tail->data;
    free(list->tail);
    current->next = NULL;
    list->tail = current;
    list->size--;
    
    return data;
}

void* list_remove_at(List* list, int index) {
    if (!list || index >= list->size) return NULL;
    
    if (index == 0) return list_remove_first(list);
    if (index == list->size - 1) return list_remove_last(list);
    
    ListNode* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }
    
    ListNode* to_remove = current->next;
    void* data = to_remove->data;
    current->next = to_remove->next;
    free(to_remove);
    
    list->size--;
    return data;
}

void* list_get_first(const List* list) {
    return (list && list->head) ? list->head->data : NULL;
}

void* list_get_last(const List* list) {
    return (list && list->tail) ? list->tail->data : NULL;
}

void* list_get_at(const List* list, int index) {
    if (!list || index >= list->size) return NULL;
    
    ListNode* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current->data;
}

int list_size(const List* list) {
    return list ? list->size : 0;
}

int list_is_empty(const List* list) {
    return !list || list->size == 0;
}

// Iterator implementation

ListIterator list_iterator_begin(List* list) {
    ListIterator iter;
    iter.list = list;
    iter.current = list ? list->head : NULL;
    iter.prev = NULL;
    return iter;
}

ListIterator list_iterator_end(List* list) {
    ListIterator iter;
    iter.list = list;
    iter.current = NULL;
    iter.prev = list ? list->tail : NULL;
    return iter;
}

int list_iterator_has_next(const ListIterator* iter) {
    return iter && iter->current != NULL;
}

void* list_iterator_next(ListIterator* iter) {
    if (!iter || !iter->current) return NULL;
    
    void* data = iter->current->data;
    iter->prev = iter->current;
    iter->current = iter->current->next;
    return data;
}

void* list_iterator_current(const ListIterator* iter) {
    return (iter && iter->current) ? iter->current->data : NULL;
}

int list_iterator_remove_current(ListIterator* iter) {
    if (!iter || !iter->current || !iter->list) return 0;
    
    ListNode* to_remove = iter->current;
    
    if (iter->prev) {
        // Removing middle or end node
        iter->prev->next = to_remove->next;
        iter->current = to_remove->next;
        
        if (to_remove == iter->list->tail) {
            iter->list->tail = iter->prev;
        }
    } else {
        // Removing head node
        iter->list->head = to_remove->next;
        iter->current = to_remove->next;
        
        if (to_remove == iter->list->tail) {
            iter->list->tail = NULL;
        }
    }
    
    free(to_remove);
    iter->list->size--;
    return 1;
}

int list_iterator_insert_after(ListIterator* iter, void* data) {
    if (!iter || !iter->current || !iter->list) return 0;
    
    ListNode* new_node = malloc(sizeof(ListNode));
    if (!new_node) return 0;
    
    new_node->data = data;
    new_node->next = iter->current->next;
    iter->current->next = new_node;
    
    if (iter->current == iter->list->tail) {
        iter->list->tail = new_node;
    }
    
    iter->list->size++;
    return 1;
}