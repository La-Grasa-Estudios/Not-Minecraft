#include "Recipe.h"

#include "block/Block.h"
#include "item/Item.h"
#include <cstdint>

bool Recipe::Match(ItemStack* grid, int width, int height)
{
    return false;
}

ItemStack Recipe::GetResult()
{
    return m_Result;
}

Recipe* CreatePickaxeRecipe(int toolId, int material)
{
    return new ShapedRecipe(
        { material, material, material,
          BLOCK_AIR, (int)Item::Stick->GetId(), BLOCK_AIR,
          BLOCK_AIR, (int)Item::Stick->GetId(), BLOCK_AIR
        }, 3, 3, ItemStack{ (uint32_t)toolId, 1 });
}

Recipe* CreateAxeRecipe(int toolId, int material)
{
    return new ShapedRecipe(
        { BLOCK_AIR, material, material,
          BLOCK_AIR, (int)Item::Stick->GetId(), material,
          BLOCK_AIR, (int)Item::Stick->GetId(), BLOCK_AIR
        }, 3, 3, ItemStack{ (uint32_t)toolId, 1 });
}

Recipe* CreateShovelRecipe(int toolId, int material)
{
    return new ShapedRecipe(
        { BLOCK_AIR, material, BLOCK_AIR,
          BLOCK_AIR, (int)Item::Stick->GetId(), BLOCK_AIR,
          BLOCK_AIR, (int)Item::Stick->GetId(), BLOCK_AIR
        }, 3, 3, ItemStack{ (uint32_t)toolId, 1 });
}

Recipe* CreateHoeRecipe(int toolId, int material)
{
    return new ShapedRecipe(
        { material                 , material,
          (int)Item::Stick->GetId(), BLOCK_AIR,
          (int)Item::Stick->GetId(), BLOCK_AIR
        }, 2, 3, ItemStack{ (uint32_t)toolId, 1 });
}

Recipe* CreateSwordRecipe(int toolId, int material)
{
    return new ShapedRecipe(
        { material,
          material,
          (int)Item::Stick->GetId()
        }, 1, 3, ItemStack{ (uint32_t)toolId, 1 });
}

Recipe* CreateHelmetRecipe(int resultId, int material)
{
    return new ShapedRecipe(
        { material, material, material,
          material, BLOCK_AIR, material
        }, 3, 2, ItemStack{ (uint32_t)resultId, 1 });
}

Recipe* CreatePlateRecipe(int resultId, int material)
{
    return new ShapedRecipe(
        { material, BLOCK_AIR, material,
          material, material, material,
          material, material, material
        }, 3, 3, ItemStack{ (uint32_t)resultId, 1 });
}

Recipe* CreatePantsRecipe(int resultId, int material)
{
    return new ShapedRecipe(
        { material, material, material,
          material, BLOCK_AIR, material,
          material, BLOCK_AIR, material
        }, 3, 3, ItemStack{ (uint32_t)resultId, 1 });
}

Recipe* CreateBootsRecipe(int resultId, int material)
{
    return new ShapedRecipe(
        { material, BLOCK_AIR, material,
          material, BLOCK_AIR, material
        }, 3, 2, ItemStack{ (uint32_t)resultId, 1 });
}

Recipe* Recipe::GetMatchingRecipe(ItemStack* grid, int width, int height)
{
    if (Recipes.size() == 0)
    {
        // Planks
        Recipes.push_back(new IngredientRecipe({ BLOCK_LOG }, ItemStack{ BLOCK_PLANKS, 4 }));

        // Stick
        Recipes.push_back(new ShapedRecipe({ BLOCK_PLANKS, BLOCK_PLANKS }, 1, 2, ItemStack{ Item::Stick->GetId(), 4}));

        // Crafting Table
        Recipes.push_back(new ShapedRecipe(
            { BLOCK_PLANKS, BLOCK_PLANKS,
              BLOCK_PLANKS, BLOCK_PLANKS
            }, 2, 2, ItemStack{ BLOCK_CRAFTING_TABLE, 1}));

        // Torches
        Recipes.push_back(new ShapedRecipe(
            { (int)Item::Coal->GetId(),
              (int)Item::Stick->GetId()
            }, 1, 2, ItemStack{ BLOCK_TORCH, 4 }));

        // Paper, Book, Bookshelf

        Recipes.push_back(new ShapedRecipe(
            { (int)Item::Reed->GetId(), (int)Item::Reed->GetId(), (int)Item::Reed->GetId()
            }, 3, 1, ItemStack{ Item::Paper->GetId(), 3}));

        Recipes.push_back(new ShapedRecipe(
            { (int)Item::Paper->GetId(),
              (int)Item::Paper->GetId(),
              (int)Item::Paper->GetId()
            }, 1, 3, ItemStack{ Item::Book->GetId(), 1 }));

        Recipes.push_back(new ShapedRecipe(
            { 
                BLOCK_PLANKS            , BLOCK_PLANKS            , BLOCK_PLANKS,
                (int)Item::Book->GetId(), (int)Item::Book->GetId(), (int)Item::Book->GetId(),
                BLOCK_PLANKS            , BLOCK_PLANKS            , BLOCK_PLANKS
            }, 3, 3, ItemStack{ BLOCK_BOOKSHELF, 1 }));

        // Clay Block, Bricks

        Recipes.push_back(new ShapedRecipe(
            { (int)Item::ClayBalls->GetId(), (int)Item::ClayBalls->GetId(),
              (int)Item::ClayBalls->GetId(), (int)Item::ClayBalls->GetId(),
            }, 2, 2, ItemStack{ BLOCK_CLAY, 1 }));

        Recipes.push_back(new ShapedRecipe(
            { (int)Item::ClayBrick->GetId(), (int)Item::ClayBrick->GetId(),
              (int)Item::ClayBrick->GetId(), (int)Item::ClayBrick->GetId(),
            }, 2, 2, ItemStack{ BLOCK_BRICK, 1 }));

        // Jack 'O' Lantern

        Recipes.push_back(new IngredientRecipe(
            {
                BLOCK_PUMPKIN, BLOCK_TORCH
            }, ItemStack(BLOCK_JACK_O_LANTERN, 1)));

        // Tools
        Recipes.push_back(CreateSwordRecipe(Item::WoodSword->GetId(), BLOCK_PLANKS));
        Recipes.push_back(CreatePickaxeRecipe(Item::WoodPickaxe->GetId(), BLOCK_PLANKS));
        Recipes.push_back(CreateAxeRecipe(Item::WoodAxe->GetId(), BLOCK_PLANKS));
        Recipes.push_back(CreateShovelRecipe(Item::WoodShovel->GetId(), BLOCK_PLANKS));
        Recipes.push_back(CreateHoeRecipe(Item::WoodHoe->GetId(), BLOCK_PLANKS));

        Recipes.push_back(CreateSwordRecipe(Item::StoneSword->GetId(), BLOCK_COBBLESTONE));
        Recipes.push_back(CreatePickaxeRecipe(Item::StonePickaxe->GetId(), BLOCK_COBBLESTONE));
        Recipes.push_back(CreateAxeRecipe(Item::StoneAxe->GetId(), BLOCK_COBBLESTONE));
        Recipes.push_back(CreateShovelRecipe(Item::StoneShovel->GetId(), BLOCK_COBBLESTONE));
        Recipes.push_back(CreateHoeRecipe(Item::StoneHoe->GetId(), BLOCK_COBBLESTONE));

        Recipes.push_back(CreateSwordRecipe(Item::IronSword->GetId(), Item::IronIngot->GetId()));
        Recipes.push_back(CreatePickaxeRecipe(Item::IronPickaxe->GetId(), Item::IronIngot->GetId()));
        Recipes.push_back(CreateAxeRecipe(Item::IronAxe->GetId(), Item::IronIngot->GetId()));
        Recipes.push_back(CreateShovelRecipe(Item::IronShovel->GetId(), Item::IronIngot->GetId()));
        Recipes.push_back(CreateHoeRecipe(Item::IronHoe->GetId(), Item::IronIngot->GetId()));

        Recipes.push_back(CreateSwordRecipe(Item::GoldenSword->GetId(), Item::GoldIngot->GetId()));
        Recipes.push_back(CreatePickaxeRecipe(Item::GoldenPickaxe->GetId(), Item::GoldIngot->GetId()));
        Recipes.push_back(CreateAxeRecipe(Item::GoldenAxe->GetId(), Item::GoldIngot->GetId()));
        Recipes.push_back(CreateShovelRecipe(Item::GoldenShovel->GetId(), Item::GoldIngot->GetId()));
        Recipes.push_back(CreateHoeRecipe(Item::GoldHoe->GetId(), Item::GoldIngot->GetId()));

        Recipes.push_back(CreateSwordRecipe(Item::DiamondSword->GetId(), Item::Diamond->GetId()));
        Recipes.push_back(CreatePickaxeRecipe(Item::DiamondPickaxe->GetId(), Item::Diamond->GetId()));
        Recipes.push_back(CreateAxeRecipe(Item::DiamondAxe->GetId(), Item::Diamond->GetId()));
        Recipes.push_back(CreateShovelRecipe(Item::DiamondShovel->GetId(), Item::Diamond->GetId()));
        Recipes.push_back(CreateHoeRecipe(Item::DiamondHoe->GetId(), Item::Diamond->GetId()));

        // Armor

		Recipes.push_back(CreateHelmetRecipe(Item::LeatherHelmet->GetId(), Item::Leather->GetId()));
		Recipes.push_back(CreatePlateRecipe(Item::LeatherPlate->GetId(), Item::Leather->GetId()));
		Recipes.push_back(CreatePantsRecipe(Item::LeatherPants->GetId(), Item::Leather->GetId()));
		Recipes.push_back(CreateBootsRecipe(Item::LeatherBoots->GetId(), Item::Leather->GetId()));

        // TO DO: Add fire
		// Recipes.push_back(CreateHelmetRecipe(Item::ChainHelmet->GetId(), Item::IronIngot->GetId()));
		// Recipes.push_back(CreatePlateRecipe(Item::ChainPlate->GetId(), Item::IronIngot->GetId()));
		// Recipes.push_back(CreatePantsRecipe(Item::ChainPants->GetId(), Item::IronIngot->GetId()));
		// Recipes.push_back(CreateBootsRecipe(Item::ChainBoots->GetId(), Item::IronIngot->GetId()));

		Recipes.push_back(CreateHelmetRecipe(Item::IronHelmet->GetId(), Item::IronIngot->GetId()));
		Recipes.push_back(CreatePlateRecipe(Item::IronPlate->GetId(), Item::IronIngot->GetId()));
		Recipes.push_back(CreatePantsRecipe(Item::IronPants->GetId(), Item::IronIngot->GetId()));
		Recipes.push_back(CreateBootsRecipe(Item::IronBoots->GetId(), Item::IronIngot->GetId()));

		Recipes.push_back(CreateHelmetRecipe(Item::DiamondHelmet->GetId(), Item::Diamond->GetId()));
		Recipes.push_back(CreatePlateRecipe(Item::DiamondPlate->GetId(), Item::Diamond->GetId()));
		Recipes.push_back(CreatePantsRecipe(Item::DiamondPants->GetId(), Item::Diamond->GetId()));
		Recipes.push_back(CreateBootsRecipe(Item::DiamondBoots->GetId(), Item::Diamond->GetId()));

		Recipes.push_back(CreateHelmetRecipe(Item::GoldHelmet->GetId(), Item::GoldIngot->GetId()));
		Recipes.push_back(CreatePlateRecipe(Item::GoldPlate->GetId(), Item::GoldIngot->GetId()));
		Recipes.push_back(CreatePantsRecipe(Item::GoldPants->GetId(), Item::GoldIngot->GetId()));
		Recipes.push_back(CreateBootsRecipe(Item::GoldBoots->GetId(), Item::GoldIngot->GetId()));

        // Furnace
        Recipes.push_back(new ShapedRecipe(
            { BLOCK_COBBLESTONE, BLOCK_COBBLESTONE, BLOCK_COBBLESTONE,
              BLOCK_COBBLESTONE, BLOCK_AIR        , BLOCK_COBBLESTONE,
              BLOCK_COBBLESTONE, BLOCK_COBBLESTONE, BLOCK_COBBLESTONE
            }, 3, 3, ItemStack{ BLOCK_FURNACE, 1 }));

        // Chest
        Recipes.push_back(new ShapedRecipe(
            { BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS,
              BLOCK_PLANKS, BLOCK_AIR   , BLOCK_PLANKS,
              BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS
            }, 3, 3, ItemStack{ BLOCK_CHEST, 1 }));

        // Stairs
        Recipes.push_back(new ShapedRecipe(
            { BLOCK_AIR   , BLOCK_AIR   , BLOCK_PLANKS,
              BLOCK_AIR   , BLOCK_PLANKS, BLOCK_PLANKS,
              BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS
            }, 3, 3, ItemStack{ BLOCK_WOOD_STAIRS, 4 }));

        Recipes.push_back(new ShapedRecipe(
            { BLOCK_AIR        , BLOCK_AIR        , BLOCK_COBBLESTONE,
              BLOCK_AIR        , BLOCK_COBBLESTONE, BLOCK_COBBLESTONE,
              BLOCK_COBBLESTONE, BLOCK_COBBLESTONE, BLOCK_COBBLESTONE
            }, 3, 3, ItemStack{ BLOCK_COBBLESTONE_STAIRS, 4 }));

        // Slabs
        Recipes.push_back(new ShapedRecipe({ BLOCK_PLANKS     , BLOCK_PLANKS     , BLOCK_PLANKS }, 3, 1, ItemStack{ BLOCK_WOOD_SLAB, 6 }));
        Recipes.push_back(new ShapedRecipe({ BLOCK_COBBLESTONE, BLOCK_COBBLESTONE, BLOCK_COBBLESTONE }, 3, 1, ItemStack{ BLOCK_COBBLESTONE_SLAB, 6 }));
        Recipes.push_back(new ShapedRecipe({ BLOCK_STONE      , BLOCK_STONE      , BLOCK_STONE }, 3, 1, ItemStack{ BLOCK_STONE_SLAB, 6 }));

        // Bread
        Recipes.push_back(new ShapedRecipe({ (int)Item::Wheat->GetId(), (int)Item::Wheat->GetId(), (int)Item::Wheat->GetId() }, 3, 1, ItemStack{ Item::Bread->GetId(), 1 }));

        // Glowstone
        Recipes.push_back(new ShapedRecipe(
            { (int)Item::GlowstoneDust->GetId(), (int)Item::GlowstoneDust->GetId(), (int)Item::GlowstoneDust->GetId(),
              (int)Item::GlowstoneDust->GetId(), (int)Item::GlowstoneDust->GetId(), (int)Item::GlowstoneDust->GetId(),
              (int)Item::GlowstoneDust->GetId(), (int)Item::GlowstoneDust->GetId(), (int)Item::GlowstoneDust->GetId()
            }, 3, 3, ItemStack{ BLOCK_GLOWSTONE, 1 }));
        
        // Flint and Steel
        Recipes.push_back(new IngredientRecipe(
            { (int)Item::IronIngot->GetId(), (int)Item::Flint->GetId()
            }, ItemStack{ Item::FlintAndSteel->GetId(), 1 }));

        // Wooden door
        Recipes.push_back(new ShapedRecipe(
            { BLOCK_PLANKS, BLOCK_PLANKS,
              BLOCK_PLANKS, BLOCK_PLANKS,
              BLOCK_PLANKS, BLOCK_PLANKS
            }, 2, 3, ItemStack{ Item::WoodenDoor->GetId(), 1 }));

        // Bucket
        Recipes.push_back(new ShapedRecipe(
            { (int)Item::IronIngot->GetId(), BLOCK_AIR, (int)Item::IronIngot->GetId(),
              BLOCK_AIR, (int)Item::IronIngot->GetId(), BLOCK_AIR
            }, 3, 2, ItemStack{ Item::Bucket->GetId(), 1}));
    }

    for (int i = 0; i < Recipes.size(); i++)
    {
        if (Recipes[i]->Match(grid, width, height))
        {
            return Recipes[i];
        }
    }

    return nullptr;
}

IngredientRecipe::IngredientRecipe(const std::vector<int>& ids, ItemStack result)
{
    m_Result = result;
    m_MatchingIngredients = ids;
}

bool IngredientRecipe::Match(ItemStack* grid, int width, int height)
{
    int ingredientCount = 0;
    int count = width * height;

    for (int i = 0; i < count; i++)
    {
        if (grid[i].Id != 0)
        {
            ingredientCount++;
        }
    }
    for (int j = 0; j < m_MatchingIngredients.size(); j++)
    {
        bool foundIngredient = false;
        for (int i = 0; i < count; i++)
        {
            if (grid[i].Id == m_MatchingIngredients[j])
            {
                foundIngredient = true;
                break;
            }
        }
        if (!foundIngredient)
            return false;
    }

    return ingredientCount == m_MatchingIngredients.size();
}

ShapedRecipe::ShapedRecipe(const std::vector<int>& ids, int width, int height, ItemStack result)
{
    m_MatchingIngredients = ids;
    m_Width = width;
    m_Height = height;
    m_Result = result;
}

bool ShapedRecipe::Match(ItemStack* grid, int gridwidth, int gridheight)
{
    int count = m_Width * m_Height;
    if (count > gridwidth * gridheight)
        return false;
    // Slide the recipe over the grid
    for (int offsetY = 0; offsetY <= gridheight - m_Height; ++offsetY) {
        for (int offsetX = 0; offsetX <= gridwidth - m_Width; ++offsetX) {

            bool match = true;

            // Check recipe footprint
            for (int ry = 0; ry < m_Height && match; ++ry) {
                for (int rx = 0; rx < m_Width; ++rx) {
                    int recipeIndex = ry * m_Width + rx;
                    int recipeId = m_MatchingIngredients[recipeIndex];

                    int gridIndex = (offsetY + ry) * gridwidth + (offsetX + rx);
                    int gridId = grid[gridIndex].Id;

                    if (recipeId != gridId) {
                        match = false;
                        break;
                    }
                }
            }

            if (match) {
                // Ensure no extra items outside the recipe footprint
                for (int gy = 0; gy < gridheight && match; ++gy) {
                    for (int gx = 0; gx < gridwidth; ++gx) {
                        bool insideFootprint =
                            gx >= offsetX && gx < offsetX + m_Width &&
                            gy >= offsetY && gy < offsetY + m_Height;

                        if (!insideFootprint) {
                            int gridIndex = gy * gridwidth + gx;
                            if (grid[gridIndex].Id != 0) { // 0 = AIR
                                match = false;
                                break;
                            }
                        }
                    }
                }
            }

            if (match) {
                return true; // Found a valid match
            }
        }
    }

    return false; // No match found
}
