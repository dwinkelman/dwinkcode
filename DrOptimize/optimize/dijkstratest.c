/******************************************************************************
 * Testing module for the Dijkstra path-finding algorithm
 */

#include <stdio.h>

#include "dijkstra.h"

int main() {
	/*
	Connection cons[] = {
		{2, 3, 5.0f},
		{1, 4, 4.0f},
		{0, 2, 3.0f},
		{2, 1, 4.0f},
		{4, 3, 3.0f},
		{1, 3, 6.0f},
		{0, 3, 1.0f}
	};
	*/
	Connection cons[] = {
		{0, 1, 7.0},
		{0, 2, 2.0},
		{0, 3, 3.0},
		{1, 2, 3.0},
		{1, 4, 4.0},
		{2, 4, 4.0},
		{2, 8, 1.0},
		{3, 12, 2.0},
		{4, 6, 5.0},
		{5, 7, 2.0},
		{5, 11, 5.0},
		{6, 8, 3.0},
		{7, 8, 2.0},
		{9, 10, 6.0},
		{9, 11, 4.0},
		{9, 12, 4.0},
		{10, 11, 4.0},
		{10, 12, 4.0}
	};
	Connection * twoway = Connection_TwoWay(cons, 18);
	DijkstraOutput data = Dijkstra_ShortestPath(twoway, 36, 0, 5);
	printf("Path [%i Nodes, %f Cost, { ", data.n_elements, data.total_cost);
	for (unsigned int i = 0; i < data.n_elements; i++) {
		printf("%i", data.path[i]);
	}
	printf("}]\n");

	free(twoway);
	free(data.path);

	return 0;
}