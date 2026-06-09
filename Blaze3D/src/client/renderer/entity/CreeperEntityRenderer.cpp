#include "CreeperEntityRenderer.h"

#include <common/entity/CreeperEntity.h>

void CreeperEntityRenderer::OnModelRender(CreatureEntity* pEntity, riDevice* device, const glm::mat4& model)
{
	if (GetEntity<CreeperEntity>(pEntity)->m_CreeperState == 1)
	{
		RenderEntityModel(pEntity, device, model, glm::vec4(1.0f), CREATURE_RENDER_FLAG_NOTHING);
	}
}
