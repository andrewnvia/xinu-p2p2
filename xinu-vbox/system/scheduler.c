/* scheduler.c - scheduler, userinsert, boostprio */
#include <xinu.h>
#include <stdlib.h>

qid16 userlist[UPRIORITY_QUEUES];
/*------------------------------------------------------------------------
 * scheduler - Schedules processes using a MLFQ scheduling algorithm
 *------------------------------------------------------------------------
 */
pid32 scheduler(void)
{
	int i;
	int prio = MAX_UPRIO;
	pid32 pid;
	for (i = 0; i < UPRIORITY_QUEUES; i++) {
		if(isempty(userlist[prio])) {
			prio--;
			continue;
		}
		pid = dequeue(userlist[prio]);
		return pid;
	}
	return NULLPROC;
}

/*------------------------------------------------------------------------
 *  userinsert  -  Insert a process into user queue, sorted
 *------------------------------------------------------------------------
 */
status	userinsert(
	  pid32		pid		/* ID of process to insert	*/
	)
{
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	uprio;
	prptr = &proctab[pid];
	uprio = prptr->uprio;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	if (uprio == 0) {
		/* Ignore allotment, already lowest priority	*/
		enqueue(pid, userlist[0]);
		return OK;
	}
	if (prptr->prallotment >= (TIME_ALLOTMENT << (MAX_UPRIO - uprio))) {
		prptr->uprio--;
		prptr->prallotment = 0;
		prptr->downgrades++;
	}
	enqueue(pid, userlist[prptr->uprio]);
	return OK;
}

/*------------------------------------------------------------------------
 *  boostprio  -  Increase priority of all user processes
 *------------------------------------------------------------------------
 */
status	boostprio(void) {
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int i;
	int prio;
	qid16	tail, prev, start, end;		/* Tail & previous node indexes	*/

	prptr = &proctab[currpid];
	if (prptr->prprio == USERPROC) {
		enqueue(currpid, userlist[prptr->uprio]);
		prptr->prprio = BOOSTED;
	}

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate != PR_FREE
				&& (prptr->prprio == USERPROC 
					|| prptr->prprio == BOOSTED)) {
			prptr->uprio = MAX_UPRIO;
			prptr->prallotment = 0;
			prptr->upgrades++;
		}
	}

	prio = MAX_UPRIO - 1;
	for (i = 0; i < MAX_UPRIO; i++) {
		if(isempty(userlist[prio])) {
			prio--;
			continue;
		}
		tail = queuetail(userlist[MAX_UPRIO]);
		prev = queuetab[tail].qprev;

		start = firstid(userlist[prio]);
		end = lastid(userlist[prio]);
		queuetab[end].qnext  = tail;	/* Insert just before tail node	*/
		queuetab[start].qprev  = prev;
		queuetab[prev].qnext = start;
		queuetab[tail].qprev = end;
		queuetab[queuehead(userlist[prio])].qnext = queuetail(userlist[prio]);
		queuetab[queuetail(userlist[prio])].qprev = queuehead(userlist[prio]);
		prio--;
	}
	

	resched();

	return OK;
}