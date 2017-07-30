LIBS = -lGL -L/Cygwin/lib/lglfw3dll.a
test: test.c
    gcc -o test.exe test.c glad.c $(LIBS)