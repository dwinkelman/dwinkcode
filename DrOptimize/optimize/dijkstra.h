/******************************************************************************
 * Header for the Dijkstra implementation.
 */

#pragma once

#include "priorityqueue.h"

typedef struct Node {
	unsigned int id;
	unsigned int best_id;
	float min_cost_from_start;
	unsigned int visited : 1;
	unsigned int status : 7;
} Node;

typedef struct Connection {
	unsigned int start, end;
	float cost;
} Connection;

#define DIJKSTRA_SUCCESS 0
#define DIJKSTRA_ERROR_NO_CONNECTIONS -1
#define DIJKSTRA_ERROR_NEGATIVE_COSTS -2
#define DIJKSTRA_ERROR_MISSING_NODE_IDS -3
#define DIJKSTRA_ERROR_INVALID_START -4
#define DIJKSTRA_ERROR_INVALID_END -5
#define DIJKSTRA_ERROR_START_AND_END_NOT_CONNECTED -6

typedef struct DijkstraOutput {
	unsigned int * path;
	unsigned int n_elements;
	float total_cost;
	unsigned int error;
} DijkstraOutput;

Node Node_Init(const unsigned int id);
Connection Connection_Init(const unsigned int start, const unsigned int end, const float cost);

Connection * Connection_TwoWay(const Connection * const connections, const unsigned int n_connections);

DijkstraOutput Dijkstra_ShortestPath(const Connection * const connections, const unsigned int n_connections,
	const unsigned int start, const unsigned int end);