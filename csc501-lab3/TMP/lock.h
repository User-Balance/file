#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define	NLOCKS		50	/* number of locks, if not defined		*/
#endif

#define ALL		 -1
#define READ		 10
#define WRITE		 11

#define	LFREE	0			
#define	LUSED	1		
#define	isbadlock(x)	(x<0 || x>=NLOCKS)

struct	lentry	{		/* lock table entry				*/
	char	lstate;		/* the state LFREE or LUSED			*/
	int 	ltype;		/* lock acquired by R or W			*/
	int 	lprio;		/* max priority among the processes waiting for this lock 	*/
	int		lacquired[50];	/* which processes have acquired this lock	*/
	int		ldeleted;	/* 1 if the lock has been deleted before, 0 otherwise		*/
	int		lreader;	/* number of active readers with this lock	*/
	int		count;	/* count for this lock (initialized to 1)	*/
	int		lhd;		/* q index of head of list			*/
	int		ltl;		/* q index of tail of list			*/
};
struct  lentry  locktab[NLOCKS]; 	                        
int	nextlock; 


void linit (void);
SYSCALL lcreate (void);
SYSCALL ldelete (int ldes);
SYSCALL lock (int ldes, int type, int priority);
void prio_lock_to_proc(struct pentry *);
int calculate_maxprio(struct lentry *);
int inqueue(int, int, int, int);

#endif

