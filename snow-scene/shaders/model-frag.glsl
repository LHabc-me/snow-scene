#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;  // ���շ�������
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

void main()
{    
   // �������
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    vec3 norm = normalize(Normal); // ʹ�ô��ݵķ�������
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    vec3 ambient = 0.1 * objectColor;
    vec3 result = ambient + diffuse + specular;

    // ���������ɫ�͹���Ч��
    FragColor = vec4(result, 1.0) * texColor;  
}
