#include <iostream>
#include <string>
#include <assert.h>
#include <vector>

using namespace std;

// GLAD
#include <C:\Users\I588364\OneDrive - SAP SE\PG\PGCCHIB-main\PGCCHIB-main\include\glad\glad.h>

// GLFW
#include <C:\Users\I588364\OneDrive - SAP SE\PG\PGCCHIB-main\PGCCHIB-main\include\GLFW\glfw3.h>

// GLM
#include <C:\Users\I588364\OneDrive - SAP SE\PG\PGCCHIB-main\PGCCHIB-main\include\glm\glm.hpp>
#include <C:\Users\I588364\OneDrive - SAP SE\PG\PGCCHIB-main\PGCCHIB-main\include\glm\gtc\matrix_transform.hpp>
#include <C:\Users\I588364\OneDrive - SAP SE\PG\PGCCHIB-main\PGCCHIB-main\include\glm\gtc\type_ptr.hpp>

using namespace glm;

#include <cmath>

const int WIDTH = 800;
const int HEIGHT = 600;

struct Triangle
{
    vec3 position;
    vec3 dimensions;
    vec3 color;
};

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

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", NULL, NULL);
    if (!window)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, WIDTH, HEIGHT);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderID);

    GLuint colorLoc = glGetUniformLocation(shaderID, "color");

    // Criação da projeção ortográfica
    mat4 projection = ortho(0.0f, static_cast<float>(WIDTH), 0.0f, static_cast<float>(HEIGHT));
    GLuint projectionLoc = glGetUniformLocation(shaderID, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

    // Criação de 5 triângulos
    vector<GLuint> VAOs;
    vector<Triangle> triangles;
    for (int i = 0; i < 5; ++i)
    {
        // Triângulo padrão no centro (sistema de coordenadas normalizado)
        GLuint vao = createTriangle(-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f);
        VAOs.push_back(vao);

        Triangle tri;
        tri.position = vec3(100.0f + i * 120.0f, 300.0f, 0.0f);
        tri.dimensions = vec3(100.0f, 100.0f, 1.0f);
        tri.color = vec3((i+1)*0.2f, 0.5f, 1.0f - i*0.15f);
        triangles.push_back(tri);
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < triangles.size(); i++)
        {
            glBindVertexArray(VAOs[i]);

            mat4 model = mat4(1.0f);
            model = translate(model, triangles[i].position);
            model = scale(model, triangles[i].dimensions);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

            glUniform4f(colorLoc, triangles[i].color.r, triangles[i].color.g, triangles[i].color.b, 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}