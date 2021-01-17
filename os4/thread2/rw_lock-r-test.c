#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
  //	Write the code for initializing your read-write lock.
	pthread_rwlock_init(&rw->rwlock, NULL);
	rw->read=0;
	rw->write=0;
}

void r_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
	while(pthread_rwlock_rdlock(&rw->rwlock)!=0);
	rw->read=1;
}

void r_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
	pthread_rwlock_unlock(&rw->rwlock);
	rw->read=0;
}

void w_lock(struct rw_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
	rw->write=2;
	while(pthread_rwlock_wrlock(&rw->rwlock)!=0);
	rw->write=1;
}

void w_unlock(struct rw_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
	pthread_rwlock_unlock(&rw->rwlock);
	rw->writeCnt--;
	rw->write==0;
}
