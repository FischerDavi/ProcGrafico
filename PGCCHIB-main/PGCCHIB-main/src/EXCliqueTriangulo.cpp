#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
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
GLuint modelLoc;

mat4 projection;

GLuint triangleVAO = 0;

// Estrutura do triângulo
struct Triangle {
    float x, y;     // posição
    vec3 color;     // cor RGB
};

vector<Triangle> triangles;

// Função que cria um triângulo base com os 3 vértices passados
GLuint createTriangle(float x0, float y0, float x1, float y1, float x2, float y2)
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

// Callback do clique do mouse
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float x = static_cast<float>(xpos);
        float y = static_cast<float>(HEIGHT - ypos); // Inverte eixo Y

        // Gera cor aleatória
        float r = static_cast<float>(rand()) / RAND_MAX;
        float g = static_cast<float>(rand()) / RAND_MAX;
        float b = static_cast<float>(rand()) / RAND_MAX;

        // Adiciona novo triângulo
        triangles.push_back({ x, y, vec3(r, g, b) });
    }
}

// Shaders
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
uniform vec3 color;
void main()
{
    FragColor = vec4(color, 1.0);
}
)";

int main()
{
    srand(static_cast<unsigned int>(time(0)));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Triângulos com clique", NULL, NULL);
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

    // Uniforms
    colorLoc = glGetUniformLocation(shaderID, "color");
    projectionLoc = glGetUniformLocation(shaderID, "projection");
    modelLoc = glGetUniformLocation(shaderID, "model");

    // Projeção ortográfica
    projection = ortho(0.0f, static_cast<float>(WIDTH), 0.0f, static_cast<float>(HEIGHT));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

    // Cria triângulo base (será usado para todos)
    triangleVAO = createTriangle(-0.1f * WIDTH, -0.1f * HEIGHT, 0.1f * WIDTH, -0.1f * HEIGHT, 0.0f, 0.1f * HEIGHT);

    // Adiciona o triângulo fixo no centro da tela
    triangles.push_back({ WIDTH / 2.0f, HEIGHT / 2.0f, vec3(0.0f, 1.0f, 0.0f) }); // verde

    // Loop principal
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderID);
        glBindVertexArray(triangleVAO);

        // Desenha todos os triângulos
        for (const Triangle& t : triangles)
        {
            mat4 model = translate(mat4(1.0f), vec3(t.x, t.y, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));
            glUniform3fv(colorLoc, 1, value_ptr(t.color));
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}