#include "ChunkProvider.h"

#include "common/World.h"
#include "common/CachedChunkAccess.h"
#include "common/gen/WorldGenCaves.h"
#include "common/gen/TerrainDecorator.h"
#include "common/noise/PerlinNoise.h"
#include "BiomeBase.h"

float* SampleChunkNoise(float* noiseArray, int localX, int localY, int localZ, int sizeX, int sizeY, int sizeZ, uint64_t seed, BiomeProvider& provider) {
	if (noiseArray == NULL) {
		noiseArray = new float[sizeX * sizeY * sizeZ];
	}

	float coordinateScaleXZ = 684.412f;
	float coordinateScaleY = 684.412f;
	// float* var12 = worldObj.func_4075_a().temperature;
	// float* var13 = worldObj.func_4075_a().humidity;

	static float* field_4185_d = NULL;
	static float* field_4184_e = NULL;
	static float* field_4183_f = NULL;
	static float* field_4182_g = NULL;
	static float* field_4181_h = NULL;

	Random random = { seed };
	auto field_912_k = PerlinNoiseOctaves(random, 16);
	auto field_911_l = PerlinNoiseOctaves(random, 16);
	auto field_910_m = PerlinNoiseOctaves(random, 8);
	auto field_909_n = PerlinNoiseOctaves(random, 4);
	auto field_908_o = PerlinNoiseOctaves(random, 4);
	auto field_922_a = PerlinNoiseOctaves(random, 10);
	auto field_921_b = PerlinNoiseOctaves(random, 16);

	field_4182_g = field_922_a.Sample2D(field_4182_g, localX, localZ, sizeX, sizeZ, 1.121f, 1.121f);
	field_4181_h = field_921_b.Sample2D(field_4181_h, localX, localZ, sizeX, sizeZ, 200.0f, 200.0f);
	field_4185_d = field_910_m.Sample(field_4185_d, (float)localX, (float)localY, (float)localZ, sizeX, sizeY, sizeZ, coordinateScaleXZ / 80.0f, coordinateScaleY / 160.0f, coordinateScaleXZ / 80.0f);
	field_4184_e = field_912_k.Sample(field_4184_e, (float)localX, (float)localY, (float)localZ, sizeX, sizeY, sizeZ, coordinateScaleXZ, coordinateScaleY, coordinateScaleXZ);
	field_4183_f = field_911_l.Sample(field_4183_f, (float)localX, (float)localY, (float)localZ, sizeX, sizeY, sizeZ, coordinateScaleXZ, coordinateScaleY, coordinateScaleXZ);

	int var14 = 0;
	int var15 = 0;
	int var16 = 16 / sizeX;

	for (int var17 = 0; var17 < sizeX; ++var17) {
		int var18 = var17 * var16 + var16 / 2;

		for (int var19 = 0; var19 < sizeZ; ++var19) {
			int var20 = var19 * var16 + var16 / 2;
			float var21 = provider.GetTemperatureForColumn(var18 + localX, var20 + localZ);
			float var23 = provider.GetHumidityForColumn(var18 + localX, var20 + localZ) * var21; //var13[var18 * 16 + var20] * var21;
			float var25 = 1.0f - var23;
			var25 *= var25;
			var25 *= var25;
			var25 = 1.0f - var25;
			float var27 = (field_4182_g[var15] + 256.0f) / 512.0f;
			var27 *= var25;
			if (var27 > 1.0f) {
				var27 = 1.0f;
			}

			float var29 = field_4181_h[var15] / 8000.0f;
			if (var29 < 0.0f) {
				var29 = -var29 * 0.3f;
			}

			var29 = var29 * 3.0f - 2.0f;
			if (var29 < 0.0f) {
				var29 /= 2.0f;
				if (var29 < -1.0f) {
					var29 = -1.0f;
				}

				var29 /= 1.4f;
				var29 /= 2.0f;
				var27 = 0.0f;
			}
			else {
				if (var29 > 1.0f) {
					var29 = 1.0f;
				}

				var29 /= 8.0f;
			}

			if (var27 < 0.0f) {
				var27 = 0.0f;
			}

			var27 += 0.5f;
			var29 = var29 * (float)sizeY / 16.0f;
			float var31 = (float)sizeY / 2.0f + var29 * 4.0f;
			++var15;

			for (int var33 = 0; var33 < sizeY; ++var33) {
				float var34 = 0.0f;
				float var36 = ((float)var33 - var31) * 12.0f / var27;
				if (var36 < 0.0f) {
					var36 *= 4.0f;
				}

				float var38 = field_4184_e[var14] / 512.0f;
				float var40 = field_4183_f[var14] / 512.0f;
				float var42 = (field_4185_d[var14] / 10.0f + 1.0f) / 2.0f;
				if (var42 < 0.0f) {
					var34 = var38;
				}
				else if (var42 > 1.0f) {
					var34 = var40;
				}
				else {
					var34 = var38 + (var40 - var38) * var42;
				}

				var34 -= var36;
				if (var33 > sizeY - 4) {
					float var44 = (float)((float)(var33 - (sizeY - 4)) / 3.0F);
					var34 = var34 * (1.0f - var44) + -10.0f * var44;
				}

				noiseArray[var14] = var34;
				++var14;
			}
		}
	}

	return noiseArray;
}

ChunkProvider::ChunkProvider(uint64_t seed) : BiomeGenerator(seed)
{
}

void ChunkProvider::ProvideShape(Chunk& chunk, Random& random, uint64_t seed)
{
	BiomeGenerator.GenerateValuesForChunk(chunk.GetStartX(), chunk.GetStartZ());

	const int noiseScale = CHUNK_WIDTH / 4;
	const int scaledY = CHUNK_HEIGHT / 8;
	const int sizeX = noiseScale + 1;
	const int sizeY = scaledY + 1;
	const int sizeZ = noiseScale + 1;

	const float scaleLow = 4.0f / 173.01f;
	const float scaleHigh = 4.0f / 173.01f;
	const float scaleSelector = 0.03125f;

	float noiseArray[sizeX * sizeY * sizeZ]{};

	SampleChunkNoise(noiseArray, chunk.PosX * noiseScale, 0, chunk.PosZ * noiseScale, sizeX, sizeY, sizeZ, seed, BiomeGenerator);

	for (int l2 = 0; l2 < noiseScale; ++l2) {
		for (int i5 = 0; i5 < noiseScale; ++i5) {
			for (int j4 = 0; j4 < scaledY; ++j4) {
				const float d = 0.125;
				float d2 = noiseArray[((l2 + 0) * sizeZ + (i5 + 0)) * sizeY + (j4 + 0)];
				float d3 = noiseArray[((l2 + 0) * sizeZ + (i5 + 1)) * sizeY + (j4 + 0)];
				float d4 = noiseArray[((l2 + 1) * sizeZ + (i5 + 0)) * sizeY + (j4 + 0)];
				float d5 = noiseArray[((l2 + 1) * sizeZ + (i5 + 1)) * sizeY + (j4 + 0)];
				const float d6 = (noiseArray[((l2 + 0) * sizeZ + (i5 + 0)) * sizeY + (j4 + 1)] - d2) * d;
				const float d7 = (noiseArray[((l2 + 0) * sizeZ + (i5 + 1)) * sizeY + (j4 + 1)] - d3) * d;
				const float d8 = (noiseArray[((l2 + 1) * sizeZ + (i5 + 0)) * sizeY + (j4 + 1)] - d4) * d;
				const float d9 = (noiseArray[((l2 + 1) * sizeZ + (i5 + 1)) * sizeY + (j4 + 1)] - d5) * d;
				for (int k3 = 0; k3 < 8; ++k3) {
					const float d10 = 0.25;
					float d11 = d2;
					float d12 = d3;
					const float d13 = (d4 - d2) * d10;
					const float d14 = (d5 - d3) * d10;
					for (int l3 = 0; l3 < 4; ++l3) {
						const float d15 = 0.25;
						float d16 = d11;
						const float d17 = (d12 - d11) * d15;
						d16 -= d17;
						for (int k4 = 0; k4 < 4; ++k4) {
							const int x3 = l2 * 4 + l3;
							const int y0 = j4 * 8 + k3;
							const int z3 = i5 * 4 + k4;

							float noise = (d16 += d17);

							if (noise > 0.0f) {
								chunk.SetBlock(x3, y0, z3, BLOCK_STONE);
							}
						}
						d11 += d13;
						d12 += d14;
					}
					d2 += d6;
					d3 += d7;
					d4 += d8;
					d5 += d9;
				}
			}
		}
	}
	ProvideSurface(chunk, random, seed);
}

void ChunkProvider::ProvideSurface(Chunk& chunk, Random& random, uint64_t seed)
{
	float sandNoise[CHUNK_WIDTH * CHUNK_WIDTH];

	auto sandNoiseGen = PerlinNoiseOctaves(random, 4);
	sandNoiseGen.Sample(sandNoise, (float)chunk.GetStartX(), (float)chunk.GetStartZ(), 0.0f, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0.02f, 0.02f, 0.0f);

	for (int ix = 0; ix < CHUNK_WIDTH; ix++)
	{
		for (int iz = 0; iz < CHUNK_WIDTH; iz++)
		{
			int x = ix;
			int z = iz;

			bool surfaceFree = true;
			int surfaceDepth = 0;

			for (int y = CHUNK_HEIGHT - 1; y >= 0; y--)
			{
				if (chunk.GetBlock(x, y, z).GetDef()->IsSolid())
				{
					surfaceFree = false;
				}
				if (!surfaceFree)
				{
					if (chunk.GetBlock(x, y, z).Id != BLOCK_STONE)
					{
						surfaceDepth = 0;
						surfaceFree = true;
					}
					else
					{
						int fillerBlock = BiomeGenerator.GetBiomeForColumn(x, z)->FillerBlock;
						int topBlock = BiomeGenerator.GetBiomeForColumn(x, z)->TopBlock;

						if (y >= 60 && y < glm::max(67 + (int)sandNoise[iz + ix * CHUNK_WIDTH], 64))
						{
							topBlock = BLOCK_SAND;
							if (sandNoise[iz + ix * CHUNK_WIDTH] < 0.0f)
							{
								topBlock = BLOCK_GRAVEL;
							}
							fillerBlock = topBlock;
						}

						if (surfaceDepth < 3 + glm::abs((int)(sandNoise[iz + ix * CHUNK_WIDTH] * 0.5f)))
						{
							chunk.SetBlock(x, y, z, fillerBlock);
						}

						if (y < 60)
							topBlock = BLOCK_DIRT;

						if (surfaceDepth == 0)
						{
							chunk.SetBlock(x, y, z, topBlock);
						}
						surfaceDepth++;
					}
				}
				if (y <= 64 && chunk.GetBlock(x, y, z).Id == BLOCK_AIR)
				{
					chunk.SetBlock(x, y, z, { BLOCK_WATER, 8 });
				}
				if (y == 0)
				{
					chunk.SetBlock(x, y, z, BLOCK_BEDROCK);
				}
			}
		}
	}

	WorldGenCaves().GenerateRegion(chunk.ChunkWorld, &chunk);

	chunk.GenStage = TERRAIN_GEN_SURFACE;
}

void ChunkProvider::DecorateSurface(Chunk& chunk, Random& random, uint64_t seed)
{
	CachedChunkAccess chunkAccess(chunk.ChunkWorld, chunk.GetStartX(), chunk.GetStartZ(), 1);

	for (int x = 0; x < 16; x++)
	{
		for (int z = 0; z < 16; z++)
		{
			for (int y = 0; y < 5; y++)
			{
				if (y <= random.NextInt(5))
				{
					chunk.SetBlock(x, y, z, BLOCK_BEDROCK);
				}
			}
		}
	}

	TerrainDecorator decorator;
	decorator.DecorateTerrain(chunkAccess, random, chunk.GetStartX(), chunk.GetStartZ());
}

void ChunkProvider::LightChunk(Chunk& chunk)
{
	for (int x = 0; x < 16; x++)
	{
		for (int z = 0; z < 16; z++)
		{
			chunk.ComputeColumnHeight(x, z);
			for (int y = 0; y < CHUNK_HEIGHT; y++)
			{
				chunk.SetSkyLightLocal(x, y, z, chunk.GetHeightLocal(x, z) <= y ? 15 : 0);
			}
		}
	}
}

void ChunkProvider::PropogateChunk(Chunk& chunk)
{
	CachedChunkAccess access(chunk.ChunkWorld, chunk.GetStartX(), chunk.GetStartZ(), 1);

	for (int x = 0; x < 16; x++)
	{
		for (int z = 0; z < 16; z++)
		{
			int xx = chunk.GetStartX() + x;
			int zz = chunk.GetStartZ() + z;

			chunk.ChunkWorld->UpdateSkyLightColumn(xx, zz, &access);

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
