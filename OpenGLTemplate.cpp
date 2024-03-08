#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shaderinit.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void init(void);
void render();
void tranformations(Shader& ourShader);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

// variables
const unsigned int screen_width = 1200;
const unsigned int screen_height = 800;
const GLuint NumVertices = 36;
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

    // glad
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable Depth Testing
    glEnable(GL_DEPTH_TEST);
    // build and compile our shader program
    Shader ourShader("shader.vs", "shader.fs"); // you can name your shader files however you like

    init();
    // primitives spawning locations
    glm::vec3 primPositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    tranformations(ourShader);

    // camera/view transformation
    glm::mat4 view = glm::mat4(1.0f); // view matrix initialization
    // setting the radius variable 
    float radius = 10.0f;

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // Inside main() before the render loop, set the mouse callback
        glfwSetCursorPosCallback(window, mouse_callback);

        // To hide the mouse cursor and capture it, add this line as well
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // render the background
        render();
        // Shader instance use
        ourShader.use();
        // add the view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);
        // render boxes
        glBindVertexArray(VAO);

        for (int i = 0; i < 1; i++) {
            // model matrix and primitives spawning
            glm::mat4 model = glm::mat4(1.0f); // initialize the model matrix
            model = glm::translate(model, primPositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 24);
        }
        // glfw: swap buffers
        glfwSwapBuffers(window);
        // gldw: poll for events
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void init(void)
{
    float vertices[] = {
        // Rectangle in Top-Left Corner - Red
         -1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         -0.8f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         -1.0f,  0.8f, 0.0f,  1.0f, 0.0f, 0.0f,
         -0.8f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         -1.0f,  0.8f, 0.0f,  1.0f, 0.0f, 0.0f,
         -0.8f,  0.8f, 0.0f,  1.0f, 0.0f, 0.0f,

         // Rectangle 2 - Bottom-right corner
        0.8f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.8f, -0.8f, 0.0f,  0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.8f, -0.8f, 0.0f,  0.0f, 1.0f, 0.0f,
        1.0f, -0.8f, 0.0f,  0.0f, 1.0f, 0.0f,

        // Rectangle 3 - Top-right corner
        0.8f,  0.8f, 0.0f,  0.0f, 0.0f, 1.0f,
        1.0f,  0.8f, 0.0f,  0.0f, 0.0f, 1.0f,
        0.8f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        1.0f,  0.8f, 0.0f,  0.0f, 0.0f, 1.0f,
        0.8f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,

        // Rectangle 4 - Top-left corner
        -1.0f, -1.0f, 0.0f,  1.0f, 1.0f, 0.0f, // Bottom Left
        -0.8f, -1.0f, 0.0f,  1.0f, 1.0f, 0.0f, // Bottom Right
        -1.0f, -0.8f, 0.0f,  1.0f, 1.0f, 0.0f, // Top Left
        -0.8f, -1.0f, 0.0f,  1.0f, 1.0f, 0.0f, // Bottom Right
        -1.0f, -0.8f, 0.0f,  1.0f, 1.0f, 0.0f, // Top Left
        -0.8f, -0.8f, 0.0f,  1.0f, 1.0f, 0.0f, // Top Rig
    };



    // Generate and bind VAO and VBO as before
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VBO and VAO
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