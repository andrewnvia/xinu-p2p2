/* print_ready_list.c - print_ready_list */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  print_ready_list  -  Print entire ready list
 *------------------------------------------------------------------------
 */
syscall	print_ready_list(void)
{
    qid16   q;              /* ID of ready queue*/
	qid16	curr;			/* Runs through items in a queue*/
	qid16	prev;			/* Holds previous node index	*/

    q = readylist;

	if (isbadqid(q)) {
		return SYSERR;
	}

	curr = firstid(q);
	while (curr != queuetail(q)) {
        kprintf("%d\n", curr);
		curr = queuetab[curr].qnext;
	}

	return OK;
}
