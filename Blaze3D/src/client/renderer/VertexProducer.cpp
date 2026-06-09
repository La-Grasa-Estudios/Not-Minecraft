#include "VertexProducer.h"

#include "engine/memory/MemoryAllocator.h"
#include "engine/RenderingInterface.h"
#include "engine/System.h"

#include <cstring>

VertexProducer::VertexProducer()
	: m_VertexCount(0), m_VertexData(nullptr), m_CurrentColor(0), HasColor(false)
{
}

VertexProducer::~VertexProducer()
{
	delete[] m_VertexData;
}

riHandle VertexProducer::CreateVertexBuffer(riContext* context)
{
	size_t size = sizeof(uint32_t) * 3;
	if (ConcatenateTexCoord)
	{
		size += sizeof(uint32_t) * 2;
	}
	if (HasColor) {
		size += sizeof(uint32_t) * 1;
	}
	riHandle vb = context->CreateBuffer(m_VertexCount * size, m_VertexData);
	return vb;
}

riHandle VertexProducer::GetVertexPointer()
{
	return m_VertexData;
}

uint32_t VertexProducer::GetVertexCount()
{
	return m_VertexCount;
}

void VertexProducer::Reset()
{
	m_VertexCount = 0;
	m_CurrentIndex = 0;
	m_TransX = 0;
	m_TransY = 0;
	m_TransZ = 0;
	m_LightLevel = 1;
	ConcatenateTexCoord = true;
	HasColor = false;
	HasNormal = false;
}

void VertexProducer::SetColor(uint32_t rgba)
{
	rgba |= (255 << 24);
	m_CurrentColor = bswap32(rgba);
	HasColor = true;
}
void VertexProducer::SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
#ifdef __wii__
	m_CurrentColor = (r << 24) | (g << 16) | (b << 8) | (a);
#else
	m_CurrentColor = (a << 24) | (b << 16) | (g << 8) | (r);
#endif
	HasColor = true;
}

void VertexProducer::SetColor(float r, float g, float b, float a)
{
	uint8_t R = static_cast<uint8_t>(r * 255.0f * m_LightLevel);
	uint8_t G = static_cast<uint8_t>(g * 255.0f * m_LightLevel);
	uint8_t B = static_cast<uint8_t>(b * 255.0f * m_LightLevel);
	uint8_t A = static_cast<uint8_t>(a * 255.0f);
	SetColor(R, G, B, A);
}

void VertexProducer::SetColor(float r, float g, float b)
{
	SetColor(r, g, b, 1.0f);
}

void VertexProducer::SetLightMultiplier(float l)
{
	if (l < 0)
		l = 0;
	if (l > 1)
		l = 1;
	m_LightLevel = l;
}

void VertexProducer::SetTranslation(float x, float y, float z)
{
	m_TransX = x;
	m_TransY = y;
	m_TransZ = z;
}

void VertexProducer::SetNormal(float x, float y, float z)
{
	HasNormal = true;

	int nx = (int)(x * 127.0f);
	int ny = (int)(y * 127.0f);
	int nz = (int)(z * 127.0f);

	m_CurrentNormal = nx | (ny << 8) | (nz << 16);
}

void VertexProducer::AddVertex(float x, float y, float z)
{
	AddVertex(x, y, z, 0.0f, 0.0f);
}
void VertexProducer::AddVertex(float x, float y, float z, float u, float v)
{
	x += m_TransX;
	y += m_TransY;
	z += m_TransZ;

	size_t vertexSize = 3;
	if (HasColor) {
		vertexSize += 1;
	}
	if (HasNormal) {
		vertexSize += 1;
	}
	if (ConcatenateTexCoord)
	{
		vertexSize += 2;
	}

	if (m_VertexCount + 1 > m_BufferSize - 1 || m_BufferSize == 0)
	{
		size_t lsize = m_BufferSize * 7 * sizeof(uint32_t);
		size_t size = (m_VertexCount + 256) * 7 * sizeof(uint32_t);
		uint32_t* newVertexData = (uint32_t*)sysAlloc(size);
		if (m_VertexData) {
			memcpy(newVertexData, m_VertexData, lsize);
			sysFree(m_VertexData);
		}
		m_VertexData = newVertexData;
		m_BufferSize = m_VertexCount + 256;
	}
	
	m_VertexData[m_CurrentIndex++] = *reinterpret_cast<uint32_t*>(&x);
	m_VertexData[m_CurrentIndex++] = *reinterpret_cast<uint32_t*>(&y);
	m_VertexData[m_CurrentIndex++] = *reinterpret_cast<uint32_t*>(&z);

	if (ConcatenateTexCoord)
	{
		m_VertexData[m_CurrentIndex++] = *reinterpret_cast<uint32_t*>(&u);
		m_VertexData[m_CurrentIndex++] = *reinterpret_cast<uint32_t*>(&v);
	}

	if (HasColor) {
		m_VertexData[m_CurrentIndex++] = m_CurrentColor;
	}

	if (HasNormal) {
		m_VertexData[m_CurrentIndex++] = m_CurrentNormal;
	}

	m_VertexCount++;
}