// main.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <list>
#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int windowWidth = SCR_WIDTH;
int windowHeight = SCR_HEIGHT;

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
};

struct CircleInstance {
    glm::vec2 position;
    glm::vec3 color;
    float radius;
};

std::list<glm::vec2> snake;
float backgroundHue = 0.0f;

float mouseX = SCR_WIDTH / 2;
float mouseY = SCR_HEIGHT / 2;

GLuint lineShaderProgram;
GLuint circleShaderProgram;
GLuint vao, vbo;
GLuint circleVAO, circleVBO, instanceVBO;

std::vector<glm::vec2> unitCircle;


void GetOpenGLInfo() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = static_cast<float>(xpos);
    mouseY = static_cast<float>(ypos);
	//std::cout << "Mouse position: " << mouseX << ", " << mouseY << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_C) {
            snake.clear();
        }
        else if (key == GLFW_KEY_A) {
            snake.emplace_back(static_cast<float>(rand() % SCR_WIDTH), static_cast<float>(rand() % SCR_HEIGHT));
        }
        else if (key == GLFW_KEY_R) {
            if (!snake.empty()) snake.pop_back();
        }
		else if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, true);
		}
    }
}

// Convert HSB to RGB
glm::vec3 hsb2rgb(float h, float s, float b) {
    h = fmodf(h, 1.0f);
    float c = b * s;
    float x = c * (1 - fabsf(fmodf(h * 6.0f, 2.0f) - 1));
    float m = b - c;
    glm::vec3 rgb;
    if (h < 1.0f / 6) rgb = { c, x, 0 };
    else if (h < 2.1f / 6) rgb = { x, c, 0 };
    else if (h < 3.1f / 6) rgb = { 0, c, x };
    else if (h < 4.1f / 6) rgb = { 0, x, c };
    else if (h < 5.1f / 6) rgb = { x, 0, c };
    else rgb = { c, 0, x };
    return rgb + glm::vec3(m);
}

std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    if (!file) {
        std::cerr << "Failed to load shader file: " << filepath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vCode = loadShaderSource(vertexPath);
    std::string fCode = loadShaderSource(fragmentPath);
    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    GLint success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

void generateUnitCircle(int segments = 36) {
    unitCircle.clear();
    unitCircle.emplace_back(0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * 3.1415926f * i / segments;
        unitCircle.emplace_back(cos(angle), sin(angle));
    }
}

void update() {
    glm::vec2 target(mouseX, mouseY);
    for (auto& pos : snake) {
        pos = glm::mix(pos, target, 0.2f);
        target = pos;
    }
    backgroundHue = fmodf(backgroundHue + 0.001f, 1.0f);
}

void renderSnakeLine() {
    if (snake.empty()) return;

    std::vector<Vertex> vertices;
    int index = 0;
    int count = snake.size();
    for (const auto& pos : snake) {
        float hue = static_cast<float>(index++) / count;
        glm::vec3 color = hsb2rgb(hue, 0.8f, 1.0f);
        vertices.push_back({ pos, color });
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(vao);
    glUseProgram(lineShaderProgram);
    int resLocLine = glGetUniformLocation(lineShaderProgram, "u_resolution");
    glUniform2f(resLocLine, (float)windowWidth, (float)windowHeight);
    glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
}

void renderSnakeCircles() {
    if (snake.empty()) return;

    std::vector<CircleInstance> instanceData;
    int index = 0;
    int count = snake.size();
    for (const auto& pos : snake) {
        float hue = static_cast<float>(index) / count;
        float radius = glm::mix(20.0f, 5.0f, static_cast<float>(index) / count);
        glm::vec3 color = hsb2rgb(hue, 0.8f, 1.0f);
        instanceData.push_back({ pos, color, radius });
        index++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(CircleInstance), instanceData.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(circleVAO);
    glUseProgram(circleShaderProgram);

    int resLoc = glGetUniformLocation(circleShaderProgram, "u_resolution");
    glUniform2f(resLoc, (float)windowWidth, (float)windowHeight);

    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, unitCircle.size(), instanceData.size());
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLFW Snake", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSwapInterval(1);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < 20; i++) {
        snake.emplace_back(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
    }

    lineShaderProgram = createShaderProgram("snake.vert", "snake.frag");
    circleShaderProgram = createShaderProgram("circle.vert", "circle.frag");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec2)));
    glEnableVertexAttribArray(1);

    generateUnitCircle();

    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(circleVAO);

    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, unitCircle.size() * sizeof(glm::vec2), unitCircle.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CircleInstance), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CircleInstance), (void*)(sizeof(glm::vec2)));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(CircleInstance), (void*)(sizeof(glm::vec2) + sizeof(glm::vec3)));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    GetOpenGLInfo();

    while (!glfwWindowShouldClose(window)) {
        update();

        glm::vec3 c1 = hsb2rgb(fmodf(backgroundHue, 1.0f), 0.6f, 0.9f);
        glClearColor(c1.r, c1.g, c1.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderSnakeLine();
        renderSnakeCircles();

        glfwSwapBuffers(window);
        glfwPollEvents();

        //std::cout << "loop time: " << glfwGetTime() << std::endl;
    }

    glfwTerminate();
    return 0;
}
