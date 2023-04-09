#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class PlaneMesh {
private:

	GLuint shaderProgram;
    GLuint modelLoc, viewLoc, projectionLoc, MVPLoc, lightPosLoc, cameraViewPos;
    GLuint waterTextureLoc, displacementTextureLoc, timeLoc;
    GLuint waterTexture, displacementTexture;

    void setupUniforms() {
        modelLoc = glGetUniformLocation(this->shaderProgram, "model");
        viewLoc = glGetUniformLocation(this->shaderProgram, "view");
        projectionLoc = glGetUniformLocation(this->shaderProgram, "projection");
        lightPosLoc = glGetUniformLocation(this->shaderProgram, "lightPos");
        cameraViewPos = glGetUniformLocation(this->shaderProgram, "cameraView");
        timeLoc = glGetUniformLocation(this->shaderProgram, "time");
    }
    
    void setupTextures() {
        GLuint width, height;
        unsigned char* data;
        loadBMP("Assets/water.bmp", &data, &width, &height);


        // Generate texture object
        glGenTextures(1, &waterTexture);
        glBindTexture(GL_TEXTURE_2D, waterTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);


        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(this->shaderProgram);

        waterTextureLoc = glGetUniformLocation(this->shaderProgram, "waterTexture");
        glUniform1i(waterTextureLoc, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterTexture);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        free(data);

        loadBMP("Assets/displacement-map1.bmp", &data, &width, &height);

        // Generate texture object
        glGenTextures(1, &displacementTexture);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(this->shaderProgram);

        displacementTextureLoc = glGetUniformLocation(this->shaderProgram, "disptex");
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(displacementTextureLoc, 1);
        
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        free(data);

    }


    void setupBuffers() {

        

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &NBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        
        // Position attribute
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float) * 3, &verts[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        

        // Normal attribute
        glBindBuffer(GL_ARRAY_BUFFER, NBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(int) * 3, &normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), &indices[0], GL_STATIC_DRAW);


        glBindVertexArray(0);
    }

public:
	std::vector<float> verts;
    std::vector<float> normals;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO, NBO;
    PlaneMesh(float min, float max, float stepsize, GLuint shaderProgram) {

		this->shaderProgram = shaderProgram;
        planeMeshQuads(min, max, stepsize);
        setupBuffers();
        setupUniforms();
        setupTextures();
    }

    void planeMeshQuads(float min, float max, float stepsize) {

		// The following coordinate system works as if (min, 0, min) is the origin
		// And then builds up the mesh from that origin down (in z)
		// and then to the right (in x).
		// So, one "row" of the mesh's vertices have a fixed x and increasing z

		//manually create a first column of vertices
		float x = min;
		float y = 0;
		for (float z = min; z <= max; z += stepsize) {
			verts.push_back(x);
			verts.push_back(y);
			verts.push_back(z);
			normals.push_back(0);
			normals.push_back(1);
			normals.push_back(0);
		}

		for (float x = min+stepsize; x <= max; x += stepsize) {
			for (float z = min; z <= max; z += stepsize) {
				verts.push_back(x);
				verts.push_back(y);
				verts.push_back(z);
				normals.push_back(0);
				normals.push_back(1);
				normals.push_back(0);
			}
		}

		int nCols = (max-min)/stepsize + 1;
		int i = 0, j = 0;
		for (float x = min; x < max; x += stepsize) {
			j = 0;
			for (float z = min; z < max; z += stepsize) {
				indices.push_back(i*nCols + j);
				indices.push_back(i*nCols + j + 1);
				indices.push_back((i+1)*nCols + j + 1);
				indices.push_back((i+1)*nCols + j);
				++j;
			}
			++i;
		}

        GLint max_vertices, max_indices, max_patches;
        glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &max_vertices);
        glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &max_indices);
        glGetIntegerv(GL_MAX_PATCH_VERTICES, &max_patches);

        std::cout << max_vertices << " " << verts.size() << " " << max_indices << " " << indices.size() << " " << "\n";

	}

    void draw(glm::mat4 view, glm::mat4 projection, glm::mat4 model, glm::vec3 lightPos, float time, vec3 cameraView) {
        glUseProgram(this->shaderProgram);
        glBindVertexArray(VAO);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(cameraViewPos, 1, glm::value_ptr(cameraView));
        glUniform1f(timeLoc, time);

        // Bind the water texture
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterTexture);

        // Bind the displacement texture
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, displacementTexture);
        
        glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glUseProgram(0);
}

};
