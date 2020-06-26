
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include "SortedList.h"



void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    if (list == NULL)
        list = element;
    if (opt_yield & INSERT_YIELD)
        sched_yield();

    SortedListElement_t *curr = list;
    while (curr->next != list && curr->next != NULL)
    {
        if (curr->next->key != NULL && strcmp(curr->next->key, element->key) >= 0)
        {
            element->next = curr->next;
            element->prev = curr;
            curr->next->prev = element;
            curr->next = element;
            return;
        }
        else
            curr = curr->next;
    }
    curr->next = element;
    element->prev = curr;
    element->next = list;
    list->prev = element;

}




int SortedList_delete(SortedListElement_t *element)
{
    if (element == NULL)
        return 1;
    if (element->next->prev != element || element->prev->next != element)
        return 1;
    if (opt_yield & DELETE_YIELD) 
        sched_yield();
    element->next->prev = element->prev;
    element->prev->next = element->next;
    return 0;
}



SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    if (key == NULL)
        return NULL;

    SortedListElement_t *curr = list->next;
    if (opt_yield & LOOKUP_YIELD) 
        sched_yield();
    while (curr != list)
    {
        if (strcmp(curr->key, key) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}



int SortedList_length(SortedList_t *list)
{
    int len;
    SortedListElement_t *curr = list;

    if (curr == NULL)
        return -1;
    len = 0;
    if (opt_yield & LOOKUP_YIELD) {
        sched_yield();
    }

    while (curr->next != list && curr->next != NULL)
    {
        if (curr->prev->next != curr || curr->next->prev != curr){
            return -1;
        }
        len++;
        curr = curr->next;
    }
    return len;
}
