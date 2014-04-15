#ifndef _TASKINFO_H
#define _TASKINFO_H

#include "tasks.h"

void ti_init();
void ti_deinit();
void ti_show(int, int);
void ti_is_active();
int ti_current_taskId();

#endif
