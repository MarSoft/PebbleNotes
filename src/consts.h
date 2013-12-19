#ifndef _CONSTS_H
#define _CONSTS_H

// AppMessage key
enum {
	KEY_CODE = 0, // message code
	KEY_SCOPE = 1, // message scope (tasklists, one list or one task)
	KEY_COUNT = 5, // items count (for code=array_start)
	KEY_ITEM = 6, // item number (for code=array_item)
	KEY_LISTID = 10, // (internal) ID of tasklist
	KEY_TASKID = 11, // (internal) ID for task
	KEY_TITLE = 20, // string: title for list or task
	KEY_SIZE = 21, // count of tasks in tasklist
	KEY_ISDONE = 22, // is the task done
	KEY_HASCOMMENT = 23, // if task has comment field
	KEY_COMMENT = 24, // string: task's comment field
	KEY_ERROR = 50, // string: error text
};
// Message codes
enum {
	CODE_READY = 0, // JS side is ready (and have access token)
	CODE_GET = 10, // get some info
	CODE_CHANGE = 11, // change some info (e.g. mark task as done/undone)
	CODE_ARRAY_START = 20, // start array transfer; app must allocate memory (includes count)
	CODE_ARRAY_ITEM = 21, // array item
	CODE_ARRAY_END = 22, // end array transfer; transaction is finished
	CODE_ERROR = 50, // some error occured; description may be included
};
// Message scopes
enum {
	SCOPE_LISTS = 0,
	SCOPE_TASKS = 1,
	SCOPE_TASK = 2,
};

#endif
