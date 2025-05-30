#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 projection;

out vec2 TexCoord;

void main()
{
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
)";

// Shader Compilation
GLuint createShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR: Vertex Shader compilation failed\n" << infoLog << std::endl;
    }

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR: Fragment Shader compilation failed\n" << infoLog << std::endl;
    }

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR: Shader Program linking failed\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

// Carregar textura
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

struct Sprite {
    GLuint VAO, VBO, EBO;
    glm::vec2 position, scale;
    float rotation;
    glm::vec2 uvMin, uvMax;
    GLuint textureID;
    GLuint shaderProgram;

    Sprite(GLuint shaderProgram, GLuint textureID, glm::vec2 uvMin, glm::vec2 uvMax)
        : shaderProgram(shaderProgram), textureID(textureID), uvMin(uvMin), uvMax(uvMax),
          position(0.0f), scale(1.0f), rotation(0.0f)
    {
        setupSprite();
    }

    void setupSprite() {
        float vertices[] = {
            -0.5f, -0.5f,  uvMin.x, uvMin.y,
             0.5f, -0.5f,  uvMax.x, uvMin.y,
             0.5f,  0.5f,  uvMax.x, uvMax.y,
            -0.5f,  0.5f,  uvMin.x, uvMax.y
        };
        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw(glm::mat4& projection) {
        glUseProgram(shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(scale, 1.0f));

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Sprite House Example", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, 800, 600);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    stbi_set_flip_vertically_on_load(true);  // Inverter textura na carga

    GLuint shaderProgram = createShaderProgram();
    GLuint bgTextureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/8bitLib/PNG/grass.png");
    glm::vec2 uv_min_bg(0.0f, 0.0f);
    glm::vec2 uv_max_bg(1.0f, 1.0f);
    Sprite background(shaderProgram, bgTextureID, uv_min_bg, uv_max_bg);
    background.position = glm::vec2(400.0f, 300.0f);
    background.scale = glm::vec2(800.0f, 600.0f);

    GLuint shaderProgram2 = createShaderProgram();
    GLuint textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/8bitLib/PNG/exterior.png");

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    float textureWidth = 240.0f;
    float textureHeight = 800.0f;

    float x1 = 0.0f, y1 = 0.0f, width1 = 145.0f, height1 = 128.0f;

    glm::vec2 uv_min(x1 / textureWidth, 1.0f - (y1 + height1) / textureHeight);
    glm::vec2 uv_max((x1 + width1) / textureWidth, 1.0f - y1 / textureHeight);

    Sprite houseSprite(shaderProgram2, textureID, uv_min, uv_max);

    houseSprite.position = glm::vec2(400.0f, 300.0f); // centro da janela
    houseSprite.scale = glm::vec2(width1*2, height1*2);     // tamanho real da casa em pixels

    GLuint shaderProgram3 = createShaderProgram();

    glm::mat4 projection2 = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    float textureWidth2 = 240.0f;
    float textureHeight2 = 800.0f;

    float x2 = 0.0f, y2 = 315.0f, width2 = 61.0f, height2 = 79.0f;

    glm::vec2 uv_min2(x2 / textureWidth2, 1.0f - (y2 + height2) / textureHeight2);
    glm::vec2 uv_max2((x2 + width2) / textureWidth2, 1.0f - y2 / textureHeight2);

    Sprite treeSprite(shaderProgram3, textureID, uv_min2, uv_max2);

    treeSprite.position = glm::vec2(200.0f, 250.0f); 
    treeSprite.scale = glm::vec2(width2*2, height2*2);

    GLuint shaderProgram4 = createShaderProgram();

    glm::mat4 projection3 = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    float textureWidth3 = 240.0f;
    float textureHeight3 = 800.0f;

    float x3 = 160.0f, y3 = 0.0f, width3 = 39.0f, height3 = 64.0f;

    glm::vec2 uv_min3(x3 / textureWidth3, 1.0f - (y3 + height3) / textureHeight3);
    glm::vec2 uv_max3((x3 + width3) / textureWidth3, 1.0f - y3 / textureHeight3);

    Sprite fenceSprite(shaderProgram4, textureID, uv_min3, uv_max3);

    fenceSprite.position = glm::vec2(600.0f, 100.0f); 
    fenceSprite.scale = glm::vec2(width3*2, height3*2);

    GLuint shaderProgram5 = createShaderProgram();

    glm::mat4 projection4 = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    float textureWidth4 = 240.0f;
    float textureHeight4 = 800.0f;

    float x4 = 0.0f, y4 = 542.0f, width4 = 58.0f, height4 = 61.0f;

    glm::vec2 uv_min4(x4 / textureWidth4, 1.0f - (y4 + height4) / textureHeight4);
    glm::vec2 uv_max4((x4 + width4) / textureWidth4, 1.0f - y4 / textureHeight4);

    Sprite scarecrowSprite(shaderProgram5, textureID, uv_min4, uv_max4);

    scarecrowSprite.position = glm::vec2(550.0f, 500.0f); 
    scarecrowSprite.scale = glm::vec2(width4*2, height4*2);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        background.draw(projection);
        houseSprite.draw(projection);
        treeSprite.draw(projection2);
        fenceSprite.draw(projection3);
        scarecrowSprite.draw(projection4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}