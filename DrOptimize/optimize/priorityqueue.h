/******************************************************************************
 * Header file for the Priority Queue.
 */

#pragma once

#include <stdlib.h>

/******************************************************************************
 * PriorityQueue has two trees:
 *   1. A BST of Pairs sorted by priority. This is used for looking up the
 *		highest/lowest in the queue. This tree is composed of PriorityNode.
 *	 2. A BST of Pairs sorted by the value of the pointer to data (in other
 *		words the literal value of the pointer). This is used for looking up
 *		and changing the priority of an item. This tree is composed of
 *		DataNode.
 */

typedef const int PQError;
PQError PQ_SUCCESS;
PQError PQ_ERROR_KEY_DOES_NOT_EXIST;
PQError PQ_ERROR_KEY_ALREADY_EXISTS;
PQError PQ_ERROR_EMPTY_QUEUE;

/******************************************************************************
 * Change these typedefs to change the data type of the Priority Queue
 */
typedef int Data_t;
typedef float Priority_t;

typedef int(*DataCompareFunc)(Data_t left, Data_t right);
typedef int(*PriorityCompareFunc)(Priority_t left, Priority_t right);


typedef struct Pair {
	Data_t data;
	Priority_t priority;
} Pair;

typedef void(*FreePairFunc)(Pair * pair);

typedef struct PriorityNode {
	Pair * pair;
	struct PriorityNode * left, * right;
} PriorityNode;

typedef struct DataNode {
	Pair * pair;
	struct DataNode * left, * right;
} DataNode;

typedef struct PriorityQueue {
	PriorityNode * priority_tree;
	DataNode * data_tree;
	PriorityCompareFunc priority_compare;
	DataCompareFunc data_compare;
	FreePairFunc free_pair;
	unsigned int n_data;
} PriorityQueue;


PriorityQueue PriorityQueue_New(DataCompareFunc, PriorityCompareFunc, FreePairFunc);
void PriorityQueue_Free(PriorityQueue * const);
PQError PriorityQueue_Insert(PriorityQueue * const, const Data_t, const Priority_t);
PQError PriorityQueue_Remove(PriorityQueue * const, const Data_t);
PQError PriorityQueue_GetPriority(const PriorityQueue * const, const Data_t, Priority_t * const);
PQError PriorityQueue_SetPriority(PriorityQueue * const, const Data_t, const Priority_t);
PQError PriorityQueue_PopMin(PriorityQueue * const, Data_t * const, Priority_t * const);
PQError PriorityQueue_PopMax(PriorityQueue * const, Data_t * const, Priority_t * const);

Pair * PriorityQueue_SerializeByPriority(PriorityQueue * const);
Pair * PriorityQueue_SerializeByData(PriorityQueue * const);

void PriorityQueue_PrintTree(const PriorityQueue * const, const char * pattern);

inline unsigned int PriorityQueue_Length(const PriorityQueue * const queue) {
	return queue->n_data;
}
size_t PriorityQueue_Allocation(const PriorityQueue * const queue);