// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <iostream>
#include <vector>

#define TOP_LEFT     8
#define TOP_RIGHT    4
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT  1

typedef float (*scalar_field_2d)(float, float);

int marching_squares_lut[32][4] = {
	{-1, -1, -1, -1},
	{2, 3, -1, -1},
	{1, 2, -1, -1},
	{1, 3, -1, -1},
	{0, 1, -1, -1},
	{0, 1, 2, 3},
	{0, 2, -1, -1},
	{0, 3, -1, -1},
	{0, 3, -1, -1},
	{0, 2, -1, -1},
	{0, 3, 1, 2},
	{0, 1, -1, -1},
	{1, 3, -1, -1},
	{1, 2, -1, -1},
	{2, 3, -1, -1},
	{-1, -1, -1, -1}, 
	{-1, -1, -1, -1},
	{2, 3, -1, -1},
	{1, 2, -1, -1},
	{1, 3, -1, -1},
	{0, 1, -1, -1},
	{0, 1, 2, 3},
	{0, 2, -1, -1},
	{0, 3, -1, -1},
	{0, 3, -1, -1},
	{0, 2, -1, -1},
	{0, 3, 1, 2},
	{0, 1, -1, -1},
	{1, 3, -1, -1},
	{1, 2, -1, -1},
	{2, 3, -1, -1},
	{-1, -1, -1, -1}
};

float g_verts[4][2] = {
	{0.5f, 1.0f},
	{1.0f, 0.5f},
	{0.5f, 0.0f},
	{0.0f, 0.5f}
};

float f1(float x, float y) {
	return x*x + y*y;
}

float f2(float x, float y) {
	return sin(x*y);
}

float f3(float x, float y) {
	return sin(x)*cos(y);
}


std::vector<float> marching_squares(scalar_field_2d f, float isoval, float minx, float maxx, float miny, float maxy, float stepsize) {

	std::vector<float> vertices;

	float x = minx;
	float y = miny;
	float tl, tr, br, bl;
	int which = 0;
	int* verts;
	for ( ; y < maxy; y += stepsize) {
		for (x = minx ; x < maxx; x += stepsize) {
			//test the square
			tl = (*f)(x, y+stepsize);
			tr = (*f)(x+stepsize, y+stepsize);
			br = (*f)(x+stepsize, y);
			bl = (*f)(x, y);
			printf("%f %f %f %f", tl, tr, br, bl);

			which = 0;
			if (tl < isoval) {
				which |= TOP_LEFT;
			}
			if (tr < isoval) {
				which |= TOP_RIGHT;
			}
			if (br < isoval) {
				which |= BOTTOM_RIGHT;
			}
			if (bl < isoval) {
				which |= BOTTOM_LEFT;
			}

			verts = marching_squares_lut[which];

		
			if (verts[0] >= 0) {
				vertices.emplace_back(x+stepsize*g_verts[verts[0]][0]);
				vertices.emplace_back(y+stepsize*g_verts[verts[0]][1]);
				vertices.emplace_back(x+stepsize*g_verts[verts[1]][0]);
				vertices.emplace_back(y+stepsize*g_verts[verts[1]][1]);
			}
			if (verts[2] >= 0) {
				vertices.emplace_back(x+stepsize*g_verts[verts[2]][0]);
				vertices.emplace_back(y+stepsize*g_verts[verts[2]][1]);
				vertices.emplace_back(x+stepsize*g_verts[verts[3]][0]);
				vertices.emplace_back(y+stepsize*g_verts[verts[3]][1]);
			}
		} 
	}

	return vertices;
}

void marching_squares_step(float (*f)(float, float), float iso, float x, float y, std::vector<float>& marchingVerts, float stepsize) {
    
	//for ( float y = miny; y < maxy; y += stepsize) {
	//	for (float x = minx ; x < maxx; x += stepsize) {
	
			int square_index = 0;
			if (f(x, y + stepsize) > iso) square_index |= 1;
			if (f(x + stepsize, y + stepsize) > iso) square_index |= 2;
			if (f(x + stepsize, y) > iso) square_index |= 4;
			if (f(x, y) > iso) square_index |= 8;

			int* edges = marching_squares_lut[square_index];
			//for (int i = 0; i < 4 && edges[i] != -1; i++) {
				int edge_index = 0;

				float v1x = x + g_verts[edge_index][0];
				float v1y = y + g_verts[edge_index][1];
				float v2x = x + g_verts[(edge_index + 1) % 4][0];
				float v2y = y + g_verts[(edge_index + 1) % 4][1];

				// add the vertices for this edge to the marchingVerts vector
				marchingVerts.push_back(v1x);
				marchingVerts.push_back(v1y);
				marchingVerts.push_back(0.0f);  // z-coordinate
				marchingVerts.push_back(v2x);
				marchingVerts.push_back(v2y);
				marchingVerts.push_back(0.0f);  // z-coordinate
			//}
		//}
	//}
}



std::vector<float> generate_grid(float minx, float maxx, float miny, float maxy, float stepsize) {
    std::vector<float> vertices;

    // Generate horizontal lines
    float y = miny;
    while (y <= maxy) {
        vertices.emplace_back(minx);
        vertices.emplace_back(y);
        vertices.emplace_back(maxx);
        vertices.emplace_back(y);
        y += stepsize;
    }

    // Generate vertical lines
    float x = minx;
    while (x <= maxx) {
        vertices.emplace_back(x);
        vertices.emplace_back(miny);
        vertices.emplace_back(x);
        vertices.emplace_back(maxy);
        x += stepsize;
    }

    return vertices;
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
	float isoval = 1;

	if (argc > 1 ) {
		screenW = atoi(argv[1]);
	}
	if (argc > 2) {
		screenH = atoi(argv[2]);
	}
	if (argc > 3) {
		stepsize = atof(argv[3]);
	}
	if (argc > 4) {
		xmin = atof(argv[4]);
	}
	if (argc > 5) {
		xmax = atof(argv[5]);
	}
	if (argc > 6) {
		isoval = atof(argv[6]);
	}
	float ymin = xmin;
	float ymax = xmax;
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
	window = glfwCreateWindow( screenW, screenH, "Marching", NULL, NULL);
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

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(xmin, xmax, ymin, ymax, -1, 1);

	std::vector<float> marchingVerts1 = marching_squares(f1, isoval, xmin, xmax, ymin, ymax, stepsize);
	std::vector<float> marchingVerts2 = marching_squares(f2, isoval, xmin, xmax, ymin, ymax, stepsize);
	std::vector<float> marchingVerts3 = marching_squares(f3, isoval, xmin, xmax, ymin, ymax, stepsize);
	printf("%ld %ld %ld\n", marchingVerts1.size(), marchingVerts2.size(), marchingVerts3.size());


	std::vector<float> grid = generate_grid(xmin, xmax, ymin, ymax, 0.2);

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(2.0f);
		
			for (float i = xmin; i < xmax;  i += 20 * stepsize) {
				for (float j = ymin; j < ymax; j+= 15 * stepsize) {
					glBegin(GL_LINES);
					for (size_t k = 0; k < marchingVerts1.size(); k += 2) {
						glVertex2f(marchingVerts1[k] + i, marchingVerts1[k+1] +j);
						//glVertex2f(marchingVerts2[k] + i, marchingVerts2[k+1] +j);
						//glVertex2f(marchingVerts3[k] + i, marchingVerts3[k+1] +j);
					}
					glEnd();
				}
			}
		

		glLineWidth(1.0f);
		glBegin(GL_LINES);

			for (size_t i = 0; i < grid.size(); i += 2) {
				glVertex2f(grid[i], grid[i+1]);
			}
		glEnd();

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

