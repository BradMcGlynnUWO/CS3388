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
#include <fstream>
#include <functional>
#include <sstream>
#include <vector>
#include <map>
#include "bmaploader.hpp"
#include "LoadBMP.hpp"
#include "shader.hpp"
#include "PlaneMesh.hpp"
#include "Axes.hpp"

// sgn function for computing phi
template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 lookAt) {
        cameraPosition = position;
        cameraDirection = lookAt;
        radius = glm::length(cameraPosition - cameraDirection);
        theta = glm::acos(position.z / radius);
        phi = sgn(position.y) * glm::acos(position.x / sqrt(pow(position.x, 2) + pow(position.y, 2)));
        
    }

    glm::vec3 getPosition() const {
        return cameraPosition;
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(cameraPosition, cameraDirection, glm::vec3(0, 1, 0));
    }

    // update based on cursor position and direction
    void updateOrientation(float deltaTheta, float deltaPhi) {
        theta += deltaTheta;
        phi += deltaPhi;
        phi = glm::clamp(phi, 0.1f, glm::pi<float>() - 0.1f);

        glm::vec3 relativePos(
            radius * sinf(phi) * cosf(theta),
            radius * cosf(phi),
            radius * sinf(phi) * sinf(theta)
        );

        cameraPosition = cameraDirection + relativePos;
    }

    // update camera based on zoom input from keyboard
    void updateRadius(float deltaRadius) {
        radius += deltaRadius;
        if (radius <= 0) {
            radius = 0.1f;
        }

        glm::vec3 relativePos(
            radius * sinf(phi) * cosf(theta),
            radius * cosf(phi),
            radius * sinf(phi) * sinf(theta)
        );

        cameraPosition = cameraDirection + relativePos;
    }

private:
    glm::vec3 cameraPosition;
    glm::vec3 cameraDirection;
    float theta;
    float phi;
    float radius;
};
// Globals, needed for callback functions
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
    // updates camera orientation based on cursor position / direction of movement in the window
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
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetKeyCallback(window, keyboardCallback);
    
}

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
    std::vector<GLfloat> vertexTextureCoordinates;
    std::vector<GLfloat> vertexPositions;
    std::vector<GLfloat> normalPositions;
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    //glm::vec3 rotation;

    GLuint vao; 
    GLuint vboVertexPosition; 
    GLuint vboTextureCoordinates; 
    GLuint vboNormalPosition;
    GLuint eboFacesIndices; 
    GLuint textureID; 
    GLuint shaderID; 
    GLuint framebuffer;
    glm::vec3 position;
    
public:
    TexturedMesh(const char* plyFilePath, const char* bmpFilePath, GLuint shaderID) {
        // read PLY files and store date in appropriate vectors
        this->shaderID = shaderID;
        readPLYFile(plyFilePath, vertices, faces);
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

        for (const auto& vertex : vertices) {
            normalPositions.push_back(vertex.normal.x);
            normalPositions.push_back(vertex.normal.y);
            normalPositions.push_back(vertex.normal.z);
		}
        // read bmp files and store results in appropriate variables
        GLuint width, height;
        unsigned char* data;
        loadARGB_BMP(bmpFilePath, &data, &width, &height);


        // create texture bitmap
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        glUseProgram(shaderID);
        glUniform1i(glGetUniformLocation(shaderID, "tex"), 3); // bind the texture to texture unit 0
		glActiveTexture(GL_TEXTURE3); // activate texture unit 0
		glBindTexture(GL_TEXTURE_2D, textureID); // bind the texture object to the currently active texture unit
        glUseProgram(0);
        
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
        glGenBuffers(1, &vboNormalPosition);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormalPosition);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normalPositions.size(), normalPositions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);

        // creates attribute buffer for texture coordinates       
        glGenBuffers(1, &vboTextureCoordinates);
        glBindBuffer(GL_ARRAY_BUFFER, vboTextureCoordinates);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexTextureCoordinates.size(), vertexTextureCoordinates.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                         // array buffer offset
		);
        
        // creates element buffer for indices of faces vertex
        glGenBuffers(1, &eboFacesIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFacesIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * faces.size(), faces.data(), GL_STATIC_DRAW);   
        glBindVertexArray(0);
        
    }

    void updateVerticesandNormals(glm::vec3 displacedPosition, glm::vec3 displacedNormals) {
        for (int i = 0; i < vertexPositions.size() - 2; i++) {
            vertexPositions[i] = vertexPositions[i] + displacedPosition.x;
            vertexPositions[i + 1] = vertexPositions[i + 1] + displacedPosition.y;
            vertexPositions[i + 2] = vertexPositions[i + 2] + displacedPosition.z;

        }

        for (int i = 0; i < normalPositions.size() - 2; i++) {
            normalPositions[i] = normalPositions[i] + displacedNormals.x;
            normalPositions[i + 1] = normalPositions[i + 1] + displacedNormals.y;
            normalPositions[i + 2] = normalPositions[i + 2] + displacedNormals.z;

        }


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
        glGenBuffers(1, &vboNormalPosition);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormalPosition);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normalPositions.size(), normalPositions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);

        // creates attribute buffer for texture coordinates       
        glGenBuffers(1, &vboTextureCoordinates);
        glBindBuffer(GL_ARRAY_BUFFER, vboTextureCoordinates);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexTextureCoordinates.size(), vertexTextureCoordinates.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                         // array buffer offset
		);
        
        // creates element buffer for indices of faces vertex
        glGenBuffers(1, &eboFacesIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboFacesIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * faces.size(), faces.data(), GL_STATIC_DRAW);   
        glBindVertexArray(0);
    }

    glm::vec3 getPosition() const {
        return glm::vec3(modelMatrix[3]);
    }

    void setPosition(const glm::vec3& position) {
        modelMatrix[3] = glm::vec4(position, 1.0f);
    }

    void move(GLFWwindow* window) {
        float speed = 0.5f; // Adjust the speed as needed
        glm::vec3 movement = glm::vec3(0.0f);

        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            glm::vec3 position = glm::vec3(modelMatrix[2]);
            movement -= speed * position; // move object forwards
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            glm::vec3 position = glm::vec3(modelMatrix[2]);
            movement += speed * position; // move object backwards
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            glm::vec3 position = glm::vec3(modelMatrix[0]);
            movement -= speed * position; // move object to left
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            glm::vec3 position = glm::vec3(modelMatrix[0]);
            movement += speed * position; // move object to right
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            glm::vec3 position = glm::vec3(modelMatrix[1]);
            movement += speed * position; // move object up
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            glm::vec3 position = glm::vec3(modelMatrix[1]);
            movement -= speed * position; // move object down
        }
            
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            modelMatrix = glm::rotate(modelMatrix, glm::radians(speed * 15), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate object to left
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            modelMatrix = glm::rotate(modelMatrix, glm::radians(-speed * 15), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate object to right


        glm::vec3 currentPosition = getPosition();
        currentPosition += movement;
        setPosition(currentPosition);
    }

    void setRotation(const glm::vec3 &normal) {
        //this->rotation = normal;
    }

    void draw(glm::mat4 P, glm::mat4 V) {

        // Enables blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        

        // Apply rotation based on the normal vector
        //float angle = acos(glm::dot(glm::vec3(0, 1, 0), rotation));
        //glm::vec3 axis = glm::normalize(glm::cross(glm::vec3(0, 1, 0), rotation));
        //modelMatrix = glm::rotate(modelMatrix, angle, axis);
        
        // Gets location of MVP and sets MVP uniform variable in shader program
        glUseProgram(shaderID);
        GLuint ProjectionID = glGetUniformLocation(shaderID, "P");
        glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &P[0][0]);
        GLuint ViewID = glGetUniformLocation(shaderID, "V");
        glUniformMatrix4fv(ViewID, 1, GL_FALSE, &V[0][0]);
        GLuint ModelID = glGetUniformLocation(shaderID, "M");
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &modelMatrix[0][0]);

        // Enables 2D texturing and binds the texture
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Bind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Binds the vao
        glBindVertexArray(vao);

        // Draws the object using triangles
        glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);

        // Unbinds the vao
        glBindVertexArray(0);

        // Unbind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Clears out attributes and disables blending
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_BLEND);
            
        }

};


const unsigned int SCR_WIDTH = 1400;
const unsigned int SCR_HEIGHT = 900;


float getDisplacementFromTexture(float x, float z, int textureWidth, int textureHeight, GLuint displacementTexture) {
    int texX = static_cast<int>((x + 0.5f) * textureWidth);
    int texY = static_cast<int>((z + 0.5f) * textureHeight);
    texX = std::max(0, std::min(texX, textureWidth - 1));
    texY = std::max(0, std::min(texY, textureHeight - 1));
    
    float displacement;
    glBindTexture(GL_TEXTURE_2D, displacementTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &displacement);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return displacement;
}



int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Water Simulation", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }



    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up shaders
    GLuint shaderProgram = LoadShaders("Shaders/vertex_shader.glsl", "Shaders/geometry_shader.glsl", "Shaders/fragment_shader.glsl");

    // Set up plane mesh object
    float min = -10.0f;
    float max = 10.0f;
    float stepsize = 2.0f;

    glm::vec4 modelColor = glm::vec4(0.0, 0.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = (glm::vec3(5.0f, 5.0f, 5.0f));
    PlaneMesh planeMesh(min, max, stepsize, shaderProgram);


    GLuint plyShader = LoadShaders("PhongShader.vert", "PhongShader.frag");

    glm::mat4 V = camera.getViewMatrix();

    // Sets the projection matrix
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

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
    glm::mat4 MVP = Projection * V * M;
    
    
    // use the shader program
    glUseProgram(plyShader);

    

    // set up uniform variables
    GLuint plyMatrixID = glGetUniformLocation(plyShader, "MVP");
    glUniformMatrix4fv(plyMatrixID, 1, GL_FALSE, &MVP[0][0]);

    GLuint plyViewID = glGetUniformLocation(plyShader, "V");
    glUniformMatrix4fv(plyViewID, 1, GL_FALSE, &V[0][0]);

    GLuint plyModelID = glGetUniformLocation(plyShader, "M");
    glUniformMatrix4fv(plyModelID, 1, GL_FALSE, &M[0][0]);

    GLuint plyLightID = glGetUniformLocation(plyShader, "lightPos");
    glUniform3fv(plyLightID, 1, &lightPos[0]);

    GLuint plyambientColorID = glGetUniformLocation(plyShader, "modelColor");
    glUniform4fv(plyambientColorID, 1, &modelColor[0]);


    GLuint plyshininessID = glGetUniformLocation(plyShader, "alpha");
    glUniform1f(plyshininessID, 64.0f);
	
	TexturedMesh boat("Assets/boat.ply", "Assets/boat.bmp", plyShader);
	TexturedMesh head("Assets/head.ply", "Assets/head.bmp", plyShader);
	TexturedMesh eyes("Assets/eyes.ply", "Assets/eyes.bmp", plyShader);

    // Dark blue background
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


    // Main loop
    do {


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        processInput(window);

        double currentTime = glfwGetTime();
		static double lastTime = glfwGetTime();
		float deltaTime = ( currentTime - lastTime );

        boat.move(window);
        eyes.move(window);
        head.move(window);
        

        // Sets the camera view matrix
        glm::mat4 V = camera.getViewMatrix();

        
        // Sets the projection matrix
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

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
        glm::mat4 MVP = Projection * V * M;

        
        // Render the planeMesh
        planeMesh.draw(V, Projection, M, lightPos, deltaTime);

        

        

        boat.draw(Projection, V);
		head.draw(Projection, V);
		eyes.draw(Projection, V);

        glUseProgram(0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();

    // Check if the ESC key was pressed or the window was closed
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 ); 

    // Clean up resources
    glDeleteVertexArrays(1, &planeMesh.VAO);
    glDeleteBuffers(1, &planeMesh.VBO);
    glDeleteBuffers(1, &planeMesh.EBO);

    // Clean up and exit
    glfwTerminate();
    return 0;
}