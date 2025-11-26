#include "FootprintResolver.h"

// helper method to check if moving between tile, current and next is possible by checking if their path is blocked by a pinch
// this assumes current and next tile are adjacent to each other. if not, it will fail to determine direction and will return false
bool navigation::tile::Constraints::IsPinchBlocked(
	const component::tile::TileLayer& tilemap,
	const component::tile::Tileset& tileset,
	const component::tile::TileCoord& curr,
	const component::tile::TileCoord& next,
	const spatial::SizeF& tileSize,
	const navigation::tile::Footprint& startFP,
	const navigation::tile::Footprint& goalFP,
	const component::tile::TileCoord& startTC,
	const component::tile::TileCoord& goalTC
)
{
	// check if start is curr tile, goal is next tile
	bool currIsStartTile = (curr == startTC);
	bool nextIsGoalTile = (next == goalTC);

	// compute movement direction
	int dRow = next.row - curr.row;
	int dCol = next.col - curr.col;
	navigation::tile::Direction dir = navigation::tile::Direction::Down;
	if (dRow == -1 && dCol == 0)      dir = navigation::tile::Direction::Up;
	else if (dRow == 1 && dCol == 0)  dir = navigation::tile::Direction::Down;
	else if (dRow == 0 && dCol == -1) dir = navigation::tile::Direction::Left;
	else if (dRow == 0 && dCol == 1)  dir = navigation::tile::Direction::Right;
	else if (dRow == -1 && dCol == -1) dir = navigation::tile::Direction::UpLeft;
	else if (dRow == -1 && dCol == 1)  dir = navigation::tile::Direction::UpRight;
	else if (dRow == 1 && dCol == -1)  dir = navigation::tile::Direction::DownLeft;
	else if (dRow == 1 && dCol == 1)   dir = navigation::tile::Direction::DownRight;
	else
	{
		return false; // either current and next tile are the same, or they are not adjacent
	}

	// check if footprint is wider or taller than tile. 
	// if footprint is smaller than tile on both width and height, bail out now as it's guaranteed that movement from current to next will be successful
	bool isFootprintWiderThanTile = startFP.size.width > tileSize.width;
	bool isFootprintTallerThanTile = startFP.size.height > tileSize.height;
	if (!isFootprintWiderThanTile && !isFootprintTallerThanTile)
	{
		return true;
	}

	// if footprint is wider than tile, check if next tile is pinched by obstacles
	bool isNextRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row, next.col + 1);
	bool isNextLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row, next.col - 1);
	if (isFootprintWiderThanTile && !isNextRightAdjTileWalkable && !isNextLeftAdjTileWalkable)
	{
		return true;
	}

	// if footprint is taller than tile, check if path is pinched by obstacles on top and bottom of the path
	bool isNextBottomAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row + 1, next.col);
	bool isNextTopAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row - 1, next.col);
	if (isFootprintTallerThanTile && !isNextBottomAdjTileWalkable && !isNextTopAdjTileWalkable)
	{
		return true;
	}

	// this checks for pinch where obstacles are not in same column or row
	switch (dir)
	{
	case navigation::tile::Direction::Up:
	{
		if (!isFootprintWiderThanTile) break;

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		// 0N1
		// 1C0
		//  ^
		bool isCurrLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row, curr.col - 1);
		if (!isCurrLeftAdjTileWalkable && !isNextRightAdjTileWalkable)
		{
			return true;
		}

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		// 1N0
		// 0C1
		//  ^
		// is current tile start tile?
		bool isCurrRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row, curr.col + 1);
		if (!isCurrRightAdjTileWalkable && !isNextLeftAdjTileWalkable)
		{
			return true;
		}

		// if current tile is start tile, get the position of start footprint's bottom edge in tile space
		// if it's bottom position lies in current tile or tile above it, start/current tile is on side of the next position so it will not cross the pinch
		int startFootprintBottomTileRow;
		if (currIsStartTile)
		{
			math::geometry::RectF startBounds = startFP.GetRect();
			startFootprintBottomTileRow = static_cast<int>(std::floor((startBounds.bottom) / tileSize.height));
		}

		// if current tile IS NOT start tile, perform this check
		// if current tile IS start tile, and it lies on the side where it has to cross the pinch to get to next, perform this check
		if (!currIsStartTile || startFootprintBottomTileRow > curr.row)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 0N1
			// 0C0
			// 100
			//  ^
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextRightAdjTileValid = tilemap.IsValidTile(next.row, next.col + 1);
			bool isCurrBottomLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row + 1, curr.col - 1);
			if (!isCurrBottomLeftAdjTileWalkable && !isNextRightAdjTileWalkable && isNextRightAdjTileValid)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 1N0
			// 0C0
			// 001
			//  ^
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextLeftAdjTileValid = tilemap.IsValidTile(next.row, next.col - 1);
			bool isCurrBottomRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row + 1, curr.col + 1);
			if (!isCurrBottomRightAdjTileWalkable && !isNextLeftAdjTileWalkable && isNextLeftAdjTileValid)
			{
				return true;
			}
		}

		// if next tile is goal tile, get the position of goal footprint's bottom edge in tile space
		// if it's bottom position lies in current tile or tile above it, current tile is on side of the goal/next position so it will not cross the pinch
		int goalFootprintBottomTileRow;
		if (nextIsGoalTile)
		{
			math::geometry::RectF goalBounds = goalFP.GetRect();
			goalFootprintBottomTileRow = static_cast<int>(std::floor((goalBounds.bottom) / tileSize.height));
		}

		// if next tile IS NOT goal tile, perform this check
		// if next tile IS goal tile, and it lies on the side where current tile has to cross the pinch to get to goal, perform this check
		if (!nextIsGoalTile || goalFootprintBottomTileRow <= next.row)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 001
			// 0N0
			// 1C0
			//  ^
			bool isNextTopRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row - 1, next.col + 1);
			if (!isCurrLeftAdjTileWalkable && !isNextTopRightAdjTileWalkable)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 100
			// 0N0
			// 0C1
			//  ^
			bool isNextTopLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row - 1, next.col - 1);
			if (!isCurrRightAdjTileWalkable && !isNextTopLeftAdjTileWalkable)
			{
				return true;
			}
		}

		break;
	}
	case navigation::tile::Direction::Down:
	{
		if (!isFootprintWiderThanTile) break;

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		//  v
		// 1C0
		// 0N1
		bool isCurrLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row, curr.col - 1);
		if (!isCurrLeftAdjTileWalkable && !isNextRightAdjTileWalkable)
		{
			return true;
		}

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		//  v
		// 0C1
		// 1N0
		bool isCurrRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row, curr.col + 1);
		if (!isCurrRightAdjTileWalkable && !isNextLeftAdjTileWalkable)
		{
			return true;
		}

		// if current tile is start tile, get the position of start footprint's top edge in tile space
		// if it's top position lies in current tile or tile below it, start/current tile is on side of the next position so it will not cross the pinch
		int startFootprintTopTileRow;
		if (currIsStartTile)
		{
			math::geometry::RectF startBounds = startFP.GetRect();
			startFootprintTopTileRow = static_cast<int>(std::floor((startBounds.top) / tileSize.height));
		}

		// if current tile IS NOT start tile, perform this check
		// if current tile IS start tile, and it lies on the side where it has to cross the pinch to get to next, perform this check
		if (!currIsStartTile || startFootprintTopTileRow < curr.row)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  v
			// 001
			// 0C0
			// 1N0
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextLeftAdjTileValid = tilemap.IsValidTile(next.row, next.col - 1);
			bool isCurrTopRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row - 1, curr.col + 1);
			if (!isCurrTopRightAdjTileWalkable && !isNextLeftAdjTileWalkable && isNextLeftAdjTileValid)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  v
			// 100
			// 0C0
			// 0N1
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextRightAdjTileValid = tilemap.IsValidTile(next.row, next.col + 1);
			bool isCurrTopLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row - 1, curr.col - 1);
			if (!isCurrTopLeftAdjTileWalkable && !isNextRightAdjTileWalkable && isNextRightAdjTileValid)
			{
				return true;
			}
		}

		// if next tile is goal tile, get the position of goal footprint's bottom edge in tile space
		// if it's bottom position lies in current tile or tile above it, current tile is on side of the goal/next position so it will not cross the pinch
		int goalFootprintTopTileRow;
		if (nextIsGoalTile)
		{
			math::geometry::RectF goalBounds = goalFP.GetRect();
			goalFootprintTopTileRow = static_cast<int>(std::floor((goalBounds.top) / tileSize.height));
		}

		// if next tile IS NOT goal tile, perform this check
		// if next tile IS goal tile, and it lies on the side where current tile has to cross the pinch to get to goal, perform this check
		if (!nextIsGoalTile || goalFootprintTopTileRow >= next.row)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  v
			// 0C1
			// 0N0
			// 100
			bool isNextBottomLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row + 1, next.col - 1);
			if (!isCurrRightAdjTileWalkable && !isNextBottomLeftAdjTileWalkable)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  v
			// 1C0
			// 0N0
			// 001
			bool isNextBottomRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row + 1, next.col + 1);
			if (!isCurrLeftAdjTileWalkable && !isNextBottomRightAdjTileWalkable)
			{
				return true;
			}
		}

		break;
	}
	case navigation::tile::Direction::Right:
	{
		if (!isFootprintTallerThanTile) break;

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		//  10
		// >CN
		//  01		
		bool isCurrTopAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row - 1, curr.col);
		if (!isCurrTopAdjTileWalkable && !isNextBottomAdjTileWalkable)
		{
			return true;
		}

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		//  01
		// >CN
		//  10		
		bool isCurrBottomAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row + 1, curr.col);
		if (!isCurrBottomAdjTileWalkable && !isNextTopAdjTileWalkable)
		{
			return true;
		}

		// if current tile is start tile, get the position of start footprint's left edge in tile space
		// if it's left position lies in current tile or tile right of it, start/current tile is on side of the next position so it will not cross the pinch
		int startFootprintLeftTileCol;
		if (currIsStartTile)
		{
			math::geometry::RectF startBounds = startFP.GetRect();
			startFootprintLeftTileCol = static_cast<int>(std::floor((startBounds.left) / tileSize.width));
		}

		// if current tile IS NOT start tile, perform this check
		// if current tile IS start tile, and it lies on the side where it has to cross the pinch to get to next, perform this check
		if (!currIsStartTile || startFootprintLeftTileCol < curr.col)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  100
			// >0CN
			//  001							
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextBottomAdjTileValid = tilemap.IsValidTile(next.row + 1, next.col);
			bool isCurrTopLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row - 1, curr.col - 1);
			if (!isCurrTopLeftAdjTileWalkable && !isNextBottomAdjTileWalkable && isNextBottomAdjTileValid)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  001
			// >0CN
			//  100							
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextTopAdjTileValid = tilemap.IsValidTile(next.row - 1, next.col);
			bool isCurrBottomLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row - 1, curr.col + 1);
			if (!isCurrBottomLeftAdjTileWalkable && !isNextTopAdjTileWalkable && isNextTopAdjTileValid)
			{
				return true;
			}
		}

		// if next tile is goal tile, get the position of goal footprint's left edge in tile space
		// if it's left position lies on any tile to the left of current tile, current tile is on side of the goal/next position so it will not cross the pinch
		int goalFootprintLefTileCol;
		if (nextIsGoalTile)
		{
			math::geometry::RectF goalBounds = goalFP.GetRect();
			goalFootprintLefTileCol = static_cast<int>(std::floor((goalBounds.left) / tileSize.width));
		}

		// if next tile IS NOT goal tile, perform this check
		// if next tile IS goal tile, and it lies on the side where current tile has to cross the pinch to get to goal, perform this check
		if (!nextIsGoalTile || goalFootprintLefTileCol >= next.col)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  100
			// >CN0
			//  001							
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextBottomRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row + 1, next.col + 1);
			if (!isCurrTopAdjTileWalkable && !isNextBottomRightAdjTileWalkable)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			//  001
			// >CN0
			//  100							
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextTopRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row - 1, next.col + 1);
			if (!isCurrBottomAdjTileWalkable && !isNextTopRightAdjTileWalkable)
			{
				return true;
			}
		}


		break;
	}
	case navigation::tile::Direction::Left:
	{
		if (!isFootprintTallerThanTile) break;

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		// 01
		// NC<
		// 10		
		bool isCurrTopAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row - 1, curr.col);
		if (!isCurrTopAdjTileWalkable && !isNextBottomAdjTileWalkable)
		{
			return true;
		}

		// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
		// 10
		// NC<
		// 01		
		bool isCurrBottomAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row + 1, curr.col);
		if (!isCurrBottomAdjTileWalkable && !isNextTopAdjTileWalkable)
		{
			return true;
		}

		// if current tile is start tile, get the position of start footprint's right edge in tile space
		// if it's right position lies in current tile or tile left of it, start/current tile is on side of the next position so it will not cross the pinch
		int startFootprintRightTileCol;
		if (currIsStartTile)
		{
			math::geometry::RectF startBounds = startFP.GetRect();
			startFootprintRightTileCol = static_cast<int>(std::floor((startBounds.right) / tileSize.width));
		}

		// if current tile IS NOT start tile, perform this check
		// if current tile IS start tile, and it lies on the side where it has to cross the pinch to get to next, perform this check
		if (!currIsStartTile || startFootprintRightTileCol > curr.col)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 001
			// NC0<
			// 100							
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextBottomAdjTileValid = tilemap.IsValidTile(next.row + 1, next.col);
			bool isCurrTopRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row - 1, curr.col + 1);
			if (!isCurrTopRightAdjTileWalkable && !isNextBottomAdjTileWalkable && isNextBottomAdjTileValid)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 100
			// NC0<
			// 001		
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextTopAdjTileValid = tilemap.IsValidTile(next.row - 1, next.col);
			bool isCurrBottomRightAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, curr.row + 1, curr.col + 1);
			if (!isCurrBottomRightAdjTileWalkable && !isNextTopAdjTileWalkable && isNextTopAdjTileValid)
			{
				return true;
			}
		}

		// if next tile is goal tile, get the position of goal footprint's right edge in tile space
		// if it's right position lies on any tile from current tile or to the right of it, current tile is on side of the goal/next position so it will not cross the pinch
		int goalFootprintRightTileCol;
		if (nextIsGoalTile)
		{
			math::geometry::RectF goalBounds = goalFP.GetRect();
			goalFootprintRightTileCol = static_cast<int>(std::floor((goalBounds.right) / tileSize.width));
		}

		// if next tile IS NOT goal tile, perform this check
		// if next tile IS goal tile, and it lies on the side where current tile has to cross the pinch to get to goal, perform this check
		if (!nextIsGoalTile || goalFootprintRightTileCol <= next.col)
		{
			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 001
			// 0NC<
			// 100							
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextBottomLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row + 1, next.col - 1);
			if (!isCurrTopAdjTileWalkable && !isNextBottomLeftAdjTileWalkable)
			{
				return true;
			}

			// if tile pattern is like below, direction going up, and footprint size > tile size, we can't pass
			// 100
			// 0NC<
			// 001		
			// NOTE: handle edge case where curr and next tiles are at edge of map. in this case, it is not pinched.
			bool isNextTopLeftAdjTileWalkable = component::tile::IsWalkable(tilemap, tileset, next.row - 1, next.col - 1);
			if (!isCurrBottomAdjTileWalkable && !isNextTopLeftAdjTileWalkable)
			{
				return true;
			}
		}

		break;
	}
	default:
		break;
	}

	return false;
}




// Construct a resolver with explicit parameters.
navigation::tile::FootprintResolver::FootprintResolver(
	std::function<bool(int, int)> isWalkable,						// Predicate to test walkability of a tile
	float epsilon,													// Tolerance for floating-point edge cases
	float maxHorizontalNudge,										// Maximum horizontal distance allowed for nudging
	float maxVerticalNudge,											// Maximum vertical distance allowed for nudging
	bool allowAnchorOverlap,										// Whether anchors may overlap blocked tiles
	CostStrategy costStrategy										// cost calculation strategy
) :
	m_epsilon(epsilon),
	m_maxHorizontalNudge(maxHorizontalNudge),
	m_maxVerticalNudge(maxVerticalNudge),
	m_allowAnchorOverlap(allowAnchorOverlap),
	m_isWalkable(isWalkable),
	m_currCostStrategyFunc(nullptr)
{
	// assign the cost strategy function 
	switch (costStrategy)
	{
	case CostStrategy::NormalizedEuclidianSquared:
		m_currCostStrategyFunc = &FootprintResolver::CostStrategy_NormalizedEuclidianSquared;
		break;
	case CostStrategy::EuclidianSquared:
		m_currCostStrategyFunc = &FootprintResolver::CostStrategy_EuclidianSquared;
		break;
	default:
		break;
	}
}

// Returns the axis-aligned rectangle representing the footprint in world space.
math::geometry::RectF navigation::tile::Footprint::GetRect() const
{
	switch (anchor)
	{
	case Anchor::Center:
		return {
			position.x - size.width / static_cast<float>(2),
			position.y - size.height / static_cast<float>(2),
			position.x + size.width / static_cast<float>(2),
			position.y + size.height / static_cast<float>(2)
		};
	case Anchor::TopLeft:
		return {
			position.x,
			position.y,
			position.x + size.width,
			position.y + size.height
		};
	case Anchor::TopRight:
		return {
			position.x - size.width,
			position.y,
			position.x,
			position.y + size.height
		};
	case Anchor::BottomLeft:
		return {
			position.x,
			position.y - size.height,
			position.x + size.width,
			position.y
		};
	case Anchor::BottomRight:
		return {
			position.x - size.width,
			position.y - size.height,
			position.x,
			position.y
		};
	default:
		return {
			position.x,
			position.y,
			position.x + size.width,
			position.y + size.height
		};
	}
}
		

// checks if a footprint is valid without resolution.
bool navigation::tile::FootprintResolver::IsValid(
	const component::tile::TileLayer& tileLayer,			// Tile layer to operate on
	const spatial::SizeF& tileSize,                         // Size of each tile in world units
	const Footprint& footPrint						// Footprint bounds (position, size, anchor)
) const
{
	// get the footprint rect based on anchor which is assumed to be in world space (tilemap coordinate)
	math::geometry::RectF footPrintBounds = footPrint.GetRect();

	// calculate the tile coordinates covered by the footprint
	// right and bottom uses std::ceil and subtract -1 to follow half open method. this ensures footprint that are exact tile size covers only single tile
	int left = static_cast<int>(std::floor(footPrintBounds.left / tileSize.width));
	int top = static_cast<int>(std::floor(footPrintBounds.top / tileSize.height));
	int right = static_cast<int>(std::ceil(footPrintBounds.right / tileSize.width)) - 1;
	int bottom = static_cast<int>(std::ceil(footPrintBounds.bottom / tileSize.height)) - 1;

	// iterate through all tiles covered by the footprint
	for (int row = top; row <= bottom; ++row)
	{
		for (int col = left; col <= right; ++col)
		{
			component::tile::TileCoord tileCoord{ row, col };

			// Check if the tile is within the bounds of the tile layer
			if (!tileLayer.IsValidTile(row, col) || !m_isWalkable(row, col))
			{
				return false;
			}
		}
	}

	return true;
}

// attempts to resolve placement of a footprint.
bool navigation::tile::FootprintResolver::TryResolve(
	const component::tile::TileLayer& tileLayer,
	const spatial::SizeF& tileSize,
	const Footprint& footPrint,
	Footprint& outFootPrint) const
{
	// quick check if current position is already safe
	if (IsValid(tileLayer, tileSize, footPrint))
	{
		outFootPrint = footPrint;
		return true; // already safe
	}

	// get the footprint rect based on anchor which is assumed to be in world space (tilemap coordinate)
	math::geometry::RectF footPrintBounds = footPrint.GetRect();

	// quick reject: if anchor position of footprint is inside an unwalkable tile, then no point trying to nudge
	if (!m_allowAnchorOverlap)
	{
		component::tile::TileCoord anchorTileCoord
		{
			static_cast<int>(std::floor(footPrint.position.y / tileSize.height)),
			static_cast<int>(std::floor(footPrint.position.x / tileSize.width))
		};

		if (!tileLayer.IsValidTile(anchorTileCoord) || !m_isWalkable(anchorTileCoord.row, anchorTileCoord.col))
		{
			return false; // center position is inside an unwalkable tile
		}
	}

	// get the top-left and bottom-right tile coords of the footprint at current position
	component::tile::TileCoord topLeftTileCoord
	{
		static_cast<int>(std::floor(footPrintBounds.top / tileSize.height)),
		static_cast<int>(std::floor(footPrintBounds.left / tileSize.width))
	};

	component::tile::TileCoord bottomRightTileCoord
	{
		static_cast<int>(std::floor(footPrintBounds.bottom / tileSize.height)),
		static_cast<int>(std::floor(footPrintBounds.right / tileSize.width))
	};

	// lambda wrapper for calling the current cost strategy 
	auto costOf = [this, &footPrint](const Footprint& candidate) -> float
		{
			return (this->*m_currCostStrategyFunc)(footPrint, candidate);
		};

	// used to compare candidate nudged footprint if this is the best
	float bestCost = std::numeric_limits<float>::infinity();
	auto consider = [&](const Footprint& candidate, Footprint& current)
		{
			float candidateCost = costOf(candidate);
			if (candidateCost < bestCost)
			{
				bestCost = candidateCost;
				current = candidate;
				return true;
			}
			return false;
		};


	// attempt to nudge along each edge of the footprint
	Footprint bestFootprint;
	float nudgeY = 0;
	float nudgeX = 0;
	{
		nudgeY = 0;
		// DEBUG: TODO: 
		// max nudge is not being respected. i think what happens is that the latest calculated nudge can overlap max nudge
		// and wh
		for (int step = 0; nudgeY < m_maxVerticalNudge; step++)
		{
			// check footprint's top edge if safe to land. if not, nudge downwards incrementally outwards snapping at the tile edges
			{
				// calculate the nudge distance to align with bottom edge of the tile above. the + 1 is because we want the tile below. + epsilon to avoid being exactly on the edge
				nudgeY = (topLeftTileCoord.row + 1 + step) * tileSize.height - footPrintBounds.top + m_epsilon;

				// apply the nudge
				Footprint nudgedFootprint = footPrint;
				nudgedFootprint.position.y += nudgeY;

				// check if the nudged position is safe
				if (IsValid(tileLayer, tileSize, nudgedFootprint))
				{
					consider(nudgedFootprint, bestFootprint);
					break;
				}
			}

			// check footprint's bottom edge if safe to land. if not, nudge upwards incrementally outwards snapping at the tile edges
			{
				// calculate the nudge distance to align with top edge of the tile below. the + epsilon is to avoid being exactly on the edge
				nudgeY = footPrintBounds.bottom - (bottomRightTileCoord.row - step) * tileSize.height + m_epsilon;

				// apply the nudge
				Footprint nudgedFootprint = footPrint;
				nudgedFootprint.position.y -= nudgeY;

				// check if the nudged position is safe
				if (IsValid(tileLayer, tileSize, nudgedFootprint))
				{
					consider(nudgedFootprint, bestFootprint);
					break;
				}
			}
		}

		nudgeX = 0;
		for (int step = 0; nudgeX < m_maxHorizontalNudge; step++)
		{
			// check footprint's right edge if safe to land. if not, nudge leftwards incrementally outwards snapping at the tile edges
			{
				// calculate the nudge distance to align with left edge of the tile to the left. the + epsilon is to avoid being exactly on the edge
				nudgeX = footPrintBounds.right - (bottomRightTileCoord.col - step) * tileSize.width + m_epsilon;

				// apply the nudge
				Footprint nudgedFootprint = footPrint;
				nudgedFootprint.position.x -= nudgeX;

				// check if the nudged position is safe
				if (IsValid(tileLayer, tileSize, nudgedFootprint))
				{
					consider(nudgedFootprint, bestFootprint);
					break;
				}
			}

			// check footprint's left edge if safe to land. if not, nudge rightwards incrementally outwards snapping at the tile edges
			{
				// calculate the nudge distance to align with right edge of the tile to the right. the + epsilon is to avoid being exactly on the edge
				nudgeX = (topLeftTileCoord.col + 1 + step) * tileSize.width - footPrintBounds.left + m_epsilon;

				// apply the nudge
				Footprint nudgedFootprint = footPrint;
				nudgedFootprint.position.x += nudgeX;

				// check if the nudged position is safe
				if (IsValid(tileLayer, tileSize, nudgedFootprint))
				{
					consider(nudgedFootprint, bestFootprint);
					break;
				}
			}
		}
	}

	// handle corner cases
	{
		// attempt to nudge the position of footprint at the top-left corner tile
		{
			// starting at tile where top-left of footprint is, nudge rightwards incrementally outwards snapping at the tile edges
			nudgeX = 0;
			for (int col = 0; nudgeX < m_maxHorizontalNudge; col++)
			{
				// start at col + 1 because col = 0 is the original position. we already know it's unsafe
				nudgeX = (topLeftTileCoord.col + col + 1) * tileSize.width - footPrintBounds.left + m_epsilon;

				// while at current tile column where top-left of footprint is, nudge downwards incrementally outwards snapping at the tile edges
				nudgeY = 0;
				for (int row = 0; nudgeY < m_maxVerticalNudge; row++)
				{
					// start at row + 1 because row = 0 is the original position. we already know it's unsafe
					nudgeY = (topLeftTileCoord.row + row + 1) * tileSize.height - footPrintBounds.top + m_epsilon;

					// nudge footprint based on current tile column and row and see if safe to land
					Footprint nudgedFootprint = footPrint;
					nudgedFootprint.position.x += nudgeX;
					nudgedFootprint.position.y += nudgeY;

					// check if the nudged position is safe
					if (IsValid(tileLayer, tileSize, nudgedFootprint))
					{
						consider(nudgedFootprint, bestFootprint);
					}
				}
			}
		}

		// attempt to nudge the position of footprint at the bottom-right corner tile
		{
			// starting at tile where bottom-right of footprint is, nudge leftwards incrementally outwards snapping at the tile edges
			nudgeX = 0;
			for (int col = 0; nudgeX < m_maxHorizontalNudge; col++)
			{
				// calculate the nudge distance to align with left edge of the tile to the left. the + epsilon is to avoid being exactly on the edge
				nudgeX = footPrintBounds.right - (bottomRightTileCoord.col - col) * tileSize.width + m_epsilon;

				// while at current tile column where bottom-right of footprint is, nudge upwards incrementally outwards snapping at the tile edges
				nudgeY = 0;
				for (int row = 0; nudgeY < m_maxVerticalNudge; row++)
				{
					nudgeY = footPrintBounds.bottom - (bottomRightTileCoord.row - row) * tileSize.height + m_epsilon;

					// nudge footprint based on current tile column and row and see if safe to land
					Footprint nudgedFootprint = footPrint;
					nudgedFootprint.position.x -= nudgeX;
					nudgedFootprint.position.y -= nudgeY;

					// check if the nudged position is safe
					if (IsValid(tileLayer, tileSize, nudgedFootprint))
					{
						consider(nudgedFootprint, bestFootprint);
					}
				}
			}
		}

		// attempt to nudge the position of footprint at the top-right corner tile
		{
			nudgeX = 0;
			for (int col = 0; nudgeX < m_maxHorizontalNudge; col++)
			{
				// calculate the nudge distance to align with left edge of the tile to the left. the + epsilon is to avoid being exactly on the edge
				nudgeX = footPrintBounds.right - (bottomRightTileCoord.col - col) * tileSize.width + m_epsilon;

				// while at current tile column where top-right of footprint is, nudge downwards incrementally outwards snapping at the tile edges
				nudgeY = 0;
				for (int row = 0; nudgeY < m_maxVerticalNudge; row++)
				{
					nudgeY = (topLeftTileCoord.row + 1 + row) * tileSize.height - footPrintBounds.top + m_epsilon;

					// nudge footprint based on current tile column and row and see if safe to land
					Footprint nudgedFootprint = footPrint;
					nudgedFootprint.position.x -= nudgeX;
					nudgedFootprint.position.y += nudgeY;

					// check if the nudged position is safe
					if (IsValid(tileLayer, tileSize, nudgedFootprint))
					{
						consider(nudgedFootprint, bestFootprint);
					}
				}
			}
		}

		// attempt to nudge the position of footprint at the bottom-left corner tile
		{
			// starting at tile where bottom-left of footprint is, nudge rightwards incrementally outwards snapping at the tile edges
			nudgeX = 0;
			for (int col = 0; nudgeX < m_maxHorizontalNudge; col++)
			{
				// start at col + 1 because col = 0 is the original position. we already know it's unsafe
				nudgeX = (topLeftTileCoord.col + col + 1) * tileSize.width - footPrintBounds.left + m_epsilon;

				// while at current tile column where bottom-left of footprint is, nudge upwards incrementally outwards snapping at the tile edges
				nudgeY = 0;
				for (int row = 0; nudgeY < m_maxVerticalNudge; row++)
				{
					nudgeY = footPrintBounds.bottom - (bottomRightTileCoord.row - row) * tileSize.height + m_epsilon;

					// nudge footprint based on current tile column and row and see if safe to land
					Footprint nudgedFootprint = footPrint;
					nudgedFootprint.position.x += nudgeX;
					nudgedFootprint.position.y -= nudgeY;

					// check if the nudged position is safe
					if (IsValid(tileLayer, tileSize, nudgedFootprint))
					{
						consider(nudgedFootprint, bestFootprint);
					}
				}
			}
		}
	}

	if (bestCost < std::numeric_limits<float>::infinity())
	{
		outFootPrint = bestFootprint;
		return true;
	}
	else
	{
		return false;
	}
}