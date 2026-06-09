#include "ChickenEntity.h"

DECLARE_ENTITY_CONSTRUCTOR(ChickenEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<ChickenEntity>(world);
});

ChickenEntity::ChickenEntity(World* world) : AnimalEntityBase(world)
{
	BoundingBoxWidth = 0.6f;
	BoundingBoxHeight = 0.8f;
	HeightOffset = 0.8f;
	Health = 4;
}

ChickenEntity::ChickenEntity(World* world, glm::vec3 position) : ChickenEntity(world)
{
	SetPosition(position);
}

void ChickenEntity::Update()
{
	m_LastFallDamage = FallDistance;
	AnimalEntityBase::Update();

	if (!IsGrounded && FallDistance > 1.0f)
	{
		Velocity.y *= 0.6f;
	}
}

bool ChickenEntity::TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity)
{
	if (source == DamageSource::FALL)
		return false;

	return AnimalEntityBase::TakeDamage(other, amount, source, skipImmunity);
}

//void ChickenEntity::Render(riDevice* device)
//{
//	auto model = ModelRegistry::GetInstance().GetModel(GetCreatureModel());
//
//	float wing = Interpolate(m_LastFallDamage, FallDistance, Client::GetInstance().UpdateTimer.a);
//	wing = glm::max(wing - 1.0f, 0.0f);
//
//	model->SetParameter(0, wing * 4.0f * glm::pi<float>());
//	AnimalEntityBase::Render(device);
//}

std::string_view ChickenEntity::GetCreatureModel()
{
	return "chicken";
}

std::string_view ChickenEntity::GetCreatureTaunt()
{
	return "mob.chicken";
}

std::string_view ChickenEntity::GetCreatureHurtSound()
{
	return "mob.chickenhurt";
}

std::string_view ChickenEntity::GetCreatureDieSound()
{
	return "mob.chickenhurt";
}

int ChickenEntity::GetCreatureItemDrop()
{
	return 0;
}
