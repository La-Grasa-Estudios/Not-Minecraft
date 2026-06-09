#include "ChunkProviderHell.h"

#include "common/World.h"
#include "common/CachedChunkAccess.h"
#include "common/gen/WorldGenCaves.h"
#include "common/noise/PerlinNoise.h"

#include "common/gen/WorldGenFire.h"
#include "common/gen/WorldGenFlowers.h"
#include "common/gen/WorldGenGlowstone.h"
#include "common/gen/WorldGenNetherLava.h"

// Cleaned up by Gemini, its 2am and i don't have the brain power to do this

ChunkProviderHell::ChunkProviderHell(uint64_t seed) : ChunkProvider(seed)
{
}

static float* generateBaseNoise(float* noiseArray, int localX, int localY, int localZ, int sizeX, int sizeY, int sizeZ, Random& noiseRandom)
{
	auto octavesMainLow = PerlinNoiseOctaves(noiseRandom, 16);
	auto octavesMainHigh = PerlinNoiseOctaves(noiseRandom, 16);
	auto octavesSelector = PerlinNoiseOctaves(noiseRandom, 8);
	auto octavesHeightVar = PerlinNoiseOctaves(noiseRandom, 10);
	auto octavesBaseHeight = PerlinNoiseOctaves(noiseRandom, 16);

	if (noiseArray == nullptr) {
		noiseArray = new float[sizeX * sizeY * sizeZ];
	}

	static float* heightVariationBuffer = nullptr;
	static float* baseHeightBuffer = nullptr;
	static float* noiseSelectorBuffer = nullptr;
	static float* noiseMainLowBuffer = nullptr;
	static float* noiseMainHighBuffer = nullptr;

	const float coordinateScaleXZ = 684.412f;
	const float coordinateScaleY = 2053.236f;

	heightVariationBuffer = octavesHeightVar.Sample(heightVariationBuffer, (float)localX, (float)localY, (float)localZ, sizeX, 1, sizeZ, 1.0f, 0.0f, 1.0f);
	baseHeightBuffer = octavesBaseHeight.Sample(baseHeightBuffer, (float)localX, (float)localY, (float)localZ, sizeX, 1, sizeZ, 100.0f, 0.0f, 100.0f);
	noiseSelectorBuffer = octavesSelector.Sample(noiseSelectorBuffer, (float)localX, (float)localY, (float)localZ, sizeX, sizeY, sizeZ, coordinateScaleXZ / 80.0f, coordinateScaleY / 60.0f, coordinateScaleXZ / 80.0f);
	noiseMainLowBuffer = octavesMainLow.Sample(noiseMainLowBuffer, (float)localX, (float)localY, (float)localZ, sizeX, sizeY, sizeZ, coordinateScaleXZ, coordinateScaleY, coordinateScaleXZ);
	noiseMainHighBuffer = octavesMainHigh.Sample(noiseMainHighBuffer, (float)localX, (float)localY, (float)localZ, sizeX, sizeY, sizeZ, coordinateScaleXZ, coordinateScaleY, coordinateScaleXZ);

	int volumeIndex = 0;
	int planeIndex = 0;
	float ceilingFloorBoundingEnvelope[17];

	for (int yCell = 0; yCell < sizeY; ++yCell) {
		ceilingFloorBoundingEnvelope[yCell] = glm::cos((float)yCell * glm::pi<float>() * 6.0f / (float)sizeY) * 2.0f;
		float distanceFromEdge = (float)yCell;
		if (yCell > sizeY / 2) {
			distanceFromEdge = (float)(sizeY - 1 - yCell);
		}

		if (distanceFromEdge < 4.0f) {
			distanceFromEdge = 4.0f - distanceFromEdge;
			ceilingFloorBoundingEnvelope[yCell] -= distanceFromEdge * distanceFromEdge * distanceFromEdge * 10.0f;
		}
	}

	for (int xCell = 0; xCell < sizeX; ++xCell) {
		for (int zCell = 0; zCell < sizeZ; ++zCell) {
			float heightVariation = (heightVariationBuffer[planeIndex] + 256.0f) / 512.0f;
			if (heightVariation > 1.0f) {
				heightVariation = 1.0f;
			}

			float landscapeBias = 0.0f;
			float baseHeight = baseHeightBuffer[planeIndex] / 8000.0f;
			if (baseHeight < 0.0f) {
				baseHeight = -baseHeight;
			}

			baseHeight = baseHeight * 3.0f - 3.0f;
			if (baseHeight < 0.0f) {
				baseHeight /= 2.0f;
				if (baseHeight < -1.0f) {
					baseHeight = -1.0f;
				}

				baseHeight /= 1.4f;
				baseHeight /= 2.0f;
				heightVariation = 0.0f;
			}
			else {
				if (baseHeight > 1.0f) {
					baseHeight = 1.0f;
				}

				baseHeight /= 6.0f;
			}

			heightVariation += 0.5f;
			baseHeight = baseHeight * (float)sizeY / 16.0f;
			++planeIndex;

			for (int yCellInner = 0; yCellInner < sizeY; ++yCellInner) {
				float finalDensity = 0.0f;
				float structuralWeight = ceilingFloorBoundingEnvelope[yCellInner];
				float lowNoiseSample = noiseMainLowBuffer[volumeIndex] / 512.0f;
				float highNoiseSample = noiseMainHighBuffer[volumeIndex] / 512.0f;
				float selectorValue = (noiseSelectorBuffer[volumeIndex] / 10.0f + 1.0f) / 2.0f;

				if (selectorValue < 0.0f) {
					finalDensity = lowNoiseSample;
				}
				else if (selectorValue > 1.0f) {
					finalDensity = highNoiseSample;
				}
				else {
					finalDensity = lowNoiseSample + (highNoiseSample - lowNoiseSample) * selectorValue;
				}

				finalDensity -= structuralWeight;
				float boundingFalloff;

				if (yCellInner > sizeY - 4) {
					boundingFalloff = (float)((float)(yCellInner - (sizeY - 4)) / 3.0f);
					finalDensity = finalDensity * (1.0f - boundingFalloff) + -10.0f * boundingFalloff;
				}

				if ((float)yCellInner < landscapeBias) {
					boundingFalloff = (landscapeBias - (float)yCellInner) / 4.0f;
					if (boundingFalloff < 0.0f) {
						boundingFalloff = 0.0f;
					}
					if (boundingFalloff > 1.0f) {
						boundingFalloff = 1.0f;
					}
					finalDensity = finalDensity * (1.0f - boundingFalloff) + -10.0f * boundingFalloff;
				}

				noiseArray[volumeIndex] = finalDensity;
				++volumeIndex;
			}
		}
	}

	return noiseArray;
}

void ChunkProviderHell::ProvideShape(Chunk& chunk, Random& random, uint64_t seed)
{
	Random noiseRand(seed);

	const int horizontalSections = 4;
	const int lavaOceanCeiling = 32;
	const int sizeX = horizontalSections + 1;
	const int sizeY = 17;
	const int sizeZ = horizontalSections + 1;

	const int noiseScaleX = CHUNK_WIDTH / 4;
	const int scaledY = CHUNK_HEIGHT / 8;

	static float* noiseArray = nullptr;

	int chunkGridX = chunk.PosX;
	int chunkGridZ = chunk.PosZ;

	noiseArray = generateBaseNoise(noiseArray, chunkGridX * horizontalSections, 0, chunkGridZ * horizontalSections, sizeX, sizeY, sizeZ, noiseRand);

	for (int cellX = 0; cellX < horizontalSections; ++cellX) {
		for (int cellZ = 0; cellZ < horizontalSections; ++cellZ) {
			for (int cellY = 0; cellY < scaledY; ++cellY) {
				const float invStepY = 0.125f;
				float noiseBottom_X0_Z0 = noiseArray[((cellX + 0) * sizeZ + (cellZ + 0)) * sizeY + (cellY + 0)];
				float noiseBottom_X0_Z1 = noiseArray[((cellX + 0) * sizeZ + (cellZ + 1)) * sizeY + (cellY + 0)];
				float noiseBottom_X1_Z0 = noiseArray[((cellX + 1) * sizeZ + (cellZ + 0)) * sizeY + (cellY + 0)];
				float noiseBottom_X1_Z1 = noiseArray[((cellX + 1) * sizeZ + (cellZ + 1)) * sizeY + (cellY + 0)];
				const float slopeY_X0_Z0 = (noiseArray[((cellX + 0) * sizeZ + (cellZ + 0)) * sizeY + (cellY + 1)] - noiseBottom_X0_Z0) * invStepY;
				const float slopeY_X0_Z1 = (noiseArray[((cellX + 0) * sizeZ + (cellZ + 1)) * sizeY + (cellY + 1)] - noiseBottom_X0_Z1) * invStepY;
				const float slopeY_X1_Z0 = (noiseArray[((cellX + 1) * sizeZ + (cellZ + 0)) * sizeY + (cellY + 1)] - noiseBottom_X1_Z0) * invStepY;
				const float slopeY_X1_Z1 = (noiseArray[((cellX + 1) * sizeZ + (cellZ + 1)) * sizeY + (cellY + 1)] - noiseBottom_X1_Z1) * invStepY;

				for (int blockYInCell = 0; blockYInCell < 8; ++blockYInCell) {
					const float invStepX = 0.25f;
					float currentX0_Z0 = noiseBottom_X0_Z0;
					float currentX0_Z1 = noiseBottom_X0_Z1;
					const float slopeX_Z0 = (noiseBottom_X1_Z0 - noiseBottom_X0_Z0) * invStepX;
					const float slopeX_Z1 = (noiseBottom_X1_Z1 - noiseBottom_X0_Z1) * invStepX;

					for (int blockXInCell = 0; blockXInCell < 4; ++blockXInCell) {
						const float invStepZ = 0.25f;
						float currentZ = currentX0_Z0;
						const float slopeZ = (currentX0_Z1 - currentX0_Z0) * invStepZ;
						currentZ -= slopeZ;

						for (int blockZInCell = 0; blockZInCell < 4; ++blockZInCell) {
							const int worldX = cellX * 4 + blockXInCell;
							const int worldY = cellY * 8 + blockYInCell;
							const int worldZ = cellZ * 4 + blockZInCell;

							float blockDensity = (currentZ += slopeZ);

							BlockStorage block = 0;
							if (worldY < lavaOceanCeiling) {
								block = { BLOCK_LAVA, 8 };
							}

							if (blockDensity > 0.0f) {
								block = BLOCK_NETHERRACK;
							}

							chunk.SetBlock(worldX, worldY, worldZ, block);
						}
						currentX0_Z0 += slopeX_Z0;
						currentX0_Z1 += slopeX_Z1;
					}
					noiseBottom_X0_Z0 += slopeY_X0_Z0;
					noiseBottom_X0_Z1 += slopeY_X0_Z1;
					noiseBottom_X1_Z0 += slopeY_X1_Z0;
					noiseBottom_X1_Z1 += slopeY_X1_Z1;
				}
			}
		}
	}

	ProvideSurface(chunk, random, seed);

	chunk.GenStage = TERRAIN_GEN_SURFACE;
}

void ChunkProviderHell::ProvideSurface(Chunk& chunk, Random& random, uint64_t seed)
{
	const int lavaSeaLevel = 64;
	const float noiseFrequency = 1.0f / 32.0f;

	Random noiseRand(seed);
	auto octavesGravelSand = PerlinNoiseOctaves(noiseRand, 4);
	auto octavesDepth = PerlinNoiseOctaves(noiseRand, 4);

	int chunkStartX = chunk.PosX;
	int chunkStartZ = chunk.PosZ;

	static float* gravelNoiseBuffer = nullptr;
	static float* soulSandNoiseBuffer = nullptr;
	static float* depthNoiseBuffer = nullptr;

	gravelNoiseBuffer = octavesGravelSand.Sample(gravelNoiseBuffer, (float)(chunkStartX * 16), (float)(chunkStartZ * 16), 0.0f, 16, 16, 1, noiseFrequency, noiseFrequency, 1.0f);
	soulSandNoiseBuffer = octavesGravelSand.Sample(soulSandNoiseBuffer, (float)(chunkStartZ * 16), 109.0134f, (float)(chunkStartX * 16), 16, 1, 16, noiseFrequency, 1.0f, noiseFrequency);
	depthNoiseBuffer = octavesDepth.Sample(depthNoiseBuffer, (float)(chunkStartX * 16), (float)(chunkStartZ * 16), 0.0f, 16, 16, 1, noiseFrequency * 2.0f, noiseFrequency * 2.0f, noiseFrequency * 2.0f);

	for (int localX = 0; localX < 16; ++localX) {
		for (int localZ = 0; localZ < 16; ++localZ) {
			bool spawnGravel = gravelNoiseBuffer[localX + localZ * 16] + random.NextFloat() * 0.2f > 0.0f;
			bool spawnSoulSand = soulSandNoiseBuffer[localX + localZ * 16] + random.NextFloat() * 0.2f > 0.0f;
			int surfaceDepthTarget = (int)(depthNoiseBuffer[localX + localZ * 16] / 3.0f + 3.0f + random.NextFloat() * 0.25f);
			int currentDepth = -1;

			BlockStorage surfaceBlock = BLOCK_NETHERRACK;
			BlockStorage fillerBlock = BLOCK_NETHERRACK;

			for (int localY = 127; localY >= 0; --localY) {
				if (localY >= 127 - random.NextInt(5)) {
					chunk.SetBlock(localX, localY, localZ, BLOCK_BEDROCK);
				}
				else if (localY <= 0 + random.NextInt(5)) {
					chunk.SetBlock(localX, localY, localZ, BLOCK_BEDROCK);
				}
				else {
					int blockId = chunk.GetBlock(localX, localY, localZ).Id;
					if (blockId == 0) {
						currentDepth = -1;
					}
					else if (blockId == BLOCK_NETHERRACK) {
						if (currentDepth == -1) {
							if (surfaceDepthTarget <= 0) {
								surfaceBlock = 0;
								fillerBlock = BLOCK_NETHERRACK;
							}
							else if (localY >= lavaSeaLevel - 4 && localY <= lavaSeaLevel + 1) {
								surfaceBlock = BLOCK_NETHERRACK;
								fillerBlock = BLOCK_NETHERRACK;
								if (spawnSoulSand) {
									surfaceBlock = BLOCK_GRAVEL;
								}
								if (spawnSoulSand) {
									fillerBlock = BLOCK_NETHERRACK;
								}
								if (spawnGravel) {
									surfaceBlock = BLOCK_SOUL_SAND;
								}
								if (spawnGravel) {
									fillerBlock = BLOCK_SOUL_SAND;
								}
							}

							if (localY < lavaSeaLevel && surfaceBlock.Id == 0) {
								surfaceBlock = { BLOCK_LAVA, 8 };
								chunk.ChunkWorld->ScheduleUpdate(localX + chunk.GetStartX(), localY, localZ + chunk.GetStartZ(), 0, TICK_TYPE_LIQUID);
							}

							currentDepth = surfaceDepthTarget;
							if (localY >= lavaSeaLevel - 1) {
								chunk.SetBlock(localX, localY, localZ, surfaceBlock);
							}
							else {
								chunk.SetBlock(localX, localY, localZ, fillerBlock);
							}
						}
						else if (currentDepth > 0) {
							--currentDepth;
							chunk.SetBlock(localX, localY, localZ, fillerBlock);
						}
					}
				}
			}
		}
	}
	WorldGenCaves().GenerateRegion(chunk.ChunkWorld, &chunk);
}

void ChunkProviderHell::DecorateSurface(Chunk& chunk, Random& random, uint64_t seed)
{
	int startX = chunk.GetStartX();
	int startZ = chunk.GetStartZ();

	CachedChunkAccess access(chunk.ChunkWorld, chunk.GetStartX(), chunk.GetStartZ(), 1);

	for (int i = 0; i < 8; i++)
	{
		int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
		int y = random.NextInt(120) + 4;
		int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
		WorldGenNetherLava().GenerateAt(access, random, x, y, z);
	}

	int extraCount = random.NextInt(random.NextInt(10) + 1) + 1;

	for (int i = 0; i < 10 + extraCount; i++)
	{
		int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
		int y = random.NextInt(128);
		int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
		WorldGenGlowstone(BLOCK_GLOWSTONE).GenerateAt(access, random, x, y, z);
	}

	int fireCount = random.NextInt(random.NextInt(10) + 1);

	for (int i = 0; i < fireCount; i++)
	{
		int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
		int y = random.NextInt(128);
		int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
		WorldGenFire(BLOCK_FIRE).GenerateAt(access, random, x, y, z);
	}

	if (random.NextInt(1) == 0)
	{
		int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
		int y = random.NextInt(128);
		int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
		WorldGenFlowers(BLOCK_BROWN_MUSHROOM).GenerateAt(access, random, x, y, z);
	}

	if (random.NextInt(1) == 0)
	{
		int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
		int y = random.NextInt(128);
		int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
		WorldGenFlowers(BLOCK_RED_MUSHROOM).GenerateAt(access, random, x, y, z);
	}
}

void ChunkProviderHell::LightChunk(Chunk& chunk)
{
	CachedChunkAccess access(chunk.ChunkWorld, chunk.GetStartX(), chunk.GetStartZ(), 1);

	for (int x = 0; x < 16; x++)
	{
		for (int z = 0; z < 16; z++)
		{
			int xx = chunk.GetStartX() + x;
			int zz = chunk.GetStartZ() + z;

			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				if (chunk.GetBlockLightLocal(x, y, z) < chunk.GetBlock(x, y, z).GetDef()->GetLightEmitted())
				{
					chunk.ChunkWorld->UpdateBlockLight(chunk.GetStartX() + x, y, chunk.GetStartZ() + z, &access);
				}
			}
		}
	}
}
