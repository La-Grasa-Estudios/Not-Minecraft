#pragma once

class riDevice;
class BaseEntity;

class IEntityRenderer
{
public:
	virtual void RenderType(BaseEntity* pEntity, riDevice* pDevice) = 0;
	virtual ~IEntityRenderer() = default;
};

template<typename T>
class EntityRenderer : public IEntityRenderer
{
public:
	virtual void Render(T* pEntity, riDevice* pDevice) = 0;
private:
	void RenderType(BaseEntity* pEntity, riDevice* pDevice) override final
	{
		Render(static_cast<T*>(pEntity), pDevice);
	}
};