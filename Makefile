all: build run
.PHONY: build

build:
	g++ Source/main.cpp -o Build/main.o -pthread
	
run:
	./Build/main.o