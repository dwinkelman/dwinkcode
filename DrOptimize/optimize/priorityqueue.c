/******************************************************************************
 * Implementation of the Priority Queue.
 */

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "priorityqueue.h"

/******************************************************************************
 * Errors
 */
PQError PQ_SUCCESS = 0;
PQError PQ_ERROR = -1;
PQError PQ_ERROR_KEY_DOES_NOT_EXIST = -2;
PQError PQ_ERROR_KEY_ALREADY_EXISTS = -3;
PQError PQ_ERROR_EMPTY_QUEUE = -4;

/******************************************************************************
 * Initialization
 */
PriorityQueue PriorityQueue_New(DataCompareFunc data_compare, PriorityCompareFunc priority_compare, FreePairFunc free_pair) {
	PriorityQueue queue;

	queue.data_tree = (DataNode *)malloc(sizeof(DataNode));
	queue.data_tree->left = queue.data_tree->right = NULL;
	queue.data_tree->pair = (Pair *)malloc(sizeof(Pair));
	queue.data_tree->pair->data = -1;
	queue.data_tree->pair->priority = INFINITY;

	queue.priority_tree = (PriorityNode *)malloc(sizeof(PriorityNode));
	queue.priority_tree->left = queue.priority_tree->right = NULL;
	queue.priority_tree->pair = queue.data_tree->pair;

	queue.data_compare = data_compare;
	queue.priority_compare = priority_compare;
	queue.free_pair = free_pair;

	queue.n_data = 0;

	return queue;
}

/******************************************************************************
 * Memory Free
 */
static void PriorityQueue_FreeDataNode(PriorityQueue * const queue, DataNode * node) {
	if (node->left) PriorityQueue_FreeDataNode(queue, node->left);
	if (node->right) PriorityQueue_FreeDataNode(queue, node->right);
	if (node->pair) queue->free_pair(node->pair);
	free(node);
}

static void PriorityQueue_FreePriorityNode(PriorityQueue * const queue, PriorityNode * node) {
	if (node->left) PriorityQueue_FreePriorityNode(queue, node->left);
	if (node->right) PriorityQueue_FreePriorityNode(queue, node->right);
	// Pair is already freed in FreeDataNode
	free(node);
}

void PriorityQueue_Free(PriorityQueue * const queue) {
	if (queue->data_tree) PriorityQueue_FreeDataNode(queue, queue->data_tree);
	if (queue->priority_tree) PriorityQueue_FreePriorityNode(queue, queue->priority_tree);
	queue->n_data = 0;
}

/******************************************************************************
 * Insert
 */
PQError PriorityQueue_Insert(PriorityQueue * const queue, const Data_t data, const Priority_t priority) {
	// Forward-declare new pair
	Pair * pair;

	// Insert pair into data tree
	// Needs to be done first to check for duplicates
	// If the inserted data is a duplicate, an error is returned and nothing is
	// inserted.
	DataNode * dnode = queue->data_tree;
	while (1) {
		int comp = queue->data_compare(data, dnode->pair->data);
		// Return an error if the data is a duplicate
		if (comp == 0) return PQ_ERROR_KEY_ALREADY_EXISTS;
		if (comp > 0) {
			// Data to insert is greater than current
			if (dnode->right) dnode = dnode->right;
			else {
				dnode->right = (DataNode *)malloc(sizeof(DataNode));
				dnode = dnode->right;
				break;
			}
		}
		else {
			// Data to insert is less than current
			if (dnode->left) dnode = dnode->left;
			else {
				dnode->left = (DataNode *)malloc(sizeof(DataNode));
				dnode = dnode->left;
				break;
			}
		}
	}

	// Allocate the new pair
	pair = (Pair *)malloc(sizeof(Pair));
	pair->data = data;
	pair->priority = priority;

	// Assign members to the data node
	dnode->left = NULL;
	dnode->right = NULL;
	dnode->pair = pair;

	// Insert pair into priority tree
	PriorityNode * pnode = queue->priority_tree;
	while (1) {
		int comp = queue->priority_compare(priority, pnode->pair->priority);
		if (comp > 0) {
			// Priority to insert is greater than current
			if (pnode->right) pnode = pnode->right;
			else {
				pnode->right = (PriorityNode *)malloc(sizeof(PriorityNode));
				pnode = pnode->right;
				break;
			}
		}
		else {
			// Priority to insert is less than or equal to the current
			if (pnode->left) pnode = pnode->left;
			else {
				pnode->left = (PriorityNode *)malloc(sizeof(PriorityNode));
				pnode = pnode->left;
				break;
			}
		}
	}

	// Assign members to the priority node
	pnode->left = NULL;
	pnode->right = NULL;
	pnode->pair = pair;

	queue->n_data++;
	return PQ_SUCCESS;
}

/******************************************************************************
 * Remove
 */
PQError PriorityQueue_Remove(PriorityQueue * const queue, const Data_t data) {
	// Check for existance in data tree.
	// If it exists, remove from data tree.
	DataNode * dnode = queue->data_tree;
	DataNode * dparent = NULL;
	while (1) {
		int comp = queue->data_compare(data, dnode->pair->data);
		if (comp == 0) break;
		if (comp > 0) {
			// Data to remove is greater than current
			if (dnode->right) {
				dparent = dnode;
				dnode = dnode->right;
			}
			else return PQ_ERROR_KEY_DOES_NOT_EXIST;
		}
		else {
			// Data to remove is less than current
			if (dnode->left) {
				dparent = dnode;
				dnode = dnode->left;
			}
			else return PQ_ERROR_KEY_DOES_NOT_EXIST;
		}
	}
	if (!dparent) return PQ_ERROR_KEY_DOES_NOT_EXIST;
	DataNode * dleft = dnode->left;
	DataNode * dright = dnode->right;
	if (dleft) {
		if (dright) {
			// Need to append left child to parent
			if (dnode == dparent->left) dparent->left = dleft;
			else dparent->right = dleft;
			// Insert right child elsewhere in tree
			DataNode * diter = queue->data_tree;
			while (1) {
				int comp = queue->data_compare(dright->pair->data, diter->pair->data);
				// Since keys are unique, data cannot be anywhere inside of tree
				if (comp > 0) {
					// Data to insert is greater than current
					if (diter->right) diter = diter->right;
					else {
						diter->right = dright;
						break;
					}
				}
				else {
					// Data to insert is less than current
					if (diter->left) diter = diter->left;
					else {
						diter->left = dright;
						break;
					}
				}
			}
		}
		else {
			// Replace dnode with left child
			if (dnode == dparent->left) dparent->left = dleft;
			else dparent->right = dleft;
		}
	}
	else {
		if (dright) {
			// Replace dnode with right child
			if (dnode == dparent->left) dparent->left = dright;
			else dparent->right = dright;
		}
		else {
			// No children to replace with
			if (dnode == dparent->left) dparent->left = NULL;
			else dparent->right = NULL;
		}
	}

	// Remove from priority tree.
	Priority_t priority = dnode->pair->priority;
	PriorityNode * pnode = queue->priority_tree;
	PriorityNode * pparent = NULL;
	while (1) {
		int pcomp = queue->priority_compare(priority, pnode->pair->priority);
		if (pcomp == 0) {
			int dcomp = queue->data_compare(data, pnode->pair->data);
			if (dcomp == 0) break;
		}
		if (pcomp > 0) {
			// Priority to remove is greater than current
			if (pnode->right) {
				pparent = pnode;
				pnode = pnode->right;
			}
			else return PQ_ERROR_KEY_DOES_NOT_EXIST;
		}
		else {
			// Priority to remove is less than current
			if (pnode->left) {
				pparent = pnode;
				pnode = pnode->left;
			}
			else return PQ_ERROR_KEY_DOES_NOT_EXIST;
		}
	}
	if (!pparent) return PQ_ERROR_KEY_DOES_NOT_EXIST;
	PriorityNode * pleft = pnode->left;
	PriorityNode * pright = pnode->right;
	if (pleft) {
		if (pright) {
			// Need to append left child to parent
			if (pnode == pparent->left) pparent->left = pleft;
			else pparent->right = pleft;
			// Insert right child elsewhere in tree
			PriorityNode * piter = queue->priority_tree;
			while (1) {
				int comp = queue->priority_compare(pright->pair->priority, piter->pair->priority);
				if (comp > 0) {
					// Data to insert is greater than current
					if (piter->right) piter = piter->right;
					else {
						piter->right = pright;
						break;
					}
				}
				else {
					// Data to insert is less than or equal to current
					if (piter->left) piter = piter->left;
					else {
						piter->left = pright;
						break;
					}
				}
			}
		}
		else {
			// Replace dnode with left child
			if (pnode == pparent->left) pparent->left = pleft;
			else pparent->right = pleft;
		}
	}
	else {
		if (pright) {
			// Replace dnode with right child
			if (pnode == pparent->left) pparent->left = pright;
			else pparent->right = pright;
		}
		else {
			// No children to replace with
			if (pnode == pparent->left) pparent->left = NULL;
			else pparent->right = NULL;
		}
	}

	// Free removed nodes
	queue->free_pair(dnode->pair);
	free(dnode);
	free(pnode);
	queue->n_data--;

	return PQ_SUCCESS;
}

/******************************************************************************
 * Getting and Modifying Priority
 */
PQError PriorityQueue_GetPriority(const PriorityQueue * const queue, const Data_t data, Priority_t * const out_priority) {
	DataNode * node = queue->data_tree;
	while (1) {
		int comp = queue->data_compare(data, node->pair->data);
		if (comp == 0) break;
		else if (comp > 0) {
			// Data to find is greater than current
			if (node->right) node = node->right;
			else return PQ_ERROR_KEY_DOES_NOT_EXIST;
		}
		else {
			// Data to find is less than current
			if (node->left) node = node->left;
			else return PQ_ERROR_KEY_DOES_NOT_EXIST;
		}
	}
	if (node && node != queue->data_tree) *out_priority = node->pair->priority;
	else return PQ_ERROR_KEY_DOES_NOT_EXIST;
	return PQ_SUCCESS;
}

PQError PriorityQueue_SetPriority(PriorityQueue * const queue, const Data_t data, const Priority_t priority) {
	// Remove node from priority tree
	PQError err1 = PriorityQueue_Remove(queue, data);
	if (err1) return err1;

	// Re-insert node back into the tree
	PQError err2 = PriorityQueue_Insert(queue, data, priority);
	if (err2) return err2;

	return PQ_SUCCESS;
}

/******************************************************************************
 * Pop Next
 */
PQError PriorityQueue_PopMin(PriorityQueue * const queue, Data_t * const out_data, Priority_t * const out_priority) {
	// Set up top node
	if (queue->n_data == 0) return PQ_ERROR_EMPTY_QUEUE;
	PriorityNode * node = queue->priority_tree;
	if (!node->left) node = node->right;
	
	// Go to the left-most node
	while (node->left) {
		node = node->left;
	}

	// Get data
	*out_data = node->pair->data;
	*out_priority = node->pair->priority;

	// Remove node from tree
	PriorityQueue_Remove(queue, *out_data);

	return PQ_SUCCESS;
}

PQError PriorityQueue_PopMax(PriorityQueue * const queue, Data_t * const out_data, Priority_t * const out_priority) {
	// Set up top node
	if (queue->n_data == 0) return PQ_ERROR_EMPTY_QUEUE;
	PriorityNode * node = queue->priority_tree;
	if (!node->right) node = node->left;

	// Go to the left-most node
	while (node->right) {
		node = node->right;
	}

	// Get data
	*out_data = node->pair->data;
	*out_priority = node->pair->priority;

	// Remove node from tree
	PriorityQueue_Remove(queue, *out_data);

	return PQ_SUCCESS;
}

/******************************************************************************
 * Serialization
 */
static int PriorityQueue_SerializationByPriorityWalker(const PriorityNode * const node, Pair * output, int index) {
	// Left side
	if (node->left) {
		index = PriorityQueue_SerializationByPriorityWalker(node->left, output, index);
	}
	// This (center)
	output[index].data = node->pair->data;
	output[index].priority = node->pair->priority;
	index++;
	// Right side
	if (node->right) {
		index = PriorityQueue_SerializationByPriorityWalker(node->right, output, index);
	}
	return index;
}

Pair * PriorityQueue_SerializeByPriority(PriorityQueue * const queue) {
	Pair * output = (Pair *)malloc(sizeof(Pair) * queue->n_data);
	int total = 0;
	if (queue->priority_tree->left) {
		total = PriorityQueue_SerializationByPriorityWalker(queue->priority_tree->left, output, total);
	}
	if (queue->priority_tree->right) {
		total = PriorityQueue_SerializationByPriorityWalker(queue->priority_tree->right, output, total);
	}
	if (total == queue->n_data) printf("We have all the data.\n");
	else printf("We do NOT have all the data.\n");
	return output;
}

static int PriorityQueue_SerializationByDataWalker(const DataNode * const node, Pair * output, int index) {
	// Left side
	if (node->left) {
		index = PriorityQueue_SerializationByDataWalker(node->left, output, index);
	}
	// This (center)
	output[index].data = node->pair->data;
	output[index].priority = node->pair->priority;
	index++;
	// Right side
	if (node->right) {
		index = PriorityQueue_SerializationByDataWalker(node->right, output, index);
	}
	return index;
}

Pair * PriorityQueue_SerializeByData(PriorityQueue * const queue) {
	Pair * output = (Pair *)malloc(sizeof(Pair) * queue->n_data);
	int total = 0;
	if (queue->data_tree->left) {
		total = PriorityQueue_SerializationByDataWalker(queue->data_tree->left, output, total);
	}
	if (queue->data_tree->right) {
		total = PriorityQueue_SerializationByDataWalker(queue->data_tree->right, output, total);
	}
	if (total == queue->n_data) printf("We have all the data.\n");
	else printf("We do NOT have all the data.\n");
	return output;
}

/******************************************************************************
 * Printing
 */
static void PriorityQueue_RecursePrintDataNode(const DataNode * const node, const char * pattern, const int depth) {
	for (int i = 0; i < depth; i++) printf("|  ");
	printf(pattern, node->pair->data, node->pair->priority);
	if (node->left) PriorityQueue_RecursePrintDataNode(node->left, pattern, depth + 1);
	if (node->right) PriorityQueue_RecursePrintDataNode(node->right, pattern, depth + 1);
}

void PriorityQueue_PrintTree(const PriorityQueue * const queue, const char * pattern) {
	PriorityQueue_RecursePrintDataNode(queue->data_tree, pattern, 0);
}

/******************************************************************************
 * Memory Heap Footprint
 */
static size_t PriorityQueue_RecurseDataNodeAllocation(const DataNode * const node) {
	size_t output = sizeof(DataNode);
	if (node->right) output += PriorityQueue_RecurseDataNodeAllocation(node->right);
	if (node->left) output += PriorityQueue_RecurseDataNodeAllocation(node->left);
	if (node->pair) output += sizeof(Pair);
	return output;
}

static size_t PriorityQueue_RecursePriorityNodeAllocation(const PriorityNode * const node) {
	size_t output = sizeof(PriorityNode);
	if (node->right) output += PriorityQueue_RecursePriorityNodeAllocation(node->right);
	if (node->left) output += PriorityQueue_RecursePriorityNodeAllocation(node->left);
	// Size of data taken care of in RecurseDataNodeAllocation
	return output;
}

size_t PriorityQueue_Allocation(const PriorityQueue * const queue) {
	size_t size = sizeof(PriorityQueue);
	size += PriorityQueue_RecurseDataNodeAllocation(queue->data_tree);
	size += PriorityQueue_RecursePriorityNodeAllocation(queue->priority_tree);
	return size;
}