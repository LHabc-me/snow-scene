#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawStamp(Shader shader, Model stump);
void drawHouse(Shader shader, Model house);
void drawSnowman(Shader shader, Model snowman);

// 窗口大小
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 768;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "snow-scene", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 开启深度测试
    glEnable(GL_DEPTH_TEST);

    // 着色器
    Shader shader("shaders/model-vert.glsl", "shaders/model-frag.glsl");

    // 加载模型：树桩，房屋，雪人
    Model stump("resources/stump/stump-in-winter.fbx");
    Model house("resources/house/house.obj");
    Model snowman("resources/snowman/snowman.obj");

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 记录每一帧的时间差
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 处理输入
        processInput(window);

        // 渲染
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawStamp(shader, stump);
        drawHouse(shader, house);
        drawSnowman(shader, snowman);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void drawStamp(Shader shader, Model stump)
{
    shader.use();
    glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom),
                                               (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                               0.1f,
                                               100.0f);
    glm::mat4 viewMat = camera.GetViewMatrix();
    shader.setMat4("projection", projectionMat);
    shader.setMat4("view", viewMat);
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(0.1f, 0.1f, 0.1f));
    modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, -50.0f, 0.0f));
    shader.setMat4("model", modelMat);
    stump.Draw(shader);
}

void drawHouse(Shader shader, Model house)
{
    shader.use();
    glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom),
                                               (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                               0.1f,
                                               100.0f);
    glm::mat4 viewMat = camera.GetViewMatrix();
    shader.setMat4("projection", projectionMat);
    shader.setMat4("view", viewMat);
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 0.0f));
    shader.setMat4("model", modelMat);
    house.Draw(shader);
}

void drawSnowman(Shader shader, Model snowman)
{
    shader.use();
    glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom),
                                               (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                               0.1f,
                                               100.0f);
    glm::mat4 viewMat = camera.GetViewMatrix();
    shader.setMat4("projection", projectionMat);
    shader.setMat4("view", viewMat);
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(3.0f, 3.0f, 3.0f));
    modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    modelMat = glm::translate(modelMat, glm::vec3(2.0f, -0.1f, -0.6f));
    shader.setMat4("model", modelMat);
    snowman.Draw(shader);
}

// 处理输入 用于上下左右前后移动
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// 处理窗口大小变化
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 处理鼠标移动
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// 处理鼠标滚轮
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
