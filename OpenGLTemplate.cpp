#define STB_IMAGE_IMPLEMENTATION
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shaderinit.h"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void init(void);
void render();
void texture1Rendering(const char* path);
void texture2Rendering(const char* path);
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

unsigned int texture1, texture2, appliedTexture{};
int width, height, nrChannels;

// camera 
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
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

	init();
	texture1Rendering("assets/box.png");
	texture2Rendering("assets/smilie.png");

	tranformations(ourShader);

	glm::mat4 view = glm::mat4(1.0f);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		render();

		ourShader.use();

		ourShader.setFloat("blendFactor", blendFactor);
		ourShader.setVec3("blendColor", blendColor);

		//for (int i = 0; i < 4; i++)
		//{
		//	// Determine if the current square is the selected one for blending
		//	int applyBlend = (i == selectedSquare) ? 1 : 0;
		//	ourShader.setInt("applyBlend", applyBlend);

		//	// Set other uniforms and draw the square as before
		//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(static_cast<unsigned long long>(i) * 6 * sizeof(unsigned int)));
		//}
		for (int i = 0; i < 4; i++)
		{
			int applyBlend = (i == selectedSquare) ? 1 : 0;
			ourShader.setInt("applyBlend", applyBlend);
			// Bind both textures for all squares
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);
			ourShader.setInt("texture1", 0); // Assuming your shader sampler for texture1 is set to use texture unit 0
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texture2);
			ourShader.setInt("texture2", 1); // Assuming your shader sampler for texture2 is set to use texture unit 1
			// Set blendFactor and blendColor for all squares, the shader decides to use it or not
			ourShader.setFloat("blendFactor", blendFactor);
			ourShader.setVec3("blendColor", blendColor);


			// Now draw the cube with glDrawElements
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(i * 36 * sizeof(unsigned int)));
		}

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		ourShader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		ourShader.setMat4("model", model);

		glfwSwapBuffers(window);
		glfwPollEvents();
		glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), appliedTexture);
	}

	glfwDestroyWindow(window);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteProgram(shaderProgram);
	glfwTerminate();
	return 0;
}

void tranformations(Shader& ourShader)
{
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
	ourShader.use();
	ourShader.setMat4("projection", projection);
}

void render()
{
	glClearColor(0.6f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glBindVertexArray(VAO);

	for (int i = 0; i < 4; i++)
	{
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(static_cast<unsigned long long>(i) * 6 * sizeof(unsigned int)));
	}
}

void init(void)
{
	float depth = 0.3f; // Depth for cubes
	float size = 0.3f;  // Size for cubes

	// Define the offset for placing cubes at the four corners
	float offset = 0.5f;  // Distance from the center

	// Define vertices for all 4 cubes
	float vertices[] = {
		// Cube 1 vertices (Top Left)
		-offset, offset, 0.0f,                   // Front Top Left
		-offset - size, offset, 0.0f,            // Front Top Right
		-offset - size, offset - size, 0.0f,     // Front Bottom Right
		-offset, offset - size, 0.0f,            // Front Bottom Left
		-offset, offset, -depth,                 // Back Top Left
		-offset - size, offset, -depth,          // Back Top Right
		-offset - size, offset - size, -depth,   // Back Bottom Right
		-offset, offset - size, -depth,          // Back Bottom Left

		// Cube 2 vertices (Top Right)
		offset, offset, 0.0f,
		offset + size, offset, 0.0f,
		offset + size, offset - size, 0.0f,
		offset, offset - size, 0.0f,
		offset, offset, -depth,
		offset + size, offset, -depth,
		offset + size, offset - size, -depth,
		offset, offset - size, -depth,

		// Cube 3 vertices (Bottom Left)
		-offset, -offset, 0.0f,
		-offset - size, -offset, 0.0f,
		-offset - size, -offset + size, 0.0f,
		-offset, -offset + size, 0.0f,
		-offset, -offset, -depth,
		-offset - size, -offset, -depth,
		-offset - size, -offset + size, -depth,
		-offset, -offset + size, -depth,

		// Cube 4 vertices (Bottom Right)
		offset, -offset, 0.0f,
		offset + size, -offset, 0.0f,
		offset + size, -offset + size, 0.0f,
		offset, -offset + size, 0.0f,
		offset, -offset, -depth,
		offset + size, -offset, -depth,
		offset + size, -offset + size, -depth,
		offset, -offset + size, -depth,
	};

	unsigned int indices[] = {
		// Indices for the first cube (Top Left)
		0, 1, 2, 0, 2, 3,  // Front face
		4, 5, 6, 4, 6, 7,  // Back face
		0, 4, 7, 0, 7, 3,  // Left face
		1, 5, 6, 1, 6, 2,  // Right face
		0, 1, 5, 0, 5, 4,  // Top face
		2, 3, 7, 2, 7, 6,  // Bottom face

		// Indices for the second cube (Top Right) - Each index offset by 8
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		8, 12, 15, 8, 15, 11,
		9, 13, 14, 9, 14, 10,
		8, 9, 13, 8, 13, 12,
		10, 11, 15, 10, 15, 14,

		// Indices for the third cube (Bottom Left) - Each index offset by 16
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
		16, 20, 23, 16, 23, 19,
		17, 21, 22, 17, 22, 18,
		16, 17, 21, 16, 21, 20,
		18, 19, 23, 18, 23, 22,

		// Indices for the fourth cube (Bottom Right) - Each index offset by 24
		24, 25, 26, 24, 26, 27,
		28, 29, 30, 28, 30, 31,
		24, 28, 31, 24, 31, 27,
		25, 29, 30, 25, 30, 26,
		24, 25, 29, 24, 29, 28,
		26, 27, 31, 26, 31, 30,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	
	// color attribute information
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coord attribute information
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

// glfw: user input
void processInput(GLFWwindow* window)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// increate the camera speed using the deltaTime
	float cameraSpeed = 3 * deltaTime;
	// Forward movement
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	// Backward movement
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	// Left movement
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	// Right movement
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
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