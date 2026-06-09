#include "IngameInterface.h"
#include "engine/RenderingInterface.h"
#include "engine/System.h"
#include "engine/Time.h"

#include "common/block/Block.h"
#include "client/renderer/VertexProducer.h"
#include "common/entity/PlayerEntity.h"
#include "client/Client.h"
#include "client/ResourceManager.h"
#include "client/ui/ScreenInventory.h"
#include "client/renderer/ItemRenderer.h"
#include "client/renderer/RenderHelper.h"

#include "thirdparty/glm/ext.hpp"

float easeInOut(float x) {
    return -(glm::cos(glm::pi<float>() * x) - 1) / 2;
}

extern sysInputData g_InputData;
VertexProducer producer{};

IngameInterface::IngameInterface(PlayerEntity* player, riDevice* device)
{
	m_Player = player;
	m_Device = device;
}

void RenderPlayerArm(riDevice* device, VertexProducer& producer, glm::mat4& model, float dig_lerp, float swing_lerp)
{
    Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Player, device);

    float sinHalfCircle = sinf(dig_lerp * glm::pi<float>());
    float sqrtLerpPI = sqrtf(dig_lerp) * glm::pi<float>();
    float sinHalfCircleWeird = sinf(glm::pow(dig_lerp, 2) * glm::pi<float>());

    glm::vec3 translation(
        0.64f - sinf(sqrtLerpPI) * 0.3f,
        -0.65f + sinf(sqrtLerpPI * 2.0f) * 0.4f - 0.6f * sinf(swing_lerp * glm::pi<float>()),
        -0.72f - sinHalfCircle * 0.4f
    );

    model = glm::translate(model, translation);
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(sinf(sqrtLerpPI) * 70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-sinHalfCircleWeird * 20.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::translate(model, glm::vec3(-1.0f, 3.6f, 3.5f));
    model = glm::rotate(model, glm::radians(120.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Z
    model = glm::rotate(model, glm::radians(200.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // X
    model = glm::rotate(model, glm::radians(-135.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Y
    model = glm::translate(model, glm::vec3(5.6f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-5.0f / 16.0f, 2.0f / 16.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f / 16.0f));
    model = glm::translate(model, glm::vec3(-3.0f, -2.0f, -2.0f));

    float minX = 0.0f;
    float maxX = 4;
    float minY = 0.0f;
    float maxY = 12;
    float minZ = 0.0f;
    float maxZ = 4;

    producer.SetColor(1.0f, 1.0f, 1.0f);

    for (int face = 0; face < 3; face++)
    {
        float u0 = 44.0f / 64;
        float u2 = 48.0f / 64;
        float v0 = 32.0f / 32;
        float v2 = 20.0f / 32;

        float x2 = minX;
        float x3 = maxX;
        float y2 = minY;
        float y3 = maxY;
        float z2 = minZ;
        float z3 = maxZ;

        producer.SetColor(1.0f, 1.0f, 1.0f);
        if (face == 0) {
            producer.SetNormal(0.0f, -1.0f, 0.0f);
            producer.AddVertex(x2, y2, z3, u0, v2);
            producer.AddVertex(x2, y2, z2, u0, v0);
            producer.AddVertex(x3, y2, z2, u2, v0);
            producer.AddVertex(x3, y2, z3, u2, v2);
            continue;
        }
        if (face == 1) {
            producer.SetNormal(0.0f, 0.0f, -1.0f);
            producer.AddVertex(x2, y3, z2, u2, v0);
            producer.AddVertex(x3, y3, z2, u0, v0);
            producer.AddVertex(x3, y2, z2, u0, v2);
            producer.AddVertex(x2, y2, z2, u2, v2);
            continue;
        }
        if (face == 2) {
            producer.SetNormal(-1.0f, 0.0f, 0.0f);
            producer.AddVertex(x2, y3, z3, u2, v0);
            producer.AddVertex(x2, y3, z2, u0, v0);
            producer.AddVertex(x2, y2, z2, u0, v2);
            producer.AddVertex(x2, y2, z3, u2, v2);
            continue;
        }
    }
}

void IngameInterface::Render()
{
    Client::GetInstance().RenderBlockOutline();
    auto& client = Client::GetInstance();

    static uint32_t RenderId = 0;
    static uint32_t SwapId = 0;
    static float swing_time = 0;
    static bool DoSwing = false;
    static glm::vec3 camLag;
    static float lastDeltaY;

    float deltaY = 8.0f * client.Player->Interpolate(client.Player->LastVelocity, client.Player->Velocity, client.UpdateTimer.a).y;
    lastDeltaY = glm::mix(lastDeltaY, deltaY, glm::min(Time::DeltaTime * 32.0f, 0.9f));

	float place_lerp = 0.0f;
	float swing_lerp = 0.0f;
	float dig_lerp = 0.0f;
    static float dig_time = 0.0f;
    static bool is_place_swing = false;

    IsAnimating = false;

    if (DoPlaceSwing)
    {
        DoPlaceSwing = false;
        dig_time = 0.07f;
        is_place_swing = true;
        IsAnimating = true;
    }

    if (DoDigSwing)
    {
        DoDigSwing = false;
        dig_time = 0.01f;
		is_place_swing = false;
        IsAnimating = true;
    }

    if (g_InputData.MainActionDown)
    {
        dig_time = 0.0f;
    }

	if ((g_InputData.MainAction) || dig_time != 0.0f)
	{
        float warp = is_place_swing ? 0.6f : 1.0f;
		dig_time += Time::DeltaTime * warp;


		if (g_InputData.MainAction && Client::GetInstance().DidHitBlock && !is_place_swing)
		{
			dig_time = fmod(dig_time, 0.2f);
        }

		if (dig_time > 0.3f)
		{
			dig_time = 0.0f;
            is_place_swing = false;
		}
    }

    if (DoSwing)
    {
        swing_time += Time::DeltaTime;
        if (swing_time >= 0.25f)
        {
            swing_time = 0.0f;
            DoSwing = false;
        }
    }

	place_lerp = dig_time / 0.30f;
	dig_lerp = dig_time / 0.30f;
    swing_lerp = swing_time / 0.25f;

    if (DoSwing)
    {
        if (swing_lerp > 0.5f)
        {
            RenderId = SwapId;
        }
    }

	float sinHalfCircle = sinf(dig_lerp * glm::pi<float>());
	float sqrtLerpPI = sqrtf(dig_lerp) * glm::pi<float>();
	float sinHalfCircleWeird = sinf(glm::pow(dig_lerp, 2) * glm::pi<float>());

	auto swingFactor = Client::GetInstance().Player->Interpolate(Client::GetInstance().Player->LastSwingFactor, Client::GetInstance().Player->SwingFactor, Client::GetInstance().UpdateTimer.a);

    swingFactor = easeInOut(swingFactor);

	glm::mat4 model = glm::mat4(1.0f);

    glm::vec3 bob = {};
    float interpBob = glm::pi<float>() * glm::two_over_pi<float>() * Client::GetInstance().Player->Interpolate(Client::GetInstance().Player->LastWalkDist, Client::GetInstance().Player->WalkDist, Client::GetInstance().UpdateTimer.a);

    bob = glm::vec3(sin(interpBob) * 0.05f, -glm::abs(glm::cos(interpBob)) * 0.1f, 0.0f) * swingFactor;

    model = glm::translate(model, bob);

    auto playerHeldId = client.Player->Inventory.Resources[client.Player->HotbarSlot].Id;
    if (playerHeldId != SwapId && !DoSwing)
    {
        SwapId = playerHeldId;
        DoSwing = true;
        swing_time = 0.0f;
    }

    producer.Reset();
    producer.SetLightMultiplier(client.Player->GetLightLevel());

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view, glm::radians(-lastDeltaY), glm::vec3(1, 0, 0));
    //view = glm::rotate(view, glm::radians(delta.y), glm::vec3(0, 1, 0));
    //view = glm::rotate(view, glm::radians(delta.z), glm::vec3(0, 0, 1));
    view = glm::rotate(view, glm::radians(client.Player->GetCameraTilt(client.UpdateTimer.a)), glm::vec3(0, 0, 1));
    //view = view * glm::mat4_cast(deltaQuat);

    if (RenderId != 0)
    {
        
        model = glm::translate(model,
            {
            0.56F - sinf(sqrtLerpPI) * 0.4F,
                -0.52F + sinf(sqrtLerpPI * 2.0F) * 0.2F
                - 0.6F * sinf(swing_lerp * glm::pi<float>()),
                -0.72F - sinHalfCircle * 0.2F
            });

        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-sinHalfCircleWeird * 20.0F), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-sinf(sqrtLerpPI) * 20.0F), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(-sinf(sqrtLerpPI) * 80.0F), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f));
        model = glm::translate(model, glm::vec3(-0.5f));

        ItemStack stack{ RenderId, 1 };

        model = view * model;

        m_Device->PushMatrixStack();

        auto light = glm::mat4(1.0f);
        light = glm::rotate(light, glm::radians(client.Player->Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        light = glm::rotate(light, glm::radians(client.Player->Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        light = glm::rotate(light, glm::radians(client.Player->Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 proj = glm::mat4(1.0f);
        m_Device->PushMatrixStack();
        m_Device->SetModelViewMatrix(glm::value_ptr(light));
        RenderHelper::EnableItemLighting();
        m_Device->PopMatrixStack();

        m_Device->SetModelViewMatrix(glm::value_ptr(model));
        m_Device->SetDepthRange(0.0f, 0.1f);
        m_Device->SetTexturing(true);
        ItemRenderer::RenderItemStack(stack, &producer, m_Device, model, ITEM_RENDERER_ENV_FIRST_PERSON);
        m_Device->SetDepthRange(0.0f, 1.0f);
        m_Device->PopMatrixStack();

        RenderHelper::DisableItemLighting();
        return;
    }
    else
    {
        RenderPlayerArm(m_Device, producer, model, dig_lerp, swing_lerp);
    }

    auto light = glm::mat4(1.0f);
    light = glm::rotate(light, glm::radians(client.Player->Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    light = glm::rotate(light, glm::radians(client.Player->Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    light = glm::rotate(light, glm::radians(client.Player->Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    m_Device->PushMatrixStack();
    m_Device->SetModelViewMatrix(glm::value_ptr(light));
    RenderHelper::EnableItemLighting();
    m_Device->PopMatrixStack();

    auto projection = glm::perspective(glm::radians(75.0f), client.ScreenSize.x / client.ScreenSize.y, 0.01f, 4.0f);

    model = view * model;



    m_Device->SetDepthRange(0.0f, 0.1f); 

    m_Device->PushMatrixStack();
    m_Device->SetProjectionMatrix(glm::value_ptr(projection));
    m_Device->SetModelViewMatrix(glm::value_ptr(model));
    m_Device->SetTexturing(true);

	m_Device->Draw(producer.GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	m_Device->SetDepthRange(0.0f, 1.0f);
    m_Device->PopMatrixStack();
    RenderHelper::DisableItemLighting();
}