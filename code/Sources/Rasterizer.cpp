// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "Rasterizer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h> // to be able to use the time
#include "Resources.h"
#include "Error.h"

void Rasterizer::init(const std::string &basePath, const std::shared_ptr<Scene> scenePtr)
{
	glCullFace(GL_BACK);						  // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable(GL_CULL_FACE);					  // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc(GL_LESS);					  // Specify the depth test for the z-buffer
	glEnable(GL_DEPTH_TEST);				  // Enable the z-buffer test in the rasterization
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
	// GPU resources
	initScreenQuad();
	loadShaderProgram(basePath);
	initDisplayedImage();
	// Allocate GPU ressources for the heavy data components of the scene
	size_t numOfMeshes = scenePtr->numOfMeshes();
	for (size_t i = 0; i < numOfMeshes; i++)
		m_vaos.push_back(toGPU(scenePtr->mesh(i)));
}

void Rasterizer::setResolution(int width, int height)
{
	glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
	m_width = width;
	m_height = height;
	// adapt the size of the textures to the size of the window
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, colorTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Rasterizer::loadShaderProgram(const std::string &basePath)
{
	m_pbrShaderProgramPtr.reset();
	try
	{
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_pbrShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "/CalculationVertexShader.glsl",
																						 shaderPath + "/PBRFragmentShader.glsl");
	}
	catch (std::exception &e)
	{
		exitOnCriticalError(std::string("[Error loading shader program]") + e.what());
	}
	m_displayShaderProgramPtr.reset();
	try
	{
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_displayShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "/DisplayVertexShader.glsl",
																							  shaderPath + "/DisplayFragmentShader.glsl");
		m_displayShaderProgramPtr->set("imageTex", 0);
	}
	catch (std::exception &e)
	{
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}
	m_ZBufferShaderProgramPtr.reset();
	try
	{
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_ZBufferShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "/CalculationVertexShader.glsl",
																							  shaderPath + "/ZBufferFragmentShader.glsl");
	}
	catch (std::exception &e)
	{
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}
	m_DisplayZBufferShaderProgramPtr.reset();
	try
	{
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_DisplayZBufferShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "/DisplayVertexShader.glsl",
																										shaderPath + "/DisplayZBufferFragmentShader.glsl");
		m_DisplayZBufferShaderProgramPtr->set("imageTexZ", 0);
	}
	catch (std::exception &e)
	{
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}
	m_AmbientOcclusionShaderProgramPtr.reset();
	try
	{
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_AmbientOcclusionShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "/DisplayVertexShader.glsl",
																										  shaderPath + "/AmbientOcclusionFragmentShader.glsl");
		m_AmbientOcclusionShaderProgramPtr->set("imageTexZ", 0);
		m_AmbientOcclusionShaderProgramPtr->set("imageTexNormal", 1);
	}
	catch (std::exception &e)
	{
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}
	m_AmbientOcclusionPRBShaderProgramPtr.reset();
	try
	{
		std::string shaderPath = basePath + "/" + SHADER_PATH;
		m_AmbientOcclusionPRBShaderProgramPtr = ShaderProgram::genBasicShaderProgram(shaderPath + "/DisplayVertexShader.glsl",
																											  shaderPath + "/AmbientOcclusionPRBFragmentShader.glsl");
		m_AmbientOcclusionPRBShaderProgramPtr->set("imageTexZ", 0);
		m_AmbientOcclusionPRBShaderProgramPtr->set("imageTexNormal", 1);
		m_AmbientOcclusionPRBShaderProgramPtr->set("imageTexColor", 2);
	}
	catch (std::exception &e)
	{
		exitOnCriticalError(std::string("[Error loading display shader program]") + e.what());
	}
}

void Rasterizer::updateDisplayedImageTexture(std::shared_ptr<Image> imagePtr)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, m_displayImageTex);
	// Uploading the image data to GPU memory
	glTexImage2D(
		 GL_TEXTURE_2D,
		 0,
		 GL_RGB, // We assume only greyscale or RGB pixels
		 static_cast<GLsizei>(imagePtr->width()),
		 static_cast<GLsizei>(imagePtr->height()),
		 0,
		 GL_RGB, // We assume only greyscale or RGB pixels
		 GL_FLOAT,
		 imagePtr->pixels().data());
	// Generating mipmaps for filtered texture fetch
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Rasterizer::initDisplayedImage()
{
	// Creating and configuring the GPU texture that will contain the image to display
	glGenTextures(1, &m_displayImageTex);
	glBindTexture(GL_TEXTURE_2D, m_displayImageTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Creating the framebuffer that will contain the depth & normal
	glGenFramebuffers(1, &zBufferFramebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, zBufferFramebufferID);
	// Creating the depth texture
	glGenTextures(1, &depthTextureID);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureID, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Creating the normal texture
	glGenTextures(1, &normalTextureID);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTextureID, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Creating the PRB color texture
	glGenTextures(1, &colorTextureID);
	glBindTexture(GL_TEXTURE_2D, colorTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, colorTextureID, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Rasterizer::drawMeshes(std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Scene> scenePtr)
{
	size_t numOfMeshes = scenePtr->numOfMeshes();
	for (size_t i = 0; i < numOfMeshes; i++)
	{
		glm::mat4 projectionMatrix = scenePtr->camera()->computeProjectionMatrix();
		shader->set("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
		glm::mat4 modelMatrix = scenePtr->mesh(i)->computeTransformMatrix();
		glm::mat4 viewMatrix = scenePtr->camera()->computeViewMatrix();
		glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
		shader->set("modelViewMat", modelViewMatrix);
		shader->set("normalMat", normalMatrix);
		// Set the material properties
		shader->set("material.albedo", scenePtr->mesh(i)->material()->albedo());
		shader->set("material.roughness", scenePtr->mesh(i)->material()->roughness());
		shader->set("material.metallicness", scenePtr->mesh(i)->material()->metallicness());
		// Initialize the light sources
		shader->set("directional_lightsource.color", scenePtr->directionallightsource()->color());
		shader->set("directional_lightsource.intensity", scenePtr->directionallightsource()->intensity());
		shader->set("point_lightsource.color", scenePtr->pointlightsource()->color());
		shader->set("point_lightsource.intensity", scenePtr->pointlightsource()->intensity());
		shader->set("point_lightsource.ac", scenePtr->pointlightsource()->ac());
		shader->set("point_lightsource.al", scenePtr->pointlightsource()->al());
		shader->set("point_lightsource.aq", scenePtr->pointlightsource()->aq());
		// Set the position of the point light source
		glm::vec4 scenePointLightPosition = viewMatrix * glm::vec4(scenePtr->pointlightsource()->position(), 1.0);
		shader->set("point_lightsource.position", glm::vec3(scenePointLightPosition));
		// Set the direction of the directional light source
		glm::vec4 sceneDirectionalLightDirection = viewMatrix * glm::vec4(scenePtr->directionallightsource()->direction(), 0.0);
		shader->set("directional_lightsource.direction", glm::vec3(sceneDirectionalLightDirection));
		draw(i, scenePtr->mesh(i)->triangleIndices().size());
	}
}

// The main rendering call
void Rasterizer::render(std::shared_ptr<Scene> scenePtr)
{
	m_pbrShaderProgramPtr->use();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	const glm::vec3 &bgColor = scenePtr->backgroundColor();
	glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

	drawMeshes(m_pbrShaderProgramPtr, scenePtr);

	m_pbrShaderProgramPtr->stop();
}

void Rasterizer::display(std::shared_ptr<Image> imagePtr)
{
	updateDisplayedImageTexture(imagePtr);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
	m_displayShaderProgramPtr->use();						 // Activate the program to be used for upcoming primitive
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_displayImageTex);
	glBindVertexArray(m_screenQuadVao); // Activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, 0);
	m_displayShaderProgramPtr->stop();
}

void Rasterizer::computeZBuffer(std::shared_ptr<Scene> scenePtr)
{
	// fill the Z buffer
	m_ZBufferShaderProgramPtr->use(); // Activate the program to be used for upcoming primitive
	glBindFramebuffer(GL_FRAMEBUFFER, zBufferFramebufferID);
	GLenum drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, drawBuffers);
	glReadBuffer(GL_NONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawMeshes(m_ZBufferShaderProgramPtr, scenePtr);

	m_ZBufferShaderProgramPtr->stop();
}

void Rasterizer::displayZBuffer(std::shared_ptr<Scene> scenePtr)
{
	computeZBuffer(scenePtr);

	// Switch to the default framebuffer for displaying the Z-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// display the Z buffer
	m_DisplayZBufferShaderProgramPtr->use(); // Activate the program to be used for upcoming primitive
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_screenQuadVao); // Activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, 0);
	m_DisplayZBufferShaderProgramPtr->stop();
}

void Rasterizer::displayNormal(std::shared_ptr<Scene> scenePtr)
{
	computeZBuffer(scenePtr);

	// Switch to the default framebuffer for displaying the Z-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// display the normals
	m_displayShaderProgramPtr->use(); // Activate the program to be used for upcoming primitive
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_screenQuadVao); // Activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, 0);
	m_displayShaderProgramPtr->stop();
}

void Rasterizer::displayAmbientOcclusion(std::shared_ptr<Scene> scenePtr, bool &print_time)
{
	std::chrono::high_resolution_clock clock;
	if (print_time)
	{
		Console::print("Start rasterizing (with " + std::to_string(m_width) + "x" + std::to_string(m_height) + " resolution)...");
	}
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	computeZBuffer(scenePtr);

	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	if (print_time)
	{
		Console::print("Rasterization executed in " + std::to_string(elapsedTime) + "ms");
		Console::print("Start calculating Horizon-Based Ambient Occlusion...");
	}

	// Switch to the default framebuffer for displaying the Z-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// display the Z buffer
	m_AmbientOcclusionShaderProgramPtr->use(); // Activate the program to be used for upcoming primitive
	m_AmbientOcclusionShaderProgramPtr->set("aspectRatio", scenePtr->camera()->getAspectRatio());
	m_AmbientOcclusionShaderProgramPtr->set("width", m_width);
	m_AmbientOcclusionShaderProgramPtr->set("height", m_height);
	m_AmbientOcclusionShaderProgramPtr->set("fov", scenePtr->camera()->getFoV());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_screenQuadVao); // Activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, 0);
	m_AmbientOcclusionShaderProgramPtr->stop();

	std::chrono::time_point<std::chrono::high_resolution_clock> after2 = clock.now();
	double elapsedTime2 = (double)std::chrono::duration_cast<std::chrono::microseconds>(after2 - after).count();
	if (print_time)
	{
		Console::print("HBAO executed in " + std::to_string(elapsedTime2) + " micro seconds");
		print_time = false;
	}
}

void Rasterizer::displayAmbientOcclusionWithPRB(std::shared_ptr<Scene> scenePtr, bool &print_time)
{
	std::chrono::high_resolution_clock clock;
	if (print_time)
	{
		Console::print("Start rasterizing (with " + std::to_string(m_width) + "x" + std::to_string(m_height) + " resolution)...");
	}
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	computeZBuffer(scenePtr);

	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	if (print_time)
	{
		Console::print("Rasterization executed in " + std::to_string(elapsedTime) + "ms");
		Console::print("Start calculating Horizon-Based Ambient Occlusion with colors...");
	}

	// Switch to the default framebuffer for displaying the Z-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// display the Z buffer
	m_AmbientOcclusionPRBShaderProgramPtr->use(); // Activate the program to be used for upcoming primitive
	m_AmbientOcclusionPRBShaderProgramPtr->set("aspectRatio", scenePtr->camera()->getAspectRatio());
	m_AmbientOcclusionPRBShaderProgramPtr->set("width", m_width);
	m_AmbientOcclusionPRBShaderProgramPtr->set("height", m_height);
	m_AmbientOcclusionPRBShaderProgramPtr->set("fov", scenePtr->camera()->getFoV());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, colorTextureID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_screenQuadVao); // Activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(6), GL_UNSIGNED_INT, 0);
	m_AmbientOcclusionPRBShaderProgramPtr->stop();

	std::chrono::time_point<std::chrono::high_resolution_clock> after2 = clock.now();
	double elapsedTime2 = (double)std::chrono::duration_cast<std::chrono::microseconds>(after2 - after).count();
	if (print_time)
	{
		Console::print("HBAO with colors executed in " + std::to_string(elapsedTime2) + " micro seconds");
		print_time = false;
	}
}

void Rasterizer::clear()
{
	for (unsigned int i = 0; i < m_posVbos.size(); i++)
	{
		GLuint vbo = m_posVbos[i];
		glDeleteBuffers(1, &vbo);
	}
	m_posVbos.clear();
	for (unsigned int i = 0; i < m_normalVbos.size(); i++)
	{
		GLuint vbo = m_normalVbos[i];
		glDeleteBuffers(1, &vbo);
	}
	m_normalVbos.clear();
	for (unsigned int i = 0; i < m_ibos.size(); i++)
	{
		GLuint ibo = m_ibos[i];
		glDeleteBuffers(1, &ibo);
	}
	m_ibos.clear();
	for (unsigned int i = 0; i < m_vaos.size(); i++)
	{
		GLuint vao = m_vaos[i];
		glDeleteVertexArrays(1, &vao);
	}
	m_vaos.clear();
}

GLuint Rasterizer::genGPUBuffer(size_t elementSize, size_t numElements, const void *data)
{
	GLuint vbo;
	glGenBuffers(1, &vbo);						  // Generate a GPU buffer to store the positions of the vertices
	size_t size = elementSize * numElements; // Gather the size of the buffer from the CPU-side vector
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	return vbo;
}

GLuint Rasterizer::genGPUVertexArray(GLuint posVbo, GLuint ibo, bool hasNormals, GLuint normalVbo)
{
	GLuint vao;
	glGenVertexArrays(1, &vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	if (hasNormals)
	{
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0); // Desactive the VAO just created. Will be activated at rendering time.
	return vao;
}

GLuint Rasterizer::toGPU(std::shared_ptr<Mesh> meshPtr)
{
	GLuint posVbo = genGPUBuffer(3 * sizeof(float), meshPtr->vertexPositions().size(), meshPtr->vertexPositions().data()); // Position GPU vertex buffer
	GLuint normalVbo = genGPUBuffer(3 * sizeof(float), meshPtr->vertexNormals().size(), meshPtr->vertexNormals().data());  // Normal GPU vertex buffer
	GLuint ibo = genGPUBuffer(sizeof(glm::uvec3), meshPtr->triangleIndices().size(), meshPtr->triangleIndices().data());	  // triangle GPU index buffer
	GLuint vao = genGPUVertexArray(posVbo, ibo, true, normalVbo);
	return vao;
}

void Rasterizer::initScreenQuad()
{
	std::vector<float> pData = {-1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 0.0};
	std::vector<unsigned int> iData = {0, 1, 2, 0, 2, 3};
	m_screenQuadVao = genGPUVertexArray(
		 genGPUBuffer(3 * sizeof(float), 4, pData.data()),
		 genGPUBuffer(3 * sizeof(unsigned int), 2, iData.data()),
		 false,
		 0);
}

void Rasterizer::draw(size_t meshId, size_t triangleCount)
{
	glBindVertexArray(m_vaos[meshId]);																			 // Activate the VAO storing geometry data
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangleCount * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}
