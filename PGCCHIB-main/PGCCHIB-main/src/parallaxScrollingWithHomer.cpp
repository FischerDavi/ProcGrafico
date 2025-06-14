#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

struct Layer {
    unsigned int textureID;
    float speed;
    float offset;
};

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, STBI_rgb_alpha);

    if (data) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform float offset;
uniform float scale;
uniform vec2 translation;
uniform float flipX;

void main()
{
    vec3 pos = aPos * vec3(flipX * scale, scale, 1.0) + vec3(translation, 0.0);
    gl_Position = vec4(pos, 1.0);
    TexCoord = vec2(aTexCoord.x + offset, aTexCoord.y);
}
)";

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

enum Direction {
    NONE,
    LEFT,
    RIGHT
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Parallax", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float homerWidth = 45.0f / (SCR_WIDTH / 2.0f);
    float homerHeight = 72.0f / (SCR_HEIGHT / 2.0f);

    float homerVertices[] = {
        -homerWidth,  homerHeight, 0.0f,  0.0f, 1.0f,
        -homerWidth, -homerHeight, 0.0f,  0.0f, 0.0f,
         homerWidth, -homerHeight, 0.0f,  1.0f, 0.0f,

        -homerWidth,  homerHeight, 0.0f,  0.0f, 1.0f,
         homerWidth, -homerHeight, 0.0f,  1.0f, 0.0f,
         homerWidth,  homerHeight, 0.0f,  1.0f, 1.0f
    };

    unsigned int homerVBO, homerVAO;
    glGenVertexArrays(1, &homerVAO);
    glGenBuffers(1, &homerVBO);

    glBindVertexArray(homerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, homerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(homerVertices), homerVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Layer layers[5];
    layers[0].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Sky.png");
    layers[1].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/BG_Decor.png");
    layers[2].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Middle_Decor.png");
    layers[3].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Foreground.png");
    layers[4].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Ground.png");

    layers[0].speed = 0.07f;
    layers[1].speed = 0.15f;
    layers[2].speed = 0.25f;
    layers[3].speed = 0.35f;
    layers[4].speed = 0.5f;

    for (int i = 0; i < 5; ++i)
        layers[i].offset = 0.0f;

    unsigned int homerTextures[3];
    homerTextures[0] = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/homernorm.png");
    homerTextures[1] = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/homeresq.png");
    homerTextures[2] = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/homerdir.png");

    int homerFrame = 0;
    bool keyHeld = false;
    float lastSwitchTime = 0.0f;
    bool toggle = false;

    float homerOffsetY = -150.0f / (SCR_HEIGHT / 2.0f);

    Direction currentDirection = NONE;

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    float lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Detecta direção (esquerda, direita, ou parado)
        bool leftPressed = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
        bool rightPressed = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;

        if (leftPressed && !rightPressed) {
            currentDirection = LEFT;
        } else if (rightPressed && !leftPressed) {
            currentDirection = RIGHT;
        } else {
            currentDirection = NONE;
        }

        // Atualiza offset das camadas apenas se estiver andando (para dar efeito de movimento na paisagem)
        if (currentDirection == LEFT) {
            for (int i = 0; i < 5; ++i) {
                layers[i].offset -= layers[i].speed * deltaTime * 0.5f;
                if (layers[i].offset < 0.0f)
                    layers[i].offset += 1.0f;
            }
        } else if (currentDirection == RIGHT) {
            for (int i = 0; i < 5; ++i) {
                layers[i].offset += layers[i].speed * deltaTime * 0.5f;
                if (layers[i].offset > 1.0f)
                    layers[i].offset -= 1.0f;
            }
        }

        // Controla animação do Homer (alternando entre pernas)
        if (currentDirection != NONE) {
            if (!keyHeld) {
                keyHeld = true;
                lastSwitchTime = currentFrame;
                toggle = false;
            }
            if (currentFrame - lastSwitchTime >= 0.3f) { // alterna a cada 0.3s para efeito de caminhada mais natural
                toggle = !toggle;
                lastSwitchTime = currentFrame;
            }
            homerFrame = toggle ? 1 : 2; // alterna entre homeresq e homerdir
        } else {
            keyHeld = false;
            homerFrame = 0; // textura normal parado
        }

        // Limpa tela
        glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Desenha camadas do fundo
        glBindVertexArray(VAO);
        for (int i = 0; i < 5; ++i) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layers[i].textureID);
            glUniform1f(glGetUniformLocation(shaderProgram, "offset"), layers[i].offset);
            glUniform1f(glGetUniformLocation(shaderProgram, "scale"), 1.0f);
            glUniform2f(glGetUniformLocation(shaderProgram, "translation"), 0.0f, 0.0f);
            glUniform1f(glGetUniformLocation(shaderProgram, "flipX"), 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Define flipX para Homer: -1 para esquerda (espelhado), 1 para direita ou parado
        float flipX = (currentDirection == LEFT) ? -1.0f : 1.0f;

        // Desenha Homer com animação e espelhamento
        glBindVertexArray(homerVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, homerTextures[homerFrame]);
        glUniform1f(glGetUniformLocation(shaderProgram, "offset"), 0.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "scale"), 1.0f);
        glUniform2f(glGetUniformLocation(shaderProgram, "translation"), 0.0f, homerOffsetY);
        glUniform1f(glGetUniformLocation(shaderProgram, "flipX"), flipX);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &homerVAO);
    glDeleteBuffers(1, &homerVBO);

    glfwTerminate();
    return 0;
}
