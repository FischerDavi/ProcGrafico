#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>
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
    bool visible;

    Sprite(GLuint shaderProgram, GLuint textureID, glm::vec2 uvMin, glm::vec2 uvMax)
        : shaderProgram(shaderProgram), textureID(textureID), uvMin(uvMin), uvMax(uvMax),
          position(0.0f), scale(1.0f), rotation(0.0f), visible(true)
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
        if (!visible) return;

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
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Sprite Control", NULL, NULL);
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
    stbi_set_flip_vertically_on_load(true);

    GLuint shaderProgram = createShaderProgram();
    GLuint textureBG = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/8bitLib/PNG/grass.png");
    GLuint textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/8bitLib/PNG/exterior.png");

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    Sprite background(shaderProgram, textureBG, glm::vec2(0,0), glm::vec2(1,1));
    background.position = glm::vec2(400, 300);
    background.scale = glm::vec2(800, 600);

    std::vector<std::pair<std::string, Sprite>> sprites;

    // Adesivos:
    auto add_sprite = [&](const std::string& name, float x, float y, float w, float h, float posX, float posY) {
        glm::vec2 uv_min(x/240.0f, 1.0f - (y+h)/800.0f);
        glm::vec2 uv_max((x+w)/240.0f, 1.0f - y/800.0f);
        Sprite spr(shaderProgram, textureID, uv_min, uv_max);
        spr.position = glm::vec2(posX, posY);
        spr.scale = glm::vec2(w*2, h*2);
        sprites.push_back({name, spr});
    };

    add_sprite("house", 0, 0, 145, 128, 400, 300);
    add_sprite("tree", 0, 315, 61, 79, 200, 250);
    add_sprite("fence", 160, 0, 39, 64, 600, 100);
    add_sprite("scarecrow", 0, 542, 58, 61, 550, 500);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        background.draw(projection);
        for (auto& sp : sprites) sp.second.draw(projection);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Terminal control
        std::cout << "\nAvailable sprites:\n";
        for (auto& sp : sprites) {
            std::cout << sp.first << " (visible: " << (sp.second.visible ? "yes" : "no") << ", scale: "
                      << sp.second.scale.x << ")\n";
        }
        std::cout << "Enter sprite name to toggle/scale (or 'skip'):\n";
        std::string input;
        std::cin >> input;
        if (input == "skip") continue;

        for (auto& sp : sprites) {
            if (sp.first == input) {
                std::cout << "1) Toggle visibility\n2) Change scale\nChoice: ";
                int choice; std::cin >> choice;
                if (choice == 1) {
                    sp.second.visible = !sp.second.visible;
                } else if (choice == 2) {
                    std::cout << "Enter new scale factor (0.1 - 2.0): ";
                    float scale; std::cin >> scale;
                    sp.second.scale = glm::vec2(sp.second.scale.x * scale, sp.second.scale.y * scale);
                }
            }
        }
    }
    glfwTerminate();
    return 0;
}
