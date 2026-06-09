#pragma once

#include "engine/Common.h"

class riContext;

class VertexProducer
{
public:
	VertexProducer();
	~VertexProducer();
	riHandle CreateVertexBuffer(riContext* context);
	riHandle GetVertexPointer();
	uint32_t GetVertexCount();

	void Reset();

	void SetColor(uint32_t rgba);
	void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void SetColor(float r, float g, float b, float a);
	void SetColor(float r, float g, float b);
	void SetLightMultiplier(float l);

	void SetTranslation(float x, float y, float z);
	void SetNormal(float x, float y, float z);

	void AddVertex(float x, float y, float z);
	void AddVertex(float x, float y, float z, float u, float v);

	bool ConcatenateTexCoord = true;

private:
	uint32_t m_BufferSize;
	uint32_t m_VertexCount;
	uint32_t m_CurrentIndex;
	uint32_t* m_VertexData;
	uint32_t m_CurrentColor;
	uint32_t m_CurrentNormal;
	bool HasColor = false;
	bool HasNormal = false;
	float m_TransX, m_TransY, m_TransZ, m_LightLevel;
};