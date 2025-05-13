#include <iostream>
#include <vector>
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
GLuint dynamicVAO = 0;
GLuint dynamicVBO = 0;

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

// Cria VAO com coordenadas fornecidas
GLuint createTriangleVAO(float x0, float y0, float x1, float y1, float x2, float y2)
{
    GLuint VAO, VBO;
    float vertices[] = {
        x0, y0, 0.0f,
        x1, y1, 0.0f,
        x2, y2, 0.0f
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return VAO;
}

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

            if (dynamicVAO == 0) {
                glGenVertexArrays(1, &dynamicVAO);
                glGenBuffers(1, &dynamicVBO);
            }

            glBindVertexArray(dynamicVAO);
            glBindBuffer(GL_ARRAY_BUFFER, dynamicVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glBindVertexArray(0);

            clickPositions.clear();
        }
    }
}

int main()
{
    // Inicialização
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Clique para criar triângulo", NULL, NULL);
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

    // Triângulo fixo no centro
    GLuint baseVAO = createTriangleVAO(-0.1f * WIDTH, -0.1f * HEIGHT, 0.1f * WIDTH, -0.1f * HEIGHT, 0.0f, 0.1f * HEIGHT);

    // Loop principal
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderID);

        // Triângulo inicial
        glBindVertexArray(baseVAO);
        mat4 model = mat4(1.0f);
        model = translate(model, vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
        glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // verde
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Triângulo do clique (se houver)
        if (dynamicVAO != 0)
        {
            glBindVertexArray(dynamicVAO);
            mat4 modelClick = mat4(1.0f);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(modelClick));
            glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // vermelho
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
