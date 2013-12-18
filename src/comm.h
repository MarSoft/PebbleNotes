/*
 * This module handles most of the watch-phone communication.
 */

#ifndef _COMM_H
#define _COMM_H

void comm_query_tasklists();
void comm_query_tasks(int);
void comm_query_task_details(int, int);

void comm_in_received_handler(DictionaryIterator*, void*);

#endif
