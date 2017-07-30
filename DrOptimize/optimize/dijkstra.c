/******************************************************************************
 * Source code for the Dijkstra algorithm. Depends on Priority Queue.
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dijkstra.h"

Node Node_Init(const unsigned int id) {
	Node node;
	node.id = id;
	node.best_id = id;
	node.min_cost_from_start = INFINITY;
	node.visited = 0;
	node.status = 1;
	return node;
}

int float_compare(float left, float right) {
	if (left > right) return 1;
	else if (left < right) return -1;
	else return 0;
}

int int_compare(int left, int right) {
	return left - right;
}

Connection Connection_Init(const unsigned int start, const unsigned int end, const float cost) {
	Connection con;
	con.start = start;
	con.end = end;
	con.cost = cost;
	return con;
}

static int Connection_Sort(const void * left, const void * right) {
	Connection * n1 = (Connection *)left;
	Connection * n2 = (Connection *)right;
	int start_comp = (signed int)n1->start - (signed int)n2->start;
	if (start_comp == 0) {
		return (signed int)n1->end - (signed int)n2->end;
	}
	return start_comp;
}

Connection * Connection_TwoWay(const Connection * const connections, const unsigned int n_connections) {
	Connection * output = (Connection *)malloc(n_connections * 2 * sizeof(Connection));
	memcpy(output, connections, n_connections * sizeof(Connection));
	for (unsigned int i = 0; i < n_connections; i++) {
		output[n_connections + i].start = connections[i].end;
		output[n_connections + i].end = connections[i].start;
		output[n_connections + i].cost = connections[i].cost;
	}
	return output;
}


static void FreePQNode(Pair * pair) {
	return;
}

/******************************************************************************
 * Use Dijkstra's Shortest Path Algorithm to find the shortest path through a
 * network defined as a list of connections, each with a cost.
 *
 * NOTES:
 *	 -	Node ids should be consecutive, starting at 0. An error will be thrown
 *		if they are not.
 *	 -	An error will be thrown if n_connections is not greater than 0.
 *	 -	Connections cannot have negative costs.
 *	 -	An error will be thrown if start or end does not exist within the
 *		connections.
 *	 -	This algorithm checks for connectivity of all nodes within the network,
 *		starting at start. If end is not included in the set of nodes connected
 *		to start, an error will be thrown.
 */
DijkstraOutput Dijkstra_ShortestPath(const Connection * const connections, const unsigned int n_connections,
	const unsigned int start, const unsigned int end) {

	DijkstraOutput output;
	output.error = DIJKSTRA_SUCCESS;
	output.n_elements = 0;
	output.path = NULL;
	output.total_cost = INFINITY;

	// Check that there are 1 or more connections
	if (n_connections == 0) {
		output.error = DIJKSTRA_ERROR_NO_CONNECTIONS;
		return output;
	}

	// Find the maximum node id
	unsigned int max_id = 0;
	for (unsigned int i = 0; i < n_connections; i++) {
		if (connections[i].start > max_id) max_id = connections[i].start;
		if (connections[i].end > max_id) max_id = connections[i].end;

		// There should not be negative costs
		if (connections[i].cost < 0) {
			output.error = DIJKSTRA_ERROR_NEGATIVE_COSTS;
			return output;
		}
	}
	unsigned int n_nodes = max_id + 1;

	if (start > max_id) {
		output.error = DIJKSTRA_ERROR_INVALID_START;
		return output;
	}
	else if (end > max_id) {
		output.error = DIJKSTRA_ERROR_INVALID_END;
		return output;
	}

	// Allocate nodes
	Node * nodes = (Node *)malloc(n_nodes * sizeof(Node));
	memset(nodes, 0, n_nodes * sizeof(Node));
	for (unsigned int i = 0; i < n_connections; i++) {
		nodes[connections[i].start] = Node_Init(connections[i].start);
		nodes[connections[i].end] = Node_Init(connections[i].end);
	}

	// Check that all nodes exist
	for (unsigned int i = 0; i < n_nodes; i++) {
		if (nodes[i].status != 1) {
			output.error = DIJKSTRA_ERROR_MISSING_NODE_IDS;
			return output;
		}
	}

	// Sort connections by start then end
	Connection * sorted_cons = (Connection *)malloc(n_connections * sizeof(Connection));
	memcpy(sorted_cons, connections, n_connections * sizeof(Connection));
	qsort(sorted_cons, n_connections, sizeof(Connection), Connection_Sort);

	// Get index to first item for each start
	unsigned int * offset = (unsigned int *)malloc((n_nodes + 1) * sizeof(unsigned int));
	memset(offset, 0, (n_nodes + 1) * sizeof(unsigned int));
	int counter = 0;
	for (unsigned int i = 0; i < n_nodes; i++) {
		while (sorted_cons[counter].start < i) {
			counter++;
		}
		offset[i] = counter;
	}
	offset[0] = 0;
	offset[n_nodes] = n_connections;

	// Create node queue using binary selection
	PriorityQueue queue = PriorityQueue_New(int_compare, float_compare, FreePQNode);
	int interval = 1 << (32 - __builtin_clz(n_nodes - 1));
	while (interval > 1) {
		for (int i = (interval >> 1) - 1; i < n_nodes; i += interval) {
			PriorityQueue_Insert(&queue, i, INFINITY);
		}
		interval >>= 1;
	}
	PriorityQueue_SetPriority(&queue, start, 0.0f);
	nodes[start].min_cost_from_start = 0;

	// Iterate through nodes in the queue
	while (queue.n_data) {
		// Get closest node to start that is unexplored
		int current_index;
		float priority;
		PriorityQueue_PopMin(&queue, &current_index, &priority);
		Node * current = nodes + current_index;

		// If priority > INFINITY, there are no more nodes connected to start's
		// network, and the end can never be reached
		if (priority == INFINITY) {
			output.error = DIJKSTRA_ERROR_START_AND_END_NOT_CONNECTED;
			return output;
		}

		// Iterate through each child that has not been visited yet
		unsigned int first_connection = offset[current->id];
		unsigned int last_connection = offset[current->id + 1];
		for (unsigned int i = first_connection; i < last_connection; i++) {
			Connection * con = sorted_cons + i;
			Node * dest = nodes + con->end;
			if (!dest->visited) {
				if (current->min_cost_from_start + con->cost < dest->min_cost_from_start) {
					
					dest->min_cost_from_start = current->min_cost_from_start + con->cost;
					dest->best_id = current->id;
					PriorityQueue_SetPriority(&queue, dest->id, dest->min_cost_from_start);
				}
			}
		}

		current->visited = 1;

		if (current->id == end) break;
	}

	// Generate tree
	unsigned int n_elements = 1;
	unsigned int current = end;
	while (current != start) {
		current = nodes[current].best_id;
		n_elements++;
	}

	// Assemble output
	output.total_cost = nodes[end].min_cost_from_start;
	output.path = (unsigned int *)malloc(n_elements * sizeof(unsigned int));
	current = end;
	unsigned int i = n_elements - 1;
	while (current != start) {
		output.path[i--] = current;
		current = nodes[current].best_id;
	}
	output.path[0] = start;
	output.n_elements = n_elements;

	// Free memory
	free(nodes);
	free(sorted_cons);
	free(offset);
	
	return output;
}