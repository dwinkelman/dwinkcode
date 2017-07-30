from distutils.core import setup, Extension

ext = Extension("_dijkstra", ["_dijkstra.c", "dijkstra.c", "priorityqueue.c"])

setup(ext_modules=[ext])
