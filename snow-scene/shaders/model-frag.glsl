#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;  // 接收法线向量
in vec3 FragPos;
in vec4 FragPosLightSpace; // 添加此行

uniform sampler2D texture_diffuse1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform sampler2D shadowMap; // 阴影贴图

float ShadowCalculation(vec4 fragPosLightSpace) {
    // 获取当前片段的深度
    float currentDepth = fragPosLightSpace.z / fragPosLightSpace.w;
    // 从深度贴图中获取深度
    float shadow = 0.0;
    float bias = 0.005;
    float closestDepth = texture(shadowMap, fragPosLightSpace.xy).r; 
    shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main()
{    
   // 纹理采样
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    vec3 norm = normalize(Normal); // 使用传递的法线向量
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    vec3 ambient = 0.1 * objectColor;
    vec3 result = ambient + diffuse + specular;
    float shadow = ShadowCalculation(FragPosLightSpace); // 计算阴影
    // 结合纹理颜色和光照效果
    FragColor = vec4(result, 1.0) * texColor;  
}
