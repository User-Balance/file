/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int is_lockfree(int lock);
LOCAL int is_needwait(int lock, int priority);
extern unsigned long ctr1000;


/*------------------------------------------------------------------------
 *  * lock  --  make current process wait on a lock
 *   *------------------------------------------------------------------------
 *    */
SYSCALL	lock(int lock, int type, int priority)
{
	STATWORD ps;    
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	int i;

	disable(ps);
	if (isbadlock(lock) || (lptr= &locktab[lock])->lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	if(((pptr = &proctab[currpid])->plbefore[lock] == 1) && lptr->ldeleted == 1){
		restore(ps);
		return(SYSERR);
	}
	
	pptr->plbefore[lock] = 1;
	
	if (lptr->count <= 0) {
		if(type == READ){
			pptr->pstate = PRWAIT; //process waits
			pptr->lock_id = lock;
			inqueue(currpid, lptr->lhd, priority, WRITE);
			if(getpri(pptr) > lptr->lprio)
				lptr->lprio = getpri(pptr);
			
			for(i=0; i< NPROC; i++){
				if(lptr->lacquired[i])
						prio_lock_to_proc(&proctab[i]);                      
			}
			pptr->pwaitret = OK;
			resched();	
		}else{
			pptr->pstate = PRWAIT; //process waits
			pptr->lock_id = lock;
			inqueue(currpid, lptr->lhd, priority, WRITE);
			if(getpri(pptr) > lptr->lprio)
				lptr->lprio = getpri(pptr);
			for(i=0; i< NPROC; i++){
				if(lptr->lacquired[i])
						prio_lock_to_proc(&proctab[i]);                      
			}
			pptr->pwaitret = OK;
			resched();
		}	
		restore(ps);
		return pptr->pwaitret;
	}
	
	lptr->ltype = type;
	lptr->count--;
	if(type == READ)
		lptr->lreader = 1;	/* first reader */
	lptr->lacquired[currpid] = 1;
	pptr->plafter[lock] = 1;
	restore(ps);
	return(OK);
}

LOCAL int is_lockfree(int lock){
	struct lentry* lptr = &locktab[lock];
	int i;
	if(lptr->ltype == WRITE){
		for(i=0;i<NPROC; i++){
			if(lptr->lacquired[i])
				return 1;
		}		
	}
	return 0;

}	

LOCAL int is_needwait(int lock, int priority){
	struct lentry* lptr;

	if(isempty((lptr = &locktab[lock])->lhd))
		return 0;

	int prev = q[lptr->ltl].qprev;
	while(prev != lptr->lhd){
		if(q[prev].qltype == WRITE && q[prev].qkey >= priority)
		 	return 1;
		prev = q[prev].qprev;
	}
	return 0;
	
}

void prio_lock_to_proc(struct pentry * pptr){
	struct lentry *bestlptr = NULL;
	struct lentry *lptr;
	int numlocks = 0;
	int i;

	for(i =0; i < NLOCKS; i++){           
		lptr = &locktab[i];            
		if(pptr->plafter[i]){
			numlocks++;
			if(numlocks == 1 || (bestlptr != NULL && getpri(pptr) > bestlptr->lprio ) )
				bestlptr = lptr;
		}	

	}

	if(!numlocks)		/* all locks released */
		pptr->ipp = 0;	
	else{
	
		pptr->ipp = bestlptr->lprio;
		if(pptr->pprio > pptr->ipp)
			pptr->ipp = 0;
		
	}
	if(pptr->lock_id != -1){
		lptr = &locktab[pptr->lock_id];
		lptr->lprio = calculate_maxprio(lptr);

		for(i=0; i< NPROC; i++){
			if(lptr->lacquired[i])
				prio_lock_to_proc(&proctab[i]);
			
		}
	}
}

int calculate_maxprio(struct lentry *lptr){
	if(lptr == NULL)
		return -1;
	struct pentry *pptr;
	int max_schedprio = 0;
	int prev = q[lptr->ltl].qprev;
	if(nonempty(lptr->lhd)){
		while(prev != lptr->lhd){
			pptr = &proctab[prev];
			if(getpri(pptr) > max_schedprio)
				max_schedprio = getpri(pptr);
			prev = q[prev].qprev;
		}
	}
	return max_schedprio;
}

int inqueue(int proc, int head, int wprio, int ltype)
{
	int	next;			
	int	prev;
	for(
		next = q[head].qnext;
		q[next].qkey<wprio;
		next = q[next].qnext
	);
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = wprio;
	q[proc].qltype = ltype;
	q[proc].qlwstime = ctr1000;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	
	return(OK);
}