#include "BlockPreviewRenderer.h"
#include "BlockRenderer.h"

#include "common/block/Block.h"

#include "client/renderer/VertexProducer.h"

void BlockPreviewRenderer::RenderBlock(VertexProducer* producer, Block* block)
{
    if (block->GetRenderType() == BLOCK_RENDER_TYPE_STAIRS)
    {
        RenderBlockStairs(producer, block);
        return;
    }

	RenderBlockGeneric(producer, block);
}

void BlockPreviewRenderer::RenderBlockGeneric(VertexProducer* producer, Block* block)
{
    const float f1 = 1.0F;
    const float f2 = 1.0F;
    const float f3 = 1.0F;

    producer->SetColor(f1 * block->m_TintR, f1 * block->m_TintG, f1 * block->m_TintB);
    producer->SetNormal(0.0f, -1.0f, 0.0f);
    RenderFacePreview(producer, block, 0);
    producer->SetColor(block->m_TintR, block->m_TintG, block->m_TintB);
    producer->SetNormal(0.0f, 1.0f, 0.0f);
    RenderFacePreview(producer, block, 1);
    producer->SetColor(f2 * block->m_TintR, f2 * block->m_TintG, f2 * block->m_TintB);
    producer->SetNormal(0.0f, 0.0f, -1.0f);
    RenderFacePreview(producer, block, 2);
    producer->SetColor(f2 * block->m_TintR, f2 * block->m_TintG, f2 * block->m_TintB);
    producer->SetNormal(0.0f, 0.0f, 1.0f);
    RenderFacePreview(producer, block, 3);
    producer->SetColor(f3 * block->m_TintR, f3 * block->m_TintG, f3 * block->m_TintB);
    producer->SetNormal(-1.0f, 0.0f, 0.0f);
    RenderFacePreview(producer, block, 4);
    producer->SetColor(f3 * block->m_TintR, f3 * block->m_TintG, f3 * block->m_TintB);
    producer->SetNormal(1.0f, 0.0f, 0.0f);
    RenderFacePreview(producer, block, 5);
}

void BlockPreviewRenderer::RenderBlockStairs(VertexProducer* producer, Block* block)
{
    block->SetShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
    RenderBlockGeneric(producer, block);
    block->SetShape(0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f);
    RenderBlockGeneric(producer, block);
    block->SetShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockPreviewRenderer::RenderFacePreview(VertexProducer* producer, Block* block, int face)
{
    BlockRenderer::RenderFaceWorld(producer, block, NULL, 0, 0, 0, face);
}
