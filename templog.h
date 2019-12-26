/* vim: set ts=4 sw=4 expandtab autoindent fileformat=unix: */
#ifndef __TEMPLOG_H__

#include <time.h>

#define MAX_SENSORS 10

typedef struct {
	time_t updated;
	int32_t last;
	int32_t average;
	char name[12];
} pitempmon_sensor;

typedef struct {
	int32_t pid;
	int32_t interval;
	pitempmon_sensor sensors[MAX_SENSORS];
} pitempmon_shmem;

#endif /* !__TEMPLOG_H__ */

