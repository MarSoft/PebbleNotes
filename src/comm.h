/*
 * This module handles most of the watch-phone communication.
 */

#ifndef _COMM_H
#define _COMM_H

void comm_query_tasklists();
void comm_query_tasks(int);
void comm_query_task_details(int, int);

typedef void(* CommJsReadyCallback)(void *data);
// Do something when JS will be ready
void comm_on_js_ready(CommJsReadyCallback*);

void comm_init();
void comm_deinit();

#endif
