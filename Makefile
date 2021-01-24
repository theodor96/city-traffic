all: main

main: Main.cpp
	@echo "Building executable..."
	@g++ -std=c++17 -o main Main.cpp

clean:
	rm -rf main
