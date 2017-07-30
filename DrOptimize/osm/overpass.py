'''
Created on Jul 16, 2017

@author: Daniel
'''

import osm.util as util

class Path:
    
    def __init__(self, way, nodes):
        """
        @param way: Object from Open Street Map with data about this way (path)
        @param nodes: All the nodes gathered from the Open Street Map query
        @param intersections: A dict of all the intersections identified {id: Intersection}
        """
        self.tags = way.tags
        self.id = way.id
        self.nodes = nodes
        self.start = nodes[0]
        self.end = nodes[-1]
        self.length = self.Dist()
        
    def __repr__(self):
        return "<Path (%i Nodes); Start: %i, End: %i>" % (len(self.nodes), self.start.id, self.end.id)
    
    def Dist(self):
        total = 0.0
        for i in range(len(self.nodes) - 1):
            total += util.HaversineDist(self.nodes[i], self.nodes[i + 1])
        return total

class Intersection:
    
    def __init__(self, node):
        self.node = node
        self.paths = []
        
    def __repr__(self):
        output = "<Intersection (%i Connections) at %i; " % (len(self.paths), self.node.id)
        output += ", ".join([str(path.id) for path in self.paths]) + ">"
        return output
        
        
def BuildMap(nodes, ways):
    # convert arrays into dicts with ids as the key
    nodedict = {i.id: i for i in nodes}
    nodecounts = {k: 0 for k in nodedict.keys()}
    
    # find roads amongst ways
    roads = [way for way in ways if way.tags.get("highway")]
    
    # find nodes with more than one way reference and make into intersections
    for road in roads:
        if road.tags.get("highway"):
            for node in road.nodes:
                nodecounts[node.id] += 1
    intersection_ids = [nodeid for nodeid, count in nodecounts.items() if count > 1]
    intersections = []
    intersection_added = []
    
    # go through each road, split at intersections, make into a path,
    # and join to intersections
    paths = []
    for road in roads:
        points = [i for i in range(len(road.nodes)) if road.nodes[i].id in intersection_ids]
        if not 0 in points: points.insert(0, 0)
        if not len(road.nodes) - 1 in points: points.append(len(road.nodes) - 1)
        for i in range(len(points) - 1):
            path = Path(road, [road.nodes[j] for j in range(points[i], points[i + 1] + 1)])
            if not path.start.id in intersection_added:
                intersections.append(Intersection(path.start))
                intersection_added.append(path.start.id)
            if not path.end.id in intersection_added:
                intersections.append(Intersection(path.end))
                intersection_added.append(path.end.id)
            path.start_intersection = intersections[intersection_added.index(path.start.id)]
            path.start_intersection.paths.append(path)
            path.end_intersection = intersections[intersection_added.index(path.end.id)]
            path.end_intersection.paths.append(path)
            
            paths.append(path)
    
    return paths, intersections, nodedict

def Prune(paths, intersections, whitelist=[]):
    while 1:
        for inter in intersections:
            if len(inter.paths) == 1:
                if not inter.paths[0] in whitelist:
                    path = inter.paths[0]
                    if inter == path.start_intersection:
                        path.end_intersection.paths.remove(path)
                    else:
                        path.start_intersection.paths.remove(path)
                    paths.remove(path)
                    del path
                    inter.paths = []
                    
        prev_n_intersections = len(intersections)
        intersections[:] = [inter for inter in intersections if len(inter.paths)]
        if len(intersections) == prev_n_intersections:
            break
    
    return paths, intersections