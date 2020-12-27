/* NAME: Steven Chu
 * EMAIL: schu92620@gmail.com
 * ID: 905094800
 */

#include "SortedList.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	// If list or element is invalid, return immediately
	if(list == NULL || element == NULL)
		return;
	// Initialize pointer to first element
	SortedListElement_t *curnode = list->next;
	// Find first element in list greater than input element
	while(curnode != list)
	{
		if(strcmp(curnode->key, element->key) <= 0)
			break;
		curnode = curnode->next;
	}

	// Yield
	if(opt_yield & INSERT_YIELD)	
		sched_yield();
	// Insert input element before
	SortedListElement_t *prevnode = curnode->prev;
	prevnode->next = element;
	element->prev = prevnode;
	element->next = curnode;
	curnode->prev = element;
}

int SortedList_delete(SortedListElement_t *element)
{	
	// Return 1 if element is invalid
	if(element == NULL)
		return 1;
	// Return 1 if prev/next pointers are corrupted
	if(element->next->prev != element || element->prev->next != element)
		return 1;
	// Yield
	if(opt_yield & DELETE_YIELD)
		sched_yield();
	// Delete element otherwise and return 0
	SortedListElement_t *prevnode = element->prev;
	SortedListElement_t *nextnode = element->next;
	prevnode->next = nextnode;
	nextnode->prev = prevnode;
	return 0;	
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	// If list is invalid, return null.
	if(list == NULL)
		return NULL;
	// Linear search through list and find key
	SortedListElement_t *curnode = list->next;

	while(curnode != list)
	{
		if(strcmp(curnode->key, key) == 0)
			return curnode;
		// Yield
		if(opt_yield & LOOKUP_YIELD)
			sched_yield();
		curnode = curnode->next;
	}
	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	// If list is invalid, return -1
	if(list == NULL)
		return -1;
	SortedListElement_t *curnode = list->next;
	int count = 0;
	while(curnode != list)
	{
		if(curnode->next->prev != curnode || curnode->prev->next != curnode)
			return -1;
		// Yield
		if(opt_yield & LOOKUP_YIELD)
			sched_yield();
		count++;
		curnode = curnode->next;
	}
	return count;
}

