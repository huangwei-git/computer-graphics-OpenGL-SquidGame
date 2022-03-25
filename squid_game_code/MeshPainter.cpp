#include "MeshPainter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

MeshPainter::MeshPainter(){};
MeshPainter::~MeshPainter(){};

std::vector<std::string> MeshPainter::getMeshNames(){ return mesh_names;};
std::vector<TriMesh *> MeshPainter::getMeshes(){ return meshes;};
std::vector<openGLObject> MeshPainter::getOpenGLObj(){ return opengl_objects;};

void MeshPainter::bindObjectAndData(TriMesh *mesh, openGLObject &object, const std::string &texture_image, const std::string &vshader, const std::string &fshader){
    // Initialize various objects

    std::vector<glm::vec3> points = mesh->getPoints();
    std::vector<glm::vec3> normals = mesh->getNormals();
    std::vector<glm::vec3> colors = mesh->getColors();
    std::vector<glm::vec2> textures = mesh->getTextures();

	// Create a vertex array object
	glGenVertexArrays(1, &object.vao);  	// Assign 1 vertex array object
	glBindVertexArray(object.vao);  	// Bind vertex array object


	// Create and initialize vertex cache objects
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 points.size() * sizeof(glm::vec3) +
                     normals.size() * sizeof(glm::vec3) +
                     colors.size() * sizeof(glm::vec3) +
                     textures.size() * sizeof(glm::vec2),
                 NULL, GL_STATIC_DRAW);

    // Bind vertex data
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::vec3), points.data());
    // Bind color data
    glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), colors.size() * sizeof(glm::vec3), colors.data());
    // Bind normal vector data
    glBufferSubData(GL_ARRAY_BUFFER, (points.size() + colors.size()) * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3), normals.data());
    // Bind texture data
    glBufferSubData(GL_ARRAY_BUFFER, (points.size() + normals.size() + colors.size()) * sizeof(glm::vec3), textures.size() * sizeof(glm::vec2), textures.data());


	object.vshader = vshader;
	object.fshader = fshader;
	object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());

    // Pass vertices into shader
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    // Pass color into shader
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(glm::vec3)));

    // Pass normal vector into shader
	object.nLocation = glGetAttribLocation(object.program, "vNormal");
	glEnableVertexAttribArray(object.nLocation);
	glVertexAttribPointer(object.nLocation, 3, 
		GL_FLOAT, GL_FALSE, 0, 
		BUFFER_OFFSET( (points.size() + colors.size())  * sizeof(glm::vec3)));

	object.tLocation = glGetAttribLocation(object.program, "vTexture");
	glEnableVertexAttribArray(object.tLocation);
	glVertexAttribPointer(object.tLocation, 2, 
		GL_FLOAT, GL_FALSE, 0, 
		BUFFER_OFFSET( ( points.size() + colors.size() + normals.size())  * sizeof(glm::vec3)));


	// Get matrix position
	object.modelLocation = glGetUniformLocation(object.program, "model");
	object.viewLocation = glGetUniformLocation(object.program, "view");
	object.projectionLocation = glGetUniformLocation(object.program, "projection");

	object.shadowLocation = glGetUniformLocation(object.program, "isShadow");

    // Number of texture pictures read
    object.texture_image = texture_image;
    // Create cached objects for textures
    glGenTextures(1, &object.texture);
    // Call STB_ Image generation texture
    load_texture_STBImage(object.texture_image, object.texture);
    
    // Clean up
    glUseProgram(0);
    glBindVertexArray(0);
};


void MeshPainter::bindLightAndMaterial( TriMesh* mesh, openGLObject &object, Light* light, Camera* camera ) {
    // Transfer data such as materials and light sources to shaders
    
    // Transfer camera position
    glUniform3fv(glGetUniformLocation(object.program, "eye_position"), 1, &camera->eye[0]);

    // Transfer the material of the object
    glm::vec4 meshAmbient = mesh->getAmbient();
    glm::vec4 meshDiffuse = mesh->getDiffuse();
    glm::vec4 meshSpecular = mesh->getSpecular();
    float meshShininess = mesh->getShininess();

    glUniform4fv(glGetUniformLocation(object.program, "material.ambient"), 1, &meshAmbient[0]);
    glUniform4fv(glGetUniformLocation(object.program, "material.diffuse"), 1, &meshDiffuse[0]);
    glUniform4fv(glGetUniformLocation(object.program, "material.specular"), 1, &meshSpecular[0]);
    glUniform1f(glGetUniformLocation(object.program, "material.shininess"), meshShininess);


    // Transfer light source information
    glm::vec4 lightAmbient = light->getAmbient();
    glm::vec4 lightDiffuse = light->getDiffuse();
    glm::vec4 lightSpecular = light->getSpecular();
    glm::vec3 lightPosition = light->getTranslation();
    glUniform4fv(glGetUniformLocation(object.program, "light.ambient"), 1, &lightAmbient[0]);
    glUniform4fv(glGetUniformLocation(object.program, "light.diffuse"), 1, &lightDiffuse[0]);
    glUniform4fv(glGetUniformLocation(object.program, "light.specular"), 1, &lightSpecular[0]);
    glUniform3fv(glGetUniformLocation(object.program, "light.position"), 1, &lightPosition[0]);

    glUniform1f(glGetUniformLocation(object.program, "light.constant"), light->getConstant());
    glUniform1f(glGetUniformLocation(object.program, "light.linear"), light->getLinear());
    glUniform1f(glGetUniformLocation(object.program, "light.quadratic"), light->getQuadratic());

}


void MeshPainter::addMesh( TriMesh* mesh, const std::string &name, const std::string &texture_image, bool havaShadow, const std::string &vshader, const std::string &fshader ){
	mesh_names.push_back(name);
    meshes.push_back(mesh);

    openGLObject object;
    // Bind OpenGL objects and pass the data of vertex attributes
    object.haveShadow = havaShadow;
    bindObjectAndData(mesh, object, texture_image, vshader, fshader);

    opengl_objects.push_back(object);
};



void MeshPainter::drawMesh(TriMesh* mesh, openGLObject &object, Light *light, Camera* camera){
    
    // Camera matrix calculation
	camera->viewMatrix = camera->getViewMatrix();
	camera->projMatrix = camera->getProjectionMatrix(false);


	glBindVertexArray(object.vao);
	glUseProgram(object.program);

	// Transformation matrix of object
    glm::mat4 modelMatrix = mesh->getModelMatrix();
    if (mesh->robot) modelMatrix = mesh->modelMatrix;

	// transfer matrix
	glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(object.viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
	glUniformMatrix4fv(object.projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
	// If shadow is a normal shader, it will be set to IS0
	glUniform1i(object.shadowLocation, 0);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, object.texture);
    // Pass the texture data and the generated texture to the shader
    glUniform1i(glGetUniformLocation(object.program, "texture"), 0);
    
	// Passing material and light data to shaders
	bindLightAndMaterial(mesh, object, light, camera);
	// draw
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());

    // Does the object have shadows
    if (object.haveShadow) {
        glBindVertexArray(object.vao);
        glUseProgram(object.program);
        glm::mat4 m = light->getShadowProjectionMatrix(1, shadowLoc);
        modelMatrix = m * modelMatrix;
        glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(object.viewLocation, 1, GL_FALSE, &camera->viewMatrix[0][0]);
        glUniformMatrix4fv(object.projectionLocation, 1, GL_FALSE, &camera->projMatrix[0][0]);
        glUniform1i(object.shadowLocation, 1);
        glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
        }
  

    
	glBindVertexArray(0);

	glUseProgram(0);
}

// Draw all models
void MeshPainter::drawMeshes(Light *light, Camera* camera){
    for (int i = 0; i < meshes.size(); i++)
    {
        drawMesh(meshes[i], opengl_objects[i], light, camera);
    }
};

//
void MeshPainter::shadowMapDraw(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera,GLuint program) {
    GLuint aPosLocation = glGetAttribLocation(program, "aPos");
    glEnableVertexAttribArray(aPosLocation);
    glVertexAttribPointer(aPosLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glBindVertexArray(object.vao);
    glUseProgram(program);

    // Transformation matrix of object
    glm::mat4 modelMatrix = mesh->getModelMatrix();

    // transfer matrix
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    // 将着色器 ‘isShadow’ 设置为零，表示正常绘制的颜色，如果是一着表示阴影
    glUniform1i(object.shadowLocation, 0);


    // Passing material and light data to shaders
    bindLightAndMaterial(mesh, object, light, camera);
    // draw
    glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());

    glBindVertexArray(0);

    glUseProgram(0);
}

void MeshPainter::shadowMapDraws(Light* light, Camera* camera,GLuint program) {
    for (int i = 0; i < meshes.size(); i++)
    {
        shadowMapDraw(meshes[i], opengl_objects[i], light, camera, program);
    }
};

void MeshPainter::cleanMeshes(){
    // Clear and release all data
    mesh_names.clear();

    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->cleanData();

        delete meshes[i];
        meshes[i] = NULL;
        glDeleteVertexArrays(1, &opengl_objects[i].vao);
        glDeleteBuffers(1, &opengl_objects[i].vbo);
        glDeleteProgram(opengl_objects[i].program);
    }

    meshes.clear();
    opengl_objects.clear();
};


void MeshPainter::load_texture_STBImage(const std::string &file_name, GLuint& texture){
    // Read the texture picture and pass it to the shader

	int width, height, channels = 0;
    unsigned char *pixels = NULL;
    // When reading the picture, first flip the picture. If it is not set, the reverse picture will be displayed
    stbi_set_flip_vertically_on_load(true);
    // Read picture data
    pixels = stbi_load(file_name.c_str(), &width, &height, &channels, 0);

    // Adjust line alignment format
    if (width * channels % 4 != 0)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLenum format = GL_RGB;
    // Format channel
    switch (channels)
    {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default:
        format = GL_RGB;
        break;
    }

    // Bind texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Specifies the amplification and reduction filtering of the texture, 
    // and uses the linear method, that is, the interpolation method when the image is enlarged
    // Upload the RGB data of the picture to opengl
    glTexImage2D(
        GL_TEXTURE_2D,    // Specifies the target texture. This value must be GL_TEXTURE_2D
        0,                // Execute the level of detail. 0 is the most basic image level, and N represents the nth level of map refinement
        format,           // Color format of texture data (GPU video memory)
        width,            //Width. Early graphics cards did not support irregular textures, so the width and height must be the same 2^n
        height,           // Height. Early graphics cards did not support irregular textures, so the width and height must be the same 2^n
        0,                // Specifies the width of the border. Must be 0
        format,           // Color format of pixel data (CPU memory)
        GL_UNSIGNED_BYTE, // Specifies the data type of pixel data
        pixels            // Specifies the pointer to the image data in memory
    );

    // Generate multi-level fade texture, consume 1 / 3 more video memory, and get better effect when the resolution is small

    // Specifies the interpolation method
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Restore initial alignment format
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    // Freeing graphics memory
    stbi_image_free(pixels);
};