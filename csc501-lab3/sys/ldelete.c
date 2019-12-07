/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  * ldelete  --  delete a lock by releasing its table entry
 *   *------------------------------------------------------------------------
 *    */
SYSCALL ldelete(int lock)
{
	STATWORD ps;    
	int	pid, i;
	struct	lentry	*lptr;
	
	disable(ps);
	if (isbadlock(lock) || locktab[lock].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locktab[lock];
	lptr->lstate = LFREE;
	lptr->ldeleted = 1;	

	for(i=0; i< NPROC; i++){
		if(lptr->lacquired[i]){
			lptr->lacquired[i] = 0;
			proctab[i].plafter[lock] = DELETED; 
		}
	}
	if (nonempty(lptr->lhd)) {
		while( (pid=getfirst(lptr->lhd)) != EMPTY)
		  {
		    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO); 
		  }
		resched();
	}
	restore(ps);
	return(OK);
}
