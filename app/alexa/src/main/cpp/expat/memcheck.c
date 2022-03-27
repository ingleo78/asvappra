#include <stdio.h>
#include <stdlib.h>
#include "memcheck.h"

typedef struct allocation_entry {
  struct allocation_entry *next;
  struct allocation_entry *prev;
  void *allocation;
  size_t num_bytes;
} AllocationEntry;
static AllocationEntry *alloc_head = NULL;
static AllocationEntry *alloc_tail = NULL;
static AllocationEntry *find_allocation(void *ptr);
void *tracking_malloc(size_t size) {
  AllocationEntry *entry = malloc(sizeof(AllocationEntry));
  if (entry == NULL) {
      printf("Allocator failure\n");
      return NULL;
  }
  entry->num_bytes = size;
  entry->allocation = malloc(size);
  if (entry->allocation == NULL) {
      free(entry);
      return NULL;
  }
  entry->next = NULL;
  if (alloc_head == NULL) {
      entry->prev = NULL;
      alloc_head = alloc_tail = entry;
  } else {
      entry->prev = alloc_tail;
      alloc_tail->next = entry;
      alloc_tail = entry;
  }
  return entry->allocation;
}
static AllocationEntry *find_allocation(void *ptr) {
  AllocationEntry *entry;
  for (entry = alloc_head; entry != NULL; entry = entry->next) {
      if (entry->allocation == ptr) return entry;
  }
  return NULL;
}
void tracking_free(void *ptr) {
  AllocationEntry *entry;
  if (ptr == NULL) return;
  entry = find_allocation(ptr);
  if (entry != NULL) {
      if (entry->prev != NULL) entry->prev->next = entry->next;
      else alloc_head = entry->next;
      if (entry->next != NULL) entry->next->prev = entry->prev;
      else alloc_tail = entry->next;
      free(entry);
  } else printf("Attempting to free unallocated memory at %p\n", ptr);
  free(ptr);
}
void *tracking_realloc(void *ptr, size_t size) {
  AllocationEntry *entry;
  if (ptr == NULL) return tracking_malloc(size);
  if (size == 0) {
      tracking_free(ptr);
      return NULL;
  }
  entry = find_allocation(ptr);
  if (entry == NULL) {
      printf("Attempting to realloc unallocated memory at %p\n", ptr);
      entry = malloc(sizeof(AllocationEntry));
      if (entry == NULL) {
          printf("Reallocator failure\n");
          return NULL;
      }
      entry->allocation = realloc(ptr, size);
      if (entry->allocation == NULL) {
          free(entry);
          return NULL;
      }
      entry->next = NULL;
      if (alloc_head == NULL) {
          entry->prev = NULL;
          alloc_head = alloc_tail = entry;
      } else {
          entry->prev = alloc_tail;
          alloc_tail->next = entry;
          alloc_tail = entry;
      }
  } else {
      entry->allocation = realloc(ptr, size);
      if (entry->allocation == NULL) {
          entry->allocation = ptr;
          return NULL;
      }
  }
  entry->num_bytes = size;
  return entry->allocation;
}
int tracking_report(void) {
  AllocationEntry *entry;
  if (alloc_head == NULL) return 1;
  for (entry = alloc_head; entry != NULL; entry = entry->next) printf("Allocated %lu bytes at %p\n", (long unsigned)entry->num_bytes, entry->allocation);
  return 0;
}