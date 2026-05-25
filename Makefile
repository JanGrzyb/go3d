LIBS=-lGL -lglfw -lGLEW
HEADERS= constants.h lodepng.h board.h shaderprogram.h
FILES= lodepng.cpp main_file.cpp shaderprogram.cpp
main_file: $(FILES) $(HEADERS)
	g++ -o main_file $(FILES)  $(LIBS) -I./glm/
