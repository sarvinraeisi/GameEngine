#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shaderinit.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdlib>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void init(void);
void render(Shader shader);

void tranformations(Shader&, bool, int);

const unsigned int screen_width = 1200;
const unsigned int screen_height = 800;

glm::mat4 transformationsMatrix[4]{};

unsigned int selectedRect{};
bool rotating = false;
bool scaling = false;
bool moveToCenter = false;
float angle = 0.0f;
GLuint NumVertices = 6;
GLuint VBO;
GLuint VAO;
GLuint EBO;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window;
    window = glfwCreateWindow(screen_width, screen_height, "OpenGLTwoTrianglesExternalShaderFiles", NULL, NULL);
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


    Shader ourShader("shader.vs", "shader.fs"); // you can name your shader files however you like

    init();

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
        //std::cout << "selected: " << selectedRect << std::endl;
       
        render(ourShader);
        
       
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void tranformations(Shader& ourShader, bool isSelected, int rectIdx)
{
    glm::vec3 center;
    glm::vec3 screenCenter = glm::vec3(0.0f, 0.0f, 0.0f);


    // array of centers for the rectangles
    glm::vec3 centers[] = {
        glm::vec3(0.8f, 0.8f, 0.0f),   
        glm::vec3(0.8f, -0.8f, 0.0f),  
        glm::vec3(-0.8f, -0.8f, 0.0f), 
        glm::vec3(-0.8f, 0.8f, 0.0f)   
    };
    center = centers[rectIdx];

    if (isSelected)
    {
        angle += 45.0f;
        transformationsMatrix[rectIdx] = glm::translate(transformationsMatrix[rectIdx], center); 
        transformationsMatrix[rectIdx] = glm::rotate(transformationsMatrix[rectIdx], glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        transformationsMatrix[rectIdx] = glm::translate(transformationsMatrix[rectIdx], -center); 
    }

    glm::mat4 transform = transformationsMatrix[rectIdx];

    if (scaling && isSelected)
    {
        // current scale
        glm::vec3 scale = glm::vec3(glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2]));

       
        if (glm::length(scale - glm::vec3(2.5f, 2.5f, 1.0f)) > 0.01f)
        {
            // Scale the rectangle by a factor of 2.5
            transform = glm::translate(transform, center); 
            transform = glm::scale(transform, glm::vec3(2.5f, 2.5f, 1.0f) / scale);
            transform = glm::translate(transform, -center); 

        }
        scaling = false; 
    }

    transformationsMatrix[rectIdx] = transform;

    if (moveToCenter && isSelected)
    {
        glm::vec4 currentCenter = transformationsMatrix[rectIdx] * glm::vec4(center, 1.0f);

        glm::vec3 translation = screenCenter - glm::vec3(currentCenter);

        if (glm::length(translation) > 0.01f)
        {
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);

            transformationsMatrix[rectIdx] = translationMatrix * transformationsMatrix[rectIdx];
        }
        moveToCenter = false;
    }

    ourShader.use();
    unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformationsMatrix[rectIdx]));
}

void render(Shader shader)
{
    static const float black[] = { 1.0f, 0.0f, 0.0f, 0.0f };
    glClearBufferfv(GL_COLOR, 0, black);
    glClear(GL_COLOR_BUFFER_BIT);  
    glBindVertexArray(VAO); 
    // Draw each rectangle
    for (int i = 0; i < 4; i++)
    {
        if (rotating && i == selectedRect) 
        {
            tranformations(shader, true, i);
        }
        else
        {
            tranformations(shader, false, i); 
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(static_cast<unsigned long long>(i) * 6 * sizeof(unsigned int)));
    }
}

void init(void)
{
    float vertices[] = {
      // Rectangle 1 
      0.95f,  0.95f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.65f,  0.95f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.65f,  0.65f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.95f,  0.65, 0.0f, 1.0f, 0.0f, 0.0f,

      // Rectangle 2
      0.95f, -0.95f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.65f, -0.95f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.65f, -0.65f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.95f, -0.65f, 0.0f, 0.0f, 1.0f, 0.0f,

      // Rectangle 3 
      -0.95f, -0.95f, 0.0f, 0.5f, 0.0f, 1.0f,
      -0.65f, -0.95f, 0.0f, 0.5f, 0.0f, 1.0f,
      -0.65f, -0.65f, 0.0f, 0.5f, 0.0f, 1.0f,
      -0.95f, -0.65f, 0.0f, 0.5f, 0.0f, 1.0f,

      // Rectangle 4
      -0.95f,  0.95f, 0.0f, 1.0f, 0.8f, 0.0f,
      -0.65f,  0.95f, 0.0f, 1.0f, 0.8f, 0.0f,
      -0.65f,  0.65f, 0.0f, 1.0f, 0.8f, 0.0f,
      -0.95f,  0.65f, 0.0f, 1.0f, 0.8f, 0.0f,

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

    NumVertices = sizeof(indices) / sizeof(indices[0]);
    
    for (int i = 0; i < 4; i++)
    {
        transformationsMatrix[i] = glm::mat4(1.0f);
    }

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

// user input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) 
    {
        glfwSetWindowShouldClose(window, true);
        std::cout << "user closed the window by pressing M" << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        rotating = true;
        float newRand = static_cast<int>(rand() % 4);
     
        selectedRect = newRand;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        scaling = true;
       
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        moveToCenter = true;

    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        rotating = false;
        scaling = false;
        moveToCenter = false;
        angle = 0.0f;

        for (int i = 0; i < 4; i++)
        {
            transformationsMatrix[i] = glm::mat4(1.0f);
        }
    }
}

// glfw: viewport to window adjustment
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}