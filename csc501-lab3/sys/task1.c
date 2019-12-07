#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

void sreader (char *msg, int sem)
{
        int     ret;

        kprintf ("  %s: to acquire semaphore\n", msg);
        wait (sem);
        kprintf ("  %s: acquired semaphore\n", msg);
        kprintf ("  %s: to release semaphore\n", msg);
        signal (sem);
}

void sreader2 (char *msg, int sem)
{
        int     ret;

        kprintf ("  %s: to acquire semaphore\n", msg);
        wait (sem);
        kprintf ("  %s: acquired semaphore\n", msg);

        int count = 0;
        int i;
        while (count++ < 50) {
                kprintf("    %s\n", msg);
                for (i = 0; i < 1000000; i++)
                        ;
        }

        kprintf ("  %s: to release semaphore\n", msg);
        signal (sem);
}

void swriter (char *msg, int sem)
{
        kprintf ("  %s: to acquire semaphore\n", msg);
        wait(sem);
        kprintf ("  %s: acquired semaphore, starts writing\n", msg);
        int count = 0;
        int i;
        while (count++ < 50) {
                kprintf("    %s\n", msg);
                for (i = 0; i < 1000000; i++)
                        ;
        }


        kprintf ("  %s: to release semaphore\n", msg);
        signal (sem);
}

void preader (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void preader2 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);

        int count = 0;
        int i;
        while (count++ < 50) {
                kprintf("    %s\n", msg);
                for (i = 0; i < 1000000; i++)
                        ;
        }

        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void pwriter (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, starts writing\n", msg);
        
	int count = 0;
        int i;
        while (count++ < 50) {
                kprintf("    %s\n", msg);
                for (i = 0; i < 1000000; i++)
                        ;
        }

        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}


void task1 ()
{
        int     lck,lck2,sem,sem2;
        int     rd1, rd2, sr1,sr2;
        int     wr1, sw1;

        kprintf("\nTask: comapare the basic priority inheritence with semaphore implementation\n");
        lck  = lcreate ();
	lck2  = lcreate ();
        sem = screate(1);
	sem2= screate(1);

        // Priority Inheritance
        kprintf("\nPriority inheritance implementation:\n");
        rd1 = create(preader, 2000, 30, "preader", 2, "lock reader A", lck);
        rd2 = create(preader2, 2000, 25, "preader2", 2, "lock reader B", lck2);
        wr1 = create(pwriter, 2000, 20, "pwriter", 2, "lock writer", lck);

        kprintf("-start lock writer. lock granted to write (prio 20)\n");
        resume(wr1);

        kprintf("-start lock reader A. reader A(prio 30) blocked on the lock\n");
        resume(rd1);

        kprintf("-start lock reader B. reader B(prio 25) should not get lock before reader A(prio 30) and should be blocked\n");
        resume (rd2);

        //Semaphore Implementation
        kprintf("\nSemaphore implementation: Priority inversion problem\n");
        sr1 = create(sreader, 2000, 30, "sreader", 2, "Sem reader A", sem);
        sr2 = create(sreader2, 2000, 25, "sreader2", 2, "Sem reader B", sem2);
        sw1 = create(swriter, 2000, 20, "swriter", 2, "Sem writer", sem);
        kprintf("-start SEM writer. lock granted to write (prio 20)\n");
        resume(sw1);

        kprintf("-start SEM reader A. reader A(prio 30) blocked on the lock\n");
        resume(sr1);

        kprintf("-start SEM reader B. reader B(prio 25) should not get lock before reader A(prio 30) and should be blocked\n");
        resume (sr2);
	

        sleep (8);

        


        sleep (8);
}