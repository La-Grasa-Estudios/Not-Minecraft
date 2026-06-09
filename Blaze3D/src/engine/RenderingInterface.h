#pragma once

#include <cstdint>
#include <cstddef>

#include <thirdparty/glm/ext.hpp>

#include "Common.h"

struct riNativeData;

enum riPrimitiveType
{
	RI_PRIMITIVE_TYPE_TRIANGLES,
	RI_PRIMITIVE_TYPE_TRIANGLE_STRIP,
	RI_PRIMITIVE_TYPE_LINE_LIST,
	RI_PRIMITIVE_TYPE_LINE_STRIP,
	RI_PRIMITIVE_TYPE_POINT_LIST,
	RI_PRIMITIVE_TYPE_QUAD
};

enum riFormat
{
	RI_FORMAT_R8G8B8A8_UNORM,
	RI_FORMAT_B8G8R8A8_UNORM,
	RI_FORMAT_R16G16B16A16_FLOAT,
	RI_FORMAT_R32G32B32A32_FLOAT,
};

enum riVertexFormat
{
	RI_VERTEX_FORMAT_FLOAT,
	RI_VERTEX_FORMAT_FLOAT2,
	RI_VERTEX_FORMAT_FLOAT3,
	RI_VERTEX_FORMAT_FLOAT4,
	RI_VERTEX_FORMAT_UINT8_4,
	RI_VERTEX_FORMAT_UINT16_2,
	RI_VERTEX_FORMAT_UINT16_4
};

enum riComparisonMode
{
	RI_COMPARISON_MODE_LESS,
	RI_COMPARISON_MODE_LESS_EQUAL,
	RI_COMPARISON_MODE_EQUAL,
	RI_COMPARISON_MODE_GREATER_EQUAL,
	RI_COMPARISON_MODE_GREATER
};

enum riFilter
{
	RI_FILTER_NEAREST,
	RI_FILTER_LINEAR
};

enum riCullMode
{
	RI_CULL_MODE_NONE,
	RI_CULL_MODE_FRONT,
	RI_CULL_MODE_BACK
};

enum riBlendMode
{
	BLEND_MODE_ZERO,
	BLEND_MODE_ONE,
	BLEND_MODE_SRC_COLOR,
	BLEND_MODE_INV_SRC_COLOR,
	BLEND_MODE_DST_COLOR,
	BLEND_MODE_INV_DST_COLOR,
	BLEND_MODE_SRC_ALPHA,
	BLEND_MODE_INV_SRC_ALPHA,
};

class riTexture
{

};

struct riVertexAttribute
{
	bool Enabled = false;
	uint16_t Offset = 0;
	riVertexFormat Format;
};

struct riVertexAttributeTable
{
	riVertexAttribute Position[8];
	riVertexAttribute Normal[8];
	riVertexAttribute Color[8];
	riVertexAttribute TexCoord[8];
	size_t Strides[8]; // Auto populated
};

enum riLightType
{
	RI_LIGHT_TYPE_DIRECTIONAL,
};

struct riLight
{
	riLightType LightType;
	union
	{
		struct
		{
			glm::vec3 Direction;
			glm::vec3 Diffuse;
			glm::vec3 Ambient;
			glm::vec3 Specular;
			float Intensity = 1.0f;
		} Directional;
	} Data;
};

class riBuffer
{
public:
	uint32_t VertexCount;
	uint32_t Stride;
	void* Data;
};

class riDevice;

class riContext
{
	friend riDevice;
public:
	riNativeData* NativeData;

	void Init();
	void Shutdown();
	bool ShouldClose();

	riHandle GetSurfaceHandle();
	void SwapBuffers();

	riHandle LoadTexture(const char* path);
	riHandle CreateTexture2D(uint32_t width, uint32_t height, uint32_t mipLevels, riFormat format, const void* pData);
	void UpdateTexturePart(riHandle texture, uint32_t xoff, uint32_t yoff, uint32_t width, uint32_t height, const void* data);
	void SetTextureFilter(riHandle texture, riFilter filter);
	void SetSync(bool sync);

	void DestroyTexture2D(riHandle pTexture);

	riHandle CreateBuffer(uint32_t size, void* data);
	void DestroyBuffer(riHandle pBuffer);

	void SetVertexAttributeTable(riVertexAttributeTable table);

	uint32_t Width;
	uint32_t Height;

private:

	riVertexAttributeTable m_ActiveVTA;
};

class riDevice
{
public:
	void Init(riContext* pContext);
	void Shutdown();

	void SetViewport(float orgX, float orgY, float width, float height);
	void SetDepthRange(float zNear, float zFar);
	void ClearSurface(riHandle handle, float r, float g, float b, float a, bool depth, bool color);

	void SetProjectionMatrix(float* matrix);
	void SetModelViewMatrix(float* matrix);
	void PushMatrixStack();
	void PopMatrixStack();

	void SetDepthTest(bool enable, riComparisonMode mode);
	void SetCullMode(riCullMode mode);
	void SetTexturing(bool enable);
	void SetFogStart(float start);
	void SetFogEnd(float end);
	void SetFogColor(float r, float g, float b);
	void SetFog(bool enabled);
	void SetLighting(bool enabled);
	void SetSmoothShade(bool enabled);
	void SetAlphaTest(bool enabled);
	void SetBlending(bool enabled);
	void SetBlendMode(riBlendMode src, riBlendMode dst);

	void SetLight(riLight* pLight, bool enabled, uint32_t idx);

	void SetTexture(riHandle texture, uint32_t slot);
	void Draw(riHandle buffer, uint8_t vtfIndex, riPrimitiveType type, uint32_t count);
	void DrawIndexed(riHandle buffer, riHandle indexBuffer, uint8_t vtfIndex, riPrimitiveType type, uint32_t count);

	void GetWindowSize(float* d);
private:
	riContext* m_Context;
};