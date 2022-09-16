#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define unlock pthread_mutex_unlock
#define lock pthread_mutex_lock


pthread_mutex_t mutexes[100];
pthread_mutex_t destroy_mutex = PTHREAD_MUTEX_INITIALIZER;

HashTable* hash_init(int N, int M){
	int i;
	if( (N < MIN_N || N > MAX_N) || (M < MIN_M || M > MAX_M)){
		printf("N and M must be between 100 - 1000.\n");
		exit(1);
	}
	else{
		// allocate space as big as table
		HashTable *new_table;
		new_table = (HashTable *) malloc(sizeof( HashTable ));

		// initialize members
		new_table->size = N;
		new_table->m_val = M;
		new_table->table = (Node *) malloc(sizeof(Node*) * N);

		// If divides with no remainder, K = N/M
		if(N % M == 0){
			new_table->region = ceil(N/M);
		}
		// Else, +1
		else{
			new_table->region = ceil(N/M) + 1;
		}

		// All bucket heads to be NULL at first
		for( i = 0; i < N; i++ ) {
			new_table->table[i] = NULL;
		}

		// init all mutex locks
		for(i = 0; i < (new_table->region); i++){
			pthread_mutex_init(&mutexes[i], NULL);
		}
		return (new_table);
	}
	return (NULL);
}

// Simple hash function, hash = key % N
int hash_func(int key, int N){
	while(key <= 0 )
	{
		key += N;
	}
	int hash = key % N;
	return hash;
}

// Function inserting to hp hash table, key k and v value of k.
int hash_insert (hash_table const *hp, int k, int v[]){

	// The right bucket
    int hashed = hash_func(k, hp->size);
	// Correct lock for region K
	int kth_lock = hashed / hp->m_val;

	lock(&mutexes[kth_lock]);
	struct Node* place = hp->table[hashed];
	struct Node* toAdd;

	// Our bucket is initially empty
    if(place == NULL){
    	hp->table[hashed] = (struct Node*) malloc (sizeof(Node));
    	hp->table[hashed]->key = k;
        hp->table[hashed]->next = NULL;
		hp->table[hashed]->value = v;
    	unlock(&mutexes[kth_lock]);
    	return 0;
    }

	// Searching if the key exists somewhere in the buckets
	else{
    	while(place->next != NULL){
	    	place = place->next;
	    }

		// If the execution made down until here, then the key was not found in any buckets
		toAdd = (struct Node*)malloc(sizeof(struct Node));
		place->next = toAdd;
        toAdd->next = NULL;
	    toAdd->key = k;
		toAdd->value = v;

		//unlock and return 0
		unlock(&mutexes[kth_lock]);
		return (0);
	}
}


bool was_inserted_since(time_t sinceTime, int key, hash_table *hash) {
  	int hashed = hash_func(key, hash->size);
	int kth_lock = hashed / hash->m_val;

	lock(&mutexes[kth_lock]);
	struct Node* place = hash->table[hashed];
  	printf("cur time %d\n", (int)sinceTime);
  	if(place == NULL)
  	{
  		printf("didn't find the key %d\n", key);
  		unlock(&mutexes[kth_lock]);
	    return false;
	}
  	while(place != NULL)
  	{
  		if(place->key == key && ((int)place->value[0] >= ((int)sinceTime)) && place->value[1]==1)
  		{
    		unlock(&mutexes[kth_lock]);
    		return 1;
  		}
  		printf("looking on key %d with time = %d\n", place->key, ((int)place->value[0]));
  		place = place->next;
	}
	unlock(&mutexes[kth_lock]);
  	return 0;
}

int hash_clean(hash_table const *hp)
{
	int i;
	lock(&destroy_mutex);

	// Declaring the node to be deleted
	struct Node* dest_node;

	//iterating whole table and freeing all
	for(i = 0; i < hp->size; ++i){
		dest_node = hp->table[i];
		while (dest_node != NULL){
			struct Node* del_ptr = dest_node;
			dest_node = dest_node->next;
			free(del_ptr->value);
			free(del_ptr);
		}
		hp->table[i] = NULL;
	}
	unlock(&destroy_mutex);
	return 1;

}
// Function deleting from hp hash table, key k and it's properties
int hash_delete (hash_table const *hp, int k) {
	if(k <= 0){
		printf("invalid key value\n");
		return -1;
	}
	struct Node* toDelete;

	// The right bucket
	int hashed = hash_func(k, hp->size);
	// Correct lock for region K
	int kth_lock = hashed / hp->m_val;

	lock(&mutexes[kth_lock]);
	struct Node* place = hp->table[hashed];

    if(place != NULL){
    	if(place->key == k){
    		hp->table[hashed] = place->next;
    		free(place->value);
    		free(place);
			unlock(&mutexes[kth_lock]);
    		return 0;
    	}
    }
    else{
    	unlock(&mutexes[kth_lock]);
    	return -1;
    }

    while(place->next != NULL){
		// having found our key in place's next
    	if(place->next->key == k){
			//toDelete is set to place->next
    		toDelete = place->next;
			//before deleting, it's set to the upcoming one
    		place->next = place->next->next;
			//delete
    		free(toDelete);
    		unlock(&mutexes[kth_lock]);
    		return 0;
    	}
    	place = place->next;
    }

	// If the key cannot be deleted
    printf("Key: %d -- cannot be deleted.\n", k);
    unlock(&mutexes[kth_lock]);

	return -1;
}

// Function returning 0 on success, -1 on fail
// Retrieves key k's value into vptr from hash table hp
int hash_get (hash_table const *hp, int k, int *vptr ){
	if(k <= 0){
		printf("invalid key value\n");
		return -1;
	}

	// finding the correct bucket
	int hashed = hash_func(k, hp->size);
	int kth_lock = hashed / hp->m_val;

	// locking respective region
	lock(&mutexes[kth_lock]);
	struct Node* place = hp->table[hashed];

	// iterate to find the key
	while(place != NULL){
		// when key's found
		if(place->key == k){
			*vptr = place->value;
			unlock(&mutexes[kth_lock]);
			return 0;
		}
		place = place->next;
	}
	unlock(&mutexes[kth_lock]);

	return -1;
}

// Destroying whole table with a single function
int hash_destroy (hash_table const *hp){
	int i;
	lock(&destroy_mutex);

	// Declaring the node to be deleted
	struct Node* dest_node;

	//iterating whole table and freeing all
	for(i = 0; i < hp->size; ++i){
		dest_node = hp->table[i];
		while (dest_node != NULL){
			struct Node* del_ptr = dest_node;
			dest_node = dest_node->next;
			free(del_ptr->value);
			free(del_ptr);
		}
	}
	free(hp->table);
	free(hp);
	unlock(&destroy_mutex);
	return (0);
}
