'''
Retrieve and process data from the Google Maps Elevation API.

Created on Jul 17, 2017

@author: Daniel
'''


import googlemaps

import util as util

#
#
# PROJECT API KEY (DrOptimize)
# AIzaSyCaoStheVeS9UNRRQH-nMTjQ2vdJQahvic
#
# Elevation API
#   2500 requests/day
#   50 requests/sec
#   512 points/request
#
# Geocoding API
#   2500 requests/day
#   50 requests/sec
#
#

def NumberElevationSamples(paths, intersections, unique_nodes):
    """Get the number of samples to take from Google Maps API based on number
    and priority of intersections and path distance"""
    # Absolute max: 2560 samples (5 requests)
    # Absolute min: 512 samples (1 request)
    # Intervals of 512 (to maximize requests)
    # Preferable: 1 sample for each intersection and 1 sample for every 200m
    # If distance and intersections exceeds limit, intersections take priority
    
    n_intersections = len(intersections)
    if n_intersections >= 2560: return 2560
    
    # 1 midpoint for every 200 meters over 200 meters
    ideal_midpoints = sum([int(path.length / 0.2) for path in paths])
    
    rounded_up = ((ideal_midpoints + n_intersections) / 512 + 1) * 512
    return min(rounded_up, 2560)

def MakeGoogleMapsClient():
    myclient = googlemaps.client.Client(key="AIzaSyCaoStheVeS9UNRRQH-nMTjQ2vdJQahvic", queries_per_second=50)
    return myclient

def GetElevationSamples(paths, intersections, n_samples, client):
    """
    @param paths: list of Path objects
    @param intersections: list of Intersection objects
    @param n_samples: number of elevation samples to take
    """
    """Get Google Maps elevation samples
    1. Find a certain number of coordinates in the network to get elevation for
        a. Samples automatically for all intersections
                (unless there are more intersections than samples)
        b. Find average distance between samples
        c. Distribute samples amongst paths according to length
        d. Interpolate along paths for exact sample locations
    2. Pass latitude-longitude coordinates into Google Maps
        a. Separate into 512-coordinate requests
    3. Get elevation for each node by interpolation
    4. Return raw data for redundancy
    """
    
    elevation_inputs = [util.Coord(0, 0) for i in range(n_samples)]
    # _________________1A______________________________________________________
    # intersection coords encoded as Coords
    elevation_inputs[:len(intersections)] = [util.Coord.FromNode(inter.node) for inter in intersections]
    
    # _________________1B______________________________________________________
    total_distance = sum([path.length for path in paths])
    midpoint_samples = n_samples - len(intersections)
    if midpoint_samples <= 0: raise ValueError("Too many intersections, not enough elevation samples.")
    # maximum distance to separate each elevation sample by
    distance_per_sample = float(total_distance) / midpoint_samples
    
    # _________________1C______________________________________________________
    # figure out how many samples can be fit into a path
    # since distance_per_sample is a maximum and we are truncating, there will
    # be a couple left over, so we store how much leftover there is
    for path in paths:
        path.n_elevation_samples = int(path.length / distance_per_sample)
        path.el_sample_remainder = path.length % distance_per_sample
    
    # get number of samples already claimed by midpoints
    midpoint_allocated = sum([path.n_elevation_samples for path in paths])
    # distribute the extras out to the longest leftover segments
    sorted_paths = sorted(paths, reverse=True, key=lambda a: a.el_sample_remainder)
    for i in range(midpoint_samples - midpoint_allocated):
        sorted_paths[i].n_elevation_samples += 1
        
    offset = len(intersections)
    for path in paths:
        path.elevation_data_offset = offset
        
        path.dist_per_sample = path.length / (path.n_elevation_samples + 1)
        
        path.nodes[0].dist_from_start = 0;
        for i in range(1, len(path.nodes)):
            path.nodes[i].dist_from_start = path.nodes[i - 1].dist_from_start + util.HaversineDist(
                path.nodes[i], path.nodes[i - 1])
        
        node_walker = 1
        for i in range(path.n_elevation_samples):
            while path.nodes[node_walker].dist_from_start < (i + 1) * path.dist_per_sample:
                node_walker += 1
            
            dA = path.nodes[node_walker - 1].dist_from_start
            dB = path.nodes[node_walker].dist_from_start
            dC = (i + 1) * path.dist_per_sample
            latA, latB = float(path.nodes[node_walker - 1].lat), float(path.nodes[node_walker].lat)
            lonA, lonB = float(path.nodes[node_walker - 1].lon), float(path.nodes[node_walker].lon)
            # use a proportion to interpolate
            point = util.Coord(0, 0)
            point.lat = latB - (dB - dC) * (latB - latA) / (dB - dA)
            point.lon = lonB - (dB - dC) * (lonB - lonA) / (dB - dA)
            
            elevation_inputs[offset + i] = point
        
        offset += path.n_elevation_samples
        
    # _________________2_______________________________________________________
    elevation_inputs_tuples = [(i.lat, i.lon) for i in elevation_inputs]
    
    # break inputs into queries and pass to Google Maps
    queries = [elevation_inputs_tuples[i:min(i + 512, n_samples)] for i in range(0, n_samples, 512)]
    results = [googlemaps.elevation.elevation(client, query) for query in queries]
    flattened_results = [i for j in results for i in j]
    elevation_outputs = [
        (util.Coord(res["location"]["lat"], res["location"]["lng"]), res["elevation"])
        for res in flattened_results]
    
    for i in range(len(intersections)):
        intersections[i].node.elevation = elevation_outputs[i][1]
    
    for path in paths:
        data = [(util.Coord.FromNode(path.start), path.start.elevation)]
        data += elevation_outputs[path.elevation_data_offset:path.elevation_data_offset + path.n_elevation_samples]
        data += [(util.Coord.FromNode(path.end), path.end.elevation)]
        for i in range(1, len(path.nodes) - 1):
            offset = int(path.nodes[i].dist_from_start / path.dist_per_sample)
            dA = util.HaversineDist(data[offset][0], path.nodes[i])
            dB = util.HaversineDist(data[offset + 1][0], path.nodes[i])
            eA, eB = data[offset][1], data[offset + 1][1]
            path.nodes[i].elevation = (eA * dA + eB * dB) / (dA + dB)
        
        # clean up extra data
        del path.dist_per_sample
        del path.el_sample_remainder
        del path.elevation_data_offset
        del path.n_elevation_samples
    
    return elevation_outputs, paths