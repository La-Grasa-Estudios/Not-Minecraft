#ifndef PIXELSHADER_H
#define PIXELSHADER__H

static const char VertexShader_GLSL[] = R"----(#version 320 es

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec3 a_normal;

out vec2 v_uv;
out vec4 v_color;
out vec3 v_normal;
out vec4 v_viewPos;

uniform mat4x4 ProjectionMatrix;
uniform mat4x4 ModelViewMatrix;
uniform mat4x4 NormalMatrix;

void main() {

    v_viewPos = vec4(a_position, 1.0f) * ModelViewMatrix;
    gl_Position = v_viewPos * ProjectionMatrix;

    v_uv = a_texCoord;
    v_color = a_color;
    v_normal = normalize(a_normal * mat3x3(NormalMatrix));

}

)----";

#endif

