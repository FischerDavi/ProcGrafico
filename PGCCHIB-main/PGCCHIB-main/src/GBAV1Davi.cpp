#include <iostream>
#include <vector>
#include <cstdlib> // para rand()
#include <ctime>   // para time()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

const int WIDTH = 800;
const int HEIGHT = 600;

GLuint shaderID;
GLuint colorLoc;
GLuint projectionLoc;
mat4 projection;

std::vector<vec2> clickPositions;
std::vector<GLuint> triangleVAOs;
std::vector<GLuint> triangleVBOs;
std::vector<mat4> triangleModels;
std::vector<vec4> triangleColors;

// Vertex & Fragment Shaders
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 model;
void main()
{
    gl_Position = projection * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main()
{
    FragColor = color;
}
)";

// Captura clique do mouse
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float x = static_cast<float>(xpos);
        float y = static_cast<float>(HEIGHT - ypos); // inverte eixo Y

        clickPositions.push_back(vec2(x, y));

        if (clickPositions.size() == 3)
        {
            float vertices[] = {
                clickPositions[0].x, clickPositions[0].y, 0.0f,
                clickPositions[1].x, clickPositions[1].y, 0.0f,
                clickPositions[2].x, clickPositions[2].y, 0.0f
            };

            GLuint VAO, VBO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glBindVertexArray(0);

            triangleVAOs.push_back(VAO);
            triangleVBOs.push_back(VBO);
            triangleModels.push_back(mat4(1.0f));

            // Gera uma cor aleatória (RGB entre 0.2 e 1.0)
            float r = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.8f;
            float g = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.8f;
            float b = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.8f;
            triangleColors.push_back(vec4(r, g, b, 1.0f));

            clickPositions.clear();
        }
    }
}

int main()
{
    srand(static_cast<unsigned int>(time(0))); // inicializa aleatoriedade

    // Inicialização
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Clique para criar triângulos coloridos", NULL, NULL);
    if (!window)
    {
        cout << "Erro ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glViewport(0, 0, WIDTH, HEIGHT);

    // Compila shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderID);

    // Localizações dos uniforms
    colorLoc = glGetUniformLocation(shaderID, "color");
    projectionLoc = glGetUniformLocation(shaderID, "projection");

    // Projeção ortográfica
    projection = ortho(0.0f, static_cast<float>(WIDTH), 0.0f, static_cast<float>(HEIGHT));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

    // Loop principal
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderID);

        // Triângulos criados por clique
        for (size_t i = 0; i < triangleVAOs.size(); ++i)
        {
            glBindVertexArray(triangleVAOs[i]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(triangleModels[i]));
            vec4 color = triangleColors[i];
            glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    // Liberação de recursos
    for (size_t i = 0; i < triangleVAOs.size(); ++i)
    {
        glDeleteVertexArrays(1, &triangleVAOs[i]);
        glDeleteBuffers(1, &triangleVBOs[i]);
    }

    glfwTerminate();
    return 0;
}