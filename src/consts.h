#ifndef _CONSTS_H
#define _CONSTS_H

// How many bytes should we reserve.
// If this is 640 or less then task drawing fails.
#define OOM_SAFEGUARD 768
// Minimum free memory required to show taskinfo window
#define OOM_MIN_TASKINFO 1024
#define OUTBOX_DESIRED_MAX 1024

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
	KEY_HASNOTES = 23, // if task has notes field
	KEY_NOTES = 24, // string: task's notes field
	KEY_ACCESS_TOKEN = 40, // string
	KEY_REFRESH_TOKEN = 41, // string
	KEY_OPTION_ID = 45, // int
	KEY_OPTION_VALUE = 46, // int or bool
	KEY_ERROR = 50, // string: error text
};
// Message codes
enum {
	CODE_READY = 0, // JS side is ready (and have access token)
	CODE_GET = 10, // get some info
	CODE_UPDATE = 11, // change some info (e.g. mark task as done/undone)
	CODE_POST = 12, // insert new info (e.g. add new task)
	CODE_ARRAY_START = 20, // start array transfer; app must allocate memory (includes count)
	CODE_ARRAY_ITEM = 21, // array item
	CODE_ARRAY_END = 22, // end array transfer; transaction is finished
	CODE_ITEM_UPDATED = 23, // very similar to ARRAY_ITEM, but contains only changed fields
	CODE_ITEM_ADDED = 24,
	CODE_SAVE_TOKEN = 40, // save (new) access token to watchapp as a backup; no need to reply. args: key_*_token or none to delete
	CODE_RETRIEVE_TOKEN = 41, // token lost, try to retrieve; query - no args, answer - args: key_*_token or none
	CODE_SET_OPTION = 45,
	CODE_ERROR = 50, // some error occured; description may be included
};
// Message scopes
enum {
	SCOPE_LISTS = 0,
	SCOPE_TASKS = 1,
	SCOPE_TASK = 2,
};

// Options (used for storage), should not override with token!
enum {
	OPTION_LARGE_FONT = 1,
	OPTION_TASK_ACTIONS_POSITION = 2,
};

#endif
