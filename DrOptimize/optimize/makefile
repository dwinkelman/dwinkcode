CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
PQ_OBJECTS=priorityqueue.o pqtest.o
DK_OBJECTS=priorityqueue.o dijkstra.o dijkstratest.o
EXECUTABLES=pqtest dijkstra

all: $(PQ_OBJECTS) $(DK_OBJECTS) $(EXECUTABLES)

pqtest: $(PQ_OBJECTS)
	$(CC) $(LDFLAGS) $(PQ_OBJECTS) -o pqtest

dijkstra: $(DK_OBJECTS)
	$(CC) $(LDFLAGS) $(DK_OBJECTS) -o dijkstra

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)