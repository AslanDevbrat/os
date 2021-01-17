#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct rw_lock
{
	pthread_rwlock_t rwlock;
	int read;
	int write;
	int readCnt;
	int writeCnt;
};

void init_rwlock(struct rw_lock * rw);
void r_lock(struct rw_lock * rw);
void r_unlock(struct rw_lock * rw);
void w_lock(struct rw_lock * rw);
void w_unlock(struct rw_lock * rw);
long *max_element(long* start, long* end);
long *min_element(long* start, long* end);
