CC = g++
CFLAGS = -std=c++11 -Werror -Wall -pedantic
EXTRAFLAGS=-lpqxx -lpq
CCONFIG = `pkg-config --cflags --libs protobuf`


all: main

main: main.cpp world_ups.o ups.o
	$(CC) $(CFLAGS) main.cpp world_ups.o ups.o -o main $(CCONFIG) $(EXTRAFLAGS)

world_ups.o: world_ups.pb.cc
	$(CC) $(CFLAGS) world_ups.pb.cc -c -o world_ups.o $(CCONFIG) $(EXTRAFLAGS)

ups.o: ups.pb.cc
	$(CC) $(CFLAGS) ups.pb.cc -c -o ups.o $(CCONFIG) $(EXTRAFLAGS)

clean:
	rm *.o
