#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"

/* When the list needs to grow */
#define GROWTH_FACTOR 2

typedef struct list {
    void **storage;
    size_t capacity;
    size_t curr_size;
    FreeFunc free_item;
} List;

List *list_init(size_t initial_size, FreeFunc freer) {
    List *new_list = malloc(sizeof(List));
    assert(new_list != NULL);
    new_list->storage = malloc(initial_size * sizeof(void *));
    assert(new_list->storage != NULL);
    new_list->capacity = initial_size;
    new_list->curr_size = 0;
    new_list->free_item = freer;
    return new_list;
}

void list_free(List *list) {
    if(list->free_item != NULL){
      while (list_size(list) > 0) {
        list->free_item(list_remove(list, 0));
      }
    }
    free(list->storage);
    free(list);
}

size_t list_size(List *list) {
    return list->curr_size;
}

size_t list_capacity(List *list) {
    return list->capacity;
}

void *list_get(List *list, size_t index) {
    //assert(index < list_size(list));
    return list->storage[index];
}

void *list_remove_front(List *list) {
    return list_remove(list, 0);
}

void *list_remove_back(List *list) {
    return list_remove(list, list_size(list) - 1);
}

void *list_remove(List *list, size_t index) {
    assert(list_size(list) > 0);
    assert(index < list_size(list));
    void *return_value = list_get(list, index);
    for (size_t i = index; i < list_size(list) - 1; i++) {
        list->storage[i] = list->storage[i + 1];
    }
    list->curr_size--;
    return return_value;
}

void resize(List *list) {
    size_t new_capacity = list->capacity * GROWTH_FACTOR;
    list->storage = realloc(list->storage, sizeof(void *) * new_capacity);
    list->capacity = new_capacity;
}

void list_add(List *list, void *value) {
  if (list->capacity <= list_size(list)+2) {
      resize(list);
  }
  list->storage[list_size(list)] = value;
  list->curr_size++;

}

void list_add_front(List *list, void *value) {
  if (list->capacity <= list_size(list)+2) {
      resize(list);
  }
    list_add_at_index(list, value, 0);
}

void list_add_at_index(List *list, void *value, size_t index) {
    assert(index <= list_size(list));
    assert(value);
    if (list->capacity <= list_size(list)+2) {
        resize(list);
    }
    for (size_t i = list_size(list); i > index; i--) {
        list->storage[i] = list->storage[i - 1];
    }
    list->storage[index] = value;
    list->curr_size++;
}

void list_set(List *list, size_t index, void *value) {
    assert(value);
    assert(index < list_size(list));
    list->storage[index] = value;
}

List *list_copy(List *list) {
    size_t size = list_size(list);
    List *copy = list_init(size, list->free_item);
    copy->storage = list->storage;
    return copy;
}

bool list_contains(List *list, void *item){
  for (size_t i = 0; i < list_size(list); i++) {
      if(list->storage[i] == item){
        return true;
      }
  }
  return false;
}
