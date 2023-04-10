## OpenGL Water Simulation
This project is a 3D water simulation using OpenGL. It features a plane mesh that simulates water and various 3D objects such as a boat, head, and eyes that interact with the water surface.

## Prerequisites
To build and run the project, you will need:

- C++ compiler with support for C++11
- OpenGL 3.3 or higher
- GLFW
- GLEW
- GLM
- BMapLoader.hpp
- PlaneMesh.hpp
- LoadBitmap.cpp

A set of sample assets (boat.ply, boat.bmp, head.ply, head.bmp, eyes.ply, and eyes.bmp)
## Build Instructions
- Install the necessary libraries and tools for your platform.
- Clone the repository and navigate to the project directory.
- Compile the project using your preferred C++ compiler and linker settings, ensuring that all required libraries are included.
- Recommended approach: Navigate to the project directory, type `make` in the terminal, then `./a.out`
## Running the Simulation
- Run the compiled executable.
- The simulation window will open, displaying the water surface and the 3D objects.
- Use the camera controls to navigate around the scene.
- Press the ESC key to close the window and exit the simulation.
## Camera Controls
- Up Arrow: Zoom the camera closer to the origin.
- Down Arrow: Zoom the camera away from the origin.
- Mouse movement: Rotate the camera.
## 3D Object Controls
- W, A, S, D: Move the object forward, left, backward, and right corresponding to which direction it is facing
- Q, E: Rotate the object counter-clockwise and clockwise
- Z, X: Move the object up and down
## Features
- Plane mesh water surface that simulates water movement and reflections
- 3D objects (boat, head, and eyes) that interact with the water surface (WIP - 3D objects are able to move around the scene, but currently cannot interact with the PlaneMesh water)
- Dynamic camera control for navigating the scene
- Realistic lighting and shading
## Code Overview
- The main components of the project are:

- main(): Initializes OpenGL, GLFW, and GLEW, creates the window, sets up shaders, loads assets, and enters the main loop.
- Camera: A class that handles the different viewing angles for the user, updating based on user input.
- PlaneMesh: A class that handles the creation, rendering, and updating of the plane mesh water surface.
- TexturedMesh: A class that handles the creation, rendering, and updating of 3D objects (boat, head, and eyes).
- Shaders: Custom vertex, tessellation, geometry, and fragment shaders for rendering the water surface and 3D objects with realistic lighting and shading.
