#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Struct Sprite
struct Sprite
{
    GLuint VAO;
    GLuint texID;
    glm::vec3 position;
    glm::vec3 dimensions; // tamanho do frame
    float ds, dt;
    int iAnimation, iFrame;
    int nAnimations, nFrames;
    bool flipHorizontal = false;
};

// Vertex shader
const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main()
{
    gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
    TexCoord = texCoord;
}
)";

// Fragment shader
const char* fragmentShaderSrc = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tileTexture;
uniform vec2 offsetTex;
uniform vec2 scaleTex;

void main()
{
    vec2 tex = offsetTex + TexCoord * scaleTex;
    FragColor = texture(tileTexture, tex);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Erro ao compilar shader: " << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram()
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Erro ao linkar shader program: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

GLuint setupTileVAO(float& ds, float& dt)
{
    float vertices[] = {
        // pos      // tex coords
       -1.0f,  0.5f,  0.0f, 0.0f,
       -1.0f, -0.5f,  0.0f, 1.0f,
        1.0f,  0.5f,  1.0f, 0.0f,
        1.0f, -0.5f,  1.0f, 1.0f,
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    ds = 1.0f / 7.0f;
    dt = 1.0f;

    return VAO;
}

GLuint setupSprite(int nAnimations, int nFrames, float& ds, float& dt)
{
    float vertices[] = {
        // pos      // tex coords
       -0.5f,  0.5f,  0.0f, 0.0f,
       -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  1.0f, 1.0f,
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    ds = 1.0f / nFrames;
    dt = 1.0f / nAnimations;

    return VAO;
}

GLuint loadTexture(const char* path, int& width, int& height)
{
    int nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4);
    if (!data)
    {
        std::cerr << "Falha ao carregar textura: " << path << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return textureID;
}

void drawSprite(Sprite& s, GLuint shaderProgram, GLint uniModelLoc, GLint uniOffsetTexLoc, GLint uniScaleTexLoc)
{
    glBindTexture(GL_TEXTURE_2D, s.texID);
    glBindVertexArray(s.VAO);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), s.position);

    // Se flipHorizontal estiver true, inverta o X
    glm::vec3 scale = s.dimensions;
    if (s.flipHorizontal)
        scale.x *= -1.0f;

    model = glm::scale(model, scale);

    glUniformMatrix4fv(uniModelLoc, 1, GL_FALSE, glm::value_ptr(model));

    float offsetX = s.iFrame * s.ds;
    float offsetY = s.iAnimation * s.dt;
    glUniform2f(uniOffsetTexLoc, offsetX, offsetY);
    glUniform2f(uniScaleTexLoc, s.ds, s.dt);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void processMovement(GLFWwindow* window, Sprite &vampirao)
{
    bool moved = false;
    float speed = 0.0005f; 

    float newX = vampirao.position.x;
    float newY = vampirao.position.y;

    if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS))
    {
        newX += speed;
        vampirao.iAnimation = 0;
        vampirao.flipHorizontal = false;
        moved = true;
    }
    else if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS))
    {
        newX -= speed;
        vampirao.iAnimation = 0;
        vampirao.flipHorizontal = true;
        moved = true;
    }

    if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS))
    {
        newY += speed;
        vampirao.iAnimation = 2;
        moved = true;
    }
    else if ((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS))
    {
        newY -= speed;
        vampirao.iAnimation = 1;
        moved = true;
    }

    // Dimensões do mapa
    float tileWidth = 1.9f;
    float tileHeight = 0.95f;
    int mapCols = 3;
    int mapRows = 3;

    float mapWidth = (mapCols + mapRows) * (tileWidth / 2.0f);    // 6.0f
    float mapHeight = (mapCols + mapRows) * (tileHeight / 2.0f);  // 3.0f

    float centerX = 0.0f;
    float centerY = mapHeight / 2.0f;  // 1.5f

    // Calcula posição relativa ao centro
    float relativeX = newX - centerX;
    float relativeY = newY - centerY;

    // Checa se está dentro do losango (área do mapa)
    float normX = fabs(relativeX) / (mapWidth / 2.0f);
    float normY = fabs(relativeY) / (mapHeight / 2.0f);

    if (normX + normY <= 1.0f)
    {
        vampirao.position.x = newX;
        vampirao.position.y = newY;
    }

    if (moved)
    {
        vampirao.iFrame = (vampirao.iFrame + 1) % vampirao.nFrames;
    }
}

int main()
{
    const int WIDTH = 800;
    const int HEIGHT = 600;

    if (!glfwInit())
    {
        std::cerr << "Falha ao inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Tilemap com Vampirão", NULL, NULL);
    if (!window)
    {
        std::cerr << "Falha ao criar janela GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD\n";
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    float ds, dt;
    GLuint tileVAO = setupTileVAO(ds, dt);

    int texWidth, texHeight;
    GLuint tileTexID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/tilesetIso.png", texWidth, texHeight);
    if (tileTexID == 0)
        return -1;

    // Setup vampirao
    int vampWidth, vampHeight;
    GLuint vampTexID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/donatello.png", vampWidth, vampHeight);

    Sprite vampirao;
    vampirao.nAnimations = 3;
    vampirao.nFrames = 3;
    vampirao.VAO = setupSprite(vampirao.nAnimations, vampirao.nFrames, vampirao.ds, vampirao.dt);
    vampirao.position = glm::vec3(0.0f, 0.0f, 0.0f);
    vampirao.dimensions = glm::vec3(0.8f, 0.8f, 1.0f);
    vampirao.texID = vampTexID;
    vampirao.iAnimation = 1;
    vampirao.iFrame = 0;

    int map[3][3] = {
        {1, 3, 6},
        {3, 4, 2},
        {4, 5, 2}
    };

    float tileWidth = 2.0f;
    float tileHeight = 1.0f;

    glm::mat4 projection = glm::ortho(-4.0f, 4.0f, -1.0f, 5.0f, -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    GLint uniModelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint uniViewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint uniProjectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint uniOffsetTexLoc = glGetUniformLocation(shaderProgram, "offsetTex");
    GLint uniScaleTexLoc = glGetUniformLocation(shaderProgram, "scaleTex");

    glUniformMatrix4fv(uniViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uniProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) vampirao.position.x += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) vampirao.position.x -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) vampirao.position.y += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) vampirao.position.y -= 0.02f;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(tileVAO);
        glBindTexture(GL_TEXTURE_2D, tileTexID);

        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 3; ++col)
            {
                int tileIndex = map[row][col];
                float x = (col - row) * (tileWidth / 2.0f);
                float y = (col + row) * (tileHeight / 2.0f) + 0.25f;

                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
                glUniformMatrix4fv(uniModelLoc, 1, GL_FALSE, glm::value_ptr(model));

                float offsetX = tileIndex * ds;
                glUniform2f(uniOffsetTexLoc, offsetX, 0.0f);
                glUniform2f(uniScaleTexLoc, ds, dt);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }

        drawSprite(vampirao, shaderProgram, uniModelLoc, uniOffsetTexLoc, uniScaleTexLoc);

        processMovement(window, vampirao);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &tileVAO);
    glDeleteTextures(1, &tileTexID);
    glDeleteTextures(1, &vampTexID);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}