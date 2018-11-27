#! /bin/sh -e
g++ --std=c++17 -Wall -c MentiMemer.cpp -o MentiMemer.o
g++ -lPocoFoundation -lPocoNet -lPocoNetSSL -lPocoJSON -lpthread -O3 MentiMemer.o -o MentiMemer
