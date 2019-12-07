/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>


/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	struct 	lentry	*lptr;
	int	i=0;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	} 
	pptr->pprio = newprio;

	if(pptr->ipp != 0)
		pptr->ipp = newprio;
	

	if(pptr->pstate == PRWAIT){
		lptr = &locktab[pptr->lock_id];
        lptr->lprio = calculate_maxprio(lptr);
		while(i<NPROC){
			if(lptr->lacquired[i])
				prio_lock_to_proc(&proctab[i]);
			i++;
		}
		
	}
	restore(ps);
	return(newprio);
}
