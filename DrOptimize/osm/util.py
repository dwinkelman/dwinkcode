'''
Created on Jul 17, 2017

@author: Daniel
'''

from math import cos, asin, sqrt

class Coord:
    def __init__(self, lat, lon):
        self.lat = float(lat)
        self.lon = float(lon)
        
    def __repr__(self):
        return "<lat:%f, lon:%f>" % (self.lat, self.lon)
    
    @classmethod
    def FromNode(cls, node):
        return cls(node.lat, node.lon)

def HaversineDist(p1, p2):
    """Returns distance between two lat-lon points on Earth in kilometers"""
    # https://stackoverflow.com/questions/27928/calculate-distance-between-two-latitude-longitude-points-haversine-formula  
    p = 0.017453292519943295 # math.pi / 180
    a = 0.5 - cos((float(p2.lat) - float(p1.lat)) * p) / 2
    b = cos(float(p1.lat) * p) * cos(float(p2.lat) * p)
    c = (1 - cos((float(p2.lon) - float(p1.lon)) * p)) / 2
    return 12742 * asin(sqrt(a + b * c))