#version 330 core

in vec2 TexCoords;
in vec4 FragPosLightSpace; // 从顶点着色器传递的片段位置在光源空间中的坐标

uniform sampler2D shadowMap; // 阴影贴图
uniform sampler2D texture_diffuse1; // 物体的纹理

// 阴影计算函数
float ShadowCalculation(vec4 fragPosLightSpace) {
    // 获取当前片段的深度
    float currentDepth = fragPosLightSpace.z / fragPosLightSpace.w;
    // 从深度贴图中获取深度
    float shadow = 0.0;
    float bias = 0.005; // 防止阴影粘连
    float closestDepth = texture(shadowMap, fragPosLightSpace.xy).r; 
    shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main() {
    // 获取纹理颜色
    vec4 textureColor = texture(texture_diffuse1, TexCoords);

    // 计算阴影
    float shadow = ShadowCalculation(FragPosLightSpace);

    // 计算最终颜色
    vec4 lighting = vec4((1.0 - shadow) * textureColor.rgb, textureColor.a);

    // 输出最终片段颜色
    gl_FragColor = lighting;
}
