#pragma once
#include "Tile.h"
#include "Rect.h"
#include <optional>

namespace navigation
{
	namespace tile
	{
		enum class Direction
		{
			Up,
			Down,
			Left,
			Right,
			UpLeft,
			UpRight,
			DownLeft,
			DownRight
		};

		struct Constraints
		{
			// helper method to check if moving between tile, current and next is possible by checking if their path is blocked by a pinch
			// this assumes current and next tile are adjacent to each other. if not, it will fail to determine direction and will return false
			static bool IsPinchBlocked(
				const component::tile::TileLayer& tilemap,
				const component::tile::Tileset& tileset,
				const component::tile::TileCoord& curr,
				const component::tile::TileCoord& next,
				const spatial::SizeF& footprintSize,
				const spatial::SizeF& tileSize);
		};
	};
};

namespace navigation
{
	namespace tile
	{
		// Defines the reference point for a footprint's position.
		// Used to interpret the footprint's RectF relative to its position.
		enum class Anchor
		{
			Center,       // Position is the geometric center of the footprint
			TopLeft,      // Position is the top-left corner
			TopRight,     // Position is the top-right corner
			BottomLeft,   // Position is the bottom-left corner
			BottomRight   // Position is the bottom-right corner
		};

		// Represents the occupied area (footprint) of an actor on a tilemap.
		// Holds position, size, and anchor, and can produce a Rect<T> for collision/clearance checks.
		struct Footprint
		{
			spatial::PosF position;				// Anchor position in world space
			spatial::SizeF size;				// Width/height of the footprint
			Anchor anchor = Anchor::Center;     // Anchor point for position interpretation

			// Returns the axis-aligned rectangle representing the footprint in world space.
			math::geometry::RectF GetRect() const;
		};

		// Resolves footprint placement on a tile layer.
		// Performs validation (walkability, clearance) and resolution (nudging).
		class FootprintResolver
		{
		public:
			// types of how to compute cost of moving (nudging)
			enum class CostStrategy
			{
				EuclidianSquared,
				NormalizedEuclidianSquared,
				GeometricDistance,	// not implemented yet
				Composite			// not implemented yet
			};

			// Construct a resolver with explicit parameters.
			FootprintResolver(
				std::function<bool(int, int)> isWalkable,								// Predicate to test walkability of a tile
				float epsilon = 0.01f,													// Tolerance for floating-point edge cases
				float maxHorizontalNudge = 1.0f,										// Maximum horizontal distance allowed for nudging
				float maxVerticalNudge = 1.0f,											// Maximum vertical distance allowed for nudging
				bool allowAnchorOverlap = false,										// Whether anchors may overlap blocked tiles
				CostStrategy costStrategy = CostStrategy::NormalizedEuclidianSquared	// cost calculation strategy
			);

			// Checks if a footprint is valid without resolution.
			bool IsValid(
				const component::tile::TileLayer& tileLayer,			// Tile layer to operate on
				const spatial::SizeF& tileSize,                         // Size of each tile in world units
				const Footprint& footPrint						// Footprint bounds (position, size, anchor)
			) const;


			// Attempts to resolve placement of a footprint.
			bool TryResolve(
				const component::tile::TileLayer& tileLayer,
				const spatial::SizeF& tileSize,
				const Footprint& footPrint,
				Footprint& outFootPrint) const;

		private:
			// aliasing the cost strategy function signature. this function is to calculate the cost between original footprint position and candidate 
			using costStrategyFunc = float (FootprintResolver::*)(const Footprint& original, const Footprint& candidate) const;

			// NOTE: 
			// the epsilon is set to 0.01 by default. this might be too small for large footprints, and too big for small footprints
			// but in most cases, this is good enough. it is up to you to tune it to the right value depending on the size of your footprints
			float m_epsilon;

			// NOTE:
			// large footprints may cover large number of tiles. this could be a performance issue. handle with care and be aware of the size of the 
			// footprint and tile
			float m_maxHorizontalNudge;
			float m_maxVerticalNudge;
			bool m_allowAnchorOverlap;
			std::function<bool(int, int)> m_isWalkable;
			costStrategyFunc m_currCostStrategyFunc;

			// strategy to calculate cost. calculate the squared distance between position of original and candidate footprint
			float CostStrategy_EuclidianSquared(const Footprint& original, const Footprint& candidate) const
			{
				const float dx = candidate.position.x - original.position.x;
				const float dy = candidate.position.y - original.position.y;
				return (dx * dx + dy * dy);
			}

			// strategy to calculate cost. calculate the squared normalized distance between position of original and candidate footprint
			// this considers the aspect ratio of the footprint. for cases where aspect ratio is high e.g. width = 100, height = 10, this
			// uses the weight of the width and height in calculating the cost
			// halfwidth = original.size.width / 2
			// halfheight = original.size.height / 2
			// dx = (original.pos.x - candidate.pos.x) / halfwidth
			// dy = (original.pos.x - candidate.pos.y) / halfheight
			// cost = dx * dx + dy + dy
			float CostStrategy_NormalizedEuclidianSquared(const Footprint& original, const Footprint& candidate) const
			{
				const float halfWidth = original.size.width / 2;
				const float halfHeight = original.size.height / 2;

				const float dx = (candidate.position.x - original.position.x) / halfWidth;
				const float dy = (candidate.position.y - original.position.y) / halfHeight;

				return (dx * dx + dy * dy);
			}
		};
	}
}

