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
void tranformations(Shader& ourShader);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

// variables
const unsigned int screen_width = 1200;
const unsigned int screen_height = 800;
GLuint NumVertices{};
GLuint VBO;
GLuint VAO;
GLuint EBO;

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
float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right
float pitch = 0.0f;
float mouseSensitivity = 0.1f;

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader ourShader("shader.vs", "shader.fs");

	init();

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
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		ourShader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		ourShader.setMat4("model", model);

		glfwSwapBuffers(window);
		glfwPollEvents();
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
	glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(VAO);
	for (int i = 0; i < 4; i++)
	{
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(static_cast<unsigned long long>(i) * 6 * sizeof(unsigned int)));
	}
}

void init(void)
{
	float vertices[] = {
		// Rectangle 1 
		1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.7f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.7f,  0.7f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f,  0.7, 0.0f, 1.0f, 0.0f, 0.0f,

		// Rectangle 2
		1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.7f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.7f, -0.7f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, -0.7f, 0.0f, 0.0f, 1.0f, 0.0f,

		// Rectangle 3 
		-1.0f, -1.0f, 0.0f, 0.5f, 0.0f, 1.0f,
		-0.7f, -1.0f, 0.0f, 0.5f, 0.0f, 1.0f,
		-0.7f, -0.7f, 0.0f, 0.5f, 0.0f, 1.0f,
		-1.0f, -0.7f, 0.0f, 0.5f, 0.0f, 1.0f,

		// Rectangle 4
		-1.0f,  1.0f, 0.0f, 1.0f, 0.8f, 0.0f,
		-0.7f,  1.0f, 0.0f, 1.0f, 0.8f, 0.0f,
		-0.7f,  0.7f, 0.0f, 1.0f, 0.8f, 0.0f,
		-1.0f,  0.7f, 0.0f, 1.0f, 0.8f, 0.0f,
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0,

		4, 5, 6,
		6, 7, 4,

		8, 9, 10,
		10, 11, 8,

		12, 13, 14,
		14, 15, 12
	};

	NumVertices = sizeof(indices) / sizeof(indices[0]);;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// glfw: user input
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// increate the camera speed using the deltaTime
	float cameraSpeed = 3 * deltaTime;
	// forward movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	// backward movement
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	// left movement
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	// right movement
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
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
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