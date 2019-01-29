all:
	g++ -std=c++11 src/main.cpp src/Segment.cpp src/Image.cpp `pkg-config --libs --cflags opencv` -w -fopenmp -lGL -lglfw -lGLEW -lassimp -o p