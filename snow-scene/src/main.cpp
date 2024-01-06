#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "skybox.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawStamp(Shader shader, Model stump, glm::vec3 stumpPosition, glm::vec3 stumpRotation, glm::vec3 stumpScale);
void drawHouse(Shader shader, Model house);
void drawSnowman(Shader shader, Model snowman);
void initDepthBuffer(GLuint& depthMapFBO, GLuint& depthMap);
void renderShadowMap(Shader& depthShader, GLuint depthMapFBO, glm::vec3 lightPos);
void applyShadow(Shader& shader, GLuint depthMap, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, glm::vec3 lightColor);
glm::vec3 screenToWorldCoords(double xpos, double ypos, GLFWwindow* window, glm::mat4 view, glm::mat4 projection);
glm::vec3 ScreenPosToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, glm::mat4 ViewMatrix,
                              glm::mat4 ProjectionMatrix);
glm::vec3 RayPlaneIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planeNormal,
                               glm::vec3 planePoint);


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

glm::vec3 stumpPosition = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 stumpRotation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 stumpScale = glm::vec3(0.1f, 0.1f, 0.1f);
bool dragging = false;
glm::vec2 lastMousePos = glm::vec2(0.0f, 0.0f);
glm::vec3 previousWorldCoords = glm::vec3(0.0f, 0.0f, 0.0f);

void drawCrystal(Shader& shader, Model& crystal, glm::vec3 position)
{
    shader.use();
    glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom),
                                               (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                               0.1f,
                                               100.0f);
    glm::mat4 viewMat = camera.GetViewMatrix();
    shader.setMat4("projection", projectionMat);
    shader.setMat4("view", viewMat);
    glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), position);
    modelMat = glm::scale(modelMat, glm::vec3(0.2f, 0.2f, 0.2f));
    shader.setMat4("model", modelMat);
    crystal.Draw(shader);
}

class Snowflake
{
public:
    glm::vec3 position;
    glm::vec3 velocity;

    Snowflake(glm::vec3 pos, glm::vec3 vel) : position(pos), velocity(vel)
    {
    }

    void update(float deltaTime)
    {
        position += velocity * deltaTime;
    }
};

class SnowflakeGenerator
{
public:
    std::vector<Snowflake> snowflakes;
    float spawnInterval = 1.0f; // 每秒生成雪花的时间间隔
    float elapsedTime = 0.0f;
    float xRange = 16.0f; // x轴范围
    float yStart = 10.0f; // y轴起始高度
    float zRange = 16.0f; // z轴范围
    float xVelRange = 1.0f; // x轴速度范围
    float yVel = -2.0f; // y轴速度（向下）
    float zVelRange = 1.0f; // z轴速度范围
    void update(float deltaTime)
    {
        elapsedTime += deltaTime;
        if (elapsedTime >= spawnInterval)
        {
            elapsedTime = 0.0f;
            int count = rand() % 5 + 1; // 随机生成1到5朵雪花
            for (int i = 0; i < count; i++)
            {
                float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * xRange - 8.0f;
                float y = yStart;
                float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * zRange - 8.0f;

                float xVel = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * xVelRange) - xVelRange / 2.0f;
                float zVel = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * zVelRange) - zVelRange / 2.0f;
                glm::vec3 pos = glm::vec3(x, y, z);
                glm::vec3 vel = glm::vec3(xVel, yVel, zVel);
                snowflakes.push_back(Snowflake(pos, vel));
            }
        }

        for (auto it = snowflakes.begin(); it != snowflakes.end();)
        {
            it->update(deltaTime);
            if (it->position.y <= 0)
            {
                it = snowflakes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void draw(Shader& shader, Model& model)
    {
        for (const auto& snowflake : snowflakes)
        {
            drawCrystal(shader, model, snowflake.position);
        }
    }
};

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
    Model crystal("resources/crystal/crystal.obj");

    // 加载天空盒
    std::vector<std::string> faces
    {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };
    Skybox skybox(faces);

    glm::vec3 lightColor = glm::vec3(2.0f, 2.0f, 2.0f);
    glm::vec3 lightPos = glm::vec3(10.0f, 10.0f, 10.0f);

    /*GLuint depthMapFBO, depthMap;
    initDepthBuffer(depthMapFBO, depthMap);*/
    SnowflakeGenerator generator;
    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 记录每一帧的时间差
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 
        // 计算移动时间
        float speedFactor = 0.2f;
        float timeValue = glfwGetTime() * speedFactor;
        // 使用球面坐标计算光源位置
        float maxAltitude = 1.0f;
        float lightPosX = 20.0f * cos(timeValue);
        float lightPosY = 20.0f * sin(timeValue);
        float lightPosZ = sin(timeValue) * maxAltitude;

        glm::vec3 lightPos = glm::vec3(lightPosX, lightPosY, lightPosZ);
        // 渲染阴影贴图
        //renderShadowMap(depthShader, depthMapFBO, lightPos);
        shader.use();
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        // 渲染阴影贴图
        // renderShadowMap(depthShader, depthMapFBO, lightPos);

        // 应用阴影到场景
        //applyShadow(shader, depthMap, lightSpaceMatrix, lightPos, lightColor);

        // 处理输入
        processInput(window);

        // 渲染
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawStamp(shader, stump, stumpPosition, stumpRotation, stumpScale);
        drawHouse(shader, house);
        drawSnowman(shader, snowman);
        generator.update(deltaTime);
        generator.draw(shader, crystal);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f,
                                                100.0f);
        // 放大天空盒
        // projection = glm::scale(projection, glm::vec3(10.0f, 10.0f, 10.0f));
        skybox.draw(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void initDepthBuffer(GLuint& depthMapFBO, GLuint& depthMap)
{
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderShadowMap(Shader& depthShader, GLuint depthMapFBO, glm::vec3 lightPos)
{
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 7.5f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    depthShader.use();
    depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    // ... 渲染场景的代码 ...
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void applyShadow(Shader& shader, GLuint depthMap, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, glm::vec3 lightColor)
{
    shader.use();
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("lightColor", lightColor);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    // ... 渲染场景的代码 ...
}


void drawStamp(Shader shader, Model stump, glm::vec3 stumpPosition, glm::vec3 stumpRotation, glm::vec3 stumpScale)
{
    shader.use();

    glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f,
                                               100.0f);
    glm::mat4 viewMat = camera.GetViewMatrix();
    shader.setMat4("projection", projectionMat);
    shader.setMat4("view", viewMat);

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, stumpPosition);
    modelMat = glm::scale(modelMat, stumpScale);
    modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(stumpRotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // 应用X轴旋转
    modelMat = glm::rotate(modelMat, glm::radians(stumpRotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // 应用Y轴旋转
    modelMat = glm::rotate(modelMat, glm::radians(stumpRotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // 应用Z轴旋转
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

    // 处理树桩旋转和缩放
    bool ctrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (ctrlPressed)
    {
        if (shiftPressed)
        {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                stumpRotation.z += 10.0f;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                stumpRotation.z -= 10.0f;
        }
        else
        {
            // 处理旋转
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                stumpRotation.x += 10.0f;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                stumpRotation.x -= 10.0f;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                stumpRotation.y -= 10.0f;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                stumpRotation.y += 10.0f;
        }
    }
    else
    {
        if (shiftPressed)
        {
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                stumpScale.z -= 0.01f;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                stumpScale.z += 0.01f;
        }
        else
        {
            // 处理树桩X轴和Y轴缩放（无需按住Shift或Ctrl）
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                stumpScale.y += 0.01f;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                stumpScale.y -= 0.01f;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                stumpScale.x -= 0.01f;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                stumpScale.x += 0.01f;
        }
    }


    // 检测鼠标按下状态
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (!dragging)
        {
            dragging = true;
            lastMousePos = glm::vec2(xpos, ypos);

            glm::vec3 rayDirection = ScreenPosToWorldRay(xpos, ypos, SCR_WIDTH, SCR_HEIGHT, camera.GetViewMatrix(),
                                                         camera.GetProjectionMatrix(
                                                             static_cast<float>(SCR_WIDTH) / static_cast<float>(
                                                                 SCR_HEIGHT), 0.1f, 100.0f));
            previousWorldCoords = RayPlaneIntersection(camera.Position, rayDirection, glm::vec3(0, 1, 0),
                                                       glm::vec3(0, 0, 0)); // 假设平面为y=0
        }
        else
        {
            glm::vec3 rayDirection = ScreenPosToWorldRay(xpos, ypos, SCR_WIDTH, SCR_HEIGHT, camera.GetViewMatrix(),
                                                         camera.GetProjectionMatrix(
                                                             static_cast<float>(SCR_WIDTH) / static_cast<float>(
                                                                 SCR_HEIGHT), 0.1f, 100.0f));
            glm::vec3 currentWorldCoords = RayPlaneIntersection(camera.Position, rayDirection, glm::vec3(0, 1, 0),
                                                                glm::vec3(0, 0, 0)); // 假设平面为y=0

            glm::vec3 deltaWorld = currentWorldCoords - previousWorldCoords;
            stumpPosition += deltaWorld;
            previousWorldCoords = currentWorldCoords;
        }
    }
    else if (dragging)
    {
        dragging = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        stumpPosition = glm::vec3(0.0f, 0.0f, 5.0f);
        stumpRotation = glm::vec3(0.0f, 0.0f, 0.0f);
        stumpScale = glm::vec3(0.1f, 0.1f, 0.1f);
    }
}

glm::vec3 screenToWorldCoords(double xpos, double ypos, GLFWwindow* window, glm::mat4 view, glm::mat4 projection)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    float z;
    glReadPixels(xpos, height - ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

    glm::vec4 screenPos = glm::vec4(
        (2.0f * xpos) / width - 1.0f,
        1.0f - (2.0f * ypos) / height,
        z * 2.0f - 1.0f,
        1.0f
    );

    glm::vec4 worldPos = glm::inverse(projection * view) * screenPos;
    worldPos /= worldPos.w;

    return glm::vec3(worldPos.x, worldPos.y, worldPos.z);
}


glm::vec3 ScreenPosToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, glm::mat4 ViewMatrix,
                              glm::mat4 ProjectionMatrix)
{
    glm::vec4 rayStartNDC(
        ((float)mouseX / (float)screenWidth - 0.5f) * 2.0f,
        ((float)mouseY / (float)screenHeight - 0.5f) * 2.0f,
        -1.0f,
        1.0f
    );
    glm::vec4 rayEndNDC(
        ((float)mouseX / (float)screenWidth - 0.5f) * 2.0f,
        ((float)mouseY / (float)screenHeight - 0.5f) * 2.0f,
        0.0f,
        1.0f
    );

    glm::vec4 rayStartWorld = glm::inverse(ProjectionMatrix * ViewMatrix) * rayStartNDC;
    rayStartWorld /= rayStartWorld.w;
    glm::vec4 rayEndWorld = glm::inverse(ProjectionMatrix * ViewMatrix) * rayEndNDC;
    rayEndWorld /= rayEndWorld.w;

    glm::vec3 rayDirWorld(rayEndWorld - rayStartWorld);
    rayDirWorld = glm::normalize(rayDirWorld);

    return rayDirWorld;
}


glm::vec3 RayPlaneIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planeNormal, glm::vec3 planePoint)
{
    float denom = glm::dot(planeNormal, rayDirection);
    if (abs(denom) > 0.0001f)
    {
        // 非平行
        float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;
        return rayOrigin + rayDirection * t;
    }
    return glm::vec3(0, 0, 0);
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
