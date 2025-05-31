#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

// tamanho da janela
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// struct para camada
struct Layer {
    unsigned int textureID;
    float speed;
    float offset;
};

// funcao para carregar textura forcando canal alfa
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); 
    // força carregar com 4 canais rgba
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, STBI_rgb_alpha);
    
    if (data) {
        GLenum format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cout << "failed to load texture " << path << std::endl;
    }
    
    stbi_image_free(data);
    return textureID;
}

// vertex shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform float offset;
uniform float scale;

void main()
{
    vec3 pos = aPos * scale;
    gl_Position = vec4(pos, 1.0);
    TexCoord = vec2(aTexCoord.x + offset, aTexCoord.y);
}
)";

// fragment shader
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

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "parallax", NULL, NULL);
    if (window == NULL) {
        std::cout << "failed to create glfw window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // habilita blending para transparencia canal alfa
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build shaders
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

    // quad
    float quadVertices[] = {
        // positions        // texcoords
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

    // layers
    Layer layers[5];
    layers[0].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Sky.png");
    layers[1].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/BG_Decor.png");
    layers[2].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Middle_Decor.png");
    layers[3].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Foreground.png");
    layers[4].textureID = loadTexture("C:/Users/I588364/OneDrive - SAP SE/PG/PGCCHIB-main/PGCCHIB-main/include/Cartoon_Forest_BG_04/Layers/Ground.png");

    // velocidades da mais lenta a mais rapida
    layers[0].speed = 0.07f;
    layers[1].speed = 0.15f;
    layers[2].speed = 0.25f;
    layers[3].speed = 0.35f;
    layers[4].speed = 0.5f;

    for (int i = 0; i < 5; ++i)
        layers[i].offset = 0.0f;

    float scale = 1.0f;

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    // loop principal
    while (!glfwWindowShouldClose(window)) {
        // input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            for (int i = 0; i < 5; ++i)
                layers[i].offset += layers[i].speed * 0.01f;

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            for (int i = 0; i < 5; ++i)
                layers[i].offset -= layers[i].speed * 0.01f;

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            scale += 0.01f;

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            scale -= 0.01f;

        if (scale < 0.1f) scale = 0.1f;
        if (scale > 3.0f) scale = 3.0f;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glUseProgram(shaderProgram);

        for (int i = 0; i < 5; ++i) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, layers[i].textureID);

            glUniform1f(glGetUniformLocation(shaderProgram, "offset"), layers[i].offset);
            glUniform1f(glGetUniformLocation(shaderProgram, "scale"), scale);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}
