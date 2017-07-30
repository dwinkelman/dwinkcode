CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
OBJECTS=main.o textplayer.o fileio.o midi.o 
EXECUTABLE=main

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)