#ifndef PIXELSHADER_H
#define PIXELSHADER__H

static const char PixelShader_GLSL[] = R"----(#version 320 es

precision highp float;

in vec2 v_uv;
in vec4 v_color;
in vec3 v_normal;
in vec4 v_viewPos;

out vec4 f_color;

struct Light {
    int Type;
    vec3 Direction;
    bool Enabled;
    vec3 Diffuse;
    vec4 Ambient;
    vec4 Specular;
};

uniform sampler2D Tex;

uniform bool LightingEnabled;
uniform Light Lights[8];
uniform bool FogEnabled;
uniform float FogStart;
uniform float FogEnd;
uniform vec3 FogColor;

void main() {

    vec4 col = texture(Tex, v_uv);
    if (col.a < 0.35f) discard;

    col *= v_color;

    if (LightingEnabled)
    {
        vec3 N = normalize(v_normal);

        vec3 lightAccum = Lights[0].Ambient.rgb + Lights[1].Ambient.rgb;

        vec3 L0 = normalize(Lights[0].Direction.xyz);
        lightAccum += clamp(dot(N, L0), 0.0f, 1.0f) * Lights[0].Diffuse.rgb;

        vec3 L1 = normalize(Lights[1].Direction.xyz);
        lightAccum += clamp(dot(N, L1), 0.0f, 1.0f) * Lights[1].Diffuse.rgb;

        col.rgb *= clamp(lightAccum, vec3(0), vec3(1));
    }

    if (FogEnabled)
    {
        float dist = length(v_viewPos.xyz);

        float fogFactor = (FogEnd - dist) / (FogEnd - FogStart);
        fogFactor = clamp(fogFactor, 0.0f, 1.0f);

        col.rgb = mix(FogColor, col.rgb, fogFactor);
    }

    f_color = col;
}

)----";

#endif

