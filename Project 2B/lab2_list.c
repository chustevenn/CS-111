/* NAME: Steven Chu
 * EMAIL: schu92620@gmail.com
 * ID: 905094800
 */

#include "SortedList.h"
#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int opt_yield = 0;
int num_threads = 1;
int num_iterations = 1;
int opt_sync = 0;
pthread_t *thread_ids;
pthread_mutex_t *lock;
int *spin_lock;
char option[10] = "list";
char sync_option;
SortedList_t *list;
SortedListElement_t *elements;
int *indices;
char yields[3];
int num_sublists = 1;
long long lock_wait_time = 0;

// Exit functions to free allocated memory
void free_indices()
{
	free(indices);
}

void free_malloc()
{
	free(thread_ids);
}

void free_head()
{
	free(list);
}

void free_list()
{
	int i;
	for(i = 0; i < (num_threads * num_iterations); i++)
		free((char*) elements[i].key);
	free(elements);
}

void free_locks()
{
	free(lock);
}

void free_slocks()
{
	free(spin_lock);
}

// Segfault signal handler
void catch_segfault()
{	
	write(2, "Caught segmentation fault.\n", 27);
	exit(2);
}

// Obtain runtime from timespec structs
long long gettime(struct timespec *start, struct timespec *end)
{
	return 1000000000L * (end->tv_sec - start->tv_sec) + end->tv_nsec - start->tv_nsec;
}

// Mutex lock while recording wait time
void mutex_lock_timed(pthread_mutex_t *lock)
{
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	pthread_mutex_lock(lock);
	clock_gettime(CLOCK_MONOTONIC, &end);
	lock_wait_time += gettime(&start, &end);
}

// Spin lock while recording wait time
void spin_lock_timed(int *slock)
{
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	while(__sync_lock_test_and_set(slock, 1))
	;
	clock_gettime(CLOCK_MONOTONIC, &end);
	lock_wait_time += gettime(&start, &end);
}
// Wrapper function for pthread_create error handling
void Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	if(pthread_create(thread, attr, start_routine, arg) != 0)
	{
		fprintf(stderr, "Failed to create thread: %s\n", strerror(errno));
		exit(1);
	}
}

// Wrapper function for pthread_join error handling
void Pthread_join(pthread_t thread, void **retval)
{
        if(pthread_join(thread, retval) != 0)
        {
                fprintf(stderr, "Failed to wait on thread: %s\n", strerror(errno));
                exit(1);
        }
}

// Generate random key
char *rand_key()
{
	char *key = (char *) malloc(4);
	for(int i = 0; i < 3; i++)
		key[i] = 'A' + (rand() % 26);
	key[3] = '\0';
	return key;
}

// Compute index for hashmap based on element key
int get_hash_index(const char* key)
{
	int str_total = 0;
	for(int i = 0; i < 3; i++)
		str_total += key[i];
	int hash_index = str_total % num_sublists;
	return hash_index;
}

// Thread iterative insert function with lock handling
void list_insert_all(int index)
{
	int i;
	for(i = index; i < (num_iterations * num_threads); i += num_threads)
	{
		int hash_index = get_hash_index(elements[i].key);
		if(opt_sync)
		{
			if(sync_option == 'm')
			{
				mutex_lock_timed(&lock[hash_index]);	
				SortedList_insert(&list[hash_index], &elements[i]);
				pthread_mutex_unlock(&lock[hash_index]);
			}
			else if(sync_option == 's')
			{
				spin_lock_timed(&spin_lock[hash_index]);	
				SortedList_insert(&list[hash_index], &elements[i]);
				__sync_lock_release(&spin_lock[hash_index]);
			}
		}
		else
		{
			SortedList_insert(&list[hash_index], &elements[i]);
		}
	}
}

// Retrieve length of list with lock handling
void list_getlength()
{
	int length = 0;
	if(opt_sync)
	{
		int i;
		for(i = 0; i < num_sublists; i++)
		{
			if(sync_option == 'm')
			{
				mutex_lock_timed(&lock[i]);	
				length += SortedList_length(&list[i]);
				pthread_mutex_unlock(&lock[i]);
			}
			else if(sync_option == 's')
			{
				spin_lock_timed(&spin_lock[i]);
				length += SortedList_length(&list[i]);
				__sync_lock_release(&spin_lock[i]);
			}
		}
	}
	else
	{
		int i;
		for(i = 0; i < num_sublists; i++)
		{
			length += SortedList_length(&list[i]);
		}
	}
	if(length == -1)
	{
		fprintf(stderr, "Invalid list.\n");
		exit(2);
	}
}

// Iterative search and delete with lock handling 
void list_delete(int index)
{
	int i;
	for(i = index; i < (num_threads * num_iterations); i += num_threads)
	{
		int hash_index = get_hash_index(elements[i].key);
		if(opt_sync)
		{
			if(sync_option == 'm')
			{
				mutex_lock_timed(&lock[hash_index]);	
				SortedListElement_t *elem;
				elem = SortedList_lookup(&list[hash_index], elements[i].key);
				if(elem == NULL)
				{
					fprintf(stderr, "Error in finding element to delete.\n");
					exit(2);
				}
				if(SortedList_delete(elem) == 1)
				{
					fprintf(stderr, "Error deleting element.\n");
					exit(2);
				}
				pthread_mutex_unlock(&lock[hash_index]);
			}
			else if(sync_option == 's')
			{
				spin_lock_timed(&spin_lock[hash_index]);
				SortedListElement_t *elem;
                                elem = SortedList_lookup(&list[hash_index], elements[i].key);
                                if(elem == NULL)
                                {
                                        fprintf(stderr, "Error in finding element to delete.\n");
                                        exit(2);
                                }
                                if(SortedList_delete(elem) == 1)
                                {
                                        fprintf(stderr, "Error deleting element.\n");
                                        exit(2);
                                }
				__sync_lock_release(&spin_lock[hash_index]);
			}
		}
		else
		{
			SortedListElement_t *elem;
			elem = SortedList_lookup(&list[hash_index], elements[i].key);
			if(elem == NULL)
			{
				fprintf(stderr, "Error in finding element to delete.\n");
				exit(2);
			}
			if(SortedList_delete(elem) == 1)
			{
				fprintf(stderr, "Error deleting element.\n");
				exit(2);
			}
		}
	}
}

// Function to call when threads are created
void *thread_action(void* index)
{
	int *i = (int *) index;
	list_insert_all(*i);
	list_getlength();
	list_delete(*i);
	return NULL; 
}

void initialize_locks()
{
	// Allocate and initialize mutex locks if requested
	if(sync_option == 'm')
	{
		lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * num_sublists);
		if(lock == NULL)
		{
			fprintf(stderr, "Malloc error: %s\n", strerror(errno));
			exit(2);
		}
		atexit(free_locks);
		for(int i = 0; i < num_sublists; i++)
		{
			if(pthread_mutex_init(&lock[i], NULL) != 0)
			{
				fprintf(stderr, "Error initializing mutex lock: %s\n", strerror(errno));
				exit(1);
			}
		}
	}

	// Allocate and initialize spin locks if requested
	if(sync_option == 's')
	{
		spin_lock = (int *) malloc(sizeof(int) * num_sublists);
		if(spin_lock == NULL)
		{
			fprintf(stderr, "Malloc error: %s\n", strerror(errno));
			exit(2);
		}
		atexit(free_slocks);
		for(int i = 0; i < num_sublists; i++)
		{
			spin_lock[i] = 0;
		}
	}

}

int main(int argc, char *argv[])
{
	// Handle options
	while(1)
	{
		int c;
		static struct option long_options[] = {
		{"threads", required_argument, NULL, 1},
		{"iterations", required_argument, NULL, 2},
		{"yield", required_argument, NULL, 3},
		{"sync", required_argument, NULL, 4},
		{"lists", required_argument, NULL, 5},
		{0, 0, 0, 0}};
		c = getopt_long(argc, argv, "", long_options, NULL);
		if(c == -1)
			break;
		switch(c)
		{
			case 1:
				num_threads = atoi(optarg);
				break;
			case 2:
				num_iterations = atoi(optarg);
				break;
			case 3:
				opt_yield = 1;
				// Bitwise OR opt_yield and the selected option so that the bitwise AND
				// in SortedList.c returns true.
				for(int i = 0; i < (int) strlen(optarg); i++)
				{
					if(optarg[i] == 'i')
					{
						opt_yield |= INSERT_YIELD;
						strcat(yields, "i");
					}
					else if(optarg[i] == 'd')
					{
						opt_yield |= DELETE_YIELD;
						strcat(yields, "d");
					}
					else if(optarg[i] == 'l')
					{
						opt_yield |= LOOKUP_YIELD;
						strcat(yields, "l");
					}
					else
					{
						fprintf(stderr, "Unrecognized argument passed to --yield.\n");
						exit(1);
					}
				}
				break;
			case 4:
				opt_sync = 1;
				sync_option = optarg[0];
				break;
			case 5:
				num_sublists = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Unrecognized argument received.\n");
				exit(1);
		}
	}
	// Initialize locks
	initialize_locks();
	// Allocate memory for the sublist hash table
	list = (SortedList_t *) malloc(sizeof(SortedList_t) * num_sublists);
        if(list == NULL)
        {
                fprintf(stderr, "Malloc failed: %s\n", strerror(errno));
                exit(1);
        }
	atexit(free_head);
	for(int i = 0; i < num_sublists; i++)
	{
		list[i].key = NULL;
		list[i].next = &list[i];
		list[i].prev = &list[i];
	}
	// Allocate memory for the list
	elements = (SortedListElement_t *) malloc(sizeof(SortedListElement_t) * num_threads * num_iterations);
	if(elements == NULL)
        {
                fprintf(stderr, "Malloc failed: %s\n", strerror(errno));
                exit(1);
        }
	atexit(free_list);
	// Set random keys for the list
	int i;
	for(i = 0; i < (num_threads * num_iterations); i++)
	{
		elements[i].key = rand_key();
	}
	// Register signal handler for segfault
	signal(SIGSEGV, catch_segfault);
	// Retreive start time
	struct timespec start, end;
	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		fprintf(stderr, "Failed to retrieve start time: %s\n", strerror(errno));
		exit(1);
	}
	// Allocate memory for threads
	thread_ids = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
	if(thread_ids == NULL)
	{
		fprintf(stderr, "Malloc failed: %s\n", strerror(errno));
		exit(1);
	}
	atexit(free_malloc);
	// Allocate memory for thread indices (starting position in list)
	indices = malloc(sizeof(int) * num_threads);
        if(indices == NULL)
        {
                fprintf(stderr, "Malloc failed: %s\n", strerror(errno));
                exit(1);
        }
	atexit(free_indices);
	// Create all threads
	for(i = 0; i < num_threads; i++)
	{
		indices[i] = i;
		Pthread_create(&thread_ids[i], NULL, &thread_action, (void *) &indices[i]);	
	}
	// Wait for all threads
	for(i = 0; i < num_threads; i++)
	{
		Pthread_join(thread_ids[i], NULL);
	}
	// Retrieve end time after all threads exit
	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		fprintf(stderr, "Failed to retrieve end time: %s\n", strerror(errno));
		exit(1);
	}
	// Check that list length is equal to zero
	if(SortedList_length(list) != 0)
	{
		fprintf(stderr, "List length not zero.\n");
		exit(2);
	}
	// Generate test option string for csv
	if(opt_yield)
	{
 		strcat(option, "-");
		strcat(option, yields);
	}
	else
		strcat(option, "-none");
	if(sync_option == 'm')
		strcat(option, "-m");
	else if(sync_option == 's')
		strcat(option, "-s");
	else
		strcat(option, "-none");
	// Compute number of lock operations and avg wait time
	long long lock_ops;
	if(!opt_sync)
	{
		lock_ops = 0;
	}
	else
	{
		lock_ops = num_threads * ((2*num_iterations) + 1);
	}
	long long avg_lock_waittime = lock_wait_time/lock_ops;
	// Generate numerical data for csv
	long total_ops = num_threads * num_iterations * 3;
	long long total_time = gettime(&start, &end);
	long long avg_op_time = total_time/total_ops;
	fprintf(stdout, "%s,%d,%d,%d,%ld,%lld,%lld,%lld\n", option, num_threads, num_iterations, num_sublists, total_ops, total_time, avg_op_time, avg_lock_waittime);
	exit(0); 
}
