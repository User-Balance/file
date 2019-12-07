#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();


SYSCALL lcreate(void)
{
	STATWORD ps;    
	int i, lock;

	disable(ps);
	if ( (lock=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}	
	restore(ps);
	return(lock);
}

LOCAL int newlock()
{
	int	lock;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		lock=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (locktab[lock].lstate==LFREE) {

			if(locktab[lock].ldeleted != 1)
				locktab[lock].ldeleted = 0;
			locktab[lock].lstate = LUSED;
			locktab[lock].count = 1;
			locktab[lock].lreader = 0;
			locktab[lock].lprio = 0;
			for(i=0; i<NPROC; i++)
				locktab[lock].lacquired[i] = 0;
			return(lock);
		}
	}
	return(SYSERR);
}
