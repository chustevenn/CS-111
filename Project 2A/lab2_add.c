/*
 * NAME: Steven Chu
 * EMAIL: schu92620@gmail.com
 * ID: 905094800
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h>

int num_threads = 1;
int num_iterations = 1;
pthread_t *thread_ids;
char option[10];
int opt_yield;
char sync_option;
pthread_mutex_t lock;
int spin_lock;
int opt_sync = 0;

// Struct to pack arguments for passing to threads
struct Info
{
	long long *counter;
	int iterations;
};

// Free allocated thread space
void free_malloc()
{
	free(thread_ids);
}

// Default add operation
void add(long long *pointer, long long value)
{
	long long sum = *pointer + value;
	if(opt_yield)
		sched_yield();
	*pointer = sum;
}

// Mutex add operation
void mutex_add(long long *pointer, long long value)
{
	pthread_mutex_lock(&lock);
	add(pointer, value);
	pthread_mutex_unlock(&lock);
}

// Spin lock add operation
void spin_lock_add(long long *pointer, long long value)
{
	while(__sync_lock_test_and_set(&spin_lock, 1))
	;
	add(pointer, value);
	__sync_lock_release(&spin_lock);
}

// Compare and swap atomic add operation
void atomic_add(long long *pointer, long long value)
{
	long long oldval;
	long long newval;
	do
	{
		if(opt_yield)
			sched_yield();
		oldval = *pointer;
		newval = oldval + value;
	} while(__sync_val_compare_and_swap(pointer, oldval, newval) != oldval);
}

// Generate string for test option in csv
void get_testname(char *option)
{
	if(opt_yield)
		strcat(option, "-yield");
	if(opt_sync)
	{
		if(sync_option == 'm')
			strcat(option, "-m");
		else if(sync_option == 's')
			strcat(option, "-s");
		else if(sync_option == 'c')
			strcat(option, "-c");
	}
	else {
		strcat(option, "-none");
	}
}

// Wrapper function for error handling pthread_create
void Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void*), void *arg)
{
	if(pthread_create(thread, attr, start_routine, arg) != 0)
	{
		fprintf(stderr, "Failed to create thread: %s\n", strerror(errno));
		exit(1);
	}
}

// Wrapper function for error handling pthread_join
void Pthread_join(pthread_t thread, void **retval)
{
	if(pthread_join(thread, retval) != 0)
	{
		fprintf(stderr, "Failed to wait on thread: %s\n", strerror(errno));
		exit(1);
	}
}

// Perform the specified number of iterations on each thread
void thread_loop(long long *counter, int iterations, int value)
{
	int i;
	for(i = 0; i < iterations; i++)
	{
		if(opt_sync)
		{
			if(sync_option == 'm')
				mutex_add(counter, value);
			else if(sync_option == 's')
				spin_lock_add(counter, value);
			else if(sync_option == 'c')
				atomic_add(counter, value);
		}
		else {
			add(counter, value);
		}
	}
}

// Function called upon thread creation
void* thread_action(void *thread_info)
{
	struct Info *info = (struct Info *) thread_info;
	thread_loop(info->counter, info->iterations, 1);
	thread_loop(info->counter, info->iterations, -1);
	return NULL;
}

int main(int argc, char* argv[])
{
	long long counter = 0;
	// Handle arguments
	while(1)
	{
		int c;
		static struct option long_options[] = {
		{"threads", required_argument, NULL, 1},
		{"iterations", required_argument, NULL, 2},
		{"yield", no_argument, NULL, 3},
		{"sync", required_argument, NULL, 4},
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
				break;
			case 4:
				opt_sync = 1;
				// If mutex option indicated, initialize mutex lock
				if(optarg[0] == 'm')
				{
					if(pthread_mutex_init(&lock, NULL) != 0)
					{
						fprintf(stderr, "Error initializing mutex lock: %s\n", strerror(errno));
						exit(1); 
					} 
				}
				sync_option = optarg[0];
				break;
			default:
				fprintf(stderr, "Unrecognized argument received.\n");
				exit(1);
		}
	}
	// Get start time
	struct timespec start, end;
	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1)
	{
		fprintf(stderr, "Failed to retrieve start time: %s\n", strerror(errno));
		exit(1);
	}
	// Allocate specified number of threads
	int i;
	thread_ids = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
	if(thread_ids == NULL)
	{
		fprintf(stderr, "Malloc failed: %s\n", strerror(errno));
		exit(1);
	}
	atexit(free_malloc);
	// Pack the counter and number of iterations to be passed to each thread
	struct Info thread_info;
	thread_info.counter = &counter;
	thread_info.iterations = num_iterations;
	// Create and wait on the specified number of threads
	for(i = 0; i < num_threads; i++)
		Pthread_create(&thread_ids[i], NULL, thread_action, (void *) &thread_info);
	for(i = 0; i < num_threads; i++)
		Pthread_join(thread_ids[i], NULL);
	// When every thread has completed, note the end time
	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1)
	{
		fprintf(stderr, "Failed to retreive end time: %s\n", strerror(errno));
		exit(1);
	}
	// Generate the test option string and the csv output string
	get_testname(option);
	int total_ops = num_threads * num_iterations * 2;
	long total_time = end.tv_nsec - start.tv_nsec;
	long avg_op_time = total_time/total_ops;
	fprintf(stdout, "add%s,%d,%d,%d,%ld,%ld,%lld\n", option, num_threads, num_iterations, total_ops, total_time, avg_op_time, counter); 
	exit(0);
}
