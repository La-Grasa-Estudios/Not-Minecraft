#pragma once

#include "ItemStack.h"

#include <vector>

class Recipe
{
public:

	static inline std::vector<Recipe*> Recipes;

	virtual bool Match(ItemStack* grid, int width, int height);
	ItemStack GetResult();

	static Recipe* GetMatchingRecipe(ItemStack* grid, int width, int height);

protected:
	ItemStack m_Result;
};

class IngredientRecipe : public Recipe
{
public:
	IngredientRecipe(const std::vector<int>& ids, ItemStack result);
	bool Match(ItemStack* grid, int width, int height);
private:
	std::vector<int> m_MatchingIngredients;
};

class ShapedRecipe : public Recipe
{
public:
	ShapedRecipe(const std::vector<int>& ids, int width, int height, ItemStack result);
	bool Match(ItemStack* grid, int width, int height);
private:
	std::vector<int> m_MatchingIngredients;
	int m_Width, m_Height;
};