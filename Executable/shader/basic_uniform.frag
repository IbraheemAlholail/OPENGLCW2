#version 460

in vec3 Position;
in vec3 Normal;
in vec4 ShadowCoord;

uniform sampler2DShadow ShadowMap;
layout (location = 0) out vec4 FragColor;

uniform struct LightInfo {
    vec4 Position;
    vec3 La;
    vec3 L;
    float Intensity;
} Light;

uniform struct MaterialInfo {
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
} Material;

vec3 blinnPhong(vec3 position, vec3 n) {
    vec3 diffuse = vec3(0.0), spec = vec3(0.0);
    vec3 ambient = Light.La * Material.Ka;

    vec3 s = normalize(Light.Position.xyz - position);

    float sDotn = max(dot(s, n), 0.0);
    diffuse = Light.L * Material.Kd * sDotn;

    if (sDotn > 0.0) {
        vec3 v = normalize(-position.xyz);
        vec3 h = normalize(s + v);
        spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
    }
    
    return ambient + (diffuse + spec) * Light.L;
}

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

subroutine(RenderPassType)
void shadeWithShadow() {
    vec3 ambient = Light.Intensity * Material.Ka;
    vec3 diffAndSpec = blinnPhong(Position, normalize(Normal));
    float shadow = 1.0;
    if (ShadowCoord.z >= 0.0) {
        shadow = textureProj(ShadowMap, ShadowCoord);
    }
    FragColor = vec4(ambient + diffAndSpec * shadow, 1.0);
    FragColor = pow(FragColor, vec4(1.0 / 2.2));
}

subroutine(RenderPassType)
void recordDepth() {
    // No implementation needed for recordDepth subroutine
}

void main() {
    RenderPass();
}
