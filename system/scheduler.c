/* scheduler.c - scheduler, userinsert */
#include <xinu.h>
#include <stdlib.h>

qid16 userlist;
local long random_at_most(long);
/*------------------------------------------------------------------------
 * scheduler - Schedules processes using a lottery scheduling algorithm
 *------------------------------------------------------------------------
 */
pid32 scheduler(void)
{
    qid16   q;              /* ID of ready queue*/
	qid16	curr;			/* Runs through items in a queue*/
	uint32 ticketcount;
	uint32 usercount;
	uint32 counter;
	long ticketchoice;

	ticketcount = 0;
	usercount = 0;
	counter = 0;

	q = userlist;
	curr = firstid(q);
	while (curr != queuetail(q)) {
		ticketcount += queuetab[curr].qkey;
		usercount++;
		curr = queuetab[curr].qnext;
	}
	if (usercount == 0 || ticketcount == 0) {
		return NULLPROC;
	}
	if (usercount == 1) {
		return firstid(q);
	} else {
		curr = firstid(q);
		ticketchoice = random_at_most((long)ticketcount);
		while (curr != lastid(q)) {
			counter += queuetab[curr].qkey;
			if (counter > ticketchoice)
				break;
			curr = queuetab[curr].qnext;
		}
		return curr;
	}
}

// Taken from stackoverflow link from lottery scheduling description
// Assumes 0 <= max <= RAND_MAX
// Returns in the interval [0, max)
local long random_at_most(long max) {
  unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned long) max,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  long x;
  do {
   x = rand();
  }
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x/bin_size;
}

/*------------------------------------------------------------------------
 *  userinsert  -  Insert a process into user queue, sorted
 *------------------------------------------------------------------------
 */
status	userinsert(
	  pid32		pid		/* ID of process to insert	*/
	)
{
	qid16	curr;			/* Runs through items in a queue*/
	qid16	prev;			/* Holds previous node index	*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	uprio;
	prptr = &proctab[pid];
	uprio = prptr->uprio;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	curr = firstid(userlist);
	while (queuetab[curr].qkey > uprio) {
		curr = queuetab[curr].qnext;
	}
	while (queuetab[curr].qkey == uprio && curr < pid) {
		curr = queuetab[curr].qnext;
	}

	/* Insert process between curr node and previous node */

	prev = queuetab[curr].qprev;	/* Get index of previous node	*/
	queuetab[pid].qnext = curr;
	queuetab[pid].qprev = prev;
	queuetab[pid].qkey = uprio;
	queuetab[prev].qnext = pid;
	queuetab[curr].qprev = pid;
	return OK;
}