
class PlaneMesh {
private:	
	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<int> indices;
	float min;
	float max;
	glm::vec4 modelColor;
	GLuint numVerts;
	GLuint numIndices;
	GLuint shaderProgram;
	GLuint waterTextureID;
	GLuint displacementID;
	GLuint VAO;


	

public:

	PlaneMesh(float min, float max, float stepsize, GLuint shaderProgram) {
		this->min = min;
		this->max = max;
		this->shaderProgram = shaderProgram;
		this->modelColor = glm::vec4(0, 1.0f, 1.0f, 1.0f);

		planeMeshQuads(min, max, stepsize);
		numVerts = verts.size()/3;
		numIndices = indices.size();

		GLuint VBO, NBO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &NBO);
		glBindVertexArray(VAO);

		// bind vertices info
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		

		// bind normals info
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    	

		// creates element buffer for indices of faces vertex
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * 3 * indices.size(), &(indices[0]), GL_STATIC_DRAW);   
        glBindVertexArray(0);

		GLuint width, height;
        unsigned char* data;
        loadBMP("Assets/water.bmp", &data, &width, &height);

		// create texture bitmap
        glGenTextures(1, &waterTextureID);
        glBindTexture(GL_TEXTURE_2D, waterTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(shaderProgram); // activate the shader program that uses the sampler2D uniform
		glUniform1i(glGetUniformLocation(shaderProgram, "waterTexture"), 1); // bind the texture to texture unit 1
		glActiveTexture(GL_TEXTURE1); // activate texture unit 1
		glBindTexture(GL_TEXTURE_2D, waterTextureID); // bind the texture object to the currently active texture unit



		free(data);
		loadBMP("Assets/displacement-map1.bmp", &data, &width, &height);
		// create texture bitmap
        glGenTextures(1, &displacementID);
        glBindTexture(GL_TEXTURE_2D, displacementID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(shaderProgram); // activate the shader program that uses the sampler2D uniform
		glUniform1i(glGetUniformLocation(shaderProgram, "disptex"), 0); // bind the texture to texture unit 0
		glActiveTexture(GL_TEXTURE0); // activate texture unit 0
		glBindTexture(GL_TEXTURE_2D, displacementID); // bind the texture object to the currently active texture unit


		
		//gen and fill buffers
		//vertex attribute pointers
		//shaders and uniforms

		

		
	}

	void draw(glm::vec3 lightPos, glm::mat4 V, glm::mat4 P) {

		// Enables blending
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(this->shaderProgram);
		GLuint ViewID = glGetUniformLocation(shaderProgram, "V");
		glUniformMatrix4fv(ViewID, 1, GL_FALSE, &V[0][0]);

		GLuint ProjectionID = glGetUniformLocation(shaderProgram, "P");
		glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &P[0][0]);

		double currentTime = glfwGetTime();
		static double lastTime = glfwGetTime();
		float deltaTime = ( currentTime - lastTime );
		GLuint TimeID = glGetUniformLocation(shaderProgram, "time");
		glUniform1f(TimeID, deltaTime);

		// Enables 2D texturing and binds the texture
        glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterTextureID);
		glEnable(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, displacementID);

		// draw triangles using VAO
		glBindVertexArray(VAO);
	
		glDrawElements(GL_PATCHES, numIndices, GL_UNSIGNED_INT, (void*)0);

		// Unbinds the vao
        glBindVertexArray(0);
        
        // Clears out attributes and disables blending
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);

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
	}


};


