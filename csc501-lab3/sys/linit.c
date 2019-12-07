#include <kernel.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>	
#include <proc.h>	



void linit(){
	int i;
	struct	lentry	*lptr;
	struct  pentry  *pptr = &proctab[NULLPROC];
	nextlock = NLOCKS -1;
	for (i=0 ; i<NLOCKS ; i++) {
			(lptr = &locktab[i])->lstate = LFREE;
			lptr->ltl = 1 + (lptr->lhd = newqueue());

			pptr->plafter[i]= 0;	
			pptr->plbefore[i]= 0;
		}
}
