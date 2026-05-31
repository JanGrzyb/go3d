/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "board.h"
#include "sphere.h"
#include "go_game.h"
ShaderProgram *sp;
ShaderProgram *sp2;
ShaderProgram *sp3;

GLuint woodTex;
GLuint floorDiffTex;
GLuint floorDisplTex;
GLuint floorNormalTex;


GoGame game;
int cursorX = 10, cursorZ = 10;
int currentPlayer = 1;

int cameraMode = 1; // 1 - tilted, 0 - top-down
glm::vec3 camStartPos, camTargetPos, camCurrentPos;
glm::vec3 camStartUp, camTargetUp, camCurrentUp;
float camAnimTime = 0.0f;
float camAnimDuration = 0.7f;
bool camAnimating = false;

float rotation = 0.0f;
float targetRotation = 0.0f;
float rotationStart = 0.0f;
float rotationAnimTime = 0.0f;
float rotationAnimDuration = 0.7f; // seconds for 90 deg
bool rotating = false;

std::vector<int> arrowKeyDirections = {GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN};

//Error processing callback procedure
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void rotateScene(char direction) {
    if (rotating) return; 
    rotationStart = rotation;
    if (direction == 'l') {
        targetRotation = rotation + 90.0f;
		//adjust directions
		arrowKeyDirections = {arrowKeyDirections[3], arrowKeyDirections[2], arrowKeyDirections[0], arrowKeyDirections[1]};
    } else if (direction == 'r') {
        targetRotation = rotation - 90.0f;
		//adjust directions
		arrowKeyDirections = {arrowKeyDirections[2], arrowKeyDirections[3], arrowKeyDirections[1], arrowKeyDirections[0]};
    }
    rotationAnimTime = 0.0f;
    rotating = true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (key == arrowKeyDirections[0])  cursorX = std::min(19,  cursorX + 1);
    if (key == arrowKeyDirections[1])  cursorX = std::max(1, cursorX - 1);
    if (key == arrowKeyDirections[2])  cursorZ = std::min(19,  cursorZ + 1);
    if (key == arrowKeyDirections[3])  cursorZ = std::max(1, cursorZ - 1);
    if (key == GLFW_KEY_SPACE) game.placeStone(cursorX, cursorZ);
    if (key == GLFW_KEY_P)     game.pass();
    if (key == GLFW_KEY_S)     game.saveToSGF("game.sgf");
    if (key == GLFW_KEY_R)     game.loadFromSGF("game.sgf");
	//rotation by 90 degrees J - rotate right, L - rotate left
	if (key == GLFW_KEY_J)     rotateScene('r');
	if (key == GLFW_KEY_L)     rotateScene('l');
	    if (key == GLFW_KEY_I && !camAnimating) {
        // Animate to top-down
        camStartPos = glm::vec3(0.0f, 11.0f, -9.0f);
        camTargetPos = glm::vec3(0.0f, 15.0f, -0.1f);
        camStartUp  = glm::vec3(0.0f, 1.0f, 0.0f);
        camTargetUp = glm::vec3(0.0f, 1.0f, 0.0f); 
        camAnimTime = 0.0f;
        camAnimating = true;
        cameraMode = 0;
    }
    if (key == GLFW_KEY_K && !camAnimating) {
        // Animate to tilted
        camStartPos = glm::vec3(0.0f, 15.0f, -0.1f);
        camTargetPos = glm::vec3(0.0f, 11.0f, -9.0f);
        camStartUp  = glm::vec3(0.0f, 1.0f, 0.0f);
        camTargetUp = glm::vec3(0.0f, 1.0f, 0.0f);
        camAnimTime = 0.0f;
        camAnimating = true;
        cameraMode = 1;
    }
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	std::vector<unsigned char> image; 
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);

	if (error) {
		fprintf(stderr, "Error loading PNG: %s\n", lodepng_error_text(error));
		return 0;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // <-- add this
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // <-- add this
	glGenerateMipmap(GL_TEXTURE_2D);
	return tex;
}

//Initialization code procedure
void initOpenGLProgram(GLFWwindow* window) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Set the background color
	glEnable(GL_DEPTH_TEST); //Enable depth test
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	sp=new ShaderProgram("v_custom.glsl", NULL, "f_custom.glsl");
	sp2=new ShaderProgram("v_stone.glsl", NULL, "f_stone.glsl");
	sp3=new ShaderProgram("v_floor.glsl", NULL, "f_floor.glsl");
	woodTex = readTexture("oak_veneer_01_diff_4k.png");
	floorDiffTex = readTexture("stone_tiles_diff_4k.png");
	floorDisplTex = readTexture("stone_tiles_disp_4k.png");
	floorNormalTex = readTexture("stone_tiles_nor_gl_4k.png");
	//************Place any code here that needs to be executed once, at the program start************
}

//Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
    delete sp;
	delete sp2;
	glDeleteTextures(1, &woodTex);
	//************Place any code here that needs to be executed once, after the main loop ends************
}


void drawFloor(glm::mat4 M) {
	//board model but scaled, only top face, and with its own shader
	M = glm::translate(M, glm::vec3(0.0f, -1.0f, 0.0f));
	M = glm::scale(M, glm::vec3(12.0f, 1.0f, 12.0f));
	sp3->use();
	glUniformMatrix4fv(sp3->u("M"), 1, false, glm::value_ptr(M));
	glEnableVertexAttribArray(sp3->a("vertex"));
	glVertexAttribPointer(sp3->a("vertex"), 4, GL_FLOAT, false, 0, boardVertices);
	glEnableVertexAttribArray(sp3->a("normal"));
	glVertexAttribPointer(sp3->a("normal"), 4, GL_FLOAT, false, 0, boardNormals);
	glEnableVertexAttribArray(sp3->a("texCoord"));
	glVertexAttribPointer(sp3->a("texCoord"), 2, GL_FLOAT, false, 0, (float[]) {
	    0.0f, 0.0f,   // E
	    6.0f, 0.0f,   // F
	    6.0f, 6.0f,   // G

	    0.0f, 0.0f,   // E
		0.0f, 6.0f,   // H
	    6.0f, 6.0f,   // G
	});

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, floorDiffTex);
	glUniform1i(sp3->u("diffTex"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, floorDisplTex);
	glUniform1i(sp3->u("displTex"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, floorNormalTex);
	glUniform1i(sp3->u("normalTex"), 2);


	glDrawArrays(GL_TRIANGLES, 0, 6); // top face only



	glDisableVertexAttribArray(sp3->a("vertex"));
	glDisableVertexAttribArray(sp3->a("normal"));
	glDisableVertexAttribArray(sp3->a("texCoord"));

}

void drawBoard(glm::mat4 M) {
	sp->use();
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, boardVertices);
	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, boardNormals);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, boardTexCoords);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTex);
	glUniform1i(sp->u("myTex"), 0);


	glDrawArrays(GL_TRIANGLES, 0, boardVertexCount);



	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("texCoord"));

}

void drawStoneAtPosition(glm::mat4 M, float x, float z, bool isBlack, bool isCursor=false) {
	sp2->use();
	//conversion from board coordinates to world coordinates
	float wrldX = -5.4f + ((x-1)*0.6f);
	float wrldZ = -5.4f + ((z-1)*0.6f);
	//sphere with r=1
	M = glm::translate(M, glm::vec3(wrldX, 1.1f, wrldZ));
	M = glm::scale(M, glm::vec3(0.3f, 0.1f, 0.3f));	
	glUniformMatrix4fv(sp2->u("M"), 1, false, glm::value_ptr(M));
	glEnableVertexAttribArray(sp2->a("vertex"));
	glVertexAttribPointer(sp2->a("vertex"), 4, GL_FLOAT, false, 0, sphereVertices);
	glEnableVertexAttribArray(sp2->a("normal"));
	glVertexAttribPointer(sp2->a("normal"), 4, GL_FLOAT, false, 0, sphereVertexNormals);
	glEnableVertexAttribArray(sp2->a("color"));

	if(isCursor && game.getCell(cursorX, cursorZ) != Player::None) {
		glUniform1i(sp2->u("highlight"), 1);
	} else {
		glUniform1i(sp2->u("highlight"), 0);
	}

    if (isBlack) {
		glVertexAttribPointer(sp2->a("color"), 4, GL_FLOAT, false, 0, SphereBlack);
 	} else {
        glVertexAttribPointer(sp2->a("color"), 4, GL_FLOAT, false, 0, SphereWhite);
    }

	if (isCursor) {
		glUniform1i(sp2->u("translucent"), 1);
	} else {
		glUniform1i(sp2->u("translucent"), 0);
	}
	glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);

	glDisableVertexAttribArray(sp2->a("vertex"));
	glDisableVertexAttribArray(sp2->a("normal"));
	glDisableVertexAttribArray(sp2->a("color"));
}

//Drawing procedure
void drawScene(GLFWwindow* window, float rotation, int cursorX, int cursorZ, int currentPlayer) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 M = glm::mat4(1.0f);
	glm::mat4 P = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);
	glm::mat4 V = glm::lookAt(camCurrentPos, glm::vec3(0.0f, 0.0f, 0.0f), camCurrentUp);
	M = glm::rotate(M, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));

	sp3->use();
	glUniformMatrix4fv(sp3->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp3->u("V"), 1, false, glm::value_ptr(V));
	drawFloor(M);

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	drawBoard(M);

	sp2->use();
	glUniformMatrix4fv(sp2->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp2->u("V"), 1, false, glm::value_ptr(V));
	for (int i = 1; i <= 19; i++)
    for (int j = 1; j <= 19; j++) {
        int cell = game.getBoardInt(i, j);
        if (cell == 1) drawStoneAtPosition(M, i, j, true);
        if (cell == 2) drawStoneAtPosition(M, i, j, false);
    }
	
	glDisable(GL_DEPTH_TEST);
	drawStoneAtPosition(M, cursorX, cursorZ, (game.getCurrentPlayer() == Player::Black), true);
	glEnable(GL_DEPTH_TEST);

	glfwSwapBuffers(window);
}

int main(void)
{
	GLFWwindow* window; //Pointer to object that represents the application window

	glfwSetErrorCallback(error_callback);//Register error processing callback procedure
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

	if (!glfwInit()) { //Initialize GLFW library
		fprintf(stderr, "Can't initialize GLFW.\n");
		exit(EXIT_FAILURE); 
	}

	window = glfwCreateWindow(800, 800, "OpenGL", NULL, NULL);  //Create a window 800pxx800px titled "OpenGL" and an OpenGL context associated with it.

	if (!window) //If no window is opened then close the program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Since this moment OpenGL context corresponding to the window is active and all OpenGL calls will refer to this context.
	glfwSwapInterval(1); //During vsync wait for the first refresh

	GLenum err;
	if ((err=glewInit()) != GLEW_OK) { //Initialize GLEW library
		fprintf(stderr, "Can't initialize GLEW: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Call initialization procedure

	glfwSetKeyCallback(window, key_callback);

	double lastTime = glfwGetTime();
	//Main application loop
	while (!glfwWindowShouldClose(window)) //As long as the window shouldnt be closed yet...
	{
		double now = glfwGetTime();
		float delta = float(now - lastTime);
		lastTime = now;

		if (rotating) {
			rotationAnimTime += delta;
			float t = rotationAnimTime / rotationAnimDuration;
			if (t >= 1.0f) {
				t = 1.0f;
				rotating = false;
			}
			float ease = 0.5f - 0.5f * cos(PI * t);
			rotation = glm::mix(rotationStart, targetRotation, ease);
		}
		if (camAnimating) {
        	camAnimTime += delta;
        	float t = camAnimTime / camAnimDuration;
        	if (t >= 1.0f) {
            	t = 1.0f;
            	camAnimating = false;
        	}
        	float ease = 0.5f - 0.5f * cos(PI * t);
        	camCurrentPos = glm::mix(camStartPos, camTargetPos, ease);
        	camCurrentUp  = glm::mix(camStartUp,  camTargetUp,  ease);
    	} else {
        	camCurrentPos = (cameraMode == 1) ? glm::vec3(0.0f, 11.0f, -9.0f) : glm::vec3(0.0f, 15.0f, -0.1f);
        	camCurrentUp  = glm::vec3(0.0f, 1.0f, 0.0f);
    }

		drawScene(window, rotation, cursorX, cursorZ, currentPlayer); //Execute drawing procedure
		glfwPollEvents(); //Process callback procedures corresponding to the events that took place up to now
	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Delete OpenGL context and the window.
	glfwTerminate(); //Free GLFW resources
	exit(EXIT_SUCCESS);
}
