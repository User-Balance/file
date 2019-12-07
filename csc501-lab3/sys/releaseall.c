/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*-----------------------------------------------------------
 * releaseall  --  release lock(s) for the calling  process
 *-----------------------------------------------------------
 */
SYSCALL releaseall (int numlocks, int args)
{
	STATWORD ps;    
	struct lentry *lptr;
	struct pentry *pptr = &proctab[currpid];
	int *lk;
	int i, lock;

	disable(ps);
	
	for(i=0; i< numlocks; i++){
		lk = &args + i;
		lock = *lk;

		if (isbadlock(lock) || (lptr= &locktab[lock])->lstate==LFREE || !(lptr->lacquired[currpid]) ) {
			return SYSERR;
		}
		else{	
			if(lptr->ltype == READ)
				lptr->lreader--;
			
			lptr->lacquired[currpid] = 0;
			pptr->plafter[lock] = 0;
			
			if(lptr->ltype == WRITE || ((lptr->ltype == READ) && (lptr->lreader == 0)) ){
				
				lptr->count++;				
				
				if( nonempty(lptr->lhd) ){	
					int br = search_lock(lock,READ);
					int bw = search_lock(lock,WRITE);
					
					if(q[br].qkey > q[bw].qkey){
						while(br > -1 && (q[br].qkey > q[bw].qkey) ){
							lptr->lacquired[br] = 1;
							proctab[br].plafter[lock] = 1;

							lptr->lreader++;
							lptr->ltype = READ;
							
							ready(dequeue(br), RESCHNO);
							lptr->lprio = calculate_maxprio(lptr);

							proctab[br].lock_id = -1; 	
							br = search_lock(lock,READ);
						}
						
					}
					else if(q[br].qkey < q[bw].qkey){
                                       		lptr->lacquired[bw] = 1;  
						proctab[bw].plafter[lock] = 1;
                                     		lptr->ltype = WRITE;
						
						ready(dequeue(bw), RESCHNO);         
					        lptr->lprio = calculate_maxprio(lptr);
	
                                                proctab[bw].lock_id = -1;         
					}
					else if((q[br].qlwstime >= q[bw].qlwstime) && (q[br].qlwstime - q[bw].qlwstime <= 600 )) {	
						lptr->lacquired[bw] = 1;
						proctab[bw].plafter[lock] = 1;
                                                lptr->ltype = WRITE;
						
						ready(dequeue(bw), RESCHNO);
					        lptr->lprio = calculate_maxprio(lptr);
	
		                                proctab[bw].lock_id = -1;         
					}	
					else{							
						lptr->lacquired[br] = 1;
						proctab[br].plafter[lock] = 1;
                                                lptr->lreader++;
                                                lptr->ltype = READ;
						
						ready(dequeue(br), RESCHNO);
                                                lptr->lprio = calculate_maxprio(lptr);

                                                proctab[br].lock_id = -1;       

					}					
					resched();
				}
			}
		}
	}

	prio_lock_to_proc(pptr);
	restore(ps);
	return OK;
}

int search_lock(int lock, int ltype){
	struct lentry *lptr = &locktab[lock];
	if(isempty(lptr->lhd))
		return -1;
	int prev = q[lptr->ltl].qprev;
	while(prev != lptr->lhd){
		if(q[prev].qltype == ltype)
			return prev;
		prev = q[prev].qprev;
	}
	return -1;	
}
