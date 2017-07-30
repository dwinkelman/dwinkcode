'''
Created on Jul 18, 2017

@author: Daniel
'''

import sys
import pygame
pygame.init()

import osm.overpass as overpass
import osm.util as util

class MapDisplay():
    
    def __init__(self, resolution, geobounds):
        self.width, self.height = resolution
        self.screen = pygame.display.set_mode(resolution)
        self.screen.fill((255, 255, 255))
        #self.screen.set_caption("Dr. Optimize Map Display")
        self.min_lat, self.min_lon, self.max_lat, self.max_lon = geobounds
        
    def DrawNode(self, node, color, radius):
        point = self.MapPointToCanvas(node.lat, node.lon)
        if point[0] != None and point[1] != None:
            pygame.draw.circle(self.screen, color, point, radius)
            
    def DrawPath(self, nodes, color, width):
        points = [self.MapPointToCanvas(node.lat, node.lon) for node in nodes]
        for i in range(len(points) - 1):
            if points[i][0] != None and points[i][1] != None and points[i + 1][0] != None and points[i + 1][1] != None:
                pygame.draw.line(self.screen, color, points[i], points[i + 1], width)
        
    def MapPointToCanvas(self, lat, lon):
        lat, lon = float(lat), float(lon)
        if self.min_lat <= lat and lat <= self.max_lat and self.min_lon <= lon and lon <= self.max_lon:
            return (
                int(self.width * (lon - self.min_lon) / (self.max_lon - self.min_lon)),
                self.height - int(self.height * (lat - self.min_lat) / (self.max_lat - self.min_lat)))
        else: return (None, None)
        
    def Show(self):
        while 1:
            for event in pygame.event.get():
                if event.type == pygame.QUIT: sys.exit()
            pygame.display.flip();
            
            # throttle
            pygame.time.wait(50)
            
    def DisplayElevationDots(self, data):
        """Displays an array of (Coord, elevation [float]) color-coded to
        relative elevation"""
        min_elevation = min([elevation for point, elevation in data])
        max_elevation = max([elevation for point, elevation in data])
        midpoint = (max_elevation + min_elevation) / 2
        half = (max_elevation - min_elevation) / 2
        for point, elevation in data:
            if elevation > midpoint:
                self.DrawNode(point, (255, 255 * (1 - (elevation - midpoint) / half), 0), 2)
            else:
                self.DrawNode(point, (255 * (1 - (midpoint - elevation) / half), 255, 0), 2)