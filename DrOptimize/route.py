'''
Uses the Dijsktra C extension to calculate optimal routes through the network.

Created on Jul 18, 2017

@author: Daniel
'''

import _dijkstra as dijkstra

def OptimizeDistance(paths, intersections, start, end):
    intersection_indexes = {intersections[i]: i for i in range(len(intersections))}
    int_cons = [(intersection_indexes[path.start_intersection], intersection_indexes[path.end_intersection], path.length)
        for path in paths]
    int_start = intersection_indexes[start]
    int_end = intersection_indexes[end]
    
    path, cost = dijkstra.Dijkstra(int_cons, int_start, int_end, True);
    path_nodes = [intersections[i].node for i in path]
    
    return path_nodes