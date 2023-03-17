#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>
#include "TriTable.hpp"

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
            radius_ = 1.0f;
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
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetKeyCallback(window, keyboardCallback);
    
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


struct Vertex {
    float x, y, z;
};

/*std::vector<float> marching_cubes(std::function<float(float, float, float)> f, float isovalue, float min, float max, float stepsize) {
    std::vector<float> vertices;

    // Compute the number of cubes along each axis
    int n_cubes = static_cast<int>((max - min) / stepsize) + 1;

    // Loop over each cube in the grid
    for (int i = 0; i < n_cubes; i++) {
        for (int j = 0; j < n_cubes; j++) {
            for (int k = 0; k < n_cubes; k++) {
                // Compute the position of the current cube
                float x = min + i * stepsize;
                float y = min + j * stepsize;
                float z = min + k * stepsize;

                // Evaluate the scalar field at the vertices of the cube
                float values[8] = {
                    f(x, y, z),
                    f(x + stepsize, y, z),
                    f(x + stepsize, y + stepsize, z),
                    f(x, y + stepsize, z),
                    f(x, y, z + stepsize),
                    f(x + stepsize, y, z + stepsize),
                    f(x + stepsize, y + stepsize, z + stepsize),
                    f(x, y + stepsize, z + stepsize)
                };

                // Compute the signs of the field values
                int signs = 0;
                for (int v = 0; v < 8; v++) {
                    if (values[v] < isovalue) {
                        signs |= 1 << v;
                    }
                }

                // Determine which edges are intersected by the isosurface
                int edge_mask = edge_table[signs];

                // If the cube is entirely inside or outside the isosurface, skip it
                if (edge_mask == 0 || edge_mask == 255) {
                    continue;
                }

                // Compute the positions of the vertices on the intersected edges
                Vertex verts[12];
                for (int e = 0; e < 12; e++) {
                    if (edge_mask & (1 << e)) {
                        int v1 = corner_index[e][0];
                        int v2 = corner_index[e][1];
                        float lerp = (isovalue - values[v1]) / (values[v2] - values[v1]);
                        verts[e].x = x + vertex_offset[v1][0] + lerp * edge_direction[e][0];
                        verts[e].y = y + vertex_offset[v1][1] + lerp * edge_direction[e][1];
                        verts[e].z = z + vertex_offset[v1][2] + lerp * edge_direction[e][2];
                    }
                }

                // Add the vertices to the output list
                for (int t = 0; tri_table[signs][t] != -1; t += 3) {
                    for (int v = 0; v < 3; v++) {
                        int idx = tri_table[signs][t + v];
                        float x = x0 + vertex_offset[idx][0] * stepsize;
                        float y = y0 + vertex_offset[idx][1] * stepsize;
                        float z = z0 + vertex_offset[idx][2] * stepsize;
                        vertices.push_back(x);
                        vertices.push_back(y);
                        vertices.push_back(z);
                        }
                    }
                }
            }
        }
    return vertices;
} */


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

void drawCube(float min, float max) {
    // Set the color to white
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw the edges of the cube
    glBegin(GL_LINES);

    // Bottom edges
    glVertex3f(min, min, min);
    glVertex3f(max, min, min);

    glVertex3f(max, min, min);
    glVertex3f(max, min, max);

    glVertex3f(max, min, max);
    glVertex3f(min, min, max);

    glVertex3f(min, min, max);
    glVertex3f(min, min, min);

    // Top edges
    glVertex3f(min, max, min);
    glVertex3f(max, max, min);

    glVertex3f(max, max, min);
    glVertex3f(max, max, max);

    glVertex3f(max, max, max);
    glVertex3f(min, max, max);

    glVertex3f(min, max, max);
    glVertex3f(min, max, min);

    // Vertical edges
    glVertex3f(min, min, min);
    glVertex3f(min, max, min);

    glVertex3f(max, min, min);
    glVertex3f(max, max, min);

    glVertex3f(max, min, max);
    glVertex3f(max, max, max);

    glVertex3f(min, min, max);
    glVertex3f(min, max, max);

    glEnd();
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



float f1(float x, float y, float z) {
    return x*x + y*y + z*z;
}

float f2(float x, float y, float z) {
    return sin(x*y*z);
}

float f3(float x, float y, float z) {
    return sin(x)*cos(y)*sin(z);
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

    // generate VAO and VBOs
    GLuint vao, vboVerts, vboNormals, eboIndices;
    std::vector<int> indices;
    for (int i = 0; i < vertices.size(); ++i) {
        indices.push_back(i);
    }


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // generate VBO for vertex positions
    glGenBuffers(1, &vboVerts);
    glBindBuffer(GL_ARRAY_BUFFER, vboVerts);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // generate VBO for vertex normals
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normalVertices.size() * sizeof(GLfloat), normalVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // creates element buffer for indices of faces vertex
    glGenBuffers(1, &eboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * indices.size(), indices.data(), GL_STATIC_DRAW);   
    glBindVertexArray(0);

    // Gets location of MVP and sets MVP uniform variable in shader program
    glUseProgram(shaderID);
    GLuint MatrixID = glGetUniformLocation(shaderID, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw triangles using VAO
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    // cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vboVerts);
    glDeleteBuffers(1, &vboNormals);
}

int main (int argc, char* argv[]){

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
		min = atof(argv[4]);
	}
	if (argc > 5) {
		max = atof(argv[5]);
	}
	if (argc > 6) {
		isoval = atof(argv[6]);
	}

    // to help minimize screen tearing
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwSwapInterval(1);

	GLFWwindow* window = glfwCreateWindow( screenW, screenH, "Assignment 5", NULL, NULL);
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

    
    

    // initializes model-view-projection matrix
    glm::mat4 MVP;


    std::vector<float> vertices = marching_cubes(
        f1,
        isoval,
        min,
        max,
        stepsize);

    
    std::vector<float> normalVertices = compute_normals(vertices);

    #include <fstream>


    // Open the file
    std::ofstream outputFile("vertices.txt");
    outputFile << vertices.size() << "\n";
    // Print the vertices to the console and to the file
    for (const auto& vertex : vertices) {
        outputFile << vertex << " ";
        outputFile << "\n";
    }
    outputFile << std::endl;
    // Close the file
    outputFile.close();

    // Open the file
    std::ofstream outputFile1("normalVertices.txt");
    outputFile1 << normalVertices.size() << "\n";
    // Print the normal vertices to the console and to the file
    for (const auto& vertex : normalVertices) {
        outputFile1 << vertex << " ";
        outputFile1 << "\n";
    }
    outputFile1 << std::endl;

    // Close the file
    outputFile1.close();


    writePLY(vertices, normalVertices, "F1.ply");

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
        //render(vertices, normalVertices, MVP);

        // Restores the modelview and projection matrices to their previous states
        //glPopMatrix();
        //glMatrixMode(GL_PROJECTION);
        //glPopMatrix();
        //glMatrixMode(GL_MODELVIEW);

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

