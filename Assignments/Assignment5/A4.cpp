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
#include <functional>
#include <sstream>
#include <vector>
#include <map>
#include "bmaploader.hpp"
#include "TriTable.hpp"
//#include "shader.h"
#include "shader.hpp"


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


void drawBoundingBox(float min, float max) {

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable writing to depth buffer for drawing in the background
    glDepthMask(GL_FALSE);

    // Set line width and point size to be thin and small
    glLineWidth(1.0f);
    glPointSize(1.0f);

    // Set color to white with transparency
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    // Draw line
    glBegin(GL_LINES);

    // bottom face
    glVertex3f(min, min, min);
    glVertex3f(max, min, min);
    glVertex3f(max, min, min);
    glVertex3f(max, max, min);
    glVertex3f(max, max, min);
    glVertex3f(min, max, min);
    glVertex3f(min, max, min);
    glVertex3f(min, min, min);

    // top face
    glVertex3f(min, min, max);
    glVertex3f(max, min, max);
    glVertex3f(max, min, max);
    glVertex3f(max, max, max);
    glVertex3f(max, max, max);
    glVertex3f(min, max, max);
    glVertex3f(min, max, max);
    glVertex3f(min, min, max);

    // sides
    glVertex3f(min, min, min);
    glVertex3f(min, min, max);
    glVertex3f(max, min, min);
    glVertex3f(max, min, max);
    glVertex3f(max, max, min);
    glVertex3f(max, max, max);
    glVertex3f(min, max, min);
    glVertex3f(min, max, max);
    glEnd();

    // Re-enable writing to depth buffer and disable blending
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void drawAxes(glm::vec3 origin, glm::vec3 extents)
{


    glm::vec3 xcol = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 ycol = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 zcol = glm::vec3(0.0f, 0.0f, 1.0f);

    // Draw x-axis
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();


    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(xcol.x, xcol.y, xcol.z);
    glVertex3f(origin.x, origin.y, origin.z);
    glVertex3f(origin.x + extents.x, origin.y, origin.z);

    glVertex3f(origin.x + extents.x, origin.y, origin.z);
    glVertex3f(origin.x + extents.x, origin.y, origin.z+0.1);
    glVertex3f(origin.x + extents.x, origin.y, origin.z);
    glVertex3f(origin.x + extents.x, origin.y, origin.z-0.1);
    // draw y-axis
    glColor3f(ycol.x, ycol.y, ycol.z);
    glVertex3f(origin.x, origin.y, origin.z);
    glVertex3f(origin.x, origin.y + extents.y, origin.z);

    glVertex3f(origin.x, origin.y + extents.y, origin.z);
    glVertex3f(origin.x, origin.y + extents.y, origin.z+0.1);
    glVertex3f(origin.x, origin.y + extents.y, origin.z);
    glVertex3f(origin.x, origin.y + extents.y, origin.z-0.1);
    // draw z axis
    glColor3f(zcol.x, zcol.y, zcol.z);
    glVertex3f(origin.x, origin.y, origin.z);
    glVertex3f(origin.x, origin.y, origin.z + extents.z);
    
    glVertex3f(origin.x, origin.y, origin.z + extents.z);
    glVertex3f(origin.x+0.1, origin.y, origin.z + extents.z);

    glVertex3f(origin.x, origin.y, origin.z + extents.z);
    glVertex3f(origin.x-0.1, origin.y, origin.z + extents.z);
    glEnd();


    glPopMatrix();
}


void processInput (GLFWwindow* window) {
    double currentXPos = lastXPos;
    double currentYPos = lastYPos;

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetKeyCallback(window, keyboardCallback);
    
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


float f1(float x, float y, float z) {
    return x*x + y*y + z*z;
}

float f2(float x, float y, float z) {
    return sin(x*y*z);
}

float f3(float x, float y, float z) {
    return sin(x)*cos(y)*sin(z);
}

float f4(float x, float y, float z) {
	return y - sin(x)*cos(z);
}

float f5(float x, float y, float z) {
	return x*x - y*y - z*z - z;
}


void render (std::vector<float> vertices, std::vector<float> normalVertices, glm::mat4 MVP) {

    // create shader for object
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string VertexShaderCode = "\
        #version 330 core\n\
        layout (location = 0) in vec3 vertexPosition;\n\
        layout (location = 1) in vec3 vertexNormal;\n\
        out vec3 worldPosition;\n\
        out vec3 worldNormal;\n\
        out vec3 viewDirection;\n\
        uniform mat4 modelMatrix;\n\
        uniform mat4 viewMatrix;\n\
        uniform mat4 projectionMatrix;\n\
        void main()\n\
        {\n\
            worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));\n\
            worldNormal = normalize(mat3(modelMatrix) * vertexNormal);\n\
            vec3 viewPosition = vec3(inverse(viewMatrix) * vec4(0.0, 0.0, 0.0, 1.0));\n\
            viewDirection = normalize(worldPosition - viewPosition);\n\
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);\n\
        }\n";

    std::string FragmentShaderCode = "\
        #version 330 core\n\
        in vec3 worldPosition;\n\
        in vec3 worldNormal;\n\
        in vec3 viewDirection;\n\
        uniform vec3 lightDirection;\n\
        uniform vec3 lightColor;\n\
        uniform float shininess;\n\
        uniform vec3 baseColor;\n\
        out vec4 fragmentColor;\n\
        void main()\n\
        {\n\
            vec3 lightDirectionWorld = normalize(lightDirection);\n\
            float diffuse = max(dot(worldNormal, lightDirectionWorld), 0.0);\n\
            vec3 reflection = reflect(-lightDirectionWorld, worldNormal);\n\
            float specular = pow(max(dot(viewDirection, reflection), 0.0), shininess);\n\
            vec3 ambientColor = vec3(0.1);\n\
            vec3 diffuseColor = lightColor * baseColor * diffuse;\n\
            vec3 specularColor = lightColor * specular;\n\
            vec3 finalColor = ambientColor + diffuseColor + specularColor;\n\
            fragmentColor = vec4(finalColor, 1.0);\n\
        }\n";

    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Compile Fragment Shader
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);
    
    // store shaderID for later use
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, VertexShaderID);
    glAttachShader(shaderID, FragmentShaderID);
    glLinkProgram(shaderID);

    glDetachShader(shaderID, VertexShaderID);
    glDetachShader(shaderID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    GLuint VBO, VAO, NBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glBindVertexArray(VAO);



    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normalVertices.size() * sizeof(float), normalVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Gets location of MVP and sets MVP uniform variable in shader program
    //glUseProgram(shaderProgram);
   // GLuint MatrixID = glGetUniformLocation(shaderID, "MVP");
   // glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
//
    // draw triangles using VAO
    //glBindVertexArray(vao);
    //glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    // cleanup
    //glDeleteVertexArrays(1, &vao);
    //glDeleteBuffers(1, &vboVerts);
    //glDeleteBuffers(1, &vboNormals);
}


std::vector<float> marching_cubes(
        std::function<float(float, float, float)> f,
        float isovalue,
        float min,
        float max,
        float stepsize)
{
    // Compute the number of steps in each direction
    int nsteps = static_cast<int>((max - min) / stepsize);

    // Allocate memory for the vertices
    std::vector<float> vertices;

    // Loop over all cubes in the grid
    for (int i = 0; i < nsteps; ++i)
    {
        for (int j = 0; j < nsteps; ++j)
        {
            for (int k = 0; k < nsteps; ++k)
            {
                // Compute the position of the current cube
                float x0 = min + i * stepsize;
                float y0 = min + j * stepsize;
                float z0 = min + k * stepsize;

                // Compute the isosurface crossings of the current cube
                int cubeindex = 0;
                std::array<float, 8> vals;
                std::array<std::array<float, 3>, 8> verts;

                vals[0] = f(x0, y0, z0);
                vals[1] = f(x0 + stepsize, y0, z0);
                vals[2] = f(x0 + stepsize, y0, z0 + stepsize);
                vals[3] = f(x0, y0, z0 + stepsize);
                vals[4] = f(x0, y0 + stepsize, z0);
                vals[5] = f(x0 + stepsize, y0 + stepsize, z0);
                vals[6] = f(x0 + stepsize, y0 + stepsize, z0 + stepsize);
                vals[7] = f(x0, y0 + stepsize, z0 + stepsize);

                if (vals[0] < isovalue) cubeindex |= 1;
                if (vals[1] < isovalue) cubeindex |= 2;
                if (vals[2] < isovalue) cubeindex |= 4;
                if (vals[3] < isovalue) cubeindex |= 8;
                if (vals[4] < isovalue) cubeindex |= 16;
                if (vals[5] < isovalue) cubeindex |= 32;
                if (vals[6] < isovalue) cubeindex |= 64;
                if (vals[7] < isovalue) cubeindex |= 128;

                // Use the LUTs to generate the vertices for the current cube
                for (int l = 0; marching_cubes_lut[cubeindex][l] != -1; l += 3)
                {
                    int edge0 = marching_cubes_lut[cubeindex][l];
                    int edge1 = marching_cubes_lut[cubeindex][l + 1];
                    int edge2 = marching_cubes_lut[cubeindex][l + 2];

                    float x1 = x0 + vertTable[edge0][0] * stepsize;
                    float y1 = y0 + vertTable[edge0][1] * stepsize;
                    float z1 = z0 + vertTable[edge0][2] * stepsize;
                    float x2 = x0 + vertTable[edge1][0] * stepsize;
                    float y2 = y0 + vertTable[edge1][1] * stepsize;
                    float z2 = z0 + vertTable[edge1][2] * stepsize;

                    float x3 = x0 + vertTable[edge2][0] * stepsize;
                    float y3 = y0 + vertTable[edge2][1] * stepsize;
                    float z3 = z0 + vertTable[edge2][2] * stepsize;

                    vertices.push_back(x1);
                    vertices.push_back(y1);
                    vertices.push_back(z1);

                    vertices.push_back(x2);
                    vertices.push_back(y2);
                    vertices.push_back(z2);

                    vertices.push_back(x3);
                    vertices.push_back(y3);
                    vertices.push_back(z3);
                }
            }
        }
    }
    return vertices;
}

std::vector<float> compute_normals(const std::vector<float>& vertices) {
    std::vector<float> normals(vertices.size(), 0.0f);
    for (int i = 0; i < vertices.size(); i += 9) {
        glm::vec3 v0(vertices[i], vertices[i + 1], vertices[i + 2]);
        glm::vec3 v1(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        glm::vec3 v2(vertices[i + 6], vertices[i + 7], vertices[i + 8]);
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
        normals[i] = normal.x;
        normals[i + 1] = normal.y;
        normals[i + 2] = normal.z;
        normals[i + 3] = normal.x;
        normals[i + 4] = normal.y;
        normals[i + 5] = normal.z;
        normals[i + 6] = normal.x;
        normals[i + 7] = normal.y;
        normals[i + 8] = normal.z;
    }
    return normals;
}


void writePLY(const std::vector<float>& vertices, const std::vector<float>& normals, const std::string& fileName) {
    std::ofstream file(fileName);
    file << "ply\n";
    file << "format ascii 1.0\n";
    file << "element vertex " << vertices.size() / 3 << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "property float nx\n";
    file << "property float ny\n";
    file << "property float nz\n";
    file << "element face " << vertices.size() / 9 << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";
    for (int i = 0; i < vertices.size(); i += 3) {
        file << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << " ";
        file << normals[i] << " " << normals[i + 1] << " " << normals[i + 2] << "\n";
    }
    for (int i = 0; i < vertices.size(); i += 9) {
        file << "3 " << i / 3 << " " << i / 3 + 1 << " " << i / 3 + 2 << "\n";
    }
    file.close();
}



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
    float stepsize = 0.1f;

	float min = -5;
	float max = 5;
	float isoval = 1;

    // to help minimize screen tearing
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwSwapInterval(1);

	window = glfwCreateWindow( screenW, screenH, "Phong Shader", NULL, NULL);
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

    std::vector<float> vertices = marching_cubes(
        f5,
        -1.5,
        min,
        max,
        stepsize);

    
    std::vector<float> normals = compute_normals(vertices);
    /*writePLY(vertices, normalVertices, "F1.ply");
    
    vertices = marching_cubes(
        f4,
        isoval,
        min,
        max,
        stepsize);
    normalVertices = compute_normals(vertices);

    writePLY(vertices, normalVertices, "F4.ply");

    vertices = marching_cubes(
        f5,
        -1.5,
        min,
        max,
        stepsize);
    normalVertices = compute_normals(vertices);

    writePLY(vertices, normalVertices, "F5.ply");*/


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
    //TexturedMesh testF1("F1.ply", "woodobjects.bmp");
    //TexturedMesh testF4("F4.ply", "metalobjects.bmp");
    //TexturedMesh testF5("F5.ply", "woodobjects.bmp");

    // initializes camera position
    glm::vec3 cameraPos = glm::vec3(0.5f, 0.4f, 0.5f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // initializes model-view-projection matrix
    glm::mat4 MVP;
    GLuint shaderProgram =  LoadShaders("PhongShader.vert", "PhongShader.frag");
    

    GLuint VBO, VAO, NBO;
    

    // Loops until the user closes the window or presses the ESC key
	do{
		// Clears the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Processes user input to update camera position and orientation
        processInput(window);

        // Sets the camera view matrix
        glm::mat4 V = camera.getViewMatrix();

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

        drawBoundingBox(min, max);
        drawAxes(glm::vec3(min, min, min), glm::vec3(max * 2, max * 2, max * 2));


        // Compute the vertices for the isosurface
        //std::vector<float> newVertices = marching_cubes(f1, isoval, min, max, stepsize);
        //vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());

        // Compute the normals for the vertices
        //std::vector<float> newNormals = compute_normals(vertices);
        //normals.insert(normals.end(), newNormals.begin(), newNormals.end());


        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &NBO);
        glBindVertexArray(VAO);



        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, NBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);


        glUseProgram(shaderProgram);
        GLuint MatrixID = glGetUniformLocation(shaderProgram, "MVP");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw triangles using VAO
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

        // cleanup
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &NBO);

        // Restores the modelview and projection matrices to their previous states
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        // Poll for and process events
        glfwPollEvents();

        // Swap the front and back buffers
        glfwSwapBuffers(window);

        // Draws the objects in the scene using MVP 
        /*bottlesMesh.draw(MVP);
        floorMesh.draw(MVP);
        patioMesh.draw(MVP);
        tableMesh.draw(MVP);
        wallsMesh.draw(MVP);
        woodObjectsMesh.draw(MVP);
        metalObjectsMesh.draw(MVP);
        doorBackgroundMesh.draw(MVP);
        windowBackgroundMesh.draw(MVP);
        curtainsMesh.draw(MVP);*/
        //testF1.draw(MVP);
        //testF4.draw(MVP);
        //testF5.draw(MVP);


        

        // Swaps the front and back buffers to display the rendered image
        //glfwSwapBuffers(window);
        //glfwPollEvents();


	} // Checks if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Closes OpenGL window and terminates GLFW
	glfwTerminate();

	return 0;
}

