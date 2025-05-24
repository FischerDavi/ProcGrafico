#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

// estrutura de cor
struct Color {
    float r, g, b;

    float distance(const Color& other) const { //calculo da distancia euclidiana
        return sqrtf((r - other.r) * (r - other.r) + 
                     (g - other.g) * (g - other.g) + 
                     (b - other.b) * (b - other.b));
    }
};

struct Rectangle {  //representacao do retangulo
    float x, y, width, height;
    Color color;
    bool visible = true;
};

// constantes do jogo
const int ROWS = 6, COLS = 8;
const float SIMILARITY_THRESHOLD = 0.25f;
const int MAX_ATTEMPTS = 6;

std::vector<Rectangle> grid;  //grade 

Color selectedColor = {-1, -1, -1};  //cor selecionada
int attempts = 0;   //variaveis do jogo
int currentPlayer = 1;
int scorePlayer1 = 0, scorePlayer2 = 0;
bool gameOver = false;

struct Button {  //botao de reinicio
    float x, y, width, height;
    Color color;
    bool hovered = false;
} restartButton;

unsigned int shaderProgram, VAO;  //shader e vao dos retangulos

void generateGrid() {  
    grid.clear();   //limpa grade
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            Rectangle rect;
            rect.x = -1.0f + j * 0.25f;
            rect.y = 1.0f - i * 0.25f - 0.25f;
            rect.width = 0.2f;
            rect.height = 0.2f;

            //gera cores claras aleatorias
            float r = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
            float g = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
            float b = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f;

            rect.color = {r, g, b};
            grid.push_back(rect);
        }
    }
    attempts = 0;   //reinicia jogo
    selectedColor = {-1, -1, -1};
    currentPlayer = 1;
    scorePlayer1 = 0;
    scorePlayer2 = 0;
    gameOver = false;
}

void cursorPosToGLCoords(double xpos, double ypos, float &xOut, float &yOut) {  //converte coordenadas do mouse
    xOut = static_cast<float>(xpos) / 400.0f - 1.0f;
    yOut = 1.0f - static_cast<float>(ypos) / 300.0f;
}

void processClick(double xpos, double ypos) {
    float x, y;
    cursorPosToGLCoords(xpos, ypos, x, y);

    if (x >= restartButton.x && x <= restartButton.x + restartButton.width &&  //reconhece clique no botao de reinicio
        y >= restartButton.y && y <= restartButton.y + restartButton.height) {
        generateGrid();
        std::cout << "Jogo reiniciado pelo botao.\n";
        return;
    }

    if (gameOver) {
        std::cout << "O jogo acabou. Reinicie para jogar novamente usando o clique direito do mouse ou o botao azul na parte inferior da tela.\n";
        return;
    }

    if (attempts >= MAX_ATTEMPTS) {  //maximo de tentativas
        gameOver = true;
        return;
    }

    for (Rectangle& rect : grid) {
        if (rect.visible &&
            x >= rect.x && x <= rect.x + rect.width &&
            y >= rect.y && y <= rect.y + rect.height) {
            
            selectedColor = rect.color;
            int removed = 0;

            for (Rectangle& other : grid) {  //remove retangulos similares
                if (other.visible && selectedColor.distance(other.color) < SIMILARITY_THRESHOLD) {
                    other.visible = false;
                    removed++;
                }
            }

            int points = std::max(0, removed * 10 - attempts * 5);  //calculo de pontos

            if (currentPlayer == 1) {
                scorePlayer1 += points;
            } else {
                scorePlayer2 += points;
            }

            attempts++;
            std::cout << "Jogador " << currentPlayer << " fez " << points << " pontos. "
                      << "Placar: Jogador1 = " << scorePlayer1 << ", Jogador2 = " << scorePlayer2 << "\n";

            if (attempts >= MAX_ATTEMPTS) {
                gameOver = true;
                std::cout << "Jogo acabou! ";
                if (scorePlayer1 > scorePlayer2)
                    std::cout << "Jogador 1 venceu!\n";
                else if (scorePlayer2 > scorePlayer1)
                    std::cout << "Jogador 2 venceu!\n";
                else
                    std::cout << "Empate!\n";
            }

            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            break;
        }
    }
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    float x, y;
    cursorPosToGLCoords(xpos, ypos, x, y);

    restartButton.hovered = (x >= restartButton.x && x <= restartButton.x + restartButton.width &&
                             y >= restartButton.y && y <= restartButton.y + restartButton.height);
}

void setupRectangle() { 
    float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    unsigned int VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void setupShader() {
    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 model;
        void main() {
            gl_Position = model * vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void drawRectangle(const Rectangle& rect) {  //desenho dos retangulos
    if (!rect.visible) return;

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(rect.x, rect.y, 0.0f));
    model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    int colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    glUniform3f(colorLoc, rect.color.r, rect.color.g, rect.color.b);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawButton(const Button& btn) {  //desenho do botao de reinicio
    Rectangle btnRect = {btn.x, btn.y, btn.width, btn.height, btn.color, true};
    if (btn.hovered) {
        btnRect.color.r = std::min(1.0f, btn.color.r * 1.2f);
        btnRect.color.g = std::min(1.0f, btn.color.g * 1.2f);
        btnRect.color.b = std::min(1.0f, btn.color.b * 1.2f);
    }
    drawRectangle(btnRect);
}

int main() {  //logica principal
    srand(static_cast<unsigned>(time(0)));
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Jogo das Cores - Modern OpenGL", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    generateGrid();
    setupRectangle();
    setupShader();

    restartButton = {-0.5f, -0.95f, 1.0f, 0.1f, {0.2f, 0.6f, 0.8f}, false};

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            processClick(xpos, ypos);
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            generateGrid();
            std::cout << "Jogo reiniciado pelo clique direito.\n";
        }
    });

    glfwSetCursorPosCallback(window, cursorPositionCallback);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (const Rectangle& rect : grid) drawRectangle(rect);
        drawButton(restartButton);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
