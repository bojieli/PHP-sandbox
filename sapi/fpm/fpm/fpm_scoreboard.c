
	/* $Id: fpm_status.c 312399 2011-06-23 08:03:52Z fat $ */
	/* (c) 2009 Jerome Loyet */

#include "php.h"
#include "SAPI.h"
#include <stdio.h>
#include <time.h>

#include "fpm_config.h"
#include "fpm_scoreboard.h"
#include "fpm_shm.h"
#include "fpm_sockets.h"
#include "fpm_worker_pool.h"
#include "fpm_clock.h"
#include "zlog.h"

static struct fpm_scoreboard_s *fpm_scoreboard = NULL;
static int fpm_scoreboard_i = -1;

int fpm_scoreboard_init_main() /* {{{ */
{
	struct fpm_worker_pool_s *wp;
	int i;

	for (wp = fpm_worker_all_pools; wp; wp = wp->next) {
		if (wp->config->pm_max_children < 1) {
			zlog(ZLOG_ERROR, "[pool %s] Unable to create scoreboard SHM because max_client is not set", wp->config->name);
			return -1;
		}

		if (wp->scoreboard) {
			zlog(ZLOG_ERROR, "[pool %s] Unable to create scoreboard SHM because it already exists", wp->config->name);
			return -1;
		}

		wp->scoreboard = fpm_shm_alloc(sizeof(struct fpm_scoreboard_s) + (wp->config->pm_max_children - 1) * sizeof(struct fpm_scoreboard_proc_s *));
		if (!wp->scoreboard) {
			return -1;
		}
		wp->scoreboard->nprocs = wp->config->pm_max_children;
		for (i = 0; i < wp->scoreboard->nprocs; i++) {
			wp->scoreboard->procs[i] = fpm_shm_alloc(sizeof(struct fpm_scoreboard_proc_s));
			if (!wp->scoreboard->procs[i]) {
				return -1;
			}
			memset(wp->scoreboard->procs[i], 0, sizeof(struct fpm_scoreboard_proc_s));
		}

		wp->scoreboard->pm = wp->config->pm;
		wp->scoreboard->start_epoch = time(NULL);
		strlcpy(wp->scoreboard->pool, wp->config->name, sizeof(wp->scoreboard->pool));
	}
	return 0;	
}
/* }}} */

void fpm_scoreboard_update(int idle, int active, int lq, int lq_len, int requests, int max_children_reached, int action, struct fpm_scoreboard_s *scoreboard) /* {{{ */
{
	if (!scoreboard) {
		scoreboard = fpm_scoreboard;
	}
	if (!scoreboard) {
		zlog(ZLOG_WARNING, "Unable to update scoreboard: the SHM has not been found");
		return;
	}


	fpm_spinlock(&scoreboard->lock, 0);
	if (action == FPM_SCOREBOARD_ACTION_SET) {
		if (idle >= 0) {
			scoreboard->idle = idle;
		}
		if (active >= 0) {
			scoreboard->active = active;
		}
		if (lq >= 0) {
			scoreboard->lq = lq;
		}
		if (lq_len >= 0) {
			scoreboard->lq_len = lq_len;
		}
#ifdef HAVE_FPM_LQ /* prevent unnecessary test */
		if (scoreboard->lq > scoreboard->lq_max) {
			scoreboard->lq_max = scoreboard->lq;
		}
#endif
		if (requests >= 0) {
			scoreboard->requests = requests;
		}

		if (max_children_reached >= 0) {
			scoreboard->max_children_reached = max_children_reached;
		}
	} else {
		if (scoreboard->idle + idle > 0) {
			scoreboard->idle += idle;
		} else {
			scoreboard->idle = 0;
		}

		if (scoreboard->active + active > 0) {
			scoreboard->active += active;
		} else {
			scoreboard->active = 0;
		}

		if (scoreboard->requests + requests > 0) {
			scoreboard->requests += requests;
		} else {
			scoreboard->requests = 0;
		}

		if (scoreboard->max_children_reached + max_children_reached > 0) {
			scoreboard->max_children_reached += max_children_reached;
		} else {
			scoreboard->max_children_reached = 0;
		}
	}

	if (scoreboard->active > scoreboard->active_max) {
		scoreboard->active_max = scoreboard->active;
	}

	fpm_unlock(scoreboard->lock);
}
/* }}} */

struct fpm_scoreboard_s *fpm_scoreboard_get() /* {{{*/
{
	return fpm_scoreboard;
}
/* }}} */

struct fpm_scoreboard_proc_s *fpm_scoreboard_proc_get(struct fpm_scoreboard_s *scoreboard, int child_index) /* {{{*/
{
	if (!scoreboard) {
		scoreboard = fpm_scoreboard;
	}

	if (!scoreboard) {
		return NULL;
	}

	if (child_index < 0) {
		child_index = fpm_scoreboard_i;
	}

	if (child_index < 0 || child_index >= scoreboard->nprocs) {
		return NULL;
	}

	return scoreboard->procs[child_index];
}
/* }}} */

struct fpm_scoreboard_s *fpm_scoreboard_acquire(struct fpm_scoreboard_s *scoreboard, int nohang) /* {{{ */
{
	struct fpm_scoreboard_s *s;

	s = scoreboard ? scoreboard : fpm_scoreboard;
	if (!s) {
		return NULL;
	}

	if (!fpm_spinlock(&s->lock, nohang)) {
		return NULL;
	}
	return s;
}
/* }}} */

void fpm_scoreboard_release(struct fpm_scoreboard_s *scoreboard) {
	if (!scoreboard) {
		return;
	}

	scoreboard->lock = 0;
}

struct fpm_scoreboard_proc_s *fpm_scoreboard_proc_acquire(struct fpm_scoreboard_s *scoreboard, int child_index, int nohang) /* {{{ */
{
	struct fpm_scoreboard_proc_s *proc;

	proc = fpm_scoreboard_proc_get(scoreboard, child_index);
	if (!proc) {
		return NULL;
	}

	if (!fpm_spinlock(&proc->lock, nohang)) {
		return NULL;
	}

	return proc;
}
/* }}} */

void fpm_scoreboard_proc_release(struct fpm_scoreboard_proc_s *proc) /* {{{ */
{
	if (!proc) {
		return;
	}

	proc->lock = 0;
}

void fpm_scoreboard_free(struct fpm_scoreboard_s *scoreboard) /* {{{ */
{
	int i;

	if (!scoreboard) {
		zlog(ZLOG_ERROR, "**scoreboard is NULL");
		return;
	}

	for (i = 0; i < scoreboard->nprocs; i++) {
		if (!scoreboard->procs[i]) {
			continue;
		}
		fpm_shm_free(scoreboard->procs[i], sizeof(struct fpm_scoreboard_proc_s));
	}
	fpm_shm_free(scoreboard, sizeof(struct fpm_scoreboard_s));
}
/* }}} */

void fpm_scoreboard_child_use(struct fpm_scoreboard_s *scoreboard, int child_index, pid_t pid) /* {{{ */
{
	struct fpm_scoreboard_proc_s *proc;
	fpm_scoreboard = scoreboard;
	fpm_scoreboard_i = child_index;
	proc = fpm_scoreboard_proc_get(scoreboard, child_index);
	if (!proc) {
		return;
	}
	proc->pid = pid;
}
/* }}} */

void fpm_scoreboard_proc_free(struct fpm_scoreboard_s *scoreboard, int child_index) /* {{{ */
{
	if (!scoreboard) {
		return;
	}

	if (child_index < 0 || child_index >= scoreboard->nprocs) {
		return;
	}

	if (scoreboard->procs[child_index] && scoreboard->procs[child_index]->used > 0) {
		memset(scoreboard->procs[child_index], 0, sizeof(struct fpm_scoreboard_proc_s));
	}

	/* set this slot as free to avoid search on next alloc */
	scoreboard->free_proc = child_index;
}
/* }}} */

int fpm_scoreboard_proc_alloc(struct fpm_scoreboard_s *scoreboard, int *child_index) /* {{{ */
{
	int i = -1;

	if (!scoreboard || !child_index) {
		return -1;
	}

	/* first try the slot which is supposed to be free */
	if (scoreboard->free_proc >= 0 && scoreboard->free_proc < scoreboard->nprocs) {
		if (scoreboard->procs[scoreboard->free_proc] && !scoreboard->procs[scoreboard->free_proc]->used) {
			i = scoreboard->free_proc;
		}
	}

	if (i < 0) { /* the supposed free slot is not, let's search for a free slot */
		zlog(ZLOG_DEBUG, "[pool %s] the proc->free_slot was not free. Let's search", scoreboard->pool);
		for (i = 0; i < scoreboard->nprocs; i++) {
			if (scoreboard->procs[i] && !scoreboard->procs[i]->used) { /* found */
				break;
			}
		}
	}

	/* no free slot */
	if (i < 0 || i >= scoreboard->nprocs) {
		zlog(ZLOG_ERROR, "[pool %s] no free scoreboard slot", scoreboard->pool);
		return -1;
	}

	scoreboard->procs[i]->used = 1;
	*child_index = i;

	/* supposed next slot is free */
	if (i + 1 >= scoreboard->nprocs) {
		scoreboard->free_proc = 0;
	} else {
		scoreboard->free_proc = i + 1;
	}

	return 0;
}
/* }}} */

