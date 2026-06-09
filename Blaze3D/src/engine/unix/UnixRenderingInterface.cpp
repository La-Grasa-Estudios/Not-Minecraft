//
// Linux/MacOS OpenGL renderer.
// Uses OpenGLES 3.2 or OpenGL 3.3
//

#ifdef RENDERER_GLES || RENDERER_GL3

#include "../RenderingInterface.h"
#include "../memory/MemoryAllocator.h"

#include "shaders/PixelShader.h"
#include "shaders/VertexShader.h"

#include <SDL3/SDL.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb_image.h>

#include <fstream>
#include <vector>
#include <stack>
#include <unordered_map>

#include "glad/glad.h"

struct RenderLight {
    int Type;
    glm::vec3 Direction;
    bool Enabled;
    glm::vec3 Diffuse;
    glm::vec4 Ambient;
    glm::vec4 Specular;
};

struct RenderState {

    bool Lighting;
    RenderLight Lights[8];
    bool FogEnabled;
    float FogStart;
    float FogEnd;
    float FogColor[3];

    bool TexturesEnabled = false;
    GLuint Textures[16];

    std::stack<glm::mat4> MatrixStack;

    glm::mat4 ProjectionMatrix;
    glm::mat4 ModelViewMatrix;
};

struct riNativeData
{
    SDL_Window* pWindow;
    SDL_GLContext pGLContext;

    GLuint surface;
    GLuint whiteTexture;
    GLuint program;
    GLuint vertexArrays[8];
};

const uint32_t BufferChecksum = 0x09346135;

struct BufferData {
    uint32_t Checksum;
    GLuint Handle;
};

static RenderState  g_RenderState;
static bool         g_StateDirty;
static bool         g_WantsClose;
static GLuint       g_StreamBuffer;
static GLuint       g_QuadIndexBuffer;

extern void SDL_EventCB(SDL_Event&);

void riContext::Init() {
    Width = 1280;
    Height = 720;
    NativeData = sysNew<riNativeData>();
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        auto error = SDL_GetError();
        printf("ERROR: Failed to initialize SDL: %s\n", error);
        exit(0);
    }

    NativeData->pWindow = SDL_CreateWindow("Blaze3D Window", Width, Height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

#if defined(RENDERER_GLES)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

#if defined( DEBUG ) || defined( _DEBUG ) || !defined(NDEBUG)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    NativeData->pGLContext = SDL_GL_CreateContext(NativeData->pWindow);

    if (!NativeData->pGLContext) {

        std::string error = "Could not initialize OpenGL context: ";
        error += SDL_GetError();

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Zircon fatal error", error.c_str(), NativeData->pWindow);
        exit(1);
    }

    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        std::string error = "Could not load OpenGL ES 3.2 functions";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Zircon fatal error", error.c_str(), NativeData->pWindow);

        exit(1);
    }

    // Load the shaders

    GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);

    const char* sources[2] = {
        VertexShader_GLSL,
        PixelShader_GLSL
    };

    glShaderSource(vertexHandle, 1, &sources[0], nullptr);
    glShaderSource(fragmentHandle, 1, &sources[1], nullptr);

    glCompileShader(vertexHandle);
    glCompileShader(fragmentHandle);

    int status;
    char ERROR_LOG[2048];

    glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glGetShaderInfoLog(vertexHandle, sizeof(ERROR_LOG), nullptr, ERROR_LOG);

        std::string error = "Could not load OpenGL ES 3.2 vertex shader: ";
        error += ERROR_LOG;

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Zircon fatal error", error.c_str(), NativeData->pWindow);
        exit(1);
    }

    glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glGetShaderInfoLog(fragmentHandle, sizeof(ERROR_LOG), nullptr, ERROR_LOG);

        std::string error = "Could not load OpenGL ES 3.2 pixel shader: ";
        error += ERROR_LOG;

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Zircon fatal error", error.c_str(), NativeData->pWindow);
        exit(1);
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexHandle);
    glAttachShader(program, fragmentHandle);

    glLinkProgram(program);

    glDetachShader(program, vertexHandle);
    glDetachShader(program, fragmentHandle);

    glDeleteShader(vertexHandle);
    glDeleteShader(fragmentHandle);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        glGetProgramInfoLog(fragmentHandle, sizeof(ERROR_LOG), nullptr, ERROR_LOG);

        std::string error = "Could not load OpenGL ES 3.2 shader: ";
        error += ERROR_LOG;

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Zircon fatal error", error.c_str(), NativeData->pWindow);
        exit(1);
    }

    glUseProgram(program);

    NativeData->program = program;

    // Default FBO

    NativeData->surface = 0;

    // Null texture

    uint32_t data = 0xFFFFFFFF;
    NativeData->whiteTexture = *(size_t*)CreateTexture2D(1, 1, 1, RI_FORMAT_R8G8B8A8_UNORM, &data);

    // Vertex array
    glGenVertexArrays(8, NativeData->vertexArrays);

    // Streaming buffer
    glGenBuffers(1, &g_StreamBuffer);

    // Quad rendering
    glGenBuffers(1, &g_QuadIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_QuadIndexBuffer);

    std::vector<uint32_t> quadIndices;
    uint32_t maxQuads = 32000;
    quadIndices.reserve(maxQuads * 6);

    for (uint32_t i = 0; i < maxQuads; i++)
    {
        uint16_t offset = (uint16_t)(i * 4);
        quadIndices.push_back(offset + 0);
        quadIndices.push_back(offset + 1);
        quadIndices.push_back(offset + 2);
        quadIndices.push_back(offset + 0);
        quadIndices.push_back(offset + 2);
        quadIndices.push_back(offset + 3);
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadIndices.size() * sizeof(uint32_t), quadIndices.data(), GL_STATIC_DRAW);


    printf("GLES 3.2 Initialized!\n");


}

void riContext::Shutdown() {
    SDL_GL_DestroyContext(NativeData->pGLContext);
    SDL_DestroyWindow(NativeData->pWindow);

    sysFree(NativeData);
}

bool riContext::ShouldClose() {
    return g_WantsClose;
}

void riContext::SetSync(bool sync) {
    SDL_GL_SetSwapInterval(sync ? 1 : 0);
}

void riContext::SwapBuffers() {
    SDL_GL_SwapWindow(this->NativeData->pWindow);

    int newWidth, newHeight;
    SDL_GetWindowSize(NativeData->pWindow, &newWidth, &newHeight);

    // Safety for minimized windows
    if (newWidth < 1) newWidth = 1;
    if (newHeight < 1) newHeight = 1;

    if (newWidth != Width || newHeight != Height)
    {
        Width = newWidth;
        Height = newHeight;

        glViewport(0, 0, Width, Height);
    }

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            g_WantsClose = true;
        }
        SDL_EventCB(e);
    }

    static bool f11pressed = false;
    auto window = NativeData->pWindow;
}

void EnsureStateSync(riNativeData* data) {

    if (!g_StateDirty)
        return;

    g_StateDirty = false;

    glUniform1i(glGetUniformLocation(data->program, "FogEnabled"), g_RenderState.FogEnabled ? 1 : 0);
    glUniform1i(glGetUniformLocation(data->program, "LightingEnabled"), g_RenderState.Lighting ? 1 : 0);

    glUniformMatrix4fv(glGetUniformLocation(data->program, "ProjectionMatrix"), 1, true, glm::value_ptr(g_RenderState.ProjectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(data->program, "ModelViewMatrix"), 1, true, glm::value_ptr(g_RenderState.ModelViewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(data->program, "NormalMatrix"), 1, true, glm::value_ptr(glm::transpose(glm::inverse(g_RenderState.ModelViewMatrix))));

    if (g_RenderState.FogEnabled) {
        glUniform1f(glGetUniformLocation(data->program, "FogStart"), g_RenderState.FogStart);
        glUniform1f(glGetUniformLocation(data->program, "FogEnd"), g_RenderState.FogEnd);
        glUniform3fv(glGetUniformLocation(data->program, "FogColor"), 1, g_RenderState.FogColor);
    }

    if (g_RenderState.Lighting) {

        char BUFFER_NAME[100];

        for (int i = 0; i < 8; i++) {

            sprintf(BUFFER_NAME, "Lights[%d].Type", i);
            glUniform1i(glGetUniformLocation(data->program, BUFFER_NAME), g_RenderState.Lights[i].Type);

            sprintf(BUFFER_NAME, "Lights[%d].Direction", i);
            glUniform3fv(glGetUniformLocation(data->program, BUFFER_NAME), 1, &g_RenderState.Lights[i].Direction.x);

            sprintf(BUFFER_NAME, "Lights[%d].Enabled", i);
            glUniform1i(glGetUniformLocation(data->program, BUFFER_NAME), g_RenderState.Lights[i].Enabled ? 1 : 0);

            sprintf(BUFFER_NAME, "Lights[%d].Diffuse", i);
            glUniform3fv(glGetUniformLocation(data->program, BUFFER_NAME), 1, &g_RenderState.Lights[i].Diffuse.x);

            sprintf(BUFFER_NAME, "Lights[%d].Ambient", i);
            glUniform4fv(glGetUniformLocation(data->program, BUFFER_NAME), 1, &g_RenderState.Lights[i].Ambient.x);

            sprintf(BUFFER_NAME, "Lights[%d].Specular", i);
            glUniform4fv(glGetUniformLocation(data->program, BUFFER_NAME), 1, &g_RenderState.Lights[i].Specular.x);

        }
    }

    // TODO: Bind textures

    if (g_RenderState.TexturesEnabled) {


        for (int i = 0; i < 16; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, g_RenderState.Textures[i]);
        }

    } else {
        for (int i = 0; i < 16; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, data->whiteTexture);
        }
    }

}

void riContext::UpdateTexturePart(riHandle texture, uint32_t xoff, uint32_t yoff, uint32_t width, uint32_t height, const void *data) {
    GLuint handle = *static_cast<GLuint *>(texture);

    glBindTexture(GL_TEXTURE_2D, handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xoff, yoff, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

riHandle riContext::GetSurfaceHandle() {
    return &NativeData->surface;
}

riHandle riContext::LoadTexture(const char *path) {
    // TODO: stub
    std::string p = path;

    if (!p.ends_with("rtex"))
    {
        int w, h, c;
        auto data = stbi_load(path, &w, &h, &c, 4);
        if (data)
        {
            auto handle = CreateTexture2D(w, h, 1, RI_FORMAT_R8G8B8A8_UNORM, data);
            SetTextureFilter(handle, RI_FILTER_NEAREST);
            stbi_image_free(data);
            return handle;
        }
        else
        {
            fprintf(stderr, "Could not load texture '%s': %s\n", path, stbi_failure_reason());

            uint32_t nullTexture[] = {
                0xFF00FFFF, 0xFF000000,
                0xFF000000, 0xFF00FFFF
            };

            return CreateTexture2D(2, 2, 1, RI_FORMAT_R8G8B8A8_UNORM, nullTexture);
        }
    }

    int w, h, c;
    std::ifstream file(p, std::ios::binary);

    // Should make this load a dummy error texture like missing CS:S
    if (!file.is_open())
    {
        fprintf(stderr, "Could not load texture '%s'\n", path);

        uint32_t nullTexture[] = {
            0xFF00FFFF, 0xFF000000,
            0xFF000000, 0xFF00FFFF
        };

        return CreateTexture2D(2, 2, 1, RI_FORMAT_R8G8B8A8_UNORM, nullTexture);
    }

    file.read((char*)&w, 4);
    file.read((char*)&h, 4);
    file.read((char*)&c, 4);

    if (c < 3)
    {
        exit(0);
    }

    size_t sz = w * h * c;
    std::vector<uint8_t> texData(sz);
    file.read((char*)texData.data(), sz);

    return CreateTexture2D(w, h, 1, RI_FORMAT_R8G8B8A8_UNORM, texData.data());
}

GLenum GL_INTERNAL_FORMAT_RI[] = {
    GL_RGBA8,
    GL_RGBA8,
    GL_RGBA16F,
    GL_RGBA32F
};

riHandle riContext::CreateTexture2D(uint32_t width, uint32_t height, uint32_t mipLevels, riFormat format, const void *pData) {

    GLuint handle = 0;
    glGenTextures(1, &handle);

    glBindTexture(GL_TEXTURE_2D, handle);

    // TODO: Respect the input data maybe? We're currently just loading anything given as RGBA8.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTERNAL_FORMAT_RI[format], width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    auto* ret = static_cast<GLuint *>(sysAlloc(sizeof(GLuint)));
    *ret = handle;
    return ret;

}

void riContext::DestroyTexture2D(riHandle pTexture) {
    if (!pTexture)
        return;

    GLuint handle = *static_cast<GLuint *>(pTexture);
    glDeleteTextures(1, &handle);
    sysFree(pTexture);
}

GLenum GL_FILTER_RI[] = {
    GL_NEAREST,
    GL_LINEAR
};

void riContext::SetTextureFilter(riHandle texture, riFilter filter) {
    if (!texture)
        return;

    GLuint handle = *static_cast<GLuint *>(texture);

    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_FILTER_RI[filter]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_FILTER_RI[filter]);
}

riHandle riContext::CreateBuffer(uint32_t size, void *data) {
    // TODO: stub
    GLuint handle = 0;
    glGenBuffers(1, &handle);

    glBindBuffer(GL_COPY_WRITE_BUFFER, handle);
    glBufferData(GL_COPY_WRITE_BUFFER, size, data, GL_STATIC_DRAW);


    BufferData* buffer = static_cast<BufferData *>(sysAlloc(sizeof(GLuint)));
    buffer->Checksum = BufferChecksum;
    buffer->Handle = handle;

    return buffer;
}

void riContext::DestroyBuffer(riHandle pBuffer) {
    BufferData handle = *static_cast<BufferData *>(pBuffer);
    glDeleteBuffers(1, &handle.Handle);
    sysFree(pBuffer);
}

int VERTEX_FORMAT_SIZES_RI[] = {
    1,
    2,
    3,
    4,
    4,
    2,
    4
};

GLenum VERTEX_FORMAT_TYPES_RI[] = {
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_SHORT
};

GLenum VERTEX_FORMAT_NORMALIZE_RI[] = {
    GL_FALSE,
    GL_FALSE,
    GL_FALSE,
    GL_FALSE,
    GL_TRUE,
    GL_TRUE,
    GL_TRUE
};

int VERTEX_FORMAT_BSIZES_RI[] = {
    4 * 1,
    4 * 2,
    4 * 3,
    4 * 4,
    1 * 4,
    2 * 2,
    2 * 4
};

void riContext::SetVertexAttributeTable(riVertexAttributeTable table) {

    // Why are we setting this??
    m_ActiveVTA = table;

    for (int i = 0; i < 8; i++) {
        int sz = 0;
        if (table.Position[i].Enabled)
            sz += VERTEX_FORMAT_BSIZES_RI[table.Position[i].Format];
        if (table.Normal[i].Enabled)
            sz += VERTEX_FORMAT_BSIZES_RI[table.Normal[i].Format];
        if (table.Color[i].Enabled)
            sz += VERTEX_FORMAT_BSIZES_RI[table.Color[i].Format];
        if (table.TexCoord[i].Enabled)
            sz += VERTEX_FORMAT_BSIZES_RI[table.TexCoord[i].Format];
        m_ActiveVTA.Strides[i] = sz;
    }
}



void BindAttribute(int index, riVertexAttribute* attribute, int stride) {
    glVertexAttribPointer(index, VERTEX_FORMAT_SIZES_RI[attribute->Format],
      VERTEX_FORMAT_TYPES_RI[attribute->Format],
      VERTEX_FORMAT_NORMALIZE_RI[attribute->Format],
      stride,
      reinterpret_cast<void *>(attribute->Offset));
}

void BindVertexTable(riVertexAttributeTable* table, GLuint vao, int index) {

    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    if (table->Position[index].Enabled)
        BindAttribute(0, &table->Position[index], table->Strides[index]);
    else
        glVertexAttrib3f(0, 0, 0, 0);

    if (table->TexCoord[index].Enabled)
        BindAttribute(1, &table->TexCoord[index], table->Strides[index]);
    else
        glVertexAttrib2f(0, 0, 0);

    if (table->Color[index].Enabled)
        BindAttribute(2, &table->Color[index], table->Strides[index]);
    else
        glVertexAttrib4f(0, 1, 1, 1, 1);

    if (table->Normal[index].Enabled)
        BindAttribute(3, &table->Normal[index], table->Strides[index]);
    else
        glVertexAttrib3f(0, 0, 0, 0);
}

void riDevice::Init(riContext *pContext) {
    m_Context = pContext;
}

void riDevice::SetBlending(bool enabled) {
    if (enabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void riDevice::SetViewport(float orgX, float orgY, float width, float height) {
    glViewport(orgX, orgY, width, height);
}

GLenum GL_DEPTH_TEST_FROM_RI[] = {
    GL_LESS,
    GL_LEQUAL,
    GL_EQUAL,
    GL_GEQUAL,
    GL_GREATER
};

void riDevice::SetDepthTest(bool enable, riComparisonMode mode) {
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    glDepthFunc(GL_DEPTH_TEST_FROM_RI[mode]);
}

void riDevice::SetTexturing(bool enable) {
    // TODO: stub
    g_StateDirty = true;
    g_RenderState.TexturesEnabled = enable;
}

GLenum GL_PRIMITIVE_MODE_RI[] = {
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_LINES,
    GL_LINE_STRIP,
    GL_POINTS,
    GL_QUADS // Deprecated??
};



void riDevice::Draw(riHandle buffer, uint8_t vtfIndex, riPrimitiveType type, uint32_t count) {

    EnsureStateSync(this->m_Context->NativeData);

    // Check for OpenGL buffers
    if (*static_cast<uint32_t *>(buffer) == BufferChecksum) {
        BufferData data = *static_cast<BufferData *>(buffer);
        glBindBuffer(GL_ARRAY_BUFFER, data.Handle);
    } else {
        // We've fed it streaming data.

        glBindBuffer(GL_ARRAY_BUFFER, g_StreamBuffer);
        glBufferData(GL_ARRAY_BUFFER, count * this->m_Context->m_ActiveVTA.Strides[vtfIndex], buffer, GL_STREAM_DRAW);

    }


    glBindVertexArray(m_Context->NativeData->vertexArrays[vtfIndex]);
    BindVertexTable(&this->m_Context->m_ActiveVTA, this->m_Context->NativeData->vertexArrays[vtfIndex], vtfIndex);

    if (type == RI_PRIMITIVE_TYPE_QUAD) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_QuadIndexBuffer);
        glDrawElements(GL_TRIANGLES, (count / 4) * 6, GL_UNSIGNED_INT, nullptr);

    } else {
        glDrawArrays(GL_PRIMITIVE_MODE_RI[type], 0, count);
    }
}

void riDevice::SetDepthRange(float zNear, float zFar) {
    glDepthRangef(zNear, zFar);
}

void riDevice::ClearSurface(riHandle handle, float r, float g, float b, float a, bool depth, bool color) {
    GLbitfield mask = 0;
    if (depth)
        mask |= GL_DEPTH_BUFFER_BIT;
    if (color)
        mask |= GL_COLOR_BUFFER_BIT;

    if (mask != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, *static_cast<GLuint *>(handle));
        glClearColor(r, g, b, a);
        glClear(mask);
    }
}

void riDevice::SetFog(bool enabled) {
    // TODO: stub
    g_RenderState.FogEnabled = enabled;
    g_StateDirty = true;
}

void riDevice::SetTexture(riHandle texture, uint32_t slot) {
    // TODO: stub
    if (texture)
        g_RenderState.Textures[slot] = *static_cast<GLuint *>(texture);
    else
        g_RenderState.Textures[slot] = m_Context->NativeData->whiteTexture;

    g_StateDirty = true;
}

void riDevice::Shutdown() {
    // TODO: stub
}

void riDevice::DrawIndexed(riHandle buffer, riHandle indexBuffer, uint8_t vtfIndex, riPrimitiveType type, uint32_t count) {

    // Stub in D3D11 version.
    // Could probably just ship with a stub.

    EnsureStateSync(this->m_Context->NativeData);

    // Uncomment if needed.

/*

    // TODO: This can be called with raw vertex data too?
    GLuint handle = *static_cast<GLuint *>(buffer);
    glBindBuffer(GL_ARRAY_BUFFER, handle);

    // TODO: This can be called with raw vertex data too?
    GLuint indexHandle = *static_cast<GLuint *>(buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);

    glBindVertexArray(m_Context->NativeData->vertexArrays[vtfIndex]);
    glDrawElements(GL_PRIMITIVE_MODE_RI[type], count, GL_UNSIGNED_INT, nullptr);

    */
}

void riDevice::GetWindowSize(float *d) {
    // TODO: stub
    d[0] = m_Context->Width;
    d[1] = m_Context->Height;
}

void riDevice::PopMatrixStack() {
    g_RenderState.ModelViewMatrix = g_RenderState.MatrixStack.top();
    g_RenderState.MatrixStack.pop();
    g_RenderState.ProjectionMatrix = g_RenderState.MatrixStack.top();
    g_RenderState.MatrixStack.pop();
}

void riDevice::PushMatrixStack() {
    g_RenderState.MatrixStack.push(g_RenderState.ProjectionMatrix);
    g_RenderState.MatrixStack.push(g_RenderState.ModelViewMatrix);
}

void riDevice::SetAlphaTest(bool enabled) {
    // Does nothing.
}

GLenum GL_BLEND_MODE_RI[] = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA
};

void riDevice::SetBlendMode(riBlendMode src, riBlendMode dst) {
    glBlendFunc(GL_BLEND_MODE_RI[src], GL_BLEND_MODE_RI[dst]);
}

GLenum GL_CULL_MODE_RI[] = {
    0,
    GL_FRONT,
    GL_BACK
};
void riDevice::SetCullMode(riCullMode mode) {
    if (mode == RI_CULL_MODE_NONE)
        glDisable(GL_CULL_FACE);
    else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_CULL_MODE_RI[mode]);
    }
}

void riDevice::SetFogColor(float r, float g, float b) {
    g_RenderState.FogColor[0] = r;
    g_RenderState.FogColor[1] = g;
    g_RenderState.FogColor[2] = b;
    g_StateDirty = true;
}

void riDevice::SetFogEnd(float end) {
    g_RenderState.FogEnd = end;
    g_StateDirty = true;
}

void riDevice::SetFogStart(float start) {
    g_RenderState.FogStart = start;
    g_StateDirty = true;
}

void riDevice::SetLight(riLight *pLight, bool enabled, uint32_t idx) {

    g_RenderState.Lights[idx].Enabled = enabled;
    if (enabled)
    {
        if (pLight && pLight->LightType == RI_LIGHT_TYPE_DIRECTIONAL)
        {
            auto ambient = pLight->Data.Directional.Ambient;
            auto diffuse = pLight->Data.Directional.Diffuse;
            auto specular = pLight->Data.Directional.Specular;
            auto dir = g_RenderState.ModelViewMatrix * glm::vec4(pLight->Data.Directional.Direction, 0.0f);
            g_RenderState.Lights[idx].Ambient = glm::vec4(ambient, 1.0f);
            g_RenderState.Lights[idx].Diffuse = glm::vec4(diffuse, 1.0f);
            g_RenderState.Lights[idx].Specular = glm::vec4(specular, 1.0f);
            g_RenderState.Lights[idx].Direction = glm::normalize(dir);
        }
    }
}

void riDevice::SetLighting(bool enabled) {
    g_RenderState.Lighting = enabled;
    g_StateDirty = true;
}

void riDevice::SetModelViewMatrix(float *matrix) {
    g_RenderState.ModelViewMatrix = *reinterpret_cast<glm::mat4*>(matrix);
    g_StateDirty = true;
}

void riDevice::SetProjectionMatrix(float *matrix) {
    g_RenderState.ProjectionMatrix = *reinterpret_cast<glm::mat4*>(matrix);
    g_StateDirty = true;
}

void riDevice::SetSmoothShade(bool enabled) {
    // TODO: stub
}

#endif
