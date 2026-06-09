#include "BlockRenderer.h"

#include "common/block/Block.h"
#include "common/block/BlockDoor.h"
#include "common/block/BlockLiquid.h"
#include "common/Chunk.h"
#include "common/World.h"
#include "common/world/ChunkProvider.h"

#include "client/renderer/ColorizerGrass.h"
#include "client/renderer/VertexProducer.h"

constexpr int TORCH_FACING_UP = 1;
constexpr int TORCH_FACING_NORTH = 2;
constexpr int TORCH_FACING_SOUTH = 3;
constexpr int TORCH_FACING_WEST = 4;
constexpr int TORCH_FACING_EAST = 5;

const uint8_t DOOR_FACING_MASK = 0b00000011;
const uint8_t DOOR_FLIP_FLAG_MASK = 0b00000100;
const uint8_t DOOR_OPEN_FLAG_MASK = 0b00001000;

extern VertexProducer* gfx_vp;

void BlockRenderer::RenderBlock(Block* block, Chunk* chunk, int x, int y, int z)
{
    BlockRenderType renderType = block->GetRenderType();
    switch (renderType)
    {
    case BLOCK_RENDER_TYPE_GENERIC:
    case BLOCK_RENDER_TYPE_GRASS_BLOCK:
        RenderBlockGeneric(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_CROSS:
        RenderBlockCross(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_DOOR:
        RenderBlockDoor(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_FIRE:
        RenderBlockFire(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_LIQUID:
        RenderBlockLiquid(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_PORTAL:
        RenderBlockPortal(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_STAIRS:
        RenderBlockStairs(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_TORCH:
        RenderBlockTorch(block, chunk, x, y, z);
        break;
    case BLOCK_RENDER_TYPE_CROP:
        RenderBlockCrop(block, chunk, x, y, z);
        break;
    default:
        break;
    }
}

void BlockRenderer::RenderBlockGeneric(Block* block, Chunk* chunk, int x, int y, int z)
{
    int faces = 0;

    const float f1 = 0.5F;
    const float f2 = 0.8F;
    const float f3 = 0.6F;

    float tintR = block->m_TintR;
    float tintG = block->m_TintG;
    float tintB = block->m_TintB;

    if (block->m_ColorizeWithBiome)
    {
        auto temp = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetTemperatureForColumn(x, z);
        auto hum = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetHumidityForColumn(x, z);

        auto color = ColorizerGrass::GetColor(temp, hum);
        tintR *= color.r;
        tintG *= color.g;
        tintB *= color.b;
    }

    if (block->IsFaceRenderable(chunk, x, y - 1, z, 0)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 0));

        gfx_vp->SetColor(f1 * tintR, f1 * tintG, f1 * tintB);
        RenderFaceWorld(gfx_vp, block, chunk, x, y, z, 0);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y + 1, z, 1)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 1));
        gfx_vp->SetColor(tintR, tintG, tintB);

        if (block->GetRenderType() == BLOCK_RENDER_TYPE_GRASS_BLOCK)
        {
            bool snowy = chunk->GetBlockGlobal(x, y + 1, z).Id == BLOCK_SNOW;

            if (!snowy)
            {
                auto temp = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetTemperatureForColumn(x, z);
                auto hum = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetHumidityForColumn(x, z);

                auto color = ColorizerGrass::GetColor(temp, hum);
                gfx_vp->SetColor(color.r, color.g, color.b);
            }
            else
            {
                gfx_vp->SetColor(tintR, tintG, tintB);
            }
        }

        RenderFaceWorld(gfx_vp, block, chunk, x, y, z, 1);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y, z - 1, 2)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 2));
        gfx_vp->SetColor(f2 * tintR, f2 * tintG, f2 * tintB);
        RenderFaceWorld(gfx_vp, block, chunk, x, y, z, 2);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y, z + 1, 3)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 3));
        gfx_vp->SetColor(f2 * tintR, f2 * tintG, f2 * tintB);
        RenderFaceWorld(gfx_vp, block, chunk, x, y, z, 3);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x - 1, y, z, 4)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 4));
        gfx_vp->SetColor(f3 * tintR, f3 * tintG, f3 * tintB);
        RenderFaceWorld(gfx_vp, block, chunk, x, y, z, 4);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x + 1, y, z, 5)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 5));
        gfx_vp->SetColor(f3 * tintR, f3 * tintG, f3 * tintB);
        RenderFaceWorld(gfx_vp, block, chunk, x, y, z, 5);
        faces++;
    }
}

void BlockRenderer::RenderBlockCross(Block* block, Chunk* chunk, int x, int y, int z)
{
    int tex = block->m_TextureIndex;
    int xt = (tex % 16) * 16;
    int yt = (tex / 16) * 16;

    float u0 = (xt + 16) / 256.0f;
    float u2 = xt / 256.0f;
    float v0 = (yt + 16) / 256.0f;
    float v2 = yt / 256.0f;

    const float offset = 1.0f / 16.0f;

    float cornerX0 = x + offset;
    float cornerX1 = x + 1 - offset;
    float cornerZ0 = z + offset;
    float cornerZ1 = z + 1 - offset;
    float sizeY = y + 1.0f;

    float tintR = block->m_TintR;
    float tintG = block->m_TintG;
    float tintB = block->m_TintB;

    if (block->m_ColorizeWithBiome)
    {
        auto temp = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetTemperatureForColumn(x, z);
        auto hum = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetHumidityForColumn(x, z);

        auto color = ColorizerGrass::GetColor(temp, hum);
        tintR *= color.r;
        tintG *= color.g;
        tintB *= color.b;
    }

    gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, -1));

    gfx_vp->SetColor(tintR, tintG, tintB);

    gfx_vp->AddVertex(cornerX0, y, cornerZ0, u0, v0);
    gfx_vp->AddVertex(cornerX0, sizeY, cornerZ0, u0, v2);
    gfx_vp->AddVertex(cornerX1, sizeY, cornerZ1, u2, v2);
    gfx_vp->AddVertex(cornerX1, y, cornerZ1, u2, v0);

    gfx_vp->AddVertex(cornerX1, y, cornerZ1, u0, v0);
    gfx_vp->AddVertex(cornerX1, sizeY, cornerZ1, u0, v2);
    gfx_vp->AddVertex(cornerX0, sizeY, cornerZ0, u2, v2);
    gfx_vp->AddVertex(cornerX0, y, cornerZ0, u2, v0);

    gfx_vp->AddVertex(cornerX1, y, cornerZ0, u0, v0);
    gfx_vp->AddVertex(cornerX1, sizeY, cornerZ0, u0, v2);
    gfx_vp->AddVertex(cornerX0, sizeY, cornerZ1, u2, v2);
    gfx_vp->AddVertex(cornerX0, y, cornerZ1, u2, v0);

    gfx_vp->AddVertex(cornerX0, y, cornerZ1, u0, v0);
    gfx_vp->AddVertex(cornerX0, sizeY, cornerZ1, u0, v2);
    gfx_vp->AddVertex(cornerX1, sizeY, cornerZ0, u2, v2);
    gfx_vp->AddVertex(cornerX1, y, cornerZ0, u2, v0);
}

void BlockRenderer::RenderBlockTorch(Block* block, Chunk* chunk, int x, int y, int z)
{
    auto data = chunk->GetBlock(x, y, z);

    gfx_vp->SetLightMultiplier(chunk->GetMixedLightLocal(x, y, z) / 15.0f);

    const float f1 = 0.5F;
    const float f2 = 0.8F;
    const float f3 = 0.6F;

    block->MinX = 0.0f / 16.0f;
    block->MaxX = 2.0f / 16.0f;
    block->MinY = 0.0f;
    block->MaxY = 0.625f;
    block->MinZ = 0.0f / 16.0f;
    block->MaxZ = 2.0f / 16.0f;

    // Local cube vertices (torch body)
    glm::vec3 baseVerts[] = {
        {block->MinX, block->MinY, block->MinZ}, // 0
        {block->MaxX, block->MinY, block->MinZ}, // 1
        {block->MaxX, block->MinY, block->MaxZ}, // 2
        {block->MinX, block->MinY, block->MaxZ}, // 3

        {block->MinX, block->MaxY, block->MinZ}, // 4
        {block->MaxX, block->MaxY, block->MinZ}, // 5
        {block->MaxX, block->MaxY, block->MaxZ}, // 6
        {block->MinX, block->MaxY, block->MaxZ}  // 7
    };

    // Build transform
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(x + 0.4375f, y, z + 0.4375f));
    if (data.Metadata != TORCH_FACING_UP) {
        model = glm::translate(model, glm::vec3(0.0f, 0.15f, 0.0f));
    }

    // Step 3: Rotate depending on facing
    switch (data.Metadata) {
    case TORCH_FACING_NORTH:
        model = glm::translate(model, glm::vec3(0.0f, -0.05f, 0.5f)); // push to wall
        model = glm::rotate(model, glm::radians(-22.5f), glm::vec3(1, 0, 0)); // tilt inward
        break;
    case TORCH_FACING_SOUTH:
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        model = glm::rotate(model, glm::radians(22.5f), glm::vec3(1, 0, 0));
        break;
    case TORCH_FACING_WEST:
        model = glm::translate(model, glm::vec3(0.5f, -0.05f, 0.0f));
        model = glm::rotate(model, glm::radians(22.5f), glm::vec3(0, 0, 1));
        break;
    case TORCH_FACING_EAST:
        model = glm::translate(model, glm::vec3(-0.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-22.5f), glm::vec3(0, 0, 1));
        break;
    case TORCH_FACING_UP:
        // upright torch, no tilt
        break;
    }

    // Helper lambda to transform and feed into AddVertex
    auto addV = [&](int idx, float u, float v) {
        glm::vec3 vtx = glm::vec3(model * glm::vec4(baseVerts[idx], 1.0f));
        gfx_vp->AddVertex(vtx.x, vtx.y, vtx.z, u, v);
        };

    // Texture coordinates
    int tex = 80;
    int xt = (tex % 16) * 16;
    int yt = (tex / 16) * 16;

    float u0, u2, v0, v2;

    // --- Bottom face (0,1,2,3)
    u0 = (xt + 7) / 256.0f; u2 = (xt + 9) / 256.0f;
    v0 = (yt + 6.0f) / 256.0f; v2 = (yt + 8.0f) / 256.0f;

    gfx_vp->SetColor(f1, f1, f1);
    addV(0, u0, v2);
    addV(1, u0, v0);
    addV(2, u2, v0);
    addV(3, u2, v2);

    // --- Top face (4,5,6,7)
    gfx_vp->SetColor(1.0f, 1.0f, 1.0f);
    addV(6, u2, v2);
    addV(5, u2, v0);
    addV(4, u0, v0);
    addV(7, u0, v2);

    // --- North face (4,5,1,0)
    u0 = (xt + 7) / 256.0f; u2 = (xt + 9) / 256.0f;
    v0 = (yt + 6.0f) / 256.0f; v2 = (yt + 16.0f) / 256.0f;

    gfx_vp->SetColor(f2, f2, f2);
    addV(4, u2, v0);
    addV(5, u0, v0);
    addV(1, u0, v2);
    addV(0, u2, v2);

    // --- South face (7,3,2,6)
    gfx_vp->SetColor(f2, f2, f2);
    addV(7, u0, v0);
    addV(3, u0, v2);
    addV(2, u2, v2);
    addV(6, u2, v0);

    // --- West face (7,4,0,3)
    gfx_vp->SetColor(f3, f3, f3);
    addV(7, u2, v0);
    addV(4, u0, v0);
    addV(0, u0, v2);
    addV(3, u2, v2);

    // --- East face (2,1,5,6)
    gfx_vp->SetColor(f3, f3, f3);
    addV(2, u0, v2);
    addV(1, u2, v2);
    addV(5, u2, v0);
    addV(6, u0, v0);
}

void BlockRenderer::RenderBlockStairs(Block* block, Chunk* chunk, int x, int y, int z)
{
    block->SetShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
    RenderBlockGeneric(block, chunk, x, y, z);
    auto blockData = chunk->GetBlockGlobal(x, y, z);
    if (blockData.Metadata == 0)
    {
        block->SetShape(0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.5f);
    }
    if (blockData.Metadata == 1)
    {
        block->SetShape(0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f);
    }
    if (blockData.Metadata == 2)
    {
        block->SetShape(0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f);
    }
    if (blockData.Metadata == 3)
    {
        block->SetShape(0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
    }
    RenderBlockGeneric(block, chunk, x, y, z);
    block->SetShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockRenderer::RenderBlockDoor(Block* block, Chunk* chunk, int x, int y, int z)
{
    auto blockData = chunk->GetBlockGlobal(x, y, z);
    const float size = 3.0f / 16.0f;

    auto face = blockData.Metadata & DOOR_FACING_MASK;
    auto flip = blockData.Metadata & DOOR_FLIP_FLAG_MASK;
    auto open = blockData.Metadata & DOOR_OPEN_FLAG_MASK;

    if (open)
    {
        std::array<uint8_t, 8> lut =
        {
            3, 2, 0, 1,
            2, 3, 1, 0,
        };
        face = lut[face + (flip > 0 ? 1 : 0) * 4];
    }

    if (face == 0)
    {
        block->SetShape(0.0f, 0.0f, 1.0f - size, 1.0f, 1.0f, 1.0f);
    }

    if (face == 1)
    {
        block->SetShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, size);
    }

    if (face == 2)
    {
        block->SetShape(1.0f - size, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    if (face == 3)
    {
        block->SetShape(0.0f, 0.0f, 0.0f, size, 1.0f, 1.0f);
    }

    block->m_FlipTextureForRender = flip;

    int faces = 0;

    const float f1 = 0.5F;
    const float f2 = 0.8F;
    const float f3 = 0.6F;

    float tintR = block->m_TintR;
    float tintG = block->m_TintG;
    float tintB = block->m_TintB;

    if (block->m_ColorizeWithBiome)
    {
        auto temp = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetTemperatureForColumn(x, z);
        auto hum = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetHumidityForColumn(x, z);

        auto color = ColorizerGrass::GetColor(temp, hum);
        tintR *= color.r;
        tintG *= color.g;
        tintB *= color.b;
    }

    if (block->IsFaceRenderable(chunk, x, y - 1, z, 0)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 0));
        gfx_vp->SetColor(f1 * tintR, f1 * tintG, f1 * tintB);
        RenderFaceDoorWorld(gfx_vp, block, chunk, x, y, z, 0);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y + 1, z, 1)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 1));
        gfx_vp->SetColor(tintR, tintG, tintB);
        RenderFaceDoorWorld(gfx_vp, block, chunk, x, y, z, 1);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y, z - 1, 2)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 2));
        gfx_vp->SetColor(f2 * tintR, f2 * tintG, f2 * tintB);
        RenderFaceDoorWorld(gfx_vp, block, chunk, x, y, z, 2);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y, z + 1, 3)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 3));
        gfx_vp->SetColor(f2 * tintR, f2 * tintG, f2 * tintB);
        RenderFaceDoorWorld(gfx_vp, block, chunk, x, y, z, 3);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x - 1, y, z, 4)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 4));
        gfx_vp->SetColor(f3 * tintR, f3 * tintG, f3 * tintB);
        RenderFaceDoorWorld(gfx_vp, block, chunk, x, y, z, 4);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x + 1, y, z, 5)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 5));
        gfx_vp->SetColor(f3 * tintR, f3 * tintG, f3 * tintB);
        RenderFaceDoorWorld(gfx_vp, block, chunk, x, y, z, 5);
        faces++;
    }

    block->m_FlipTextureForRender = false;
}

void BlockRenderer::RenderBlockFire(Block* block, Chunk* chunk, int x, int y, int z)
{
    // Cleaned up by Gemini

    gfx_vp->SetLightMultiplier(1.0f);
    gfx_vp->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    int textureIndex = block->GetTextureForFace(0, chunk, x, y, z);
    const float textureUnitScale = 16.0f / 256.0f;
    float flameHeight = 1.4f;

    int texturePixelX = (textureIndex % 16) * 16;
    int texturePixelY = (textureIndex / 16) - 1;

    float texU0 = texturePixelX / 256.0f;
    float texU1 = (texturePixelX + 15.99f) / 256.0f;
    float texV0 = texturePixelY / 256.0f;
    float texV1 = (texturePixelY + 15.99f) / 256.0f;

    auto blockBelowDef = chunk->GetBlockGlobal(x, y - 1, z).GetDef();

    texU0 = texturePixelX / 256.0f;
    texU1 = (texturePixelX + 15.99f) / 256.0f;
    texV0 = (texturePixelY + 16) / 256.0f;
    texV1 = (texturePixelY + 15.99f + 16.0f) / 256.0f;

    if (((blockBelowDef->MaxY <= 0.99f) || !blockBelowDef->IsSolid()) && !blockBelowDef->IsFlammable())
    {
        float tempSwapU;
        float segmentStartX, segmentEndX;
        float segmentStartZ, segmentEndZ;
        float innerBoxX0, innerBoxX1;
        float innerBoxZ0, innerBoxZ1;

        float sideOffset = 0.2f;
        float baseElevationOffset = 1.0f / 16.0f;

        if (((x + y + z) & 1) == 1) {
            texU0 = texturePixelX / 256.0f;
            texU1 = (texturePixelX + 15.99f) / 256.0f;
            texV0 = (texturePixelY + 16) / 256.0f;
            texV1 = (texturePixelY + 15.99f + 16.0f) / 256.0f;
        }

        if (((x / 2 + y / 2 + z / 2) & 1) == 1) {
            tempSwapU = texU1;
            texU1 = texU0;
            texU0 = tempSwapU;
        }

        if (chunk->GetBlockGlobal(x - 1, y, z).GetDef()->IsFlammable()) {
            gfx_vp->AddVertex(x + sideOffset, y + flameHeight + baseElevationOffset, z + 1, texU1, texV0);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z + 1, texU1, texV1);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z, texU0, texV1);
            gfx_vp->AddVertex(x + sideOffset, y + flameHeight + baseElevationOffset, z, texU0, texV0);
            gfx_vp->AddVertex(x + sideOffset, y + flameHeight + baseElevationOffset, z, texU0, texV0);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z, texU0, texV1);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z + 1, texU1, texV1);
            gfx_vp->AddVertex(x + sideOffset, y + flameHeight + baseElevationOffset, z + 1, texU1, texV0);
        }

        if (chunk->GetBlockGlobal(x + 1, y, z).GetDef()->IsFlammable()) {
            gfx_vp->AddVertex(x + 1 - sideOffset, y + flameHeight + baseElevationOffset, z, texU0, texV0);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z, texU0, texV1);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z + 1, texU1, texV1);
            gfx_vp->AddVertex(x + 1 - sideOffset, y + flameHeight + baseElevationOffset, z + 1, texU1, texV0);
            gfx_vp->AddVertex(x + 1 - sideOffset, y + flameHeight + baseElevationOffset, z + 1, texU1, texV0);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z + 1, texU1, texV1);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z, texU0, texV1);
            gfx_vp->AddVertex(x + 1 - sideOffset, y + flameHeight + baseElevationOffset, z, texU0, texV0);
        }

        if (chunk->GetBlockGlobal(x, y, z - 1).GetDef()->IsFlammable()) {
            gfx_vp->AddVertex(x, y + flameHeight + baseElevationOffset, z + sideOffset, texU1, texV0);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z, texU1, texV1);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z, texU0, texV1);
            gfx_vp->AddVertex(x + 1, y + flameHeight + baseElevationOffset, z + sideOffset, texU0, texV0);
            gfx_vp->AddVertex(x + 1, y + flameHeight + baseElevationOffset, z + sideOffset, texU0, texV0);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z, texU0, texV1);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z, texU1, texV1);
            gfx_vp->AddVertex(x, y + flameHeight + baseElevationOffset, z + sideOffset, texU1, texV0);
        }

        if (chunk->GetBlockGlobal(x, y, z + 1).GetDef()->IsFlammable()) {
            gfx_vp->AddVertex(x + 1, y + flameHeight + baseElevationOffset, z + 1 - sideOffset, texU0, texV0);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z + 1, texU0, texV1);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z + 1, texU1, texV1);
            gfx_vp->AddVertex(x, y + flameHeight + baseElevationOffset, z + 1 - sideOffset, texU1, texV0);
            gfx_vp->AddVertex(x, y + flameHeight + baseElevationOffset, z + 1 - sideOffset, texU1, texV0);
            gfx_vp->AddVertex(x, y + baseElevationOffset, z + 1, texU1, texV1);
            gfx_vp->AddVertex(x + 1, y + baseElevationOffset, z + 1, texU0, texV1);
            gfx_vp->AddVertex(x + 1, y + flameHeight + baseElevationOffset, z + 1 - sideOffset, texU0, texV0);
        }

        if (chunk->GetBlockGlobal(x, y + 1, z).GetDef()->IsFlammable()) {
            segmentStartX = x + 1.0f;
            segmentEndX = x;
            segmentStartZ = z + 1.0f;
            segmentEndZ = z;
            innerBoxX0 = x;
            innerBoxX1 = x + 1.0f;
            innerBoxZ0 = z;
            float innerBoxZ1 = z + 1.0f;

            ++y;
            flameHeight = -0.2f;

            if (((x + y + z) & 1) == 0) {
                gfx_vp->AddVertex(innerBoxX0, y + flameHeight, z, texU1, texV0);
                gfx_vp->AddVertex(segmentStartX, y, z, texU1, texV1);
                gfx_vp->AddVertex(segmentStartX, y, z + 1, texU0, texV1);
                gfx_vp->AddVertex(innerBoxX0, y + flameHeight, z + 1, texU0, texV0);

                texU0 = texturePixelX / 256.0f;
                texU1 = (texturePixelX + 15.99f) / 256.0f;
                texV0 = (texturePixelY + 16) / 256.0f;
                texV1 = (texturePixelY + 15.99f + 16.0f) / 256.0f;

                gfx_vp->AddVertex(innerBoxX1, y + flameHeight, z + 1, texU1, texV0);
                gfx_vp->AddVertex(segmentEndX, y, z + 1, texU1, texV1);
                gfx_vp->AddVertex(segmentEndX, y, z, texU0, texV1);
                gfx_vp->AddVertex(innerBoxX1, y + flameHeight, z, texU0, texV0);
            }
            else {
                gfx_vp->AddVertex(x, y + flameHeight, innerBoxZ1, texU1, texV0);
                gfx_vp->AddVertex(x, y, segmentEndZ, texU1, texV1);
                gfx_vp->AddVertex(x + 1, y, segmentEndZ, texU0, texV1);
                gfx_vp->AddVertex(x + 1, y + flameHeight, innerBoxZ1, texU0, texV0);

                texU0 = texturePixelX / 256.0f;
                texU1 = (texturePixelX + 15.99f) / 256.0f;
                texV0 = (texturePixelY + 16) / 256.0f;
                texV1 = (texturePixelY + 15.99f + 16.0f) / 256.0f;

                gfx_vp->AddVertex(x + 1, y + flameHeight, innerBoxZ0, texU1, texV0);
                gfx_vp->AddVertex(x + 1, y, segmentStartZ, texU1, texV1);
                gfx_vp->AddVertex(x, y, segmentStartZ, texU0, texV1);
                gfx_vp->AddVertex(x, y + flameHeight, innerBoxZ0, texU0, texV0);
            }
        }
    }
    else
    {
        float outerX1 = x + 0.7f;
        float outerX0 = x + 0.3f;
        float outerZ1 = z + 0.7f;
        float outerZ0 = z + 0.3f;
        float innerX0 = x + 0.2f;
        float innerX1 = x + 0.8f;
        float innerZ0 = z + 0.2f;
        float innerZ1 = z + 0.8f;

        gfx_vp->AddVertex(innerX0, y + flameHeight, z + 1, texU1, texV0);
        gfx_vp->AddVertex(outerX1, y, z + 1, texU1, texV1);
        gfx_vp->AddVertex(outerX1, y, z, texU0, texV1);
        gfx_vp->AddVertex(innerX0, y + flameHeight, z, texU0, texV0);
        gfx_vp->AddVertex(innerX1, y + flameHeight, z, texU1, texV0);
        gfx_vp->AddVertex(outerX0, y, z, texU1, texV1);
        gfx_vp->AddVertex(outerX0, y, z + 1, texU0, texV1);
        gfx_vp->AddVertex(innerX1, y + flameHeight, z + 1, texU0, texV0);
        gfx_vp->AddVertex(x + 1, y + flameHeight, innerZ1, texU1, texV0);
        gfx_vp->AddVertex(x + 1, y, outerZ0, texU1, texV1);
        gfx_vp->AddVertex(x, y, outerZ0, texU0, texV1);
        gfx_vp->AddVertex(x, y + flameHeight, innerZ1, texU0, texV0);
        gfx_vp->AddVertex(x, y + flameHeight, innerZ0, texU1, texV0);
        gfx_vp->AddVertex(x, y, outerZ1, texU1, texV1);
        gfx_vp->AddVertex(x + 1, y, outerZ1, texU0, texV1);
        gfx_vp->AddVertex(x + 1, y + flameHeight, innerZ0, texU0, texV0);

        outerX0 = x;
        outerX1 = x + 1.0f;
        outerZ0 = z;
        outerZ1 = z + 1.0f;
        innerX0 = x + 0.1f;
        innerX1 = x + 0.9f;
        innerZ0 = z + 0.1f;
        innerZ1 = z + 0.9f;

        gfx_vp->AddVertex(innerX0, y + flameHeight, z, texU0, texV0);
        gfx_vp->AddVertex(outerX0, y, z, texU0, texV1);
        gfx_vp->AddVertex(outerX0, y, z + 1, texU1, texV1);
        gfx_vp->AddVertex(innerX0, y + flameHeight, z + 1, texU1, texV0);
        gfx_vp->AddVertex(innerX1, y + flameHeight, z + 1, texU0, texV0);
        gfx_vp->AddVertex(outerX1, y, z + 1, texU0, texV1);
        gfx_vp->AddVertex(outerX1, y, z, texU1, texV1);
        gfx_vp->AddVertex(innerX1, y + flameHeight, z, texU1, texV0);
        gfx_vp->AddVertex(x, y + flameHeight, innerZ1, texU0, texV0);
        gfx_vp->AddVertex(x, y, outerZ1, texU0, texV1);
        gfx_vp->AddVertex(x + 1, y, outerZ1, texU1, texV1);
        gfx_vp->AddVertex(x + 1, y + flameHeight, innerZ1, texU1, texV0);
        gfx_vp->AddVertex(x + 1, y + flameHeight, innerZ0, texU0, texV0);
        gfx_vp->AddVertex(x + 1, y, outerZ0, texU0, texV1);
        gfx_vp->AddVertex(x, y, outerZ0, texU1, texV1);
        gfx_vp->AddVertex(x, y + flameHeight, innerZ0, texU1, texV0);
    }
}

void BlockRenderer::RenderBlockPortal(Block* block, Chunk* chunk, int x, int y, int z)
{
    auto dir = chunk->GetBlockGlobal(x, y, z).Metadata;
    if (dir == 1)
    {
        block->SetShape(0.3f, 0.0f, 0.0f, 0.6f, 1.0f, 1.0f);
    }
    else
    {
        block->SetShape(0.0f, 0.0f, 0.3f, 1.0f, 1.0f, 0.6f);
    }
    RenderBlockGeneric(block, chunk, x, y, z);
}

void BlockRenderer::RenderBlockLiquid(Block* block, Chunk* chunk, int x, int y, int z)
{
    int faces = 0;

    const float f1 = 0.5F;
    const float f2 = 0.8F;
    const float f3 = 0.6F;

    float tintR = block->m_TintR;
    float tintG = block->m_TintG;
    float tintB = block->m_TintB;

    if (block->m_ColorizeWithBiome)
    {
        auto temp = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetTemperatureForColumn(x, z);
        auto hum = chunk->ChunkWorld->GetChunkProvider().BiomeGenerator.GetHumidityForColumn(x, z);

        auto color = ColorizerGrass::GetColor(temp, hum);
        tintR *= color.r;
        tintG *= color.g;
        tintB *= color.b;
    }

    if (block->IsFaceRenderable(chunk, x, y - 1, z, 0)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 0));
        gfx_vp->SetColor(f1 * tintR, f1 * tintG, f1 * tintB);
        RenderFaceLiquidWorld(gfx_vp, block, chunk, x, y, z, 0);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y + 1, z, 1)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 1));
        gfx_vp->SetColor(tintR, tintG, tintB);
        RenderFaceLiquidWorld(gfx_vp, block, chunk, x, y, z, 1);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y, z - 1, 2)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 2));
        gfx_vp->SetColor(f2 * tintR, f2 * tintG, f2 * tintB);
        RenderFaceLiquidWorld(gfx_vp, block, chunk, x, y, z, 2);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x, y, z + 1, 3)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 3));
        gfx_vp->SetColor(f2 * tintR, f2 * tintG, f2 * tintB);
        RenderFaceLiquidWorld(gfx_vp, block, chunk, x, y, z, 3);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x - 1, y, z, 4)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 4));
        gfx_vp->SetColor(f3 * tintR, f3 * tintG, f3 * tintB);
        RenderFaceLiquidWorld(gfx_vp, block, chunk, x, y, z, 4);
        faces++;
    }
    if (block->IsFaceRenderable(chunk, x + 1, y, z, 5)) {
        gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, 5));
        gfx_vp->SetColor(f3 * tintR, f3 * tintG, f3 * tintB);
        RenderFaceLiquidWorld(gfx_vp, block, chunk, x, y, z, 5);
        faces++;
    }
}

void BlockRenderer::RenderBlockCrop(Block* block, Chunk* chunk, int x, int y, int z)
{
    const float offset = 4.0f / 16.0f;
    block->SetShape(offset, 0.0f, offset, 1 - offset, 0.9f, 1 - offset);

    gfx_vp->SetLightMultiplier(block->CalculateLightLevelForFace(chunk, x, y, z, -1));
    gfx_vp->SetColor(1.0f, 1.0f, 1.0f);
    gfx_vp->SetTranslation(0.0f, -(1.0f / 16.0f), 0.0f);

    for (int face = 2; face < 6; face++)
    {
        int tex = block->GetTextureForFace(face, chunk, x, y, z);

        int xt = (tex % 16) * 16;
        int yt = (tex / 16) * 16;

        float u0 = xt / 256.0f;
        float u2 = (xt + 16.0f) / 256.0f;
        float v0 = yt / 256.0f;
        float v2 = (yt + 16.0f) / 256.0f;

        if (face == 2 || face == 3)
        {
            block->MinZ = offset;
            block->MaxZ = 1.0f - offset;
            block->MinX = 0.0f;
            block->MaxX = 1.0f;
        }

        if (face == 4 || face == 5)
        {
            block->MinX = offset;
            block->MaxX = 1.0f - offset;
            block->MinZ = 0.0f;
            block->MaxZ = 1.0f;
        }

        float x2 = x + block->MinX;
        float x3 = x + block->MaxX;
        float y2 = y + block->MinY;
        float y3 = y + block->MaxY;
        float z2 = z + block->MinZ;
        float z3 = z + block->MaxZ;
        if (face == 2) {
            // front
            gfx_vp->AddVertex(x2, y3, z2, u2, v0);
            gfx_vp->AddVertex(x3, y3, z2, u0, v0);
            gfx_vp->AddVertex(x3, y2, z2, u0, v2);
            gfx_vp->AddVertex(x2, y2, z2, u2, v2);
            // back (reverse winding, swap u0/u2)
            gfx_vp->AddVertex(x2, y2, z2, u0, v2);
            gfx_vp->AddVertex(x3, y2, z2, u2, v2);
            gfx_vp->AddVertex(x3, y3, z2, u2, v0);
            gfx_vp->AddVertex(x2, y3, z2, u0, v0);
            continue;
        }
        if (face == 3) {
            // front
            gfx_vp->AddVertex(x2, y3, z3, u0, v0);
            gfx_vp->AddVertex(x2, y2, z3, u0, v2);
            gfx_vp->AddVertex(x3, y2, z3, u2, v2);
            gfx_vp->AddVertex(x3, y3, z3, u2, v0);
            // back (swap u0/u2)
            gfx_vp->AddVertex(x3, y3, z3, u0, v0);
            gfx_vp->AddVertex(x3, y2, z3, u0, v2);
            gfx_vp->AddVertex(x2, y2, z3, u2, v2);
            gfx_vp->AddVertex(x2, y3, z3, u2, v0);
            continue;
        }
        if (face == 4) {
            // front
            gfx_vp->AddVertex(x2, y3, z3, u2, v0);
            gfx_vp->AddVertex(x2, y3, z2, u0, v0);
            gfx_vp->AddVertex(x2, y2, z2, u0, v2);
            gfx_vp->AddVertex(x2, y2, z3, u2, v2);
            // back (swap u0/u2)
            gfx_vp->AddVertex(x2, y2, z3, u0, v2);
            gfx_vp->AddVertex(x2, y2, z2, u2, v2);
            gfx_vp->AddVertex(x2, y3, z2, u2, v0);
            gfx_vp->AddVertex(x2, y3, z3, u0, v0);
            continue;
        }
        if (face == 5) {
            // front
            gfx_vp->AddVertex(x3, y2, z3, u0, v2);
            gfx_vp->AddVertex(x3, y2, z2, u2, v2);
            gfx_vp->AddVertex(x3, y3, z2, u2, v0);
            gfx_vp->AddVertex(x3, y3, z3, u0, v0);
            // back (swap u0/u2)
            gfx_vp->AddVertex(x3, y3, z3, u2, v0);
            gfx_vp->AddVertex(x3, y3, z2, u0, v0);
            gfx_vp->AddVertex(x3, y2, z2, u0, v2);
            gfx_vp->AddVertex(x3, y2, z3, u2, v2);
            continue;
        }


    }
    gfx_vp->SetTranslation(0.0f, 0.0f, 0.0f);
}

void BlockRenderer::RenderFaceWorld(VertexProducer* producer, Block* block, Chunk* chunk, int x, int y, int z, int face)
{
    int tex = block->GetTextureForFace(face, chunk, x, y, z);

    int xt = (tex % 16) * 16;
    int yt = (tex / 16) * 16;

    float u0 = xt / 256.0f;
    float u2 = (xt + block->MaxX * 16.0f) / 256.0f;
    float v0 = yt / 256.0f;
    float v2 = (yt + 16.0f) / 256.0f;

    float x2 = x + block->MinX;
    float x3 = x + block->MaxX;
    float y2 = y + block->MinY;
    float y3 = y + block->MaxY;
    float z2 = z + block->MinZ;
    float z3 = z + block->MaxZ;

    u0 = (xt + block->MinX * 16.0f) / 256.0f;
    u2 = (xt + block->MaxX * 16.0f) / 256.0f;
    v0 = (yt + block->MinZ * 16.0f) / 256.0f;
    v2 = (yt + block->MaxZ * 16.0f) / 256.0f;

    if (block->m_FlipTextureForRender)
    {
        std::swap(u0, u2);
    }

    if (face == 0) {
        producer->AddVertex(x2, y2, z3, u0, v2);
        producer->AddVertex(x2, y2, z2, u0, v0);
        producer->AddVertex(x3, y2, z2, u2, v0);
        producer->AddVertex(x3, y2, z3, u2, v2);
        return;
    }
    if (face == 1) {
        producer->AddVertex(x3, y3, z3, u2, v2);
        producer->AddVertex(x3, y3, z2, u2, v0);
        producer->AddVertex(x2, y3, z2, u0, v0);
        producer->AddVertex(x2, y3, z3, u0, v2);
        return;
    }

    v0 = (yt + (1.0f - block->MaxY) * 16.0f) / 256.0f;
    v2 = (yt + (1.0f - block->MinY) * 16.0f) / 256.0f;

    if (block->m_FlipTextureForRender)
    {
        std::swap(u0, u2);
    }

    if (face == 2) {
        u0 = (xt + block->MinX * 16.0f) / 256.0f;
        u2 = (xt + block->MaxX * 16.0f) / 256.0f;
        if (block->m_FlipTextureForRender) std::swap(u0, u2);
        producer->AddVertex(x2, y3, z2, u2, v0);
        producer->AddVertex(x3, y3, z2, u0, v0);
        producer->AddVertex(x3, y2, z2, u0, v2);
        producer->AddVertex(x2, y2, z2, u2, v2);
        return;
    }
    if (face == 3) {
        u0 = (xt + block->MinX * 16.0f) / 256.0f;
        u2 = (xt + block->MaxX * 16.0f) / 256.0f;
        if (block->m_FlipTextureForRender) std::swap(u0, u2);
        producer->AddVertex(x2, y3, z3, u0, v0);
        producer->AddVertex(x2, y2, z3, u0, v2);
        producer->AddVertex(x3, y2, z3, u2, v2);
        producer->AddVertex(x3, y3, z3, u2, v0);
        return;
    }
    if (face == 4) {
        u0 = (xt + block->MinZ * 16.0f) / 256.0f;
        u2 = (xt + block->MaxZ * 16.0f) / 256.0f;
        if (block->m_FlipTextureForRender) std::swap(u0, u2);
        producer->AddVertex(x2, y3, z3, u2, v0);
        producer->AddVertex(x2, y3, z2, u0, v0);
        producer->AddVertex(x2, y2, z2, u0, v2);
        producer->AddVertex(x2, y2, z3, u2, v2);
        return;
    }
    if (face == 5) {
        u0 = (xt + block->MinZ * 16.0f) / 256.0f;
        u2 = (xt + block->MaxZ * 16.0f) / 256.0f;
        if (block->m_FlipTextureForRender) std::swap(u0, u2);
        producer->AddVertex(x3, y2, z3, u0, v2);
        producer->AddVertex(x3, y2, z2, u2, v2);
        producer->AddVertex(x3, y3, z2, u2, v0);
        producer->AddVertex(x3, y3, z3, u0, v0);
    }
}

void BlockRenderer::RenderFaceDoorWorld(VertexProducer* producer, Block* block, Chunk* chunk, int x, int y, int z, int face)
{
    BlockDoor* blockDoor = static_cast<BlockDoor*>(block);

    auto f = blockDoor->m_FlipTextureForRender;

    if (chunk)
    {
        auto block = chunk->GetBlockGlobal(x, y, z);

        auto dir = block.Metadata & DOOR_FACING_MASK;
        auto flip = block.Metadata & DOOR_FLIP_FLAG_MASK;
        auto open = block.Metadata & DOOR_OPEN_FLAG_MASK;

        if (blockDoor->m_DoorPart == DoorPart::BOTTOM && face == 1)
        {
            return;
        }
        if (blockDoor->m_DoorPart == DoorPart::TOP && face == 0)
        {
            return;
        }

        if (open)
        {
            std::array<uint8_t, 8> lut =
            {
                2, 3, 1, 0,
                3, 2, 0, 1,
            };
            dir = lut[dir + (flip > 0 ? 1 : 0) * 4];
        }

        if (dir == face - 2)
        {
            blockDoor->m_FlipTextureForRender = !f;
        }
    }

    RenderFaceWorld(producer, block, chunk, x, y, z, face);

    blockDoor->m_FlipTextureForRender = f;
}

void BlockRenderer::RenderFaceLiquidWorld(VertexProducer* producer, Block* block, Chunk* chunk, int x, int y, int z, int face)
{
    BlockLiquid* blockLiquid = static_cast<BlockLiquid*>(block);

    if (blockLiquid->GetId() == BLOCK_LAVA && face == 1)
    {
        producer->SetLightMultiplier(blockLiquid->CalculateLightLevelForFace(chunk, x, y, z, -1));
        producer->SetColor(1.0f, 1.0f, 1.0f);
    }
    if (!chunk)
    {
        RenderFaceWorld(producer, block, chunk, x, y, z, face);
        return;
    }

    const int level = chunk->GetBlockGlobal(x, y, z).Metadata;
    const auto dir = blockLiquid->GetFlowDirection(chunk->ChunkWorld, x, y, z);
    const float angle = glm::atan(-dir.x, -dir.z);

    float ww = 256.0f;

    int xt = (blockLiquid->m_TextureIndex % 16) * 16;
    int yt = (blockLiquid->m_TextureIndex / 16) * 16;

    auto below = chunk->GetBlockGlobal(x, y - 1, z);

    World* world = chunk->ChunkWorld;

    const int nb = blockLiquid->GetLiquidLevel(world, block, x, y, z);

    if (dir.x != 0.0 || dir.z != 0.0 || nb == 9 || face != 1) {
        xt += 16;
    }

    float w = 16.0f;
    float u0 = xt / ww;
    float u2 = (xt + 16.0f) / ww;
    float v0 = yt / ww;
    float v2 = (yt + 16.0f) / ww;
    float n6 = u0;
    float n7 = v0 + 16.0f / 256.0f;
    const float n8 = (-glm::sin(angle) * 8.0f) / ww;
    const float n9 = (glm::cos(angle) * 8.0f) / ww;
    const float x2 = x + blockLiquid->MinX;
    const float x3 = x + blockLiquid->MaxX;
    const float y2 = y + blockLiquid->MinY;
    const float z2 = z + blockLiquid->MinZ;
    const float z3 = z + blockLiquid->MaxZ;

    const int belowl = blockLiquid->GetLiquidLevel(world, block, x, y - 1, z);

    const float i = (float)blockLiquid->GetLevel(chunk, x, y, z);
    const float lx = (float)blockLiquid->GetLevel(chunk, x + 1, y, z);
    const float ly = (float)blockLiquid->GetLevel(chunk, x, y, z + 1);
    const float lxy = (float)blockLiquid->GetLevel(chunk, x + 1, y, z + 1);

    if (face == 1) {

        if (dir.x != 0.0 || dir.z != 0.0) {
            xt += 20;
            yt += 4;
        }

        u0 = (xt + blockLiquid->MinX * w) / 256.0f;
        u2 = (xt + blockLiquid->MaxX * w) / 256.0f;
        v0 = (yt + blockLiquid->MinZ * w) / 256.0f;
        v2 = (yt + blockLiquid->MaxZ * w) / 256.0f;

        n6 = u0;
        n7 = v0 + 16.0f / 256.0f;

        if (dir.x == 0.0 && dir.z == 0.0) {
            producer->AddVertex(x3, lxy + y, z3, u2, v2);
            producer->AddVertex(x3, lx + y, z2, u2, v0);
            producer->AddVertex(x2, i + y, z2, u0, v0);
            producer->AddVertex(x2, ly + y, z3, u0, v2);
        }
        else {
            producer->AddVertex(x3, lxy + y, z3, n6 - n9 - n8, n7 - n9 + n8);
            producer->AddVertex(x3, lx + y, z2, n6 - n9 + n8, n7 + n9 + n8);
            producer->AddVertex(x2, i + y, z2, n6 + n9 + n8, n7 + n9 - n8);
            producer->AddVertex(x2, ly + y, z3, n6 + n9 - n8, n7 - n9 - n8);
        }

        return;
    }
    if (face == 0) {
        u0 = (xt + blockLiquid->MinX * w) / 256.0f;
        u2 = (xt + blockLiquid->MaxX * w) / 256.0f;
        v0 = (yt + blockLiquid->MinZ * w) / 256.0f;
        v2 = (yt + blockLiquid->MaxZ * w) / 256.0f;
        producer->AddVertex(x2, y2, z3, u0, v2);
        producer->AddVertex(x2, y2, z2, u0, v0);
        producer->AddVertex(x3, y2, z2, u2, v0);
        producer->AddVertex(x3, y2, z3, u2, v2);
        return;
    }
    if (face == 2) {
        u0 = (xt + blockLiquid->MinX * w) / 256.0f;
        u2 = (xt + blockLiquid->MaxX * w) / 256.0f;
        v0 = (yt + blockLiquid->MinY * w) / 256.0f;
        v2 = (yt + lx * w) / 256.0f;
        producer->AddVertex(x2, y + i, z2, u2, v0);
        producer->AddVertex(x3, y + lx, z2, u0, v0);
        producer->AddVertex(x3, y2, z2, u0, v2);
        producer->AddVertex(x2, y2, z2, u2, v2);
        return;
    }
    if (face == 3) {
        u0 = (xt + blockLiquid->MinX * w) / 256.0f;
        u2 = (xt + blockLiquid->MaxX * w) / 256.0f;
        v0 = (yt + blockLiquid->MinY * w) / 256.0f;
        v2 = (yt + blockLiquid->MaxY * w) / 256.0f;
        producer->AddVertex(x2, ly + y, z3, u0, v0);
        producer->AddVertex(x2, y2, z3, u0, v2);
        producer->AddVertex(x3, y2, z3, u2, v2);
        producer->AddVertex(x3, lxy + y, z3, u2, v0);
        return;
    }
    if (face == 4) {
        u0 = (xt + blockLiquid->MinZ * w) / 256.0f;
        u2 = (xt + blockLiquid->MaxZ * w) / 256.0f;
        v0 = (yt + blockLiquid->MinY * w) / 256.0f;
        v2 = (yt + blockLiquid->MaxY * w) / 256.0f;
        producer->AddVertex(x2, ly + y, z3, u2, v0);
        producer->AddVertex(x2, i + y, z2, u0, v0);
        producer->AddVertex(x2, y2, z2, u0, v2);
        producer->AddVertex(x2, y2, z3, u2, v2);
        return;
    }
    if (face == 5) {
        u0 = (xt + blockLiquid->MinZ * w) / 256.0f;
        u2 = (xt + blockLiquid->MaxZ * w) / 256.0f;
        v0 = (yt + blockLiquid->MinY * w) / 256.0f;
        v2 = (yt + blockLiquid->MaxY * w) / 256.0f;
        producer->AddVertex(x3, y2, z3, u0, v2);
        producer->AddVertex(x3, y2, z2, u2, v2);
        producer->AddVertex(x3, lx + y, z2, u2, v0);
        producer->AddVertex(x3, lxy + y, z3, u0, v0);
    }
}
