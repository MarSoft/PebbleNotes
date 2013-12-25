/*
 * This module handles most of the watch-phone communication.
 */

#ifndef _COMM_H
#define _COMM_H

void comm_query_tasklists();
void comm_query_tasks(int);
void comm_query_task_details(int, int);
void comm_update_task_status(int, int, bool);

typedef void(* CommJsReadyCallback)(void *data);
// Do something when JS will be ready
void comm_on_js_ready(CommJsReadyCallback*, void*);
// Returns false if there is no bluetooth connection
// or there is unsent message waiting (usually if JS was not loaded yet)
// If not available, show message in statusbar
bool comm_is_available();
bool comm_is_available_silent(); // don't update statusbar

void comm_init();
void comm_deinit();

#endif
