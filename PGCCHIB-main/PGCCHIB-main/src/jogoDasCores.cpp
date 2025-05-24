#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

struct Color { //estrutura para representar cor rgb
    float r, g, b;

    float distance(const Color& other) const {  //calculo da distancia euclidiana entre duas cores
        return std::sqrt(
            (r - other.r) * (r - other.r) +
            (g - other.g) * (g - other.g) +
            (b - other.b) * (b - other.b)
        );
    }
};

struct Rectangle {   //representa retangulo na tela
    float x, y, width, height;
    Color color;
    bool visible = true;
};

const int ROWS = 6, COLS = 8;   //numero de linhas e colunas de retangulos
std::vector<Rectangle> grid;

const float SIMILARITY_THRESHOLD = 0.25f; // cor próxima
const int MAX_ATTEMPTS = 6;

Color selectedColor = { -1, -1, -1 };   //cor selecionada no clique atual

int attempts = 0;   //conta tentativas
int currentPlayer = 1; // seleciona player 1 ou 2
int scorePlayer1 = 0, scorePlayer2 = 0;   //guarda score
bool gameOver = false;  //seta final do jogo

// Botão reiniciar na parte inferior
struct Button {
    float x, y, width, height;
    Color color;
    bool hovered = false;
} restartButton;

void generateGrid() {    //gera a grade com os retangulos de cor aleatoria
    grid.clear();
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            Rectangle rect;
            rect.x = -1.0f + j * 0.25f;
            rect.y = 1.0f - i * 0.25f - 0.25f;
            rect.width = 0.2f;
            rect.height = 0.2f;

            // gera cores claras para evitar tons de preto
            float r = static_cast<float>(rand()) / RAND_MAX;
            float g = static_cast<float>(rand()) / RAND_MAX;
            float b = static_cast<float>(rand()) / RAND_MAX;
            // ajusta para evitar cores muito escuras
            r = 0.5f + r * 0.5f;
            g = 0.5f + g * 0.5f;
            b = 0.5f + b * 0.5f;

            rect.color = { r, g, b };
            rect.visible = true;
            grid.push_back(rect);
        }
    }
    attempts = 0;   //reset das variaves do jogo
    selectedColor = { -1, -1, -1 };
    currentPlayer = 1;
    scorePlayer1 = 0;
    scorePlayer2 = 0;
    gameOver = false;
}

void drawRectangle(const Rectangle& rect) {   //desenha retangulo na tela
    if (!rect.visible) return;
    glColor3f(rect.color.r, rect.color.g, rect.color.b);
    glBegin(GL_QUADS);
    glVertex2f(rect.x, rect.y);
    glVertex2f(rect.x + rect.width, rect.y);
    glVertex2f(rect.x + rect.width, rect.y + rect.height);
    glVertex2f(rect.x, rect.y + rect.height);
    glEnd();
}

void drawButton(const Button& btn) {   //desenha botao na tela
    // Cor muda se hover
    if (btn.hovered) {
        glColor3f(btn.color.r * 1.2f > 1.0f ? 1.0f : btn.color.r * 1.2f,
                  btn.color.g * 1.2f > 1.0f ? 1.0f : btn.color.g * 1.2f,
                  btn.color.b * 1.2f > 1.0f ? 1.0f : btn.color.b * 1.2f);
    } else {
        glColor3f(btn.color.r, btn.color.g, btn.color.b);
    }
    glBegin(GL_QUADS);
    glVertex2f(btn.x, btn.y);
    glVertex2f(btn.x + btn.width, btn.y);
    glVertex2f(btn.x + btn.width, btn.y + btn.height);
    glVertex2f(btn.x, btn.y + btn.height);
    glEnd();
}

// converte posição do cursor para coordenadas OpenGL (-1..1)
void cursorPosToGLCoords(double xpos, double ypos, float &xOut, float &yOut) {
    xOut = static_cast<float>(xpos) / 400.0f - 1.0f;
    yOut = 1.0f - static_cast<float>(ypos) / 300.0f;
}

void processClick(double xpos, double ypos) {
    float x, y;
    cursorPosToGLCoords(xpos, ypos, x, y);

    // verifica clique no botão primeiro
    if (x >= restartButton.x && x <= restartButton.x + restartButton.width &&
        y >= restartButton.y && y <= restartButton.y + restartButton.height) {
        generateGrid();
        std::cout << "Jogo reiniciado pelo botao.\n";
        return;
    }

    if (gameOver) {
        std::cout << "O jogo acabou. Reinicie para jogar novamente usando o clique direito do mouse ou o botao azul na parte inferior da tela.\n";
        return;
    }

    if (attempts >= MAX_ATTEMPTS) {
        gameOver = true;
        return;
    }

    for (Rectangle& rect : grid) {
        if (rect.visible &&
            x >= rect.x && x <= rect.x + rect.width &&
            y >= rect.y && y <= rect.y + rect.height) {
            
            selectedColor = rect.color;

            int removed = 0;
            for (Rectangle& other : grid) {
                if (other.visible && selectedColor.distance(other.color) < SIMILARITY_THRESHOLD) {
                    other.visible = false;
                    removed++;
                }
            }

            int points = std::max(0, removed * 10 - attempts * 5);

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

            // alterna jogador
            currentPlayer = (currentPlayer == 1) ? 2 : 1;

            break;
        }
    }
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {  //detectar hover
    float x, y;
    cursorPosToGLCoords(xpos, ypos, x, y);

    // atualiza hover do botão
    if (x >= restartButton.x && x <= restartButton.x + restartButton.width &&
        y >= restartButton.y && y <= restartButton.y + restartButton.height) {
        restartButton.hovered = true;
    } else {
        restartButton.hovered = false;
    }
}

int main() {  //funcao principal para rodar o jogo
    srand(static_cast<unsigned>(time(0)));

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Jogo das Cores", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    generateGrid();

    // define botão reiniciar na parte inferior da tela
    restartButton.x = -0.5f;
    restartButton.y = -0.95f;
    restartButton.width = 1.0f;
    restartButton.height = 0.1f;
    restartButton.color = {0.2f, 0.6f, 0.8f};
    restartButton.hovered = false;

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            processClick(xpos, ypos);
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            generateGrid(); // reinicia com clique direito
            std::cout << "Jogo reiniciado pelo clique direito.\n";
        }
    });

    glfwSetCursorPosCallback(window, cursorPositionCallback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        for (const Rectangle& rect : grid) drawRectangle(rect);
        drawButton(restartButton);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
