LIBS=-lGL -lglfw -lGLEW
HEADERS= constants.h lodepng.h shaderprogram.h board.h sphere.h go_game.h
FILES= lodepng.cpp main_file.cpp shaderprogram.cpp go_game.cpp
main_file: $(FILES) $(HEADERS)
	g++ -o main_file $(FILES)  $(LIBS) -I./glm/
