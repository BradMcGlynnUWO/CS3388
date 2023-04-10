## OpenGL Marching Cubes and Phong Shading
This is a C++ project that uses OpenGL and the marching cubes algorithm to create a 3D representation of a mathematical function, and applies Phong shading to render the object with realistic lighting.

## Dependencies
GLFW
GLEW
GLM
## Build Instructions
To build this project, you will need a C++ compiler. Follow these steps:

- Clone the repository
- Navigate to the project directory and run `make`
- run `./a.out [screen_width] [screen_height] [stepsize] [min] [max]`

screen_width: Width of the window, default is 1400.
screen_height: Height of the window, default is 900.
stepsize: The step size for the marching cubes algorithm, default is 0.05.
min: The minimum value for the coordinate axes, default is -5.
max: The maximum value for the coordinate axes, default is 5.
For example, run the program with default parameters:

`./a.out`

Or with custom parameters:

`./a.out 800 600 0.1 -3 3`

## Camera Controls
- Up Arrow: Zoom the camera closer to the origin.
- Down Arrow: Zoom the camera away from the origin.
- Mouse movement: Rotate the camera.

## Project Structure
- main.cpp: The main file of the project, which initializes GLFW and GLEW, sets up the window and OpenGL context, and runs the main loop.
- Camera: Class that controls the users mouse movement to rotate the scene.
- shader.hpp: Header file containing utility functions for loading and compiling shaders.
- TriTable.hpp: Header file containing the triangle lookup table for the marching cubes algorithm.
- PhongShader.vert: Vertex shader file for Phong shading.
- PhongShader.frag: Fragment shader file for Phong shading.
## Features
- Implements the marching cubes algorithm to generate 3D geometry from a mathematical function.
- Applies Phong shading to render the object with realistic lighting.
- Exports the generated geometry as a PLY file for further use.
- Provides a customizable camera for viewing the scene.
- Supports user-defined window size, step size, and coordinate range.
