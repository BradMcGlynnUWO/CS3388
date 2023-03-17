// Include standard headers
#include <stdio.h>
#include <stdlib.h>

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
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "bmaploader.hpp"


struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 textureCoords;

    VertexData(glm::vec3 pos, glm::vec3 norm, glm::vec3 col, glm::vec2 tex) :
        position(pos),
        normal(norm),
        color(col),
        textureCoords(tex)
    {}
};

struct TriData {
    GLuint vertex_indices[3];
};

void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp) {

    // initialize camera speed and rotation speed
    float cameraSpeed = 0.05;
    float cameraRotateSpeed = 3.0f;

    // process user input
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront; // moves camera forward
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront; // moves camera backward
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraFront = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateSpeed), cameraUp) * glm::vec4(cameraFront, 1.0f))); // rotates camera left
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraFront = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotateSpeed), cameraUp) * glm::vec4(cameraFront, 1.0f))); // rotates camera right
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp; // moves camera "upwards"
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp; // moves camera "downwards"
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; // moves camera left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; // moves camera right
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraFront = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateSpeed * 0.5f), glm::normalize(glm::cross(cameraFront, cameraUp))) * glm::vec4(cameraFront, 1.0f))); // pitches camera yaw upwards
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraFront = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotateSpeed * 0.5f), glm::normalize(glm::cross(cameraFront, cameraUp))) * glm::vec4(cameraFront, 1.0f))); // pitches camera yaw downwards

    // Calculates the new normalized up vector of the camera based on the cross product of the current camera front and previous up vector
    cameraUp = glm::normalize(glm::cross(glm::cross(cameraFront, cameraUp), cameraFront));
}

void readPLYFile(std::string fname, std::vector<VertexData>& vertices, std::vector<TriData>& faces) {
    // Open the file for reading
    std::ifstream file(fname);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fname << std::endl;
        return;
    }

    // Initialize variables for parsing
    std::string line;
    std::istringstream stream;
    std::string token;
    int numVerts = 0;
    int numFaces = 0;
    bool normalVector = false;
    bool colour = false;
    bool coordinates = false;
    int maxLength = 0;
    std::map <int, const char*> propertyOrder = {};

    // Parse the header until the end of the file is reached
    while (getline(file, line)) {
        stream.clear();
        stream.str(line);
        stream >> token;

        // checks for number of vertices and faces to determine length 
        // needed to loop through for capturing the vertex data and face data later
        if (token == "element") {
            stream >> token;

            if (token == "vertex") {
                stream >> numVerts;
            }
            if (token == "face") {
                stream >> numFaces;
            }
        } // checks for optional property data
        else if (token == "property") {
            stream >> token;
            if (token == "float") {
                stream >> token;
                // vector positions
                if (token == "x") {
                    propertyOrder.insert(std::make_pair(maxLength, "x"));
                }
                else if (token == "y") {
                    propertyOrder.insert(std::make_pair(maxLength, "y"));
                }
                else if (token == "z") {
                    propertyOrder.insert(std::make_pair(maxLength, "z"));
                }
                // normal vectors
                if (token == "nx") {
                    propertyOrder.insert(std::make_pair(maxLength, "nx"));
                }
                else if (token == "ny") {
                    propertyOrder.insert(std::make_pair(maxLength, "ny"));
                }
                else if (token == "nz") {
                    propertyOrder.insert(std::make_pair(maxLength, "nz"));
                }
                // case for if colours are floats
                if (token == "red") {
                    propertyOrder.insert(std::make_pair(maxLength, "red"));
                }
                else if (token == "green") {
                    propertyOrder.insert(std::make_pair(maxLength, "green"));
                }
                else if (token == "blue") {
                    propertyOrder.insert(std::make_pair(maxLength, "blue"));
                }
                // texture coordinates
                else if (token == "u") {
                    propertyOrder.insert(std::make_pair(maxLength, "u"));
                }
                else if (token == "v") {
                    propertyOrder.insert(std::make_pair(maxLength, "v"));
                }
            } // Case if colours are unsigned characters
            else if (token == "uchar") {
                stream >> token;
                // colours
                if (token == "red") {
                    propertyOrder.insert(std::make_pair(maxLength, "red"));
                }
                else if (token == "green") {
                    propertyOrder.insert(std::make_pair(maxLength, "green"));
                }
                else if (token == "blue") {
                    propertyOrder.insert(std::make_pair(maxLength, "blue"));
                }
            } maxLength++;
        }
     
        else if (token == "end_header") {
            break;
        }
    }

    // Parses vertex data
    for (int i = 0; i < numVerts; i++) {
        glm::vec3 position(0.0f);
        glm::vec3 normal(0.0f);
        glm::vec3 color(0.0f);
        glm::vec2 textureCoords(0.0f);

        getline(file, line);
        stream.clear();
        stream.str(line);
        int counter = 0;
        do {
            auto property = propertyOrder.find(counter);
            // dynamically allocates variables depending on their key order pairing in map
            if (property->second == "x") {
                stream >> position.x;
            }
            else if (property->second == "y") {
                stream >> position.y;
            }
            else if (property->second == "z") {
                stream >> position.z;
            }
            else if (property->second == "nx") {
                stream >> normal.x;
            }
            else if (property->second == "ny") {
                stream >> normal.y;
            }
            else if (property->second == "nz") {
                stream >> normal.z;
            }
            else if (property->second == "red") {
                stream >> color.r;
            }
            else if (property->second == "green") {
                stream >> color.g;
            }
            else if (property->second == "blue") {
                stream >> color.b;
            }
            else if (property->second == "u") {
                stream >> textureCoords.x;
            }
            else if (property->second == "v") {
                stream >> textureCoords.y;
            }
            counter++;
        } while (counter < propertyOrder.size());
        
        vertices.push_back(VertexData(position, normal, color, textureCoords));
    }

    // Parses face data
    for (int i = 0; i < numFaces; i++) {
        GLuint v0, v1, v2;
        getline(file, line);
        stream.clear();
        stream.str(line);
        stream >> token >> v0 >> v1 >> v2;
        // sends new TriData object to faces
        faces.push_back(TriData{ {v0, v1, v2} });
    }

    // Close the file
    file.close();
}

class TexturedMesh {
private:
    std::vector <VertexData> vertices;
    std::vector <TriData> faces;
    GLuint vao; 
    GLuint vboVertexPosition; 
    GLuint vboTextureCoordinates; 
    GLuint eboFacesIndices; 
    GLuint textureID; 
    GLuint shaderID; 
public:
    TexturedMesh(const char* plyFilePath, const char* bmpFilePath) {
        // read PLY files and store date in appropriate vectors
        readPLYFile(plyFilePath, vertices, faces);
        std::vector<GLfloat> vertexTextureCoordinates;
        std::vector<GLfloat> vertexPositions;
        // Initialize vertexPositions vector with appropriate space, then fill with appropriate data
        vertexPositions.reserve(vertices.size() * 3);
        for (const auto& vertex : vertices) {
            vertexPositions.push_back(vertex.position.x);
            vertexPositions.push_back(vertex.position.y);
            vertexPositions.push_back(vertex.position.z);
        }
        // Initialize vertexTextureCoordinates vector with appropriate space, then fill with appropriate data
        vertexTextureCoordinates.reserve(vertices.size() * 2);
        for (const auto& vertex : vertices) {
            vertexTextureCoordinates.push_back(vertex.textureCoords.x);
            vertexTextureCoordinates.push_back(vertex.textureCoords.y);
        }
        // read bmp files and store results in appropriate variables
        GLuint width, height;
        unsigned char* data;
        loadARGB_BMP(bmpFilePath, &data, &width, &height);

        // create shader for object
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        std::string VertexShaderCode = "\
    	#version 330 core\n\
		// Input vertex data, different for all executions of this shader.\n\
		layout(location = 0) in vec3 vertexPosition;\n\
		layout(location = 1) in vec2 uv;\n\
		// Output data ; will be interpolated for each fragment.\n\
		out vec2 uv_out;\n\
		// Values that stay constant for the whole mesh.\n\
		uniform mat4 MVP;\n\
		void main(){ \n\
			// Output position of the vertex, in clip space : MVP * position\n\
			gl_Position =  MVP * vec4(vertexPosition,1);\n\
			// The color will be interpolated to produce the color of each fragment\n\
			uv_out = uv;\n\
		}\n";
        std::string FragmentShaderCode = "\
		#version 330 core\n\
		in vec2 uv_out; \n\
		uniform sampler2D tex;\n\
		void main() {\n\
			gl_FragColor = texture(tex, uv_out);\n\
		}\n";
        char const* VertexSourcePointer = VertexShaderCode.c_str();
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
        glCompileShader(VertexShaderID);

        // Compile Fragment Shader
        char const* FragmentSourcePointer = FragmentShaderCode.c_str();
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
        glCompileShader(FragmentShaderID);
        
        // store shaderID for later use
        shaderID = glCreateProgram();
        glAttachShader(shaderID, VertexShaderID);
        glAttachShader(shaderID, FragmentShaderID);
        glLinkProgram(shaderID);

        glDetachShader(shaderID, VertexShaderID);
        glDetachShader(shaderID, FragmentShaderID);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        // create texture bitmap
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // sets up vertex attribute object
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // creates attribute buffer for vertex positions
        glGenBuffers(1, &vboVertexPosition);
        glBindBuffer(GL_ARRAY_BUFFER, vboVertexPosition);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexPositions.size(), vertexPositions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // creates attribute buffer for texture coordinates       
        glGenBuffers(1, &vboTextureCoordinates);
        glBindBuffer(GL_ARRAY_BUFFER, vboTextureCoordinates);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexTextureCoordinates.size(), vertexTextureCoordinates.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        // creates element buffer for indices of faces vertex
        glGenBuffers(1, &eboFacesIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFacesIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * faces.size(), faces.data(), GL_STATIC_DRAW);   
        glBindVertexArray(0);
        
    }

    void draw(glm::mat4 MVP) {

        // Enables blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Gets location of MVP and sets MVP uniform variable in shader program
        glUseProgram(shaderID);
        GLuint MatrixID = glGetUniformLocation(shaderID, "MVP");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Enables 2D texturing and binds the texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Binds the vao
        glBindVertexArray(vao);

        // Draws the object using triangles
        glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);

        // Unbinds the vao
        glBindVertexArray(0);
        
        // Clears out attributes and disables blending
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        //glDisable(GL_BLEND);
            
        }

};

int main() {
	// Initializes GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	// Opens window and creates OpenGL context
	float screenW = 1400;
	float screenH = 900;

    // to help minimize screen tearing
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwSwapInterval(1);

	window = glfwCreateWindow( screenW, screenH, "Links Room", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}

    // Makes window the current context
	glfwMakeContextCurrent(window);

	// Initializes GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensures we can capture escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Sets a dark blue background
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);

    // enables depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    // creates all texture mesh objects
    TexturedMesh bottlesMesh("Bottles.ply", "bottles.bmp");
    TexturedMesh doorBackgroundMesh("DoorBG.ply", "doorbg.bmp");
    TexturedMesh curtainsMesh("Curtains.ply", "curtains.bmp");
    TexturedMesh floorMesh("Floor.ply", "floor.bmp");
    TexturedMesh metalObjectsMesh("MetalObjects.ply", "metalobjects.bmp");
    TexturedMesh patioMesh("Patio.ply", "patio.bmp"); 
    TexturedMesh tableMesh("Table.ply", "table.bmp");
    TexturedMesh wallsMesh("Walls.ply", "walls.bmp");
    TexturedMesh windowBackgroundMesh("WindowBG.ply", "windowbg.bmp");
    TexturedMesh woodObjectsMesh("WoodObjects.ply", "woodobjects.bmp");

    // initializes camera position
    glm::vec3 cameraPos = glm::vec3(0.5f, 0.4f, 0.5f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // initializes model-view-projection matrix
    glm::mat4 MVP;

    // Loops until the user closes the window or presses the ESC key
	do{
		// Clears the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Processes user input to update camera position and orientation
        processInput(window, cameraPos, cameraFront, cameraUp);

        // Sets the camera view matrix
        glm::mat4 V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Sets the projection matrix
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)screenW / (float)screenH, 0.1f, 1000.0f);

        // Loads the projection matrix onto the projection matrix stack
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(glm::value_ptr(Projection));

        // Loads the view matrix onto the modelview matrix stack
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(glm::value_ptr(V));

        // Sets the model matrix
        glm::mat4 M = glm::mat4(1.0f);

        // Calculates the model-view-projection matrix
        MVP = Projection * V * M;

        // Draws the objects in the scene using MVP 
        bottlesMesh.draw(MVP);
        floorMesh.draw(MVP);
        patioMesh.draw(MVP);
        tableMesh.draw(MVP);
        wallsMesh.draw(MVP);
        woodObjectsMesh.draw(MVP);
        metalObjectsMesh.draw(MVP);
        doorBackgroundMesh.draw(MVP);
        windowBackgroundMesh.draw(MVP);
        curtainsMesh.draw(MVP);

        // Restores the modelview and projection matrices to their previous states
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        // Swaps the front and back buffers to display the rendered image
        glfwSwapBuffers(window);
        glfwPollEvents();


	} // Checks if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Closes OpenGL window and terminates GLFW
	glfwTerminate();

	return 0;
}

