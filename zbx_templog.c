/* vim: set ts=4 sw=4 expandtab autoindent fileformat=unix: */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <time.h>
#include <stdbool.h>

#include "sysinc.h"
#include "module.h"

#define SHM_NAME "/pitempmon"
#define MAX_SENSORS 10

typedef struct {
	time_t updated;
	int32_t last;
	int32_t average;
	char name[12];
} pitempmon_sensor;

typedef struct {
	int32_t interval;
	pitempmon_sensor sensors[MAX_SENSORS];
} pitempmon_shmem;

int templog_item_last(AGENT_REQUEST*, AGENT_RESULT*);
int templog_item_avg(AGENT_REQUEST*, AGENT_RESULT*);

static ZBX_METRIC items[] = {
	{ "temperature.external.last", CF_HAVEPARAMS, templog_item_last, "Test1" },
	{ "temperature.external.avg", CF_HAVEPARAMS, templog_item_avg, "Test1" },
	{ NULL }
};

int item_timeout = 0;
int shm_fd;
pitempmon_shmem *shm;

int zbx_module_api_version(void)
{
	return ZBX_MODULE_API_VERSION_ONE;
}

int zbx_module_init(void)
{
	shm_fd = shm_open(SHM_NAME, O_RDONLY|O_CREAT, S_IRUSR|S_IWUSR);
	if (shm_fd == -1) {
		return ZBX_MODULE_FAIL;
	}

	shm = (pitempmon_shmem*)mmap(NULL, sizeof(pitempmon_shmem), PROT_READ, MAP_SHARED, shm_fd, 0);
	close(shm_fd);
	if (shm == (void*)-1) {
		return ZBX_MODULE_FAIL;
	}

	return ZBX_MODULE_OK;
}

int zbx_module_uninit(void)
{
	munmap((void*)shm, sizeof(pitempmon_shmem));
	return ZBX_MODULE_OK;
}

ZBX_METRIC *zbx_module_item_list(void)
{
	return items;
}

void zbx_module_item_timeout(int timeout)
{
	item_timeout = timeout;
}

static pitempmon_sensor *find_sensor(const char *name)
{
	for (int i = 0; i < MAX_SENSORS; ++i) {
		pitempmon_sensor *s = &shm->sensors[i];
		if (strncmp(s->name, name, sizeof(s->name)) == 0) {
			return s;
		}
	}

	return NULL;
}

static void templog_lock(int timeout)
{
	flock(shm_fd, LOCK_EX);
}

static void templog_unlock(int timeout)
{
	flock(shm_fd, LOCK_UN);
}

/* Maximum age of measurement is twice the poll interval. */
static bool is_updated(const pitempmon_sensor *sensor)
{
    return ((sensor->updated - time(NULL)) < (shm->interval * 2));
}

int templog_item_last(AGENT_REQUEST *req, AGENT_RESULT *res)
{
	int status = SYSINFO_RET_FAIL;

	if (req->nparam == 1) {
		templog_lock(item_timeout);
		pitempmon_sensor *s = find_sensor(get_rparam(req, 0));
		if (s && is_updated(s)) {
			SET_UI64_RESULT(res, s->last);
			status = SYSINFO_RET_OK;
		}
		templog_unlock(item_timeout);
	}

	return status;
}

int templog_item_avg(AGENT_REQUEST *req, AGENT_RESULT *res)
{
	int status = SYSINFO_RET_FAIL;

	if (req->nparam == 1) {
		templog_lock(item_timeout);
		pitempmon_sensor *s = find_sensor(get_rparam(req, 0));
		if (s) {
			SET_UI64_RESULT(res, s->average);
			status = SYSINFO_RET_OK;
		}
		templog_unlock(item_timeout);
	}

	return status;
}

