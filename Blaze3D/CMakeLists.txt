# Blaze3D actual CMake file

project(Blaze3D)

# Dependencies

set(ZLIB_USE_STATIC_LIBS OFF)

find_package(SDL3 REQUIRED CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(glm REQUIRED)

set(B3D_SOURCE
	Blaze3D.cpp
	src/client/Client.cpp
	src/client/MusicManager.cpp
	src/client/ParticleEngine.cpp
	src/client/PlayerHandler.cpp
	src/client/Profiler.cpp
	src/client/renderer/AnimatedFireTexture.cpp
	src/client/renderer/AnimatedLavaTexture.cpp
	src/client/renderer/AnimatedPortalTexture.cpp
	src/client/renderer/AnimatedWaterTexture.cpp
	src/client/renderer/block/BlockPreviewRenderer.cpp
	src/client/renderer/block/BlockRenderer.cpp
	src/client/renderer/ColorizerGrass.cpp
	src/client/renderer/entity/ArrowEntityRenderer.cpp
	src/client/renderer/entity/CreatureEntityRenderer.cpp
	src/client/renderer/entity/CreeperEntityRenderer.cpp
	src/client/renderer/entity/EntityRendererDispatcher.cpp
	src/client/renderer/entity/FallingSandEntityRenderer.cpp
	src/client/renderer/entity/ItemEntityRenderer.cpp
	src/client/renderer/entity/PigZombieEntityRenderer.cpp
	src/client/renderer/entity/TNTPrimedEntityRenderer.cpp
	src/client/renderer/ItemRenderer.cpp
	src/client/renderer/model/ModelBase.cpp
	src/client/renderer/model/ModelBox.cpp
	src/client/renderer/model/ModelChicken.cpp
	src/client/renderer/model/ModelCow.cpp
	src/client/renderer/model/ModelCreeper.cpp
	src/client/renderer/model/ModelHumanoid.cpp
	src/client/renderer/model/ModelPart.cpp
	src/client/renderer/model/ModelPartItem.cpp
	src/client/renderer/model/ModelPig.cpp
	src/client/renderer/model/ModelRegistry.cpp
	src/client/renderer/RenderHelper.cpp
	src/client/renderer/VertexProducer.cpp
	src/client/renderer/WorldRenderer.cpp
	src/client/ResourceManager.cpp
	src/client/SPPlayerHandler.cpp
	src/client/SPWorldAccess.cpp
	src/client/ui/IngameInterface.cpp
	src/client/ui/ScreenBase.cpp
	src/client/ui/ScreenContainer.cpp
	src/client/ui/ScreenCrafting.cpp
	src/client/ui/ScreenFurnace.cpp
	src/client/ui/ScreenGameOver.cpp
	src/client/ui/ScreenInventory.cpp
	src/client/ui/ScreenMainMenu.cpp
	src/client/ui/ScreenPause.cpp
	src/client/ui/ScreenWorldSelection.cpp
	src/common/block/BlockBookshelf.cpp
	src/common/block/BlockCactus.cpp
	src/common/block/BlockChest.cpp
	src/common/block/Block.cpp
	src/common/block/BlockCraftingTable.cpp
	src/common/block/BlockCrop.cpp
	src/common/block/BlockCrossShaped.cpp
	src/common/block/BlockDoor.cpp
	src/common/block/BlockFallingSand.cpp
	src/common/block/BlockFarmland.cpp
	src/common/block/BlockFire.cpp
	src/common/block/BlockFlower.cpp
	src/common/block/BlockFungus.cpp
	src/common/block/BlockFurnace.cpp
	src/common/block/BlockGrass.cpp
	src/common/block/BlockGravel.cpp
	src/common/block/BlockLeaves.cpp
	src/common/block/BlockLiquid.cpp
	src/common/block/BlockLog.cpp
	src/common/block/BlockNetherPortal.cpp
	src/common/block/BlockPumpkin.cpp
	src/common/block/BlockReed.cpp
	src/common/block/BlockSlab.cpp
	src/common/block/BlockStairs.cpp
	src/common/block/BlockTNT.cpp
	src/common/block/BlockTorch.cpp
	src/common/CachedChunkAccess.cpp
	src/common/Chunk.cpp
	src/common/entity/AABB.cpp
	src/common/entity/AnimalEntityBase.cpp
	src/common/entity/ArrowEntity.cpp
	src/common/entity/BaseEntity.cpp
	src/common/entity/ChickenEntity.cpp
	src/common/entity/CowEntity.cpp
	src/common/entity/CreatureEntity.cpp
	src/common/entity/CreeperEntity.cpp
	src/common/entity/FallingSandEntity.cpp
	src/common/entity/ItemEntity.cpp
	src/common/entity/MobEntity.cpp
	src/common/entity/PigEntity.cpp
	src/common/entity/PigZombieEntity.cpp
	src/common/entity/PlayerEntity.cpp
	src/common/entity/TNTPrimedEntity.cpp
	src/common/entity/ZombieEntity.cpp
	src/common/gen/TerrainDecorator.cpp
	src/common/gen/WorldGenBase.cpp
	src/common/gen/WorldGenCactus.cpp
	src/common/gen/WorldGenCaves.cpp
	src/common/gen/WorldGenClay.cpp
	src/common/gen/WorldGenFire.cpp
	src/common/gen/WorldGenFlowers.cpp
	src/common/gen/WorldGenGlowstone.cpp
	src/common/gen/WorldGenMinable.cpp
	src/common/gen/WorldGenNetherLava.cpp
	src/common/gen/WorldGenPumpkin.cpp
	src/common/gen/WorldGenReeds.cpp
	src/common/item/ItemBow.cpp
	src/common/item/ItemBucket.cpp
	src/common/item/Item.cpp
	src/common/item/ItemDoor.cpp
	src/common/item/ItemEmptyBucket.cpp
	src/common/item/ItemFlintAndSteel.cpp
	src/common/item/ItemFood.cpp
	src/common/item/ItemReed.cpp
	src/common/item/ItemWheatSeed.cpp
	src/common/ItemStack.cpp
	src/common/IVec3Set.cpp
	src/common/nbt/BigEndianDataStream.cpp
	src/common/nbt/NBTBase.cpp
	src/common/nbt/TagByteArray.cpp
	src/common/nbt/TagCompound.cpp
	src/common/nbt/TagList.cpp
	src/common/nbt/TagNumeric.cpp
	src/common/nbt/TagString.cpp
	src/common/noise/PerlinNoise.cpp
	src/common/noise/SimplexNoise.cpp
	src/common/path/PathFinder.cpp
	src/common/PortalStructure.cpp
	src/common/Random.cpp
	src/common/Recipe.cpp
	src/common/RegionFile.cpp
	src/common/SaveManager.cpp
	src/common/Teleporter.cpp
	src/common/tile/ChestTileEntity.cpp
	src/common/tile/FurnaceTileEntity.cpp
	src/common/tile/TileEntity.cpp
	src/common/Timer.cpp
	src/common/world/BiomeBase.cpp
	src/common/world/BiomeProvider.cpp
	src/common/world/ChunkProvider.cpp
	src/common/world/ChunkProviderHell.cpp
	src/common/World.cpp
	src/common/world/Explosion.cpp
	src/common/WorldHell.cpp
	src/common/world/MobSpawner.cpp
	src/engine/Audio.cpp
	src/engine/Font.cpp
	src/engine/memory/MemoryAllocator.cpp
	src/engine/ScaledResolution.cpp
	src/engine/Time.cpp
	src/engine/zlib/izlibstream.cpp
	src/engine/zlib/ozlibstream.cpp
	src/thirdparty/glm/detail/glm.cpp
	src/engine/miniaudio/miniaudio.c
)


set(B3D_HEADER
		src/client/Client.h
		src/client/MusicManager.h
		src/client/ParticleEngine.h
		src/client/PlayerHandler.h
		src/client/Profiler.h
		src/client/renderer/AnimatedFireTexture.h
		src/client/renderer/AnimatedLavaTexture.h
		src/client/renderer/AnimatedPortalTexture.h
		src/client/renderer/AnimatedWaterTexture.h
		src/client/renderer/block/BlockPreviewRenderer.h
		src/client/renderer/block/BlockRenderer.h
		src/client/renderer/ColorizerGrass.h
		src/client/renderer/entity/ArrowEntityRenderer.h
		src/client/renderer/entity/CreatureEntityRenderer.h
		src/client/renderer/entity/CreeperEntityRenderer.h
		src/client/renderer/entity/EntityRendererDispatcher.h
		src/client/renderer/entity/EntityRenderer.h
		src/client/renderer/entity/FallingSandEntityRenderer.h
		src/client/renderer/entity/ItemEntityRenderer.h
		src/client/renderer/entity/PigZombieEntityRenderer.h
		src/client/renderer/entity/TNTPrimedEntityRenderer.h
		src/client/renderer/ItemRenderer.h
		src/client/renderer/model/ModelBase.h
		src/client/renderer/model/ModelBox.h
		src/client/renderer/model/ModelChicken.h
		src/client/renderer/model/ModelCow.h
		src/client/renderer/model/ModelCreeper.h
		src/client/renderer/model/ModelHumanoid.h
		src/client/renderer/model/ModelPart.h
		src/client/renderer/model/ModelPartItem.h
		src/client/renderer/model/ModelPig.h
		src/client/renderer/model/ModelRegistry.h
		src/client/renderer/RenderHelper.h
		src/client/renderer/VertexProducer.h
		src/client/renderer/WorldRenderer.h
		src/client/ResourceManager.h
		src/client/SPPlayerHandler.h
		src/client/SPWorldAccess.h
		src/client/ui/IngameInterface.h
		src/client/ui/ScreenBase.h
		src/client/ui/ScreenContainer.h
		src/client/ui/ScreenCrafting.h
		src/client/ui/ScreenFurnace.h
		src/client/ui/ScreenGameOver.h
		src/client/ui/ScreenInventory.h
		src/client/ui/ScreenMainMenu.h
		src/client/ui/ScreenPause.h
		src/client/ui/ScreenWorldSelection.h
		src/client/ui/SlotIds.h
		src/common/block/BlockBookshelf.h
		src/common/block/BlockCactus.h
		src/common/block/BlockChest.h
		src/common/block/BlockCraftingTable.h
		src/common/block/BlockCrop.h
		src/common/block/BlockCrossShaped.h
		src/common/block/BlockDoor.h
		src/common/block/BlockFallingSand.h
		src/common/block/BlockFarmland.h
		src/common/block/BlockFire.h
		src/common/block/BlockFlower.h
		src/common/block/BlockFungus.h
		src/common/block/BlockFurnace.h
		src/common/block/BlockGrass.h
		src/common/block/BlockGravel.h
		src/common/block/Block.h
		src/common/block/BlockLeaves.h
		src/common/block/BlockLiquid.h
		src/common/block/BlockLog.h
		src/common/block/BlockNetherPortal.h
		src/common/block/BlockPumpkin.h
		src/common/block/BlockReed.h
		src/common/block/BlockSlab.h
		src/common/block/BlockStairs.h
		src/common/block/BlockTNT.h
		src/common/block/BlockTorch.h
		src/common/CachedChunkAccess.h
		src/common/Chunk.h
		src/common/entity/AABB.h
		src/common/entity/AnimalEntityBase.h
		src/common/entity/ArrowEntity.h
		src/common/entity/BaseEntity.h
		src/common/entity/ChickenEntity.h
		src/common/entity/CowEntity.h
		src/common/entity/CreatureEntity.h
		src/common/entity/CreeperEntity.h
		src/common/entity/FallingSandEntity.h
		src/common/entity/ItemEntity.h
		src/common/entity/MobEntity.h
		src/common/entity/PigEntity.h
		src/common/entity/PigZombieEntity.h
		src/common/entity/PlayerEntity.h
		src/common/entity/TNTPrimedEntity.h
		src/common/entity/ZombieEntity.h
		src/common/FastNoise.h
		src/common/gen/TerrainDecorator.h
		src/common/gen/WorldGenBase.h
		src/common/gen/WorldGenCactus.h
		src/common/gen/WorldGenCaves.h
		src/common/gen/WorldGenClay.h
		src/common/gen/WorldGenFire.h
		src/common/gen/WorldGenFlowers.h
		src/common/gen/WorldGenGlowstone.h
		src/common/gen/WorldGenMinable.h
		src/common/gen/WorldGenNetherLava.h
		src/common/gen/WorldGenPumpkin.h
		src/common/gen/WorldGenReeds.h
		src/common/item/ItemBow.h
		src/common/item/ItemBucket.h
		src/common/item/ItemDoor.h
		src/common/item/ItemEmptyBucket.h
		src/common/item/ItemFlintAndSteel.h
		src/common/item/ItemFood.h
		src/common/item/Item.h
		src/common/item/ItemReed.h
		src/common/item/ItemWheatSeed.h
		src/common/ItemStack.h
		src/common/IVec3Set.h
		src/common/IWorldAccess.h
		src/common/nbt/BigEndianDataStream.h
		src/common/nbt/NBTBase.h
		src/common/nbt/NBT.h
		src/common/nbt/TagByteArray.h
		src/common/nbt/TagCompound.h
		src/common/nbt/TagList.h
		src/common/nbt/TagNumeric.h
		src/common/nbt/TagString.h
		src/common/noise/PerlinNoise.h
		src/common/noise/SimplexNoise.h
		src/common/path/NavPath.h
		src/common/path/PathFinder.h
		src/common/PortalStructure.h
		src/common/Random.h
		src/common/Recipe.h
		src/common/RegionFile.h
		src/common/SaveManager.h
		src/common/Teleporter.h
		src/common/tile/ChestTileEntity.h
		src/common/tile/FurnaceTileEntity.h
		src/common/tile/TileEntity.h
		src/common/Timer.h
		src/common/ToolTier.h
		src/common/world/BiomeBase.h
		src/common/world/BiomeProvider.h
		src/common/world/ChunkProvider.h
		src/common/world/ChunkProviderHell.h
		src/common/WorldCommon.h
		src/common/world/Explosion.h
		src/common/World.h
		src/common/WorldHell.h
		src/common/world/MobSpawner.h
		src/engine/Audio.h
		src/engine/Common.h
		src/engine/Font.h
		src/engine/Frustum.h
		src/engine/memory/MemoryAllocator.h
		src/engine/miniaudio/miniaudio.h
		src/engine/RenderingInterface.h
		src/engine/ScaledResolution.h
		src/engine/System.h
		src/engine/Time.h
		src/engine/win32/rsc/PixelShader.h
		src/engine/win32/rsc/VertexShader.h
		src/engine/zlib/izlibstream.h
		src/engine/zlib/ozlibstream.h
		src/engine/zlib/zlib_streambuf.h
		src/thirdparty/glm/simd/common.h
		src/thirdparty/glm/simd/exponential.h
		src/thirdparty/glm/simd/geometric.h
		src/thirdparty/glm/simd/integer.h
		src/thirdparty/glm/simd/matrix.h
		src/thirdparty/glm/simd/neon.h
		src/thirdparty/glm/simd/packing.h
		src/thirdparty/glm/simd/platform.h
		src/thirdparty/glm/simd/trigonometric.h
		src/thirdparty/glm/simd/vector_relational.h
		src/thirdparty/stb_image.h
		src/Util.h
)

set(B3D_EXTRA_LIBS)

if (NOT B3D_BACKEND)

	if (WIN32)
		set(B3D_BACKEND "WIN32")
	elseif(UNIX)
		set(B3D_BACKEND "UNIX")
	endif()
endif()


if (B3D_BACKEND STREQUAL "WIN32")

	message("Using WIN32 backend code")

	if (NOT B3D_RENDERER)
		set(B3D_RENDERER D3D11)
	endif()

	list(APPEND B3D_SOURCE
			src/engine/win32/PragmaLibs.cpp
			src/engine/win32/System.cpp
			src/engine/win32/Win32RenderingInterface.cpp
	)

	set(B3D_EXTRA_LIBS d3d11)
	

elseif (B3D_BACKEND STREQUAL "UNIX")

	message("Using UNIX backend code")

	if (NOT B3D_RENDERER)
		set(B3D_RENDERER GLES)
	endif()

	list(APPEND B3D_SOURCE
			src/engine/unix/System.cpp
			src/engine/unix/UnixRenderingInterface.cpp
			src/engine/unix/glad/glad.c
	)

	list(APPEND B3D_HEADER
			src/engine/unix/shaders/PixelShader.h
			src/engine/unix/shaders/VertexShader.h
	)


else ()
	message(FATAL_ERROR "Invalid platform backend " ${B3D_BACKEND})
endif ()



add_executable(Blaze3D ${B3D_SOURCE} ${B3D_HEADER})
add_compile_definitions(CMAKE_BUILD)

target_include_directories(Blaze3D PRIVATE 
	${ZLIB_INCLUDE_DIRS} 
	${SDL3_INCLUDE_DIRS} 
	${GLM_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

message("Using ${B3D_RENDERER} for rendering (RENDERER_${B3D_RENDERER})")

target_compile_definitions(Blaze3D PRIVATE "RENDERER_${B3D_RENDERER}")

target_link_libraries(Blaze3D PRIVATE SDL3::SDL3 ZLIB::ZLIB glm::glm-header-only ${B3D_EXTRA_LIBS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../Runtime/mc DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
