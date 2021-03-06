'''
Quick testing module for the program.

Created on Jul 14, 2017

@author: Daniel
'''

import overpy

import elevation as elevation
import overpass as overpass
import route as route
import visualize as visualize
import util as util

def OverpassAPITest():
    api = overpy.Overpass()
    myquery = """[out:json];(way(36.05, -80.41, 36.08, -80.38)["highway"]["name"];>;);out;"""
    results = api.query(myquery)
    return results;

if __name__ == "__main__":
    print "Getting results..."
    results = OverpassAPITest()
    print "Building map..."
    paths, intersections, nodedict = overpass.BuildMap(results.nodes, results.ways)
    print "Pruning..."
    overpass.Prune(paths, intersections)
    print len(intersections)
    
    n_samples = elevation.NumberElevationSamples(paths, intersections, nodedict)
    client = elevation.MakeGoogleMapsClient()
    print "Querying Google Maps Elevation..."
    elpoints, paths = elevation.GetElevationSamples(paths, intersections, n_samples, client)
    
    flattened_nodes = [node for path in paths for node in path.nodes]
    
    print "Finding shortest route..."
    myroute = route.OptimizeDistance(paths, intersections, intersections[0], intersections[20])
    
    print "Displaying map..."
    disp = visualize.MapDisplay(resolution=(500, 700), geobounds=(36.045, -80.415, 36.085, -80.375))
    for path in paths:
        disp.DrawPath(path.nodes, (0, 0, 0), 1)
    #disp.DisplayElevationDots([(util.Coord.FromNode(node), node.elevation) for node in flattened_nodes])
    disp.DisplayElevationDots(elpoints)
    for node in myroute:
        disp.DrawNode(node, (0, 0, 255), 4)
    disp.Show()