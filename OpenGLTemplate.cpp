#define STB_IMAGE_IMPLEMENTATION
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "shaderinit.h"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void init(void);
void render();
void texture1Rendering(const char* path);
void texture2Rendering(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void tranformations(Shader& ourShader);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// variables
const unsigned int screen_width = 1200;
const unsigned int screen_height = 800;
GLuint NumVertices{};
GLuint VBO;
GLuint VAO;
GLuint EBO;
unsigned int skyboxVAO, skyboxVBO;

unsigned int texture1, texture2, appliedTexture{};
int width, height, nrChannels;

// camera 
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4 cameraView = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = screen_width / 2.0f;
float lastY = screen_height / 2.0f;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float mouseSensitivity = 0.1f;

float blendFactor = 0.5f;
glm::vec3 blendColor(1.0f, 0.0f, 0.0f);

int selectedSquare = -1;

bool rotateRandomCube = false;
int rotatingCubeIndex = -1;
float rotationAngle = 0.0f;

float scale = 1.0f;
bool moveToCenter = false;
bool resetTransformations = false;
glm::vec3 originalPositions[4];
glm::vec3 centerPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::mat4 model = glm::mat4(1.0f);


int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _APPLE_
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	srand(static_cast<unsigned int>(time(0)));
	// glfw window creation
	// --------------------
	GLFWwindow* window;
	window = glfwCreateWindow(screen_width, screen_height, "OpenGLCameraAuto", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader ourShader("shader.vs", "shader.fs");
	//Shader ourShader("shader.vs", "shader.fs", "shader.gs");
	Shader skyboxShader("skyboxshader.vs", "skyboxshader.fs");
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	init();
	texture1Rendering("assets/box.png");
	texture2Rendering("assets/smilie.png");
	std::vector<std::string> faces
	{
		"assets/skybox/right.jpg",
		"assets/skybox/left.jpg",
		"assets/skybox/top.jpg",
		"assets/skybox/bottom.jpg",
		"assets/skybox/front.jpg",
		"assets/skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	tranformations(ourShader);

	glEnable(GL_DEPTH_TEST);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		render();
		ourShader.use();
		ourShader.setInt("isPlane", 1);
		ourShader.setVec3("blendColor", glm::vec3(1.0f, 0.3f, 0.6f));
		// Draw Plane
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * 36 * 4));
		ourShader.setInt("isPlane", 0);

		// draw cubes
		for (int i = 0; i < 4; i++)
		{
			if (resetTransformations)
			{
				scale = 1.0f;
				moveToCenter = false;
				rotateRandomCube = false;
				rotationAngle = 0.0f;
				model = glm::translate(model, originalPositions[i]);
				resetTransformations = false;
			}
			else
			{
				if (moveToCenter && i == rotatingCubeIndex)
				{
					model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
				}
				if (i == rotatingCubeIndex)
				{
					model = glm::scale(model, glm::vec3(scale, scale, scale));
					model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
				}
			}

			int applyBlend = (i == selectedSquare) ? 1 : 0;
			ourShader.setInt("applyBlend", applyBlend);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
			ourShader.setInt("texture1", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texture2);
			ourShader.setInt("texture2", 1);
			ourShader.setFloat("blendFactor", blendFactor);
			ourShader.setVec3("blendColor", blendColor);

			ourShader.setMat4("model", model);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(i * 36 * sizeof(unsigned int)));
		}

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		//glFrontFace(GL_CCW);

		cameraView = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		ourShader.setMat4("view", cameraView);
		ourShader.setMat4("projection", projection);

		skyboxShader.use();
		cameraView = glm::mat4(glm::mat3(cameraView));
		skyboxShader.setMat4("view", cameraView);
		skyboxShader.setMat4("projection", projection);

		// skybox cube
		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
}

void tranformations(Shader& ourShader)
{
	projection = glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
	ourShader.use();
	ourShader.setMat4("projection", projection);
}

void render()
{
	glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glBindVertexArray(VAO);

	if (rotateRandomCube && rotatingCubeIndex != -1)
	{
		rotationAngle += 45.0f * deltaTime;
	}
}

void init(void)
{
	float depth = 0.3f;
	float size = 0.3f;
	float offset = 0.5f;

	float planeVertices[] = {
	 -2.0f, -2.0f, -0.6f,   0.0f, 0.0f
	  2.0f, -2.0f, -0.6f,   1.0f, 0.0f
	  2.0f, 2.0f,  -0.6f,   1.0f, 1.0f
	  - 2.0f, 2.0f, -0.6f,   0.0f, 1.0f
	};

	unsigned int planeIndices[] = {
		32, 33, 34, 32, 34, 35
	};

	float vertices[] = {
		// Cube 1 vertices (Top Left)
		-offset, offset, 0.0f,    0.0f, 1.0f,					// Front Top Left
		-offset - size, offset, 0.0f,    1.0f, 1.0f,			// Front Top Right
		-offset - size, offset - size, 0.0f,    1.0f, 0.0f,		// Front Bottom Right
		-offset, offset - size, 0.0f,    0.0f, 0.0f,			// Front Bottom Left
		-offset, offset, -depth,    1.0f, 1.0f,					// Back Top Left
		-offset - size, offset, -depth,    0.0f, 1.0f,			// Back Top Right
		-offset - size, offset - size, -depth,    0.0f, 0.0f,	// Back Bottom Right
		-offset, offset - size, -depth,    1.0f, 0.0f,			// Back Bottom Left

		// Cube 2 vertices (Top Right)
		offset, offset, 0.0f,    0.0f, 1.0f,
		offset + size, offset, 0.0f,    1.0f, 1.0f,
		offset + size, offset - size, 0.0f,    1.0f, 0.0f,
		offset, offset - size, 0.0f,    0.0f, 0.0f,
		offset, offset, -depth,    1.0f, 1.0f,
		offset + size, offset, -depth,    0.0f, 1.0f,
		offset + size, offset - size, -depth,    0.0f, 0.0f,
		offset, offset - size, -depth,    1.0f, 0.0f,

		// Cube 3 vertices (Bottom Left)
		-offset, -offset, 0.0f,    0.0f, 1.0f,
		-offset - size, -offset, 0.0f,    1.0f, 1.0f,
		-offset - size, -offset + size, 0.0f,    1.0f, 0.0f,
		-offset, -offset + size, 0.0f,    0.0f, 0.0f,
		-offset, -offset, -depth,    1.0f, 1.0f,
		-offset - size, -offset, -depth,    0.0f, 1.0f,
		-offset - size, -offset + size, -depth,    0.0f, 0.0f,
		-offset, -offset + size, -depth,    1.0f, 0.0f,

		// Cube 4 vertices (Bottom Right)
		offset, -offset, 0.0f,    0.0f, 1.0f,
		offset + size, -offset, 0.0f,    1.0f, 1.0f,
		offset + size, -offset + size, 0.0f,    1.0f, 0.0f,
		offset, -offset + size, 0.0f,    0.0f, 0.0f,
		offset, -offset, -depth,    1.0f, 1.0f,
		offset + size, -offset, -depth,    0.0f, 1.0f,
		offset + size, -offset + size, -depth,    0.0f, 0.0f,
		offset, -offset + size, -depth,    1.0f, 0.0f,
	};

	unsigned int indices[] = {
		// Indices for the first cube 
		0, 1, 2, 0, 2, 3,  // Front face
		4, 5, 6, 4, 6, 7,  // Back face
		0, 4, 7, 0, 7, 3,  // Left face
		1, 5, 6, 1, 6, 2,  // Right face
		0, 1, 5, 0, 5, 4,  // Top face
		2, 3, 7, 2, 7, 6,  // Bottom face

		// Indices for the second cube 
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		8, 12, 15, 8, 15, 11,
		9, 13, 14, 9, 14, 10,
		8, 9, 13, 8, 13, 12,
		10, 11, 15, 10, 15, 14,

		// Indices for the third cube 
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
		16, 20, 23, 16, 23, 19,
		17, 21, 22, 17, 22, 18,
		16, 17, 21, 16, 21, 20,
		18, 19, 23, 18, 23, 22,

		// Indices for the fourth cube 
		24, 25, 26, 24, 26, 27,
		28, 29, 30, 28, 30, 31,
		24, 28, 31, 24, 31, 27,
		25, 29, 30, 25, 30, 26,
		24, 25, 29, 24, 29, 28,
		26, 27, 31, 26, 31, 30,
	};

	// skybox vertices
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	float points[] = {
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
		0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 1.0f, 1.0f, 0.0f
	};

	// Calculate the size of the vertices and indices arrays
	int verticesArraySize = sizeof(vertices) + sizeof(planeVertices);
	int indicesArraySize = sizeof(indices) + sizeof(planeIndices);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesArraySize, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(planeVertices), planeVertices);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArraySize, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), sizeof(planeIndices), planeIndices);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

// glfw: user input
void processInput(GLFWwindow* window)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 3 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: viewport to window adjustment
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Mouse callback function
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	if ((key == GLFW_KEY_T) && action == GLFW_PRESS)
	{
		std::cout << "setting new texture" << std::endl;
		appliedTexture = 1;

	}
	if ((key == GLFW_KEY_Y) && action == GLFW_PRESS)
	{
		std::cout << "resseting all textures" << std::endl;
		appliedTexture = 0;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		// Randomize the blend factor and blend color
		blendFactor = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		blendColor = glm::vec3(
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
			static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		// Select one of the four squares randomly
		selectedSquare = rand() % 4;
	}
	//rptation
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		rotatingCubeIndex = rand() % 4;
		rotateRandomCube = true;
	}
	// scaling
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		scale = 2.5f;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		moveToCenter = true;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		resetTransformations = true;
	}
}

void texture1Rendering(const char* path)
{
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void texture2Rendering(const char* path)
{
	// texture 2
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(false);
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}