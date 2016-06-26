/* vim: set ts=4 sw=4 expandtab autoindent fileformat=unix: */

#include <stdio.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <time.h>
#include <stdbool.h>

#include "sysinc.h"
#include "module.h"

#include "templog.h"

#define SHM_NAME "/pitempmon"

int templog_item_last(AGENT_REQUEST*, AGENT_RESULT*);
int templog_item_avg(AGENT_REQUEST*, AGENT_RESULT*);

static ZBX_METRIC items[] = {
	{ "temperature.external.last", CF_HAVEPARAMS, templog_item_last, "Test1" },
	{ "temperature.external.avg", CF_HAVEPARAMS, templog_item_avg, "Test1" },
	{ NULL }
};

int item_timeout = 0;
int shm_fd = -1;
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

	shm = (pitempmon_shmem*)mmap(NULL, 
                                 sizeof(pitempmon_shmem),
                                 PROT_READ,
                                 MAP_SHARED,
                                 shm_fd,
                                 0);

	if (shm == (void*)-1) {
		return ZBX_MODULE_FAIL;
	}

	return ZBX_MODULE_OK;
}

int zbx_module_uninit(void)
{
    if (shm_fd != -1) {
        close(shm_fd);
    }

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

/* Look up a sensor by name.
 * This is done once per request, but there is no faster alternative.
 */
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

/* TODO: Timeout */
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

/* As long as the values are all the same type, we can same some
 * code through pointer arithmetic on the field offset.
 */
static int templog_item(AGENT_REQUEST *req, AGENT_RESULT *res, size_t offset)
{
	int status = SYSINFO_RET_FAIL;

	if (req->nparam == 1) {
		templog_lock(item_timeout);
		pitempmon_sensor *s = find_sensor(get_rparam(req, 0));
		if (s && is_updated(s)) {
            int32_t *p = (int*)((char*)s + offset);
			SET_UI64_RESULT(res, *p);
			status = SYSINFO_RET_OK;
		}
		templog_unlock(item_timeout);
	}

	return status;
}

int templog_item_last(AGENT_REQUEST *req, AGENT_RESULT *res)
{
    return templog_item(req, res, offsetof(pitempmon_sensor, last));
}

int templog_item_avg(AGENT_REQUEST *req, AGENT_RESULT *res)
{
    return templog_item(req, res, offsetof(pitempmon_sensor, average));
}

