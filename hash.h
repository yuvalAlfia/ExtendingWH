#ifndef HASH_H
#define HASH_H

#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define MIN_N 100
#define MAX_N 1000
#define MIN_M 10
#define MAX_M 1000

typedef struct Node{
    int key;
    int* value;
    struct Node* next;
}Node;

typedef struct hash_table
{
    int size;
    int region;
    int m_val;
    struct Node **table;
}hash_table;

typedef struct hash_table HashTable;

hash_table* hash_init (int N, int M);
int hash_insert (hash_table const *hp, int k, int v[]);
int hash_delete (hash_table const *hp, int k);
int hash_get (hash_table const *hp, int k, int *vptr);
int hash_search(hash_table const *hp, int k);
bool was_inserted_since(time_t sinceTime, int key, hash_table *hash);
int hash_destroy (hash_table const *hp);
int hash_clean(hash_table const *hp);


#endif /* HASH_H */
