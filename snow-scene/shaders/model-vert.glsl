#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Normal vector input from vertex data
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor; // Color attribute

out vec2 TexCoords;
out vec3 ourColor; // Color to pass to fragment shader
out vec3 FragPos; // Fragment position for lighting calculations
out vec3 Normal;  // 传递法线向量

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal; // 转换法线向量
    TexCoords = aTexCoords;
    ourColor = aColor;
    FragPos = vec3(model * vec4(aPos, 1.0)); // Calculate fragment position
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
