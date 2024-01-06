#ifndef SNOWFLAKE_H
#define SNOWFLAKE_H

#include <glm/glm.hpp>


#include "shader.h"
#include "model.h"

class Snowflake
{
public:
    glm::vec3 position;
    glm::vec3 velocity;

    Snowflake(glm::vec3 pos, glm::vec3 vel) :
        position(pos),
        velocity(vel)
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
    bool isSnowing = true; // 控制是否下雪的变量
    std::vector<Snowflake> snowflakes;
    float spawnInterval = 1.0f; // 每秒生成雪花的时间间隔
    float elapsedTime = 0.0f;
    float xRange = 16.0f; // x轴范围
    float yStart = 10.0f; // y轴起始高度
    float zRange = 16.0f; // z轴范围
    float xVelRange = 1.0f; // x轴速度范围
    float yVel = -2.0f; // y轴速度（向下）
    float zVelRange = 1.0f; // z轴速度范围

    SnowflakeGenerator()
    {
    }

    void update(float deltaTime)
    {
        if (!isSnowing) return; // 如果不下雪，直接返回
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

    void draw(Shader& shader, Model& model, Camera& camera, int SRC_WIDTH, int SRC_HEIGHT)
    {
        shader.use();
        for (const auto& snowflake : snowflakes)
        {
            drawCrystal(shader, model, snowflake.position, camera, SRC_WIDTH, SRC_HEIGHT);
        }
    }

    void clearSnowflakes()
    {
        snowflakes.clear(); // 清除所有雪花
    }

    void drawCrystal(Shader& shader, Model& crystal, glm::vec3 position, Camera& camera, int SCR_WIDTH, int SCR_HEIGHT)
    {
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
        shader.setVec4("color", glm::vec4(209.0f / 255.0f, 225.0f / 255.0f, 255.0f / 255.0f, 1.0f));
        crystal.Draw(shader);
        shader.setVec4("color", glm::vec4(0.0f));
    }
};

#endif
