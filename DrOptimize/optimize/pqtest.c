/******************************************************************************
 * Testing module for the Priority Queue.
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "priorityqueue.h"

int IntCompare(int left, int right) {
	return left - right;
}

int FloatCompare(float left, float right) {
	return left - right;
}

void MyFree(Pair * pair) {
	//if (pair->data) free(pair->data);
	free(pair);
}

int main() {
	PriorityQueue queue = PriorityQueue_New(IntCompare, FloatCompare, MyFree);
	for (int i = 0; i < 97 * 36; i += 36) {
		PriorityQueue_Insert(&queue, (i % 97), (float)(i % 5));
	}
	for (int i = 0; i < 97; i += 5) {
		PriorityQueue_Remove(&queue, i);
	}
	PriorityQueue_SetPriority(&queue, 44, 7.0);
	Data_t data;
	Priority_t priority;
	PriorityQueue_PopMax(&queue, &data, &priority);
	Pair * pairs = PriorityQueue_SerializeByPriority(&queue);
	for(int i = 0; i < queue.n_data; i++){
		printf("%i: %.2f\n", pairs[i].data, pairs[i].priority);
	}
	PriorityQueue_Free(&queue);

	return 0;
}