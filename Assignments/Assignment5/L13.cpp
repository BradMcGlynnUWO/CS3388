// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>
#include <vector>

#include "shader.hpp"
#include "Sphere.hpp"
//#include "CamControls.hpp"
#include "LoadBMP.hpp"
#include "Plane.hpp"

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 lookAt) {
        position_ = position;
        lookAt_ = lookAt;
        theta_ = glm::pi<float>();
        phi_ = glm::pi<float>() / 2.0f;
        radius_ = glm::length(position_ - lookAt_);
    }

    glm::vec3 getPosition() const {
        return position_;
    }

    glm::vec3 getLookAt() const {
        return lookAt_;
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position_, lookAt_, glm::vec3(0, 1, 0));
    }


    void updateOrientation(float theta, float phi) {
        theta_ += theta;
        phi_ += phi;
        phi_ = glm::clamp(phi_, 0.1f, glm::pi<float>() - 0.1f);

        glm::vec3 relativePos(
            radius_ * sinf(phi_) * cosf(theta_),
            radius_ * cosf(phi_),
            radius_ * sinf(phi_) * sinf(theta_)
        );

        position_ = lookAt_ + relativePos;
    }

    void updateRadius(float deltaRadius) {
        radius_ += deltaRadius;
        if (radius_ < 0) {
            radius_ = 0.1f;
        }

        glm::vec3 relativePos(
            radius_ * sinf(phi_) * cosf(theta_),
            radius_ * cosf(phi_),
            radius_ * sinf(phi_) * sinf(theta_)
        );

        position_ = lookAt_ + relativePos;
    }

private:
    glm::vec3 position_;
    glm::vec3 lookAt_;
    float theta_;
    float phi_;
    float radius_;
};
// Globals
Camera camera(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0));
bool dragging = false;
double lastXPos, lastYPos;

// Callbacks
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            dragging = true;
            glfwGetCursorPos(window, &lastXPos, &lastYPos);
        } else if (action == GLFW_RELEASE) {
            dragging = false;
        }
    }
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (dragging) {
        double deltaX = xpos - lastXPos;
        double deltaY = ypos - lastYPos;
        lastXPos = xpos;
        lastYPos = ypos;
        float theta = deltaX * 0.01f;
        float phi = -deltaY * 0.01f;
        camera.updateOrientation(theta, phi);
    }
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        camera.updateRadius(-0.1f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        camera.updateRadius(0.1f);
    }
}

void processInput (GLFWwindow* window) {
    double currentXPos = lastXPos;
    double currentYPos = lastYPos;

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetKeyCallback(window, keyboardCallback);
    
}

//////////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[])
{

	///////////////////////////////////////////////////////
	float screenW = 1400;
	float screenH = 900;
	float stepsize = 0.1f;

	float xmin = -5;
	float xmax = 5;
	float ymin = -5;
	float ymax = 5;
	float isoval = 0.2;

	if (argc > 1 ) {
		screenW = atoi(argv[1]);
	}
	if (argc > 2) {
		screenH = atoi(argv[2]);
	}
	if (argc > 3) {
		stepsize = atof(argv[3]);
	}

	///////////////////////////////////////////////////////

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( screenW, screenH, "Phong", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}


	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


	

	glm::mat4 M1(1.0f);
	glm::mat4 M2(1.0f);
	glm::mat4 M3(1.0f);
	glm::mat4 MFloor(1.0f);
	M1 = glm::translate(M2, glm::vec3(-3.0f, 0.0f, 0.0f));
	M3 = glm::translate(M2, glm::vec3(3.0f, 0.0f, 0.0f));
	MFloor = glm::translate(MFloor, glm::vec3(0.f, -1.1f, 0.f));

	Sphere sphere;
	sphere.setUpAxis(2);
	Sphere sphere2;
	Sphere sphere3;
	sphere2.setUpAxis(2);
	sphere3.setUpAxis(2);

	Plane floor(10.0f, "wood.bmp");


	glm::vec3 lightpos(5.0f, 5.0f, 5.0f);


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	glm::vec4 color1(0.f, 0.8f, 0.8f, 1.0f);
	glm::vec4 color2(0.f, 0.8f, 0.8f, 1.0f);
	glm::vec4 color3(0.f, 0.8f, 0.8f, 1.0f);
	float alpha1 = 2;
	float alpha2 = 16;
	float alpha3 = 64;

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Processes user input to update camera position and orientation
        processInput(window);

		// Sets the camera view matrix
        glm::mat4 V = camera.getViewMatrix();

        glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		// Projection = glm::mat4(1.0f);
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), screenW/screenH, 0.001f, 1000.0f);
		glLoadMatrixf(glm::value_ptr(Projection));

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();

		glLoadMatrixf(glm::value_ptr(V));

		sphere.draw(lightpos, M1, V, Projection, color1, alpha1);
		sphere.draw(lightpos, M2, V, Projection, color2, alpha2);
		sphere.draw(lightpos, M3, V, Projection, color3, alpha3);

		floor.draw(lightpos, MFloor, V, Projection, color3, alpha1);

		
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(V));


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

