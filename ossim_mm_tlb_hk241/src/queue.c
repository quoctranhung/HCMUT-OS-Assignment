 #include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
	if (q->size == MAX_QUEUE_SIZE) return;
	q->proc[q->size] = proc;
	q->size++;
        
}

struct pcb_t * dequeue(struct queue_t * q) {
	#ifdef MLQ_SCHED
	// Return process at index = 0;
	if (empty(q)) return NULL;
	struct pcb_t * proc = q->proc[0];
	for (int i = 0; i < q->size - 1; i++) {
		q->proc[i] = q->proc[i + 1];
	}
	q->size--;
	q->slot--;
	return proc;
	#else
	/* TODO: return a pcb whose prioprity is the highest
    * in the queue [q] and remember to remove it from q
    * */
	if (empty(q)) return NULL;
	int min_priority = q->proc[0]->priority;
	int rt_index = 0;
	for (int i = 1; i < q->size; i++) {
		if (q->proc[i]->priority < min_priority) {
			min_priority = q->proc[i]->priority;
			rt_index = i;
		}
	}
	for (int i = rt_index; i < q->size - 1; i++) {
		q->proc[i] = q->proc[i + 1];
	}
	q->size--;
	return q->proc[rt_index];
	#endif
}

