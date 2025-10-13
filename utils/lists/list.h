#ifndef LIST_H
#define LIST_H

// List node structure
typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode;

// List structure
typedef struct {
    ListNode* head;
    ListNode* tail;
    int size;
} List;

// Iterator structure
typedef struct {
    List* list;
    ListNode* current;
    ListNode* prev;
} ListIterator;

// Function prototypes
List* list_create(void);
void list_destroy(List* list);
void list_clear(List* list);

int list_append(List* list, void* data);
int list_prepend(List* list, void* data);
int list_insert(List* list, int index, void* data);

void* list_remove_first(List* list);
void* list_remove_last(List* list);
void* list_remove_at(List* list, int index);
void* list_remove(List* list, void* data, int (*compare)(const void*, const void*));

void* list_get_first(const List* list);
void* list_get_last(const List* list);
void* list_get_at(const List* list, int index);

int list_contains(const List* list, void* data, int (*compare)(const void*, const void*));
int list_size(const List* list);
int list_is_empty(const List* list);

// Iterator functions
ListIterator list_iterator_begin(List* list);
ListIterator list_iterator_end(List* list);
int list_iterator_has_next(const ListIterator* iter);
void* list_iterator_next(ListIterator* iter);
void* list_iterator_current(const ListIterator* iter);
int list_iterator_remove_current(ListIterator* iter);
int list_iterator_insert_after(ListIterator* iter, void* data);

#endif // LIST_H