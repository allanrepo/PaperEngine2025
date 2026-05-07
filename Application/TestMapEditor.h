#pragma once
#include <IO/CSVFileParser.h>
#include <Containers/Bucket.h>
#include <Win32/Window.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Timer/StopWatch.h>
#include <Spatial/Coord.h>
#include <Spatial/Position.h>
#include <Core/Event.h>
#include <Engine/Factory/CanvasFactory.h>
#include <Engine/Factory/RendererFactory.h>
#include <Core/Input.h>
#include <Engine/Factory/FontFactory.h>
#include <Graphics/Resource/IFontAtlas.h>
#include <Components/Tile.h>
#include <unordered_map>
#include <Containers/Dictionary.h>
#include <Algorithm/AutoTileResolver.h>
#include <Algorithm/Resolvers.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Graphics/Core/IRenderable.h>
#include <Graphics/Core/IAnimated.h>
#include <Graphics/Core/Renderable.h>
#include <Graphics/Core/Animated.h>
#include <Spatial/Size.h>
#include <Command/DrawCommand.h>
#include <Graphics/Core/Color.h>
#include <Utilities/CSVParser.h>
#include <IO/ASyncFileReader.h>
#include <Containers/Table.h>
#include <Utilities/Utilities.h>
#include <Graphics/Animation/Animation.h>
#include <Engine/Factory/AnimationFactory.h>
#include <memory>
#include <functional>
#include <Spatial/ObjectGrid.h>
#include <Algorithm/Pathfinding.h>
#include <Core/View.h>

namespace TestMapEditor
{
#pragma region // forward declaration
	class Test;
	struct PropPlacementContext;
	class PropPlacementSystem;
	class PropMap;
	class WorldMap;
#pragma endregion

#pragma region // namespaces
	using namespace engine;
	using IWindow = win32::Window;
	using Window = win32::Window;
	using ICanvas = engine::graphics::ICanvas;
	using IRenderer = engine::graphics::renderer::IRenderer;
	using StopWatch = timer::StopWatch;
	using PositionF = spatial::PositionF;
	using SizeF = spatial::SizeF;
	using Coord = spatial::Coord;
	using CanvasFactory = engine::graphics::CanvasFactory;
	using RendererFactory = engine::graphics::factory::RendererFactory;
	using Input = engine::input::Input;
	using FontFactory = engine::graphics::factory::FontFactory;
	using IFontAtlas = engine::graphics::resource::IFontAtlas;
	using IRenderable = engine::graphics::IRenderable;
	using AutoTileResolver = engine::tile::AutoTileResolver;
	using TileVariant = engine::tile::TileVariant;
	using SpriteAtlasFactory = engine::graphics::factory::SpriteAtlasFactory;
	using ISpriteAtlas = engine::graphics::resource::ISpriteAtlas;
	using IRenderable = engine::graphics::IRenderable;
	using Animated = engine::graphics::Animated;
	using Renderable = engine::graphics::Renderable;
	using DrawSortedSpritesCommand = engine::command::graphics::renderer::DrawSortedSpritesCommand;
	using DrawQuadCommand = engine::command::graphics::renderer::DrawQuadCommand;
	using Sprite = engine::graphics::Sprite;
	using ColorF = engine::graphics::ColorF;
	using AsyncFileReader = engine::io::AsyncFileReader;
	using CSVParser = engine::utilities::parser::CSVParser;
	using OnOutOfScope = engine::utilities::OnOutOfScope;
	using AnimationFactory = engine::graphics::factory::AnimationFactory;
	using VecF = engine::math::VecF;
	using InputEvent = engine::input::InputEvent;
	using TileConstraint = engine::navigation::tile::TileConstraint;
	using RectF = engine::math::geometry::RectF;
	using CSVFileParser = engine::io::CSVFileParser;
	using NavigationGrid = engine::navigation::tile::NavigationGrid;

	template<typename T>
	using View = engine::core::View<T>;

	template<typename T>
	using Grid = engine::container::Grid<T>;

	template<typename T>
	using Bucket = engine::container::Bucket<T>;

	template<typename T>
	using BucketGrid = engine::container::BucketGrid<T>;

	template<typename K, typename T>
	using ObjectGrid = engine::spatial::ObjectGrid<K, T>;

	template<typename T>
	using Animator = engine::graphics::animation::Animator<T>;

	template<typename T>
	using AnimationSet = engine::graphics::animation::AnimationSet<T>;

	template<typename T>
	using AnimationSystemCache = engine::graphics::animation::AnimationSystemCache<T>;

	template<typename T>
	using Table = engine::container::Table<T>;

	template<typename Owner>
	using AnimationController = engine::graphics::animation::AnimationController<Sprite, Owner>;

	template<typename T, typename K, typename V>
	using LookupResolver = engine::algorithm::LookupResolver<T, K, V>;

	//template<typename T>
	//using StateMachine = engine::state::StateMachine<T>;

	template<typename T>
	using Registry = engine::cache::Registry<T>;

	template<typename T>
	using TileGrid = engine::tile::TileGrid<T>;

	template<typename T>
	using Tileset = engine::tile::Tileset<T>;	

	template<typename T>
	using Tile = engine::tile::Tile<T>;	

	template<typename K, typename T>
	using Dictionary = engine::container::Dictionary<K, T>;

#pragma endregion

 #pragma region // AutoTileSystem
	// Design consideration
	// - passing AutoTileContext AutoTileConfig makes the class as a versatile tool, not tied to any specific object. it can be easily reused
	// - AutoTileContext contains only lambda to allow accessing the data (map) that needs to be resolved without directly knowing the types of the data
	// - AutoTileConfig is a separate data to pass and not part of AutoTileContext because it defines the rules on how data (map) is resolved
	// - AutoTileContext is “where/how do I operate?” while AutoTileConfig is “what rules do I follow?”
	// - we define AutoTileConfig directly from map object (TileGrid, Tileset) but AutoTileConfig is not always tied into it.
	// - it is possible for multiple map object to share AutoTileConfig, meaning it is a stand alone data
	class AutoTileSystem
	{
	public:
		struct AutoTileConfig
		{
			engine::container::Dictionary<int, TileVariant> indexToVariant;
			engine::container::Dictionary<TileVariant, int> variantToIndex;

			void Register(int index, TileVariant variant)
			{
				indexToVariant[index] = variant;
				variantToIndex[variant] = index;
			}

			bool HasIndex(int index) const
			{
				return indexToVariant.Has(index);
			}

			bool HasVariant(TileVariant variant) const
			{
				return variantToIndex.Has(variant);
			}

			TileVariant ToVariant(int index) const
			{
				return indexToVariant[index];
			}

			int ToIndex(TileVariant variant) const
			{
				return variantToIndex[variant];
			}
		};
		
		struct AutoTileContext
		{
			std::function<bool(const Coord&)> isInBounds;
			std::function<int(const Coord&)> getIndex;
			std::function<void(const Coord&, int, TileVariant)> applyTile;
		};

	private:

		TileVariant ResolveTileVariant(int mask)
		{
			switch (mask)
			{
			case 0:   return TileVariant::Island;		// surrounded by nothing
			case 15:  return TileVariant::Full;			// surrounded by same tile type on all 4 sides

			case 8:   return TileVariant::NorthEdge;	// same tile type on north only. nothing on south, east, west
			case 2:   return TileVariant::SouthEdge;	// same tile type on south only. nothing on north, east, west
			case 1:   return TileVariant::EastEdge;		// same tile type on east only. nothing on north, south, west
			case 4:   return TileVariant::WestEdge;		// same tile type on west only. nothing on north, south, east

			case 10:  return TileVariant::Vertical;		// same tile type on north+south. nothing on east, west
			case 5:   return TileVariant::Horizontal;	// same tile type on east+west. nothing on north, south

			case 7:   return TileVariant::TNorth;		// same tile type on south+east+west. nothing on north
			case 13:  return TileVariant::TSouth;		// same tile type on north+east+west. nothing on south
			case 14:  return TileVariant::TEast;		// same tile type on north+south+west. nothing on east	
			case 11:  return TileVariant::TWest;		// same tile type on north+south+east. nothing on west

			case 6: return TileVariant::NECorner;		// same tile type on north+east. nothing on south, west 
			case 3: return TileVariant::NWCorner;		// same tile type on north+west. nothing on south, east
			case 12: return TileVariant::SECorner;		// same tile type on south+east. nothing on north, west
			case 9: return TileVariant::SWCorner;		// same tile type on south+west. nothing on north, east

			default:  return TileVariant::Empty;		// default to empty tile if mask configuration not found. this should not happen if we cover all cases.
			}
		}

		void UpdateMask(
			AutoTileContext& ctx, 
			const AutoTileConfig& autoTileConfig, 
			const engine::spatial::Coord& coord, 
			unsigned int& mask, 
			unsigned int bit
		)
		{
			// these are neighbor tiles. we only check main tile for bounds, so we check this neighbor here
			if (!ctx.isInBounds(coord)) return;

			// get index. we know it's a valid coord by now
			int index = ctx.getIndex(coord);

			if (autoTileConfig.HasIndex(index) && autoTileConfig.ToVariant(index) != TileVariant::Empty)
			{
				mask |= bit;
			}
		}

		unsigned int ComputeMask(AutoTileContext& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord)
		{
			unsigned int mask = 0;

			UpdateMask(ctx, autoTileConfig, { coord.row - 1, coord.col }, mask, 8);	// N
			UpdateMask(ctx, autoTileConfig, { coord.row + 1, coord.col }, mask, 2);	// S
			UpdateMask(ctx, autoTileConfig, { coord.row, coord.col + 1 }, mask, 4);	// E
			UpdateMask(ctx, autoTileConfig, { coord.row, coord.col - 1 }, mask, 1);	// W

			return mask;
		}

		void PlaceTile(AutoTileContext& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord, const TileVariant type)
		{
			// Set the selected tile
			ctx.applyTile(coord, autoTileConfig.ToIndex(type), type);
		}

		void ResolveNeighbors(AutoTileContext& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord)
		{
			// Update self + 4 neighbors (skip diagonals)
			for (int dr = -1; dr <= 1; ++dr)
			{
				for (int dc = -1; dc <= 1; ++dc)
				{
					// Skip corners (diagonals)
					if (std::abs(dr) + std::abs(dc) > 1) continue;

					// neighbor tile coords
					engine::spatial::Coord neighborCoord = { coord.row + dr, coord.col + dc };

					// if neighbor tile is out of bounds or not walkable, skip it.
					if (!ctx.isInBounds(neighborCoord)) continue;

					// if this is the tile we just placed, so we already know its new variant. skip it since we don't need to recompute it.
					if (dr == 0 && dc == 0) continue;

					// if tile exists but is empty tile, skip it since empty tile is like "air" and doesn't affect autotiling of neighbors
					int index = ctx.getIndex(neighborCoord);
					if (autoTileConfig.HasIndex(index) && autoTileConfig.ToVariant(index) == TileVariant::Empty) continue;

					// evaluate this neighbor if this it of same tile type as the one we just placed. if not, skip it since its tile variant won't be affected by the new tile.
					if (!autoTileConfig.HasIndex(index)) continue;

					unsigned int mask = ComputeMask(ctx, autoTileConfig, neighborCoord);
					TileVariant variant = ResolveTileVariant(mask);

					TileVariant currVariant = autoTileConfig.ToVariant(index);
					if (currVariant != variant)
					{
						// Set the selected tile
						PlaceTile(ctx, autoTileConfig, neighborCoord, variant);
					}
				}
			}
		}

	public:
		AutoTileSystem(
		)
		{
		}

		AutoTileSystem(const AutoTileSystem&) = delete;
		AutoTileSystem& operator=(const AutoTileSystem&) = delete;
		AutoTileSystem(AutoTileSystem&&) = delete;
		AutoTileSystem& operator=(AutoTileSystem&&) = delete;

		virtual ~AutoTileSystem()
		{
		}

		void Set(AutoTileContext& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord, bool force = false)
		{
			// quick check if coord is within bounds
			if (!ctx.isInBounds(coord)) return;

			// evaluates cardinal neighboors of this coord to check if they are set as tiles
			unsigned int mask = ComputeMask(ctx, autoTileConfig, coord);

			// decide which variant of tile this coord is going to be
			TileVariant variant = ResolveTileVariant(mask);

			if (!force)
			{
				// get the variant of the current tile in the coord
				int currIndex = ctx.getIndex(coord);

				// if the current index is invalid, let's just place the new tile
				if (!autoTileConfig.HasIndex(currIndex))
				{
					force = true;
				}
				else
				{
					// if current variant is same from replacement variant, don't bother setting it.
					TileVariant currVariant = autoTileConfig.ToVariant(currIndex);
					force = (currVariant != variant);
				}
			}

			// if after evaluating, we don't need to apply the new tile, bail out
			if (!force) return;

			PlaceTile(ctx, autoTileConfig, coord, variant);
			ResolveNeighbors(ctx, autoTileConfig, coord);
		}

		void Remove(AutoTileContext& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord, bool force = false)
		{
			// quick check if coord is within bounds
			if (!ctx.isInBounds(coord)) return;

			// if we don't have an empty tile registered, we can't remove. just return early.
			if (!autoTileConfig.HasVariant(TileVariant::Empty)) return;

			if (!force)
			{
				// get the variant of the current tile in the coord
				int currIndex = ctx.getIndex(coord);

				// if the current index is invalid, let's just remove the current tile
				if (!autoTileConfig.HasIndex(currIndex))
				{
					force = true;
				}
				else
				{
					TileVariant currVariant = autoTileConfig.ToVariant(currIndex);
					force = (currVariant != TileVariant::Empty);
				}
			}

			if (!force) return;

			PlaceTile(ctx, autoTileConfig, coord, TileVariant::Empty);
			ResolveNeighbors(ctx, autoTileConfig, coord);
		}

		void Set(AutoTileContext& ctx, const AutoTileConfig& autoTileConfig, engine::spatial::Size<size_t> size)
		{
			for (int row = 0; row < size.height; row++)
			{
				for (int col = 0; col < size.width; col++)
				{
					Set(ctx, autoTileConfig, engine::spatial::Coord(row, col));
				}
			}
		}

		void Remove(AutoTileContext& ctx, const AutoTileConfig& autoTileConfig, engine::spatial::Size<size_t> size)
		{
			for (int row = 0; row < size.height; row++)
			{
				for (int col = 0; col < size.width; col++)
				{
					Remove(ctx, autoTileConfig, Coord(row, col));
				}
			}
		}
	};
#pragma endregion

#pragma region // GridQuery
	class GridQuery
	{
	public:
		// the "aabb" or boundingbox already implies overlap. the cellsize and gridsize implies we're querying a grid/map
		static std::vector<Coord> QueryCells(
			const RectF& aabb,
			const SizeF& cellsize,
			const Size<size_t> gridsize)
		{
			// ------------------------------------------------------------
			// 1. Normalize AABB (safety against flipped rectangles)
			// ------------------------------------------------------------
			const float left = std::min<float>(aabb.left, aabb.right);
			const float right = std::max<float>(aabb.left, aabb.right);
			const float top = std::min<float>(aabb.top, aabb.bottom);
			const float bottom = std::max<float>(aabb.top, aabb.bottom);

			// ------------------------------------------------------------
			// 2. Convert world bounds -> cell coordinates
			// ------------------------------------------------------------
			Coord minCell = PositionToCoord({ left, top }, cellsize);
			Coord maxCell = PositionToCoord({ right, bottom }, cellsize);

			// ------------------------------------------------------------
			// 3. Normalize cell ordering
			// ------------------------------------------------------------
			int startRow = std::min<int>(minCell.row, maxCell.row);
			int endRow = std::max<int>(minCell.row, maxCell.row);

			int startCol = std::min<int>(minCell.col, maxCell.col);
			int endCol = std::max<int>(minCell.col, maxCell.col);

			// ------------------------------------------------------------
			// 4. Clamp to grid bounds (avoid negative / overflow access)
			// ------------------------------------------------------------
			startRow = std::max<int>(0, startRow);
			startCol = std::max<int>(0, startCol);

			endRow = std::min<int>(static_cast<int>(gridsize.height) - 1, endRow);
			endCol = std::min<int>(static_cast<int>(gridsize.width) - 1, endCol);

			// ------------------------------------------------------------
			// 5. Early exit if no overlap
			// ------------------------------------------------------------
			if (startRow > endRow || startCol > endCol)
				return {};

			// ------------------------------------------------------------
			// 6. Collect cells
			// ------------------------------------------------------------
			std::vector<Coord> result;
			result.reserve((endRow - startRow + 1) * (endCol - startCol + 1));

			for (int row = startRow; row <= endRow; ++row)
			{
				for (int col = startCol; col <= endCol; ++col)
				{
					result.push_back({ row, col });
				}
			}

			return result;
		}
	};
#pragma endregion

#pragma region // MultiOccupantGrid
	template<typename T, typename DATA>
	struct TileOccupant
	{
		T* object = nullptr;

		DATA data;
	};

	//container should NOT decide validity
	//container should NOT evict
	//container should NOT understand overlap
	//container should NOT understand tile constraints
	//container should NOT understand subcells
	//container should NOT normalize gameplay semantics
	template<typename T, typename DATA>
	class MultiOccupantGrid
	{
	private:
		// object -> occupied cells
		Dictionary<T*, std::vector<Coord>> m_objects;

		// cell -> occupants
		Grid<std::vector<TileOccupant<T, DATA>>> m_grid;

	public:
		MultiOccupantGrid() = default;
		~MultiOccupantGrid() = default;

		void Initialize(size_t width, size_t height)
		{
			m_grid.Initialize(
				width,
				height,
				std::vector<TileOccupant<T, DATA>>());

			m_objects.Clear();
		}

		void Initialize(Size<size_t> size)
		{
			Initialize(size.width, size.height);
		}

		Size<size_t> GetSize() const
		{
			return m_grid.GetSize();
		}

		size_t GetObjectCount() const
		{
			return m_objects.Size();
		}

		// ------------------------------------------------------------------------
		// Add
		//
		// design:
		// - container does NOT evaluate DATA
		// - container does NOT resolve overlap
		// - container does NOT evict
		// - container only stores associations
		// - same object cannot exist twice in same tile
		// ------------------------------------------------------------------------
		bool Add(
			T* object,
			const Coord& cell,
			const DATA& data)
		{
			if (!object)
			{
				throw std::runtime_error(
					"MultiOccupancyGrid::Add() - null object");
			}

			if (!m_grid.IsInBounds(cell))
			{
				return false;
			}

			auto& bucket = m_grid.Get(cell);

			// object already exists in this tile?
			auto it = std::find_if(
				bucket.begin(),
				bucket.end(),
				[&](const TileOccupant<T, DATA>& occupant)
				{
					return occupant.object == object;
				});

			// strict. the caller must be responsible to remove this object if it already exists in this cell
			if (it != bucket.end())
			{
				throw std::runtime_error(
					"MultiOccupancyGrid::Add() - object already exists in tile");
			}

			// store occupant
			bucket.push_back({
				object,
				data
				});

			// remember object occupancy
			if (!m_objects.Has(object))
			{
				m_objects.Set(object, {});
			}

			auto& occupiedCells = m_objects.Get(object);

			// enforce unique coords
			auto coordIt = std::find(
				occupiedCells.begin(),
				occupiedCells.end(),
				cell);

			if (coordIt == occupiedCells.end())
			{
				occupiedCells.push_back(cell);
			}

			return true;
		}

		// ------------------------------------------------------------------------
		// Remove object from specific cell
		// ------------------------------------------------------------------------
		bool Remove(
			T* object,
			const Coord& cell)
		{
			if (!object)
			{
				throw std::runtime_error(
					"MultiOccupancyGrid::Remove() - null object");
			}

			if (!m_grid.IsInBounds(cell))
			{
				return false;
			}

			auto& bucket = m_grid.Get(cell);

			auto it = std::remove_if(
				bucket.begin(),
				bucket.end(),
				[&](const TileOccupant<T, DATA>& occupant)
				{
					return occupant.object == object;
				});

			if (it == bucket.end())
			{
				return false;
			}

			auto removedCount =
				std::distance(it, bucket.end());

			if (removedCount != 1)
			{
				throw std::runtime_error(
					"MultiOccupancyGrid::Remove() - duplicate occupants detected");
			}

			bucket.erase(it, bucket.end());

			// update reverse lookup
			if (!m_objects.Has(object))
			{
				throw std::runtime_error(
					"MultiOccupancyGrid::Remove() - object missing from m_objects");
			}

			auto& occupiedCells = m_objects.Get(object);

			auto coordIt = std::remove(
				occupiedCells.begin(),
				occupiedCells.end(),
				cell);

			if (coordIt == occupiedCells.end())
			{
				throw std::runtime_error(
					"MultiOccupancyGrid::Remove() - cell missing from object mapping");
			}

			occupiedCells.erase(coordIt, occupiedCells.end());

			// cleanup empty object entry
			if (occupiedCells.empty())
			{
				if (!m_objects.Unregister(object))
				{
					throw std::runtime_error(
						"MultiOccupancyGrid::Remove() - failed to unregister object");
				}
			}

			return true;
		}

		// ------------------------------------------------------------------------
		// Remove object from all occupied cells
		// ------------------------------------------------------------------------
		void Remove(T* object)
		{
			if (!m_objects.Has(object))
			{
				throw std::runtime_error(
					"MultiOccupancyGrid::Remove(T*) - object not found");
			}

			// copy because Remove(object, cell)
			// mutates m_objects
			auto cells = m_objects.Get(object);

			for (const auto& cell : cells)
			{
				if (!Remove(object, cell))
				{
					throw std::runtime_error(
						"MultiOccupancyGrid::Remove(T*) - failed removing object from cell");
				}
			}
		}

		bool Has(T* object) const
		{
			return m_objects.Has(object);
		}

		// ------------------------------------------------------------------------
		// Get occupants in cell
		// ------------------------------------------------------------------------
		const std::vector<TileOccupant<T, DATA>>& Get(const Coord& cell) const
		{
			if (!m_grid.IsInBounds(cell))
			{
				throw std::runtime_error("MultiOccupancyGrid::Get() - invalid cell");
			}

			return m_grid.Get(cell);
		}

		// ------------------------------------------------------------------------
		// Get data of a given object in a given cell
		// ------------------------------------------------------------------------
		const DATA& Get(T* object, const Coord& cell) const
		{
			if (!m_grid.IsInBounds(cell))
			{
				throw std::runtime_error("MultiOccupancyGrid::Get() - invalid cell");
			}

			// get all occupants in this cell
			const std::vector<TileOccupant<T, DATA>>& occupants = Get(cell);

			// find the occupant that is our object
			for (const TileOccupant<T, DATA>& occupant : occupants)
			{
				if (occupant.object == object)
				{
					return occupant.data;
				}
			}
			// we're not sure if this method requires specified cell guarantees object exist. but for now, let's be strict and make it so to avoid silent failures
			throw std::exception("the specified object does not exist in the given coord");
		}

		// ------------------------------------------------------------------------
		// Get occupied cells of object
		// ------------------------------------------------------------------------
		std::vector<Coord> GetOccupiedCells(T* object) const
		{
			if (!m_objects.Has(object))
			{
				return {};
			}

			return m_objects.Get(object);
		}

		// ------------------------------------------------------------------------
		// Validation
		// ------------------------------------------------------------------------
		void Validate() const
		{
			// ------------------------------------------------------------
			// OBJECT -> GRID
			// ------------------------------------------------------------
			for (const auto& [object, cells] : m_objects)
			{
				if (!object)
				{
					throw std::runtime_error(
						"Validate() - null object");
				}

				for (const auto& cell : cells)
				{
					if (!m_grid.IsInBounds(cell))
					{
						throw std::runtime_error(
							"Validate() - object has invalid cell");
					}

					const auto& bucket = m_grid.Get(cell);

					auto it = std::find_if(
						bucket.begin(),
						bucket.end(),
						[&](const TileOccupant<T, DATA>& occupant)
						{
							return occupant.object == object;
						});

					if (it == bucket.end())
					{
						throw std::runtime_error(
							"Validate() - object missing from tile");
					}
				}
			}

			// ------------------------------------------------------------
			// GRID -> OBJECT
			// ------------------------------------------------------------
			m_grid.ForEach(
				[&](
					size_t row,
					size_t col,
					const std::vector<TileOccupant<T, DATA>>& bucket)
				{
					Coord cell
					{
						(int)row,
						(int)col
					};

					for (const auto& occupant : bucket)
					{
						if (!occupant.object)
						{
							throw std::runtime_error(
								"Validate() - null occupant");
						}

						if (!m_objects.Has(occupant.object))
						{
							throw std::runtime_error(
								"Validate() - occupant missing from object map");
						}

						const auto& cells =
							m_objects.Get(occupant.object);

						auto it = std::find(
							cells.begin(),
							cells.end(),
							cell);

						if (it == cells.end())
						{
							throw std::runtime_error(
								"Validate() - tile missing from object mapping");
						}
					}
				});
		}
	};

#pragma endregion

#pragma region // MultiOccupancyGrid
	// design considerations:
	// - cells to be occupied are filtered to be unique so duplicates are normalized
	// - 
	template<typename T>
	class MultiOccupancyGrid
	{
	private:
		Dictionary<T*, std::vector<Coord>> m_objects;
		Grid<std::vector<T*>> m_grid;

	public:
		MultiOccupancyGrid() = default;
		~MultiOccupancyGrid() = default;

		void Initialize(size_t width, size_t height)
		{
			m_grid.Initialize(width, height, std::vector<T*>());
			m_objects.Clear();
		}

		void Initialize(Size<size_t> size)
		{
			Initialize(size.width, size.height);
		}

		Size<size_t> GetSize() const
		{
			return m_grid.GetSize();
		}

		size_t GetObjectCount() const
		{
			return m_objects.Size();
		}

		// design consideration:
		// - validate cells first. make sure is in bounds and has no duplicates. bail out if no valid cells
		// - if object to occupy already exist in this grid, vacate it first. this ensures occupants are unique. 
		// - this is like "moving" the object from old to new location
		// - update each cell to contain this object
		bool Add(T* object, const std::vector<Coord>& cells)
		{
			// -------------------------------------------------------------------------------
			// 1. VALIDATE CELLS. MAKE SURE WE HAVE VALID CELL TO OCCUPY BEFORE MUTATING
			// -------------------------------------------------------------------------------
			// validate cells first. if all cells are invalid, we won't add this object to the grid and we won't store it in m_objects since it is not really occupying any cell in the grid.
			std::vector<Coord> validCells;
			std::unordered_set<Coord> uniqueCells;
			for (const auto& cell : cells)
			{
				// we can have invalid cells in the list of cells to occupy. we will just skip those invalid cells and only occupy valid cells.
				// in case selected area in grid is occupied by object is partially out of bounds, we will just occupy the valid portion 
				// of the area and ignore the out of bounds portion.
				if (!m_grid.IsInBounds(cell)) continue;

				// remember valid cells 
				// note that we're also storing the cell in a set to ensure we avoid having duplicate cells in our valid cells.
				// this is to handle scenario where cells contain duplicate coords e.g.  (1,1), (1,1), (2,2), (2,2)
				if (uniqueCells.insert(cell).second)
				{
					validCells.push_back(cell);
				}
			}			 

			// if no valid cells, object cannot be added. bail out
			if (validCells.empty()) return false;

			// -------------------------------------------------------------------------------
			// 2. REMOVE THIS OBJECT IF ALREADY EXIST TO ENSURE OCCUPANTS ARE UNIQUE
			// -------------------------------------------------------------------------------			
			// let's enforce design rule where objects are unique in this grid. if we are adding an object that already exists, 
			// we will treat this as updating the cells occupied by this object. so we will remove previous footprint of this object 
			// and add new footprint of this object.
			if (m_objects.Has(object))
			{
				Vacate(object);
			}

			// -------------------------------------------------------------------------------
			// 3. OBJECT TO OCCUPY VALID CELLS IN GRID
			// -------------------------------------------------------------------------------		
			// write into grid. since cells are valid, it guarantees the new object will occupy these cells
			for (const auto& cell : validCells)
			{
				// since this is Add() method, we allow multiple objects to occupy the same cell. 
				// but we don't want to have duplicate entry of the same object in the same cell, so we check if this object already exist 
				// in the cell before we add it. if it already exists, we throw error because this is likely a bug from caller side. 
				auto& bucket = m_grid.Get(cell);
				if (std::find(bucket.begin(), bucket.end(), object) != bucket.end())
				{
					throw std::runtime_error("Add(T*, const std::vector<Coord>&) - duplicate object in cell");
				}
				bucket.push_back(object);
			}

			// object occupies these cells. no need to check if valid cells are empty since we already check in the beginning
			return m_objects.Set(object, validCells);
		}

		// design consideration:
		// difference from Add() - this removes existing objects that overlaps this new object
		// - validate cells first. make sure is in bounds and has no duplicates. bail out if no valid cells. 
		// - identify existing objects that overlaps this new object
		// 
		// - if object to occupy already exist in this grid, vacate it first. this ensures occupants are unique. 
		// - this is like "moving" the object from old to new location
		// - update each cell to contain this object
		bool Occupy(T* object, const std::vector<Coord>& cells)
		{
			// -------------------------------------------------------------------------------
			// 1. VALIDATE CELLS. IDENTIFY EXISTING OBJECTS THAT OVERLAPS. 
			// -------------------------------------------------------------------------------
			// let's validate first before mutating our containers
			std::vector<Coord> validCells;
			std::unordered_set<T*> toEvict;
			std::unordered_set<Coord> uniqueCells;
			for (const auto& cell : cells)
			{
				// we can have invalid cells in the list of cells to occupy. we will just skip those invalid cells and only occupy valid cells.
				// in case selected area in grid is occupied by object is partially out of bounds, we will just occupy the valid portion 
				// of the area and ignore the out of bounds portion.
				if (!m_grid.IsInBounds(cell)) continue;

				// is there existing object in this cell? if yes, we queue it for eviction. even if the object found is same as object that is 
				// trying to occupy, we queue it. this is like "moving" the object from old location to new location. 
				// so we are vacating the object from old position, then later we will add it back into new location
				for (T* existing : m_grid.Get(cell))
				{
					toEvict.insert(existing);
				}

				// remember valid cells 
				// note that we're also storing the cell in a set to ensure we avoid having duplicate cells in our valid cells.
				// this is to handle scenario where cells contain duplicate coords e.g.  (1,1), (1,1), (2,2), (2,2)
				if (uniqueCells.insert(cell).second)
				{
					validCells.push_back(cell);
				}
			}

			// if there are no valid cells to occupy, then this object cannot occupy. bail out
			if (validCells.empty()) return false;

			// -------------------------------------------------------------------------------
			// 2. REMOVE EXISTING OBJECTS THAT OVERLAPS NEW OBJECT. 
			// -------------------------------------------------------------------------------
			// evict objects (except the one that is occupying) found in cells we 're trying to occupy. 
			// we choose to vacate rather than throw error because if there is really discrepancy between m_objects and m_grid, 
			// Vacate() will likely throw error anyways
			for (T* obj : toEvict)
			{
				Vacate(obj);
			}

			// -------------------------------------------------------------------------------
			// 3. REMOVE THIS OBJECT IF ALREADY EXIST TO ENSURE OCCUPANTS ARE UNIQUE
			// -------------------------------------------------------------------------------
			// let's enforce design rule where objects are unique in this grid. if we are adding an object that already exists, 
			// we will treat this as updating the cells occupied by this object. so we will remove previous footprint of this object 
			// and add new footprint of this object.
			// note that if this object's current location is overlapped by the new location it is trying to occupy, then it is already 
			// vacated since it will be in toEvict list. but in case it is not, we vacate it here.
			// regardless, we are doing it safely by checking first if it exist before vacating. 
			if (m_objects.Has(object))
			{
				Vacate(object);
			}

			// -------------------------------------------------------------------------------
			// 4. OBJECT TO OCCUPY VALID CELLS IN GRID
			// -------------------------------------------------------------------------------	
			// new object occupies this cell. we also defer this because if we do this first then evict existing objects after, 
			// we will end up vacating the new object that we just set in the grid since it occupies the same cell as existing objects.
			for (const auto& cell : validCells)
			{
				auto& bucket = m_grid.Get(cell);
				if (!bucket.empty())
				{
					throw std::runtime_error("Occupy(T*, const std::vector<Coord>&) - cell not empty after eviction");
				}

				bucket.push_back(object);
			}

			// object occupies these cells. no need to check if valid cells are empty since we already check in the beginning
			return m_objects.Set(object, validCells);
		}

		void Vacate(T* object)
		{
			// let's be strict here. this method expects the object exist. 
			if (!m_objects.Has(object))
			{
				throw std::runtime_error("MultiOccupancyGrid::Vacate(T*) - object to remove not found");
			}

			// get the cells occupied by this object. we know this object exist in m_objects, so it must have a valid set of cells.
			const auto& cells = m_objects.Get(object);

			// remove this object from all cells it occupies in the grid. since we allow multiple objects to occupy the same cell,
			// we need to find and remove this object from the list of objects in each cell it occupies.
			for (const auto& cell : cells)
			{
				if (!m_grid.IsInBounds(cell))
				{
					throw std::runtime_error("MultiOccupancyGrid::Vacate(T*) - invalid cell");
				}

				// get all the objects that occupy this cell
				auto& bucket = m_grid.Get(cell);

				// shift non-matching elements forward and returns an iterator to the new logical end.
				auto it = std::remove(bucket.begin(), bucket.end(), object);
				if (it != bucket.end()) 
				{
					// checks how many elements were removed by calculating the distance between the new logical end and the actual end of the bucket.
					auto removedCount = std::distance(it, bucket.end());

					// since we expect only one instance of this object in the bucket, we can be strict and check if removedCount is exactly 1. 
					// if not, it means there is a data inconsistency between m_objects and m_grid.
					if (removedCount != 1)
					{
						throw std::runtime_error("MultiOccupancyGrid::Vacate(T*) - found " + std::to_string(removedCount) + " instances of object. potential data inconsistency between m_objects and m_grid");
					}

					// found one match
					bucket.erase(it, bucket.end());
				}
				else 
				{
					// let's be strict here. if we can't find this object in the cell that it's supposed to occupy,
					// it means there is a data inconsistency between m_objects and m_grid.
					throw std::runtime_error("MultiOccupancyGrid::Vacate(T*) - failed to find object in the cell it occupies. potential data inconsistency between m_objects and m_grid");
				}
			}

			// be strict here. we already vacated cells for this object in grid, if we fail to unregister this object from m_objects, 
			// it means there is a data inconsistency between m_objects and m_grid.
			if (!m_objects.Unregister(object))
			{
				throw std::runtime_error("OccupancyGrid::Vacate(T*) - failed to unregister object from m_objects");
			}
		}

		bool Has(T* object) const
		{
			return m_objects.Has(object);
		}

		// get objects that occupies these cells
		std::vector<T*> Get(const std::vector<Coord>& cells) const
		{
			std::vector<T*> result;

			for (const auto& cell : cells)
			{
				// skip invalid coord
				if (!m_grid.IsInBounds(cell)) continue;

				// get the bucket of objects in this cell. these are the objects that occupy this cell. 
				// since we allow multiple objects to occupy the same cell, we need to iterate through this bucket 
				// and add all unique objects to our result.
				const auto& bucket = m_grid.Get(cell);
				for (T* obj : bucket)
				{
					if (std::find(result.begin(), result.end(), obj) == result.end())
					{
						result.push_back(obj);
					}
				}
			}

			// returns list of unique objects found in cells given
			return result;
		}

		// get objects that occupies this cell
		std::vector<T*> Get(const Coord& cell) const
		{
			if (m_grid.IsInBounds(cell))
			{
				return m_grid.Get(cell);
			}
			return std::vector<T*>();
		}

		// get cell coords that are occupied by this object
		std::vector<Coord> GetOccupiedTiles(T* object) const
		{
			std::vector<Coord> result;

			if (!m_objects.Has(object))
			{
				return result;
			}

			return m_objects.Get(object);
		}

		void Validate() const
		{
			// 1. OBJECT -> GRID CHECK
			for (const auto& [obj, cells] : m_objects)
			{
				if (!obj)
				{
					throw std::runtime_error("Validate: null object in m_objects");
				}

				for (const auto& cell : cells)
				{
					if (!m_grid.IsInBounds(cell))
					{
						throw std::runtime_error("Validate: object has out-of-bounds cell");
					}

					const auto& bucket = m_grid.Get(cell);

					auto it = std::find(bucket.begin(), bucket.end(), obj);
					if (it == bucket.end())
					{
						throw std::runtime_error("Validate: object missing from grid cell it owns");
					}
				}
			}

			// 2. GRID -> OBJECT CHECK
			m_grid.ForEach([&](size_t row, size_t col, const std::vector<T*>& bucket)
				{
					Coord cell{ (int)row, (int)col };

					for (T* obj : bucket)
					{
						if (!obj)
						{
							throw std::runtime_error("Validate: null object in grid cell");
						}

						if (!m_objects.Has(obj))
						{
							throw std::runtime_error("Validate: grid contains object not in m_objects");
						}

						const auto& cells = m_objects.Get(obj);

						auto it = std::find(cells.begin(), cells.end(), cell);
						if (it == cells.end())
						{
							throw std::runtime_error("Validate: grid cell not listed in object mapping");
						}
					}
				});
		}
	};
#pragma endregion

#pragma region // OccupancyGrid
	template<typename T>
	class OccupancyGrid
	{
	private:
		Dictionary<T*, std::vector<Coord>> m_objects;
		Grid<T*> m_grid;

	public:
		OccupancyGrid() = default;
		~OccupancyGrid() = default;

		void Initialize(size_t width, size_t height)
		{
			m_grid.Initialize(width, height, nullptr);
			m_objects.Clear();
		}

		void Initialize(Size<size_t> size)
		{
			Initialize(size.width, size.height);
		}

		Size<size_t> GetSize() const
		{
			return m_grid.GetSize();
		}

		// this method forces an object to occupy the cells given. if any of the cells given are already occupied by another object, 
		// the existing object will be removed from those cells and replaced by this new object.
		void Occupy(T* object, const std::vector<Coord>& cells)
		{
			// if this object already exists, remove previous footprint of this object
			// DESIGN NOTE:
			// we're not enforcing gameplay rules here. we do this to maintain bidirectional mapping between m_objects and m_grid
			// when m_objects set a new set of cells for an object, if that object already exist, then its previous cells will remain
			// in the m_grid. that breaks mapping between m_grid and m_object. 
			if (m_objects.Has(object))
			{
				Vacate(object);
			}

			// set object in grid with new object. 
			std::vector<Coord> cellsToOccupy;
			std::unordered_set<T*> toEvict;
			for (const auto& cell : cells)
			{
				// we can have invalid cells in the list of cells to occupy. we will just skip those invalid cells and only occupy valid cells.
				// in case selected area in grid is occupied by object is partially out of bounds, we will just occupy the valid portion 
				// of the area and ignore the out of bounds portion.
				if (!m_grid.IsInBounds(cell)) continue;

				// is there existing object in this cell? is the cell not the same object that we're trying to occupy? 
				// if so, queue it for eviction. we will evict after we finish iterating through all cells to avoid modifying the grid while we're still iterating through it.
				T* existingProp = m_grid.Get(cell);
				if (existingProp != nullptr && existingProp != object)
				{
					// it's possible that multiple cells we're trying to occupy are occupied by the same existing object. 
					// we only need to evict it once, and vacating similar objects more than once will throw error, so we use a set to ensure uniqueness.
					toEvict.insert(existingProp);
				}

				// remember the valid cells to occupy so when we actually set the new object in the grid, we only do it in valid cells
				cellsToOccupy.push_back(cell);
			}

			// evict objects (except the one that is occupying) found in cells we 're trying to occupy. 
			// we choose to vacate rather than throw error because if there is really discrepancy between m_objects and m_grid, Vacate() will likely throw error anyways
			for (T* obj : toEvict)
			{
				Vacate(obj);
			}

			// new object occupies this cell. we also defer this because if we do this first then evict existing objects after, we will end up vacating the new object 
			// that we just set in the grid since it occupies the same cell as existing objects.
			for (const auto& cell : cellsToOccupy)
			{
				m_grid.Set(cell, object);
			}

			// if at least one cell is occupied, we consider this object as occupying the grid and we store it in m_objects. 
			// otherwise, this object is not really occupying any cell in the grid, so we won't store it in m_objects to avoid confusion.
			if(!cellsToOccupy.empty())
			{
				m_objects.Set(object, cellsToOccupy);
			}
		}

		void Vacate(T* object)
		{
			// let's be strict here. this method expects the object exist. 
			if (!m_objects.Has(object))
			{
				throw std::runtime_error("OccupancyGrid::Vacate(T*) - object to remove not found");
			}

			const auto& cells = m_objects.Get(object);
			for (const auto& cell : cells)
			{
				// we should be strict here. the cells are taken from m_objects which is the source of truth for what cells an object occupies. 
				// if any of these cells are out of bounds or doesn't contain this object in m_grid, it means there is a data inconsistency between m_objects and m_grid. 
				// we should throw an error to alert developers about this issue.
				if (m_grid.IsInBounds(cell) && m_grid.Get(cell) == object)
				{
					m_grid.Set(cell, nullptr);
				}
				else
				{
					throw std::runtime_error("OccupancyGrid::Vacate(T*) - failed to vacate cell for this object. potential data inconsistency between m_objects and m_grid");
				}
			}

			// be strict here. we already vacated cells for this object in grid, if we fail to unregister this object from m_objects, 
			// it means there is a data inconsistency between m_objects and m_grid.
			if (!m_objects.Unregister(object))
			{
				throw std::runtime_error("OccupancyGrid::Vacate(T*) - failed to unregister object from m_objects");
			}
		}

		bool Has(T* object) const
		{
			return m_objects.Has(object);
		}

		// TODO:
		// this method is O(n2) but if cells are not large it should be fine. observe if this will be called in hot stages with large cells count. 
		std::vector<T*> Get(const std::vector<Coord>& cells) const
		{
			std::vector<T*> result;

			for (const auto& cell : cells)
			{
				// skip invalid coord. we can have invalid cells if object occupies an area that is partially out of bounds. 
				// in that case, we will just ignore the out of bounds portion and only consider the valid portion of the area.
				if (!m_grid.IsInBounds(cell)) continue;

				// skip cells without objects
				T* obj = m_grid.Get(cell);
				if (!obj) continue;

				// if the object in this cell is not yet in our list, add it. this guarantees our result contains unique objects
				if (std::find(result.begin(), result.end(), obj) == result.end())
				{
					result.push_back(obj);
				}
			}

			// returns list of unique objects found in cells given
			return result;
		}

		T* Get(const Coord& cell) const
		{
			if (m_grid.IsInBounds(cell))
			{
				return m_grid.Get(cell);
			}
			return nullptr;
		}

		// TODO:
		// returning vector<Coord> by value can be expensive if the list of cells occupied by an object is large. 
		// fine if we ensure cells occupied by an object is usually small. observe
		std::vector<Coord> GetOccupiedTiles(T* object) const
		{
			static const std::vector<Coord> empty;

			if (!m_objects.Has(object))
			{
				return empty;
			}

			return m_objects.Get(object);
		}

		// debugger method to validate the internal consistency between m_objects and m_grid. this is useful for catching bugs during development.
		void Validate() const
		{
			// 1. Every object in m_objects must match grid
			for (auto& [obj, cells] : m_objects)
			{
				for (auto& cell : cells)
				{
					if (!m_grid.IsInBounds(cell) || m_grid.Get(cell) != obj)
					{
						throw std::runtime_error("Invariant broken: object->grid mismatch");
					}
				}
			}

			// 2. Every grid cell must match m_objects
			m_grid.ForEach([&](size_t r, size_t c, T* obj)
				{
					if (!obj) return;

					if (!m_objects.Has(obj))
					{
						throw std::runtime_error("Invariant broken: grid->object missing");
					}

					const auto& cells = m_objects.Get(obj);
					if (std::find(cells.begin(), cells.end(), Coord{ (int)r, (int)c }) == cells.end())
					{
						throw std::runtime_error("Invariant broken: grid not in object list");
					}
				});
		}
	};
#pragma endregion

#pragma region // TileLayer
	struct TileLayer
	{
		TileGrid<IRenderable> tilegrid;
		Tileset<IRenderable>* tileset;

		event::Event<const Coord&, TileVariant> TileVariantChangedEvent;
	};


	struct Floor
	{
		std::vector<TileLayer> tileLayers;
		std::unique_ptr<BucketGrid<IRenderable>> objectLayer;
	};

	class TileMap
	{
	private:
		std::vector<Floor> m_floors;

	public:
		void AddFloor()
		{

		}
	};

#pragma endregion

#pragma region // TileLayer editor
	// design consideration:
	// links two layers - source and target
	// source layer 
	// source layer is the observable, while target layer is the observer
	// when change happen to this layer, the target layer reacts
	// when a tile is place or removed at given tile coord in source layer, a tile is also placed or removed at that given tile coord in target layer
	// the type of tile placed or removed in target layer depends on the given lookup map
	// the map is a dictionary of TileVariant (key) and index (values). index is the tile index of target layer's tileset
	// RAII:
	// when source and target layers are linked, target layer subscribes to source layer's TileVariantChangedEvent
	// this is how target layer automatically responds when change happens on source layer
	// when this object is destroyed, the target layer unsubscribes from source layer, ensuring no dangling subscriptions between layers
	class TileLayerLink
	{
	private:
		TileLayer* m_source;
		TileLayer* m_target;
		const Dictionary<TileVariant, int>* m_map;

	public:
		TileLayerLink(TileLayer& source, TileLayer& target, const Dictionary<TileVariant, int>& map) :
			m_source(&source),
			m_target(&target),
			m_map(&map)
		{
			m_source->TileVariantChangedEvent += event::Handler(this, &TileLayerLink::OnTileChanged);
		}

		// ensures target layer is unsubscribed from source layer when link is destroyed
		~TileLayerLink()
		{
			if (m_source)
			{
				m_source->TileVariantChangedEvent -= event::Handler(this, &TileLayerLink::OnTileChanged);
			}
		}

		bool Involves(TileLayer* layer) const
		{
			return m_source == layer || m_target == layer;
		}

	private:
		void OnTileChanged(const Coord& c, TileVariant variant)
		{
			int index;
			if (m_map->TryGetValue(variant, index))
			{
				m_target->tilegrid.Set(c, m_target->tileset->MakeTile(index));
			}
		}
	};

	// design consideration:
	// manages all links between layers
	// RAII:
	// when destroyed, all links are destroyed too
	class TileLayerLinkSystem
	{
	private:
		std::vector<std::unique_ptr<TileLayerLink>> m_links;

	public:
		// links two layers - source and target
		// source layer - when a tile is placed/removed at given tile coord, it 
		void Link(TileLayer& source, TileLayer& target, const Dictionary<TileVariant, int>& map)
		{
			m_links.push_back(std::make_unique<TileLayerLink>(source, target, map));
		}

		void RemoveLinksFor(TileLayer& layer)
		{
			m_links.erase(
				std::remove_if(
					m_links.begin(),
					m_links.end(),
					[&layer](const std::unique_ptr<TileLayerLink>& link)
					{
						return link->Involves(&layer);
					}),
				m_links.end()
			);
		}

		void Clear()
		{
			m_links.clear();
		}
	};

	// design consideration:
	// this class is a system that allows a tilegrid to automatically performs the following:
	// - given tile coord, paint or erase a tile 
	// - update neighbor tiles of the given coord with appropriate tile values
	// - if a source layer is "linked" into a target layer, the given tile coord of target layer changes when source layer's given tile coord changes
	// - this behavior is managed by the given lookup map where it maps a TileVariant set on source layer to tileset index of target layer
	class TileLayerEditor
	{
	private:
		AutoTileSystem m_autoTile;
		TileLayerLinkSystem m_linkSystem;

		// design consideration:
		// let editor create AutoTileContext. it knows TileLayer
		AutoTileSystem::AutoTileContext GetAutoTileContext(TileLayer& layer)
		{
			return AutoTileSystem::AutoTileContext
			{
				[&layer](const Coord& coord) -> bool { return layer.tilegrid.IsInBounds(coord);  },
				[&layer](const Coord& coord) -> int { return layer.tilegrid.Get(coord).GetIndex();  },
				[&layer](const Coord& coord, int index, TileVariant variant)
				{
					layer.tilegrid.Set(coord, layer.tileset->MakeTile(index));
					layer.TileVariantChangedEvent(coord, variant);
				}
			};
		}

	public:
		TileLayerEditor() = default;

		// design consideration:
		// TileLayerLinkSystem is persistent, as it stores a list of linked layers
		// however, when TileLayerLinkSystem goes out of scope (destroyed), it also destroys all links
		// when links are destroyed, event subscriptions of target layer with source layer (observable) are severed
		~TileLayerEditor() = default;

		void Paint(TileLayer& layer, const AutoTileSystem::AutoTileConfig& config, const Coord& c)
		{
			auto ctx = GetAutoTileContext(layer);
			m_autoTile.Set(ctx, config, c);
		}

		void Erase(TileLayer& layer, const AutoTileSystem::AutoTileConfig& config, const Coord& c)
		{
			auto ctx = GetAutoTileContext(layer);
			m_autoTile.Remove(ctx, config, c);
		}

		void Fill(TileLayer& layer, const AutoTileSystem::AutoTileConfig& config)
		{
			auto ctx = GetAutoTileContext(layer);
			m_autoTile.Set(ctx, config, layer.tilegrid.GetSize());
		}

		void Clear(TileLayer& layer, const AutoTileSystem::AutoTileConfig& config)
		{
			auto ctx = GetAutoTileContext(layer);
			m_autoTile.Remove(ctx, config, layer.tilegrid.GetSize());
		}

		// design consideration:
		// this link layers together. source layer is the layer that when its tiles change, the target 
		// layer also changes based on the lookup map
		void LinkLayers(TileLayer& source, TileLayer& target, const Dictionary<TileVariant, int>& map)
		{
			m_linkSystem.Link(source, target, map);
		}

		// design consideration:
		// explicitly unlink a layer. any links where this layer is involved will be removed from link system
		void UnlinkLayer(TileLayer& layer)
		{
			m_linkSystem.RemoveLinksFor(layer);
		}
	};

#pragma endregion

#pragma region // base layer containg tile map + optional sub tile map and object grid
	class BaseLayer
	{
		struct MainLayer
		{
			std::unique_ptr<TileGrid<IRenderable>> tileGrid;
			std::unique_ptr<AutoTileResolver> autoTileResolver;
			Tileset<IRenderable>* tileset = nullptr;
			float depth;
			SizeF scale;
		};

		struct SubLayer
		{
			std::unique_ptr<TileGrid<IRenderable>> tileGrid;
			LookupResolver<const Coord&, TileVariant, int> lookupResolver;
			Tileset<IRenderable>* tileset = nullptr;
		};

	private:
		Dictionary<std::string,SubLayer> m_subLayers; 
		MainLayer m_mainLayer;
		Size<size_t> m_mapsize;	

	public:
		// in the constructor, we initialize the main layer with the given tileset and map size. we also set up its autotile resolver
		BaseLayer(const std::string& tilesetKey, Size<size_t>& mapsize, AutoTileResolver::AutoTileConfig& config, int defaultTileIndex):
			m_mapsize(mapsize)
		{
			// get tileset for main layer
			Tileset<IRenderable>& tileset = Registry<Tileset<IRenderable>>::Instance().Get(tilesetKey);

			m_mainLayer.tileGrid = std::make_unique<TileGrid<IRenderable>>();
			m_mainLayer.tileset = &tileset;
			m_mainLayer.autoTileResolver = std::make_unique<AutoTileResolver>(
				[this](const Coord& coord) -> bool { return m_mainLayer.tileGrid->IsInBounds(coord);  },
				[this](const Coord& coord) -> int { return m_mainLayer.tileGrid->Get(coord).GetIndex();  },
				[this](const Coord& coord, int index) { m_mainLayer.tileGrid->Set(coord, m_mainLayer.tileset->MakeTile(index)); },
				config
			);

			// now let's initialize the main layer. set its size with the given map size and fill with default tile
			m_mainLayer.tileGrid->Initialize(m_mapsize, m_mainLayer.tileset->MakeTile(defaultTileIndex));

		}

		void AddSubLayer(const std::string& name, const std::string& tilesetKey, const char* csvLookup)
		{
			// get tileset for sub layer
			Tileset<IRenderable>& tileset = Registry<Tileset<IRenderable>>::Instance().Get(tilesetKey);

			m_subLayers.Register(name, SubLayer());

			SubLayer& layer = m_subLayers[name];
			layer.tileGrid = std::make_unique<TileGrid<IRenderable>>();
			layer.tileset = &tileset;

			layer.tileGrid->Initialize(m_mapsize, m_mainLayer.tileset->MakeTile(-1));

			CSVFileParser parser;
			Table<std::string> table;
			parser.ReadImmediate(csvLookup, table);

			constexpr TileVariant allTileVariants[] =
			{
				TileVariant::Empty,
				TileVariant::Island,
				TileVariant::Full,
				TileVariant::NorthEdge,
				TileVariant::SouthEdge,
				TileVariant::EastEdge,
				TileVariant::WestEdge,
				TileVariant::NECorner,
				TileVariant::NWCorner,
				TileVariant::SECorner,
				TileVariant::SWCorner,
				TileVariant::Vertical,
				TileVariant::Horizontal,
				TileVariant::TNorth,
				TileVariant::TSouth,
				TileVariant::TEast,
				TileVariant::TWest
			};

			for (TileVariant tv : allTileVariants)
			{
				for (int row = 0; row < table.GetHeight(); row++)
				{
					// get supposed variant
					int variant = std::stoi(table.Get(row, 1));
					
					if (variant == (int)tv)
					{
						// get the tile index value
						int value = std::stoi(table.Get(row, 0));

						// register to resolver
						layer.lookupResolver.Register(tv, value);
					}
				}
			}

			// setup event handler to handle tile update on splash map when tile map changes
			layer.lookupResolver.LookupEvent += engine::event::Handler(std::function<void(const Coord&, int)>(
				[&layer](const Coord& c, int i)
				{
					layer.tileGrid->Set(c, layer.tileset->MakeTile(i));
				})
			);

			// let splash tile resolver subscribe to land auto-tile map so that it can update splash tile variants when land tiles change
			m_mainLayer.autoTileResolver->TileVariantChangedEvent += engine::event::Handler(&layer.lookupResolver, &LookupResolver<const Coord&, TileVariant, int>::Resolve);

			//m_subLayers.Register(name, std::move(layer));

		}

		Size<size_t> GetSize() const
		{
			return m_mapsize;
		}

		void FillMainLayer()
		{
			m_mainLayer.autoTileResolver->Set(m_mapsize);
		}

		void ClearMainLayer()
		{
			m_mainLayer.autoTileResolver->Remove(m_mapsize);
		}

		bool IsInBounds(int row, int col) const
		{
			return m_mainLayer.tileGrid->IsInBounds(row, col);
		}

		Tile<IRenderable>& GetMainLayerTile(int row, int col)
		{
			return m_mainLayer.tileGrid->Get(row, col);
		}

		void SetMainLayerTile(const Coord& coord)
		{
			m_mainLayer.autoTileResolver->Set(coord);
		}

		void RemoveMainLayerTile(const Coord& coord)
		{
			m_mainLayer.autoTileResolver->Remove(coord);
		}

		// method to execute a function on given tile in each sub layer 
		template<typename Predicate>
		void ForEachSubLayerTile(int row, int col, Predicate func)
		{
			for (auto& [key, subLayer] : m_subLayers)
			{
				func(key, subLayer.tileGrid->Get(row, col));
			}
		}

		// method to execute a function on a given tile in a specified sub layer
		template<typename Predicate>
		void WithSubLayerTile(const std::string& key, int row, int col, Predicate func)
		{
			if (m_subLayers.Has(key))
			{
				func(m_subLayers[key].tileGrid->Get(row, col));
			}
		}
	};
#pragma endregion

#pragma region // render support class for BaseLayer
	class MapLayerRenderer
	{
	private:
		SizeF m_tilesize;
		PositionF m_mapWorldPos;
		DrawSortedSpritesCommand& m_command;
		ColorF m_tint;

	public:
		MapLayerRenderer(
			SizeF tilesize, 
			PositionF mapWorldPos, 
			DrawSortedSpritesCommand& command,
			ColorF tint
		) :
			m_tilesize(tilesize),
			m_mapWorldPos(mapWorldPos),
			m_command(command),
			m_tint(tint)
		{
		}

		void SetColor(const ColorF& tint)
		{
			m_tint = tint;
		}

		ColorF GetColor() const
		{
			return m_tint;
		}

		void Clear()
		{
			m_command.Clear();
		}

		void Sort()
		{
			m_command.Sort();
		}

		void Draw()
		{
			m_command.Execute();
		}

		void QueueAllSubLayerTilesForDraw(BaseLayer& baseLayer, const std::string& key, float depth, VecF scale)
		{
			Size<size_t> size = baseLayer.GetSize();

			for (int row = 0; row < (int)size.height; row++)
			{
				for (int col = 0; col < (int)size.width; col++)
				{
					QueueSubLayerTileForDraw(baseLayer, key, row, col, depth, scale);
				}
			}
		}

		void QueueAllMainLayerTilesForDraw(BaseLayer& baseLayer, float depth, VecF scale)
		{
			Size<size_t> size = baseLayer.GetSize();

			for (int row = 0; row < (int)size.height; row++)
			{
				for (int col = 0; col < (int)size.width; col++)
				{
					QueueMainLayerTileForDraw(baseLayer, row, col, depth, scale);
				}
			}
		}

		void QueueSubLayerTileForDraw(
			BaseLayer& baseLayer,
			const std::string& key,
			int row, int col,
			float depth,
			engine::math::VecF scale = { 1,1 }
		)
		{
			// bail if not in bounds
			if (!baseLayer.IsInBounds(row, col))
			{
				return;
			}

			// get 
			baseLayer.WithSubLayerTile(key, row, col, [this, row, col, depth, scale](const Tile<IRenderable>& tile)
				{
					AddTile(row, col, depth, scale, tile);
				});

		}

		void QueueMainLayerTileForDraw(
			BaseLayer& baseLayer,
			int row, int col,
			float depth,
			engine::math::VecF scale = { 1,1 }
		)
		{
			// bail if not in bounds
			if (!baseLayer.IsInBounds(row, col))
			{
				return;
			}

			// get the tile from main layer.
			const Tile<IRenderable>& tile = baseLayer.GetMainLayerTile(row, col);

			AddTile(row, col, depth, scale, tile);
		}
		void QueueRenderableForDraw(
			IRenderable& renderable,
			const PositionF& pos,
			const ColorF& tint,
			float depth,
			VecF scale = { 1,1 }
		)
		{
			engine::spatial::SizeF size = renderable.GetSprite().GetSize();

			size.width *= scale.x;
			size.height *= scale.y;

			m_command.Add({
				renderable.GetSprite(),					// sprite object to draw
				pos,		// shift tile position from map space to world space
				size,						// size to draw the tile at
				tint,								// color
				0.0f,								// no rotation for tile
				depth								// depth value for sorting
				});
		}

		void QueueRenderableForDraw(
			IRenderable& renderable,
			const PositionF& pos,
			float depth,
			VecF scale = { 1,1 }
		)
		{
			QueueRenderableForDraw(renderable, pos, m_tint, depth, scale);
		}

		void QueueTileForDraw(
			TileGrid<IRenderable>& grid,
			int row, int col,
			float depth,
			VecF scale = { 1,1 }
		)
		{
			// bail if not in bounds
			if (!grid.IsInBounds(row, col))
			{
				return;
			}

			// get the tile from main layer.
			const Tile<IRenderable>& tile = grid.Get(row, col);

			AddTile(row, col, depth, scale, tile);
		}

		void QueueAllTilesForDraw(
			TileGrid<IRenderable>& grid,
			Size<size_t> size,
			float depth, 
			VecF scale = { 1,1 }
		)
		{
			for (int row = 0; row < (int)size.height; row++)
			{
				for (int col = 0; col < (int)size.width; col++)
				{
					QueueTileForDraw(grid, row, col, depth, scale);
				}
			}
		}


		void AddTile(int row, int col, float depth, const engine::math::VecF& scale, const Tile<IRenderable>& tile)
		{
			// if tile is valid, we can queue it for draw. otherwise, we skip it
			if (tile.IsValid())
			{
				// find the top-left position of the tile in map space.
				engine::spatial::PositionF tilePosFromMap =
				{
					col * m_tilesize.width,
					row * m_tilesize.height
				};

				// apply scale to tile size in case we want to draw the tile at different size. 
				// note that only size change. position is still based on original tile size 
				engine::spatial::SizeF scaledtilesize
				{
					m_tilesize.width * scale.x,
					m_tilesize.height * scale.y
				};

				m_command.Add({
					tile->GetSprite(),					// sprite object to draw
					m_mapWorldPos + tilePosFromMap,		// shift tile position from map space to world space
					scaledtilesize,						// size to draw the tile at
					m_tint,								// color
					0.0f,								// no rotation for tile
					depth								// depth value for sorting
					});
			}
		}
	};
#pragma endregion

#pragma region // AssetManager
	class AssetManager
	{
	public:
		template<typename T>
		T& Get(const std::string& key)
		{
			return Registry<T>::Instance().Get(key);
		}
		template<typename T>
		bool Has(const std::string& key)
		{
			return Registry<T>::Instance().Has(key);
		}
	};

#pragma endregion

#pragma region // SpriteAtlasLoader - factory only creates the object and creates its UV rects. loader "loads data" e.g. image file into the object
	class SpriteAtlasLoader
	{
	public:
		static ISpriteAtlas& Load(
			const std::string& name,
			const std::wstring& filepath,
			const size_t row, const size_t col
		)
		{
			auto& registry = Registry<ISpriteAtlas>::Instance();

			// if we don't have sprite atlas with this key in our registry, create one
			if (!registry.Has(name))
			{
				// using factory, create sprite atlas
				std::unique_ptr<ISpriteAtlas> atlas = SpriteAtlasFactory::Create(filepath, row, col);

				// register into cache
				registry.Register(name, std::move(atlas));
			}

			// return its reference
			return registry.Get(name);
		}

		static ISpriteAtlas& Load(
			const std::string& name,
			const std::wstring& filepath,
			const std::vector<engine::math::geometry::RectF>& uvs
		)
		{
			auto& registry = Registry<ISpriteAtlas>::Instance();

			// if we don't have sprite atlas with this key in our registry, create one
			if (!registry.Has(name))
			{
				// using factory, create sprite atlas
				std::unique_ptr<ISpriteAtlas> atlas = SpriteAtlasFactory::Create(filepath, uvs);

				// register into cache
				registry.Register(name, std::move(atlas));
			}

			// return its reference
			return registry.Get(name);
		}
	};
#pragma endregion

#pragma region // TilesetLoader. assumes Tileset is of IRenderable type
	class TilesetLoader
	{
	public:
		static Tileset<IRenderable>& Load(
			const std::string& name, // key for storing in cache
			const std::string& atlasName // key of the sprite atlas to get sprites to
			)
		{
			// if this tileset already exist, just return its reference
			auto& registry = Registry<Tileset<IRenderable>>::Instance();
			if (registry.Has(name))
			{
				return registry.Get(name);
			}
			else
			{
				// create tileset object
				std::unique_ptr<Tileset<IRenderable>> tileset = std::make_unique<Tileset<IRenderable>>();

				// if we this atlas exist, get a reference
				// TODO: a bit of a problem. if the sprite atlas does not exist, then we will have an empty tileset. this happens silently
				if (Registry<ISpriteAtlas>::Instance().Has(atlasName))
				{
					ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get(atlasName);

					// in this method, we are loading all the sprite atlas' sprites
					for (int i = 0; i < atlas.GetUVRectCount(); i++)
					{
						tileset->Register(i, std::make_unique<Renderable>(atlas.MakeSprite(i)));
					}
				}

				// finally, register into the cache
				registry.Register(name, std::move(tileset));
			}

			// return reference
			return registry.Get(name);
		}

		static Tileset<IRenderable>& Load(
			const std::string& name, // key for storing in cache
			const std::string& atlasName, // key of the sprite atlas to get sprites to
			const std::vector<int> uvrects // list of uvrects to load into tileset. 
		)
		{
			// if this tileset already exist, just return its reference
			auto& registry = Registry<Tileset<IRenderable>>::Instance();
			if (registry.Has(name))
			{
				return registry.Get(name);
			}
			else
			{
				// create tileset object
				std::unique_ptr<Tileset<IRenderable>> tileset = std::make_unique<Tileset<IRenderable>>();

				// if we this atlas exist, get a reference
				// TODO: a bit of a problem. if the sprite atlas does not exist, then we will have an empty tileset. this happens silently
				if (Registry<ISpriteAtlas>::Instance().Has(atlasName))
				{
					ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get(atlasName);

					// tileset indices will start from 0..N
					// uvrect indices will be based on given array of indices
					for (int i = 0; i < uvrects.size(); i++)
					{
						if (uvrects[i] < atlas.GetUVRectCount())
						{
							tileset->Register(i, std::make_unique<Renderable>(atlas.MakeSprite(uvrects[i])));
						}
					}
				}

				// finally, register into the cache
				registry.Register(name, std::move(tileset));
			}

			// return reference
			return registry.Get(name);
		}



	};
#pragma endregion

#pragma region // State
	class State
	{
	public:
		virtual ~State() = default;

		virtual void OnEnter() {}
		virtual void OnExit() {}

		virtual void OnUpdate(double dt) {}

		// optional input hooks
		virtual void OnKeyDown(int key) {}
		virtual void OnKeyUp(int key) {}

		virtual void OnMouseDown(int btn, int x, int y) {}
		virtual void OnMouseUp(int btn, int x, int y) {}
		virtual void OnMouseMove(int x, int y) {}

		virtual void OnInputEvent(const InputEvent& inputEvent) {}

		virtual void OnRender() {}
	};
#pragma endregion

#pragma region // StateMachine
//#include <memory>
//#include <functional>
	class StateMachine
	{
	private:
		std::unique_ptr<State> m_current;
		std::unique_ptr<State> m_next;

		bool m_isTransitioning = false;

		// --- Apply deferred transition ---
		void ApplyTransition()
		{
			if (!m_isTransitioning)
			{
				return;
			}

			if (m_current)
			{
				m_current->OnExit();
			}

			m_current = std::move(m_next);

			if (m_current)
			{
				m_current->OnEnter();
			}

			m_isTransitioning = false;
		}

	public:
		StateMachine() = default;
		~StateMachine() = default;

		// --- Set new state (deferred) ---
		template<typename T, typename... Args>
		void SetState(Args&&... args)
		{
			static_assert(std::is_base_of_v<State, T>, "T must derive from State");

			m_next = std::make_unique<T>(std::forward<Args>(args)...);
			m_isTransitioning = true;
		}

		// --- Immediate transition (rarely needed) ---
		template<typename T, typename... Args>
		void SetStateImmediate(Args&&... args)
		{
			static_assert(std::is_base_of_v<State, T>, "T must derive from State");

			m_isTransitioning = true;

			ApplyTransition();
		}

		// --- Update ---
		void OnUpdate(double dt)
		{
			ApplyTransition();

			if (m_current)
			{
				m_current->OnUpdate(dt);
			}
		}

		// --- Render ---
		void OnRender()
		{
			if (m_current)
			{
				m_current->OnRender();
			}
		}

		// --- Input forwarding ---
		void OnKeyDown(int key)
		{
			if (m_current)
			{
				m_current->OnKeyDown(key);
			}
		}

		void OnKeyUp(int key)
		{
			if (m_current)
			{
				m_current->OnKeyUp(key);
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			if (m_current)
			{
				m_current->OnMouseDown(btn, x, y);
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
			if (m_current)
			{
				m_current->OnMouseUp(btn, x, y);
			}
		}

		void OnMouseMove(int x, int y)
		{
			if (m_current)
			{
				m_current->OnMouseMove(x, y);
			}
		}

		void OnInputEvent(const InputEvent& inputEvent)
		{
			if (m_current)
			{
				m_current->OnInputEvent(inputEvent);
			}
		}

		// --- Helpers ---
		State* GetCurrent() const 
		{ 
			return m_current.get(); 
		}

		template<typename T>
		bool IsInState() const
		{
			return dynamic_cast<T*>(m_current.get()) != nullptr;
		}
	};
#pragma endregion

#pragma region // Scene
	class Scene
	{
	public:
		virtual ~Scene() = default;

		// lifecycle
		virtual void OnEnter() {}
		virtual void OnExit() {}

		// main loop
		virtual void OnUpdate(double delta) {}
		virtual void OnRender() {}

		// input (event-based)
		virtual void OnKeyDown(int key) {}
		virtual void OnKeyUp(int key) {}

		virtual void OnMouseDown(int btn, int x, int y) {}
		virtual void OnMouseUp(int btn, int x, int y) {}
		virtual void OnMouseMove(int x, int y) {}

		virtual void OnInputEvent(const InputEvent& inputEvent) {}
	};
#pragma endregion

#pragma region // SceneManager
//#include <memory>
//#include <unordered_map>
//#include <string>
//#include <cassert>
	class SceneManager
	{
	private:
		Dictionary<std::string, std::unique_ptr<Scene>> m_scenes;
		Scene* m_activeScene = nullptr;

	public:
		// Create and register scene
		template<typename T, typename... Args>
		void CreateScene(const std::string& name, Args&&... args)
		{
			static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");

			m_scenes[name] = std::make_unique<T>(std::forward<Args>(args)...);
		}

		// Switch active scene
		void SetActive(const std::string& name)
		{
			if (!m_scenes.Has(name))
			{
				return;
			}

			if (m_activeScene)
			{
				m_activeScene->OnExit();
			}

			m_activeScene = m_scenes[name].get();

			if (m_activeScene)
			{
				m_activeScene->OnEnter();
			}
		}

		Scene* GetActive()
		{
			return m_activeScene;
		}

		// Main loop forwarding
		void OnUpdate(double delta)
		{
			if (m_activeScene) 
			{
				m_activeScene->OnUpdate(delta);
			}
		}

		void OnRender()
		{
			if (m_activeScene)
			{
				m_activeScene->OnRender();
			}
		}

		// Input forwarding
		void OnKeyDown(int key)
		{
			if (m_activeScene)
			{
				m_activeScene->OnKeyDown(key);
			}
		}

		void OnKeyUp(int key)
		{
			if (m_activeScene)
			{
				m_activeScene->OnKeyUp(key);
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			if (m_activeScene)
			{
				m_activeScene->OnMouseDown(btn, x, y);
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
			if (m_activeScene)
			{
				m_activeScene->OnMouseUp(btn, x, y);
			}
		}

		void OnMouseMove(int x, int y)
		{
			if (m_activeScene)
			{
				m_activeScene->OnMouseMove(x, y);
			}
		}

		void OnInputEvent(const InputEvent& inputEvent)
		{
			if (m_activeScene)
			{
				m_activeScene->OnInputEvent(inputEvent);
			}
		}
	};
#pragma endregion

#pragma region // helper methods

	//inline static std::vector<Coord> GetOverlappedTiles(
	//	const RectF& aabb,
	//	const SizeF& tilesize,
	//	const Size<size_t> mapsize)
	//{
	//	// get top-left and bottom-right coords
	//	Coord p0 = PositionToCoord(aabb.GetTopLeft(), tilesize);
	//	Coord p1 = PositionToCoord(aabb.GetBottomRight(), tilesize);

	//	// do this in case aabb is flipped, meaning top-left is the bottom-right and vice versa
	//	Coord topLeft =
	//	{
	//		std::min<int>(p0.row, p1.row),
	//		std::min<int>(p0.col, p1.col)
	//	};

	//	Coord bottomRight =
	//	{
	//		std::max<int>(p0.row, p1.row),
	//		std::max<int>(p0.col, p1.col)
	//	};

	//	// clamp top-left to be 0,0 or greater only
	//	int startRow = std::max<int>(0, topLeft.row);
	//	int startCol = std::max<int>(0, topLeft.col);

	//	// clamp bottom-right to be max,max or less only
	//	int endRow = std::min<int>((int)mapsize.height - 1, bottomRight.row);
	//	int endCol = std::min<int>((int)mapsize.width - 1, bottomRight.col);

	//	// start and end row and column are now guaranteed to be valid coords. no need to check for bounds

	//	std::vector<Coord> result;
	//	for (int row = startRow; row <= endRow; ++row)
	//	{
	//		for (int col = startCol; col <= endCol; ++col)
	//		{
	//			result.push_back({ row, col });
	//		}
	//	}

	//	return result;
	//}


	inline static PositionF ScreenToWorld(const PositionF& screen, const PositionF& position) 
	{
		return screen - position;
	}

	inline static Coord ScreenToTileCoord(const PositionF& screen, const PositionF& position, const SizeF& tileSize)
	{
		PositionF worldPos = ScreenToWorld(screen, position);
		return PositionToCoord(worldPos, tileSize);
	}


	// TODO: put on navigation namespace along with NavigationGrid and TileConstraint but as a static helper
	inline static TileConstraint SubCellToConstraint(int r, int c)
	{
		static const TileConstraint table[9] =
		{
			TileConstraint::NW, TileConstraint::N,  TileConstraint::NE,
			TileConstraint::W,  TileConstraint::CENTER, TileConstraint::E,
			TileConstraint::SW, TileConstraint::S,  TileConstraint::SE
		};

		if (r < 0 || r >= 3 || c < 0 || c >= 3)
		{
			return TileConstraint::NONE;
		}

		return table[r * 3 + c];
	}
#pragma endregion

#pragma region // Prop
	struct CollisionShape
	{
		// can change this later for polygon class implementation for more accurate shape
		RectF shape;

		bool Intersects(const PositionF& position) const
		{
			return false; // TODO: for now we are not using this yet...
			return shape.Contains(position);
		}
	};

	struct Prop
	{
		std::unique_ptr<IRenderable> renderable;
		PositionF position; // position of this object relative to its owner
		VecF scale;
		ColorF color;
		RectF footprint;
		RectF boundingBox;
		CollisionShape collisionShape;

		Prop(std::unique_ptr<IRenderable> r, const PositionF& p, const ColorF& c, const VecF& s, const RectF& fp, const RectF& bb) :
			renderable(std::move(r)),
			position(p),
			scale(s),
			color(c),
			footprint(fp),
			boundingBox(bb)
		{
		}

		PositionF GetPosition(bool applyPivot) const
		{
			PositionF pivot = renderable->GetSprite().GetPivot();

			if (applyPivot)
			{
				return PositionF{ position.x - pivot.x, position.y - pivot.y };
			}
			else
			{
				return PositionF{ position.x, position.y };
			}
		}

		// TODO: probably shouldn't do this. probably better to just expose information needed to feed DrawCommand
		void QueueForDraw(DrawSortedSpritesCommand& drawCommand, const PositionF& worldPos, float depth) const
		{
			// scale the sprite
			SizeF size = renderable->GetSprite().GetSize();
			size.width *= scale.x;
			size.height *= scale.y;

			drawCommand.Add({
				renderable->GetSprite(),		// sprite object to draw
				position + worldPos,			// position
				size,							// scaled size
				color,							// color
				0.0f,							// no rotation
				depth							// depth value for sorting
				});
		}

		RectF GetScaledFootprintLocal(bool applyPivot = true) const
		{
			// scale the sprite size
			SizeF size = renderable->GetSprite().GetSize();
			size.width *= scale.x;
			size.height *= scale.y;

			// calculate the footprint rect in pixels. 
			RectF fp{};
			fp.left = footprint.left * size.width;
			fp.top = footprint.top * size.height;
			fp.right = footprint.right * size.width;
			fp.bottom = footprint.bottom * size.height;

			// if we're translating the footprint into pivot, go ahead
			if (applyPivot)
			{
				// get pivot position in pixel. make sure to take scale into account
				PositionF pivot = renderable->GetSprite().GetPivot();
				pivot.x *= size.width;
				pivot.y *= size.height;

				// translate the footprint into pivot
				fp.left -= pivot.x;
				fp.top -= pivot.y;
				fp.right -= pivot.x;
				fp.bottom -= pivot.y;
			}

			return fp;
		}

		RectF GetScaledFootprintWorld(const PositionF& worldPosition, bool applyPivot = true) const
		{
			RectF footprint = GetScaledFootprintLocal(applyPivot);

			footprint.left += worldPosition.x;
			footprint.top += worldPosition.y;
			footprint.right += worldPosition.x;
			footprint.bottom += worldPosition.y;

			return footprint;
		}

		RectF GetScaledBoundingBoxBLocal(bool applyPivot) const
		{
			// scale the sprite size
			SizeF size = renderable->GetSprite().GetSize();
			size.width *= scale.x;
			size.height *= scale.y;

			// calculate the bounding box rect in pixels. 
			RectF hb{};
			hb.left = boundingBox.left * size.width;
			hb.top = boundingBox.top * size.height;
			hb.right = boundingBox.right * size.width;
			hb.bottom = boundingBox.bottom * size.height;

			// if we're translating the bounding box into pivot, go ahead
			if (applyPivot)
			{
				// get pivot position in pixel. make sure to take scale into account
				PositionF pivot = renderable->GetSprite().GetPivot();
				pivot.x *= size.width;
				pivot.y *= size.height;

				// translate the the into pivot
				hb.left -= pivot.x;
				hb.top -= pivot.y;
				hb.right -= pivot.x;
				hb.bottom -= pivot.y;
			}

			return hb;
		}

		RectF GetScaledBoundingBoxWorld(const PositionF& worldPosition, bool applyPivot = true) const
		{
			RectF footprint = GetScaledBoundingBoxBLocal(applyPivot);

			footprint.left += worldPosition.x;
			footprint.top += worldPosition.y;
			footprint.right += worldPosition.x;
			footprint.bottom += worldPosition.y;

			return footprint;
		}
	};
#pragma endregion

#pragma region // PropBrush 

	struct PropBrush
	{
		std::string animationSet;
		std::string animation;
		VecF scale;
		ColorF color;
		RectF footprint;
		RectF boundingBox;
	};
#pragma endregion

#pragma region // PropBrushTool
	// PropBrushTool
	//
	// PURPOSE:
	// A lightweight editor tool responsible for defining "what to place" and
	// creating instances of objects based on the current placement definition.
	//
	// RESPONSIBILITIES:
	// - Stores the currently selected PropBrush
	// - Creates Prop instances (factory role)
	// - Owns and maintains a preview instance for visualization
	//
	// NON-RESPONSIBILITIES (IMPORTANT):
	// - Does NOT manage grid, tiles, or map logic
	// - Does NOT decide where objects are placed (no coordinate logic)
	// - Does NOT handle input (mouse/keyboard)
	// - Does NOT perform rendering directly (only prepares data for rendering)
	//
	// DESIGN NOTES:
	// - The preview object exists purely for visualization and does NOT belong to the world
	// - Prop instances created by this tool are independent from the preview
	// - This class should remain small; avoid turning it into a "god object"
	//
	// THINK OF IT AS:
	// "A brush that knows what to paint, and can produce paint strokes (Props)"
	class PropBrushTool
	{
	private:
		PropBrush m_currPlacement;

		// TODO (ARCHITECTURE):
		// Currently using Prop as preview visualization.
		// This is a temporary shortcut.
		//
		// PROBLEM:
		// - Prop represents a world entity (tile-relative position, world semantics)
		// - Preview is a screen-space visualization (mouse-driven, not part of world)
		// - Reusing Prop here mixes two different concepts (world vs editor tool)
		//
		// CURRENT ASSUMPTIONS (must NOT be broken):
		// - preview->position is meaningless and always treated as {0,0}
		// - preview is NEVER inserted into BucketGrid
		// - preview rendering position is always provided externally (mouse position)
		//
		// FUTURE REFACTOR:
		// - Introduce a dedicated Preview type:
		//     struct Preview {
		//         std::unique_ptr<IRenderable> renderable;
		//         VecF scale;
		//         ColorF color;
		//         PositionF pivot;
		//     };
		// - Move pivot/offset alignment logic into Preview
		// - Remove dependency on Prop::QueueForDraw for preview rendering
		//
		// NOTE:
		// Do NOT extend Prop to support preview behavior.
		// Keep Prop strictly as a world entity.
		// 
		// TODO:
		// Support pivot-based placement.
		// Current implementation assumes top-left alignment.
		// Need to:
		// - read pivot from sprite (or placement definition)
		// - offset preview and placed object accordingly
		std::unique_ptr<Prop> m_preview;

	public:
		PropBrushTool()
		{
		}

		void SetBrush(const PropBrush& def, AssetManager& assets)
		{
			m_currPlacement = def;

			m_preview = CreateAnimatedProp({ 0,0 }, assets);
		}

		std::unique_ptr<Prop> CreateAnimatedProp(const PositionF& position, AssetManager& assets)
		{
			auto& animSet = assets.Get<AnimationSet<Sprite>>(m_currPlacement.animationSet);

			return std::make_unique<Prop>(
				std::make_unique<Animated>(animSet, m_currPlacement.animation),
				position,
				m_currPlacement.color,
				m_currPlacement.scale,
				m_currPlacement.footprint,
				m_currPlacement.boundingBox
			);
		}

		void QueuePreviewForDraw(DrawSortedSpritesCommand& drawCommand, const PositionF& pos, float depth) const
		{
			m_preview->QueueForDraw(drawCommand, pos, depth);
		}

		RectF GetPreviewFootprintAt(const PositionF& position)
		{
			// get the footprint of preview object, scaled, and translated into its pivot and position
			RectF fp = m_preview->GetScaledFootprintWorld(position, true);

			return fp;
		}

		RectF GetPreviewBoundingBoxAt(const PositionF& position)
		{
			// get the footprint of preview object, scaled, and translated into its pivot
			RectF hb = m_preview->GetScaledBoundingBoxWorld(position, true);

			return hb;
		}
	};

#pragma endregion

#pragma region // PropPlacementContext
	// this is a data transfer object (DTO)
	struct PropPlacementContext
	{
	private:
		// only PropPlacementSystem can construct
		friend class PropPlacementSystem;

		PropPlacementContext() = default;

	public:
		PositionF worldPosition;
		std::vector<Coord> footprintTiles;
		std::vector<Coord> boundingBoxTiles;

		// prevent accidental copying if you want immutability guarantees
		PropPlacementContext(const PropPlacementContext&) = delete;
		PropPlacementContext& operator=(const PropPlacementContext&) = delete;

		// allow move if needed
		PropPlacementContext(PropPlacementContext&&) = default;
		PropPlacementContext& operator=(PropPlacementContext&&) = default;
	};

#pragma endregion

#pragma region // PropMap1
	class PropMap1
	{
	private:
		class FootprintGridView
		{
		private:
			View<OccupancyGrid<Prop>> m_view;

		public:
			FootprintGridView(OccupancyGrid<Prop>& grid) :
				m_view(&grid) 
			{
			}

			std::vector<Prop*> Get(const std::vector<Coord>& cells) const
			{
				return m_view->Get(cells);
			}

			void Validate() const
			{
				m_view->Validate();
			}
		};
	private:
		// top-left position of the world in screen space. this is basically the camera position.
		PositionF m_position;

		// size of the world in tiles
		Size<size_t> m_size;

		// size of each tile in pixels. 
		SizeF m_tileSize;

	public:
		BucketGrid<Prop> m_objectLayer;
		NavigationGrid m_navGrid;
		MultiOccupantGrid<Prop, TileConstraint> m_FootPrintGrid;
		MultiOccupancyGrid<Prop> m_BoundingBoxGrid;

	public:
		View<MultiOccupancyGrid<Prop>> BoundingBoxLayer;
		View<NavigationGrid> NavigationLayer;

	public:
		engine::event::Event<const PositionF&, const Size<size_t>&, const SizeF&> InitializeEvent;

		PropMap1() :
			m_position(0, 0),
			m_size(0, 0),
			m_tileSize(0, 0),
			BoundingBoxLayer(&m_BoundingBoxGrid),
			NavigationLayer(&m_navGrid)
		{
		}

		std::string GetDebugInfo() const 
		{
			std::string debugInfo;
			debugInfo += "fp: ";
			debugInfo += std::to_string(m_FootPrintGrid.GetObjectCount());
			debugInfo += " bb: ";
			debugInfo += std::to_string(m_BoundingBoxGrid.GetObjectCount());
			debugInfo += " obj: ";
			debugInfo += std::to_string(m_objectLayer.GetObjectCount());
			return debugInfo;
		}

		PositionF GetPosition() const
		{
			return m_position;
		}

		bool Initialize(const PositionF& position, const Size<size_t>& size, const SizeF& tilesize)
		{
			m_position = position;
			m_size = size;
			m_tileSize = tilesize;

			// initialize bucket grid
			m_objectLayer.Initialize(m_size);

			// initialize navigation grid. fill it with none meaning all tiles are walkable (no constraints)
			m_navGrid.Initialize(m_size, TileConstraint::NONE);

			// initialize footprint grid. 
			m_FootPrintGrid.Initialize(m_size.width, m_size.height);

			// initialize bounding box grid
			m_BoundingBoxGrid.Initialize(m_size.width, m_size.height);

			InitializeEvent(m_position, m_size, m_tileSize);

			return true;
		}

		PositionF ScreenToWorldPosition(const PositionF& screenPos) const
		{
			return screenPos - m_position;
		}

		PositionF ScreenToTilePosition(const PositionF& screenPos) const
		{
			PositionF mouseWorldPos = screenPos - m_position;
			Coord coord = PositionToCoord(mouseWorldPos, m_tileSize);
			PositionF coordTopLeftWorldPos = CoordToPosition(coord, m_tileSize);
			return mouseWorldPos - coordTopLeftWorldPos;
		}

		Size<size_t> GetSize() const
		{
			return m_size;
		}

		SizeF GetTileSize() const
		{
			return m_tileSize;
		}

		bool IsInBounds(int row, int col) const
		{
			return
				row >= 0 && col >= 0 &&		// make sure rows and columns are not negatives.
				col < m_size.width &&		// make sure column is within the grid's width
				row < m_size.height;	// make sure row is within the grid's height
		}

		bool IsInBounds(const Coord& coord) const
		{
			return IsInBounds(coord.row, coord.col);
		}

		bool IsInBounds(const PositionF& worldPosition) const
		{
			Coord coord = PositionToCoord(worldPosition, m_tileSize);
			return IsInBounds(coord);
		}

		void Remove(Prop* prop)
		{
			// get tiles occupied by this prop 
			std::vector<Coord> tilesOccupiedByProp = m_FootPrintGrid.GetOccupiedCells(prop);

			for (const Coord& coord : tilesOccupiedByProp)
			{
				// get the constraint of the prop in the given coord.
				// this is a strict method. it's gonna throw error if there is no prop in this coord
				TileConstraint constraint = m_FootPrintGrid.Get(prop, coord);

				// remove that constraint in the coord
				m_navGrid.RemoveFlag(coord, constraint);
			}

			// remove this object from spatial occupancy grid
			m_FootPrintGrid.Remove(prop);

			// remove this object from bounding box occupancy grid
			m_BoundingBoxGrid.Vacate(prop);

			// remove this object from object layer
			m_objectLayer.Remove(prop);
		}

		// places a given prop into the map at given position. 
		// it takes context which contains all necessary information for placing the prop into different systems.
		// this should be called only by editing tools or placement system.
		bool Place(std::unique_ptr<Prop> prop, const PositionF& worldPos)
		{
			// get tile coord where the position intersects. validate it
			Coord coord = PositionToCoord(worldPos, m_tileSize);
			if (!IsInBounds(coord))
			{
				return false;
			}

			if (m_objectLayer.Has(prop.get()))
			{
				throw std::exception("why do we already have this object???");
			}

			// get prop's footprint in world position
			RectF footprint = prop->GetScaledFootprintWorld(worldPos);

			// get list of subcells this footprint overlaps
			std::vector<Coord> subcells = GridQuery::QueryCells(footprint, m_tileSize /3.0f, m_size * 3);

			// iterate through each of these sub cells and get constraints. then build constraints for each cells
			Dictionary<Coord, TileConstraint> constraints;
			for (Coord& subcell : subcells)
			{
				// calculate the actual tile coord in map
				Coord tileCoord = { subcell.row / 3, subcell.col / 3 };

				// calculate the sub tile coord relative to this tile
				Coord subCoord = { subcell.row % 3, subcell.col % 3 };

				TileConstraint bit = SubCellToConstraint(subCoord.row, subCoord.col);

				if (!constraints.Has(tileCoord))
				{
					constraints.Register(tileCoord, TileConstraint::NONE);
				}
				constraints[tileCoord] |= bit;
			}

			// iterate through each cells the prop overlapped. we will check if the props in these cells are overlapped by the new prop
			std::unordered_set<Prop*> toEvict;
			for (auto& constraintsPerCell : constraints)
			{
				Coord coord = constraintsPerCell.first;

				// get all the existing props that occupy this cell
				const std::vector<TileOccupant<Prop, TileConstraint>>& occupants = m_FootPrintGrid.Get(coord);

				// check if this prop overlap this existing prop
				for(TileOccupant occupant: occupants)
				{
					// if they overlap, we will remove this prop
					TileConstraint tc = constraintsPerCell.second & occupant.data;
					if (tc != TileConstraint::NONE)
					{
						toEvict.insert(occupant.object);
					}
				}
			}

			// remove props overlapped by new prop
			for (Prop* prop : toEvict)
			{
				Remove(prop);
			}

			std::vector<Coord> cells;
			for (auto& constraintsPerCell : constraints)
			{
				// place the new prop into navigation grid
				Coord coord = constraintsPerCell.first;
				TileConstraint constraint = constraintsPerCell.second;
				m_navGrid.AddFlag(coord, constraint);

				// place in footprint
				m_FootPrintGrid.Add(prop.get(), coord, constraint);

				cells.push_back(coord);
			}

			// bounding box
			RectF boundingBox = prop->GetScaledBoundingBoxWorld(worldPos, true);
			std::vector<Coord> boundingBoxTiles = GridQuery::QueryCells(boundingBox, m_tileSize, m_size);

			// add the new object into bounding box occupancy grid
			m_BoundingBoxGrid.Add(prop.get(), boundingBoxTiles);

			// get the world position of the tile where prop will be store. this will be tile's top-left position in world
			PositionF coordTopLeftWorldPos = CoordToPosition(coord, m_tileSize);

			// now we translate the world position into position relative to this tile coordinate
			PositionF propPosInTile = worldPos - coordTopLeftWorldPos;

			// then we set it as position of this prop
			prop->position = propPosInTile;

			// add the actual object into our object layer
			m_objectLayer.Add(coord, std::move(prop));

			return true;
		}

		std::vector<Prop*> QueryBoundingBoxOverlaps(const Coord& coord) const
		{
			return m_BoundingBoxGrid.Get(coord);
		}

		bool Remove(const PositionF& worldPosition)
		{
			// Bounds check (early)
			if (!IsInBounds(worldPosition))
			{
				return false;
			}

			// is the position within bounds of the world? if not, bail out
			Coord coord = PositionToCoord(worldPosition, m_tileSize);

			// given the coord. get all the props in this coord where their bounding box intersects
			std::vector<Prop*> candidates = m_BoundingBoxGrid.Get(coord);
			if (candidates.empty()) return {};

			// if there are props, iterate through them and find the ones whose bounding box intersects with the cursor position (in world coordinate). return all intersecting objects. 
			Prop* topMostProp = nullptr;
			RectF topMostPropBoundingBox{};
			for (Prop* candidate : candidates)
			{
				// find the coordinate of the cell that owns this prop 
				Coord propCoordOwner;
				if (!TryGetCoord(candidate, propCoordOwner))
				{
					// if this prop does not exist in the map, throw. this must be a bug. 
					// how did we find pointer to this object in bounding box grid? where is the actual object stored?
					throw std::out_of_range("PropSelectionSystem::SelectAtPoint() - prop does not exist in the map but we have pointer to it. where is it stored?!");
				}

				// get the object's position. the object's position is relative to the cell in the map it is stored. 
				PositionF propPosInTileOwner = candidate->GetPosition(true);

				// get the world position of this object by adding the object's position relative to tile with the top-left position of the tile in world coordinate
				PositionF propPosInWorld = propPosInTileOwner + CoordToPosition(propCoordOwner, m_tileSize);

				// get this bounding box of the object in world coordinates
				RectF objectBoundingBox = candidate->GetScaledBoundingBoxWorld(propPosInWorld, true);

				// broad phase collision check: check if the mouse cursor (in world coordinate) is intersecting with the bounding box already in world coordinate. 
				if (!objectBoundingBox.Contains(worldPosition)) continue;

				// TODO:implement narrow phase collision check and execute here

				// if so, compare its depth with current candidate for top most and replace candidate if this one is higher (lower on the screen)
				if (topMostProp == nullptr || objectBoundingBox.bottom >= topMostPropBoundingBox.bottom)
				{
					topMostProp = candidate;
					topMostPropBoundingBox = objectBoundingBox;
				}
			}

			// it is possible that there is no candidate prop that actually was selected. note that candidates are only based on props that belongs to 
			// the tiles. it does not mean that any of them will intersect with the position. that is why the candidates need to do a broad phase check 
			// via their bounding box (and later narrow phase check with their collision shape).
			if (topMostProp == nullptr) return false;

			// Apply removal (delegate to PropMap / WorldState)
			Remove(topMostProp);

			return true;
		}

		//std::vector<Prop*> QueryFootprintOverlaps(const std::vector<Coord>& coord) const
		//{
		//	// if there are objects the new footprint overlaps, get their references
		//	return m_FootPrintGrid.Get(coord);
		//}

		// try to get tile coordinate of a given prop. if prop is invalid, return false
		bool TryGetCoord(Prop* prop, spatial::Coord& coord) const
		{
			return m_objectLayer.TryGetCoord(prop, coord);
		}

		void Validate() const
		{
			m_navGrid.Validate();
			m_FootPrintGrid.Validate();
			m_BoundingBoxGrid.Validate();
		}

		template<typename Predicate>
		void ForEachProp(int row, int col, Predicate func)
		{
			if (!IsInBounds(row, col)) return;

			m_objectLayer.ForEach(row, col, func);
		}

		template<typename Predicate>
		void ForEachTileConstraint(int row, int col, Predicate func) const
		{
			if (!IsInBounds(row, col)) return;

			m_navGrid.ForEach(row, col, func);
		}

	};

#pragma endregion

#pragma region // PropMap
	class PropMap
	{
	private:
		class FootprintGridView
		{
		private:
			View<OccupancyGrid<Prop>> m_view;

		public:
			FootprintGridView(OccupancyGrid<Prop>& grid) :
				m_view(&grid)
			{
			}

			std::vector<Prop*> Get(const std::vector<Coord>& cells) const
			{
				return m_view->Get(cells);
			}

			void Validate() const
			{
				m_view->Validate();
			}
		};

		struct Occupant
		{
		};

	private:
		// top-left position of the world in screen space. this is basically the camera position.
		PositionF m_position;

		// size of the world in tiles
		Size<size_t> m_size;

		// size of each tile in pixels. 
		SizeF m_tileSize;

	public:
		BucketGrid<Prop> m_objectLayer;
		NavigationGrid m_navGrid;
		OccupancyGrid<Prop> m_FootPrintGrid;
		MultiOccupancyGrid<Prop> m_BoundingBoxGrid;
		MultiOccupantGrid<Prop, TileConstraint> m_FootprintLayer;

	public:
		FootprintGridView FootprintLayer;
		View<MultiOccupancyGrid<Prop>> BoundingBoxLayer;
		View<NavigationGrid> NavigationLayer;

	public:
		engine::event::Event<const PositionF&, const Size<size_t>&, const SizeF&> InitializeEvent;

		PropMap() :
			m_position(0, 0),
			m_size(0, 0),
			m_tileSize(0, 0),
			FootprintLayer(m_FootPrintGrid),
			BoundingBoxLayer(&m_BoundingBoxGrid),
			NavigationLayer(&m_navGrid)
		{
		}

		bool Initialize(const PositionF& position, const Size<size_t>& size, const SizeF& tilesize)
		{
			m_position = position;
			m_size = size;
			m_tileSize = tilesize;

			// initialize bucket grid
			m_objectLayer.Initialize(m_size);

			// initialize navigation grid. fill it with none meaning all tiles are walkable (no constraints)
			m_navGrid.Initialize(m_size, TileConstraint::NONE);

			// initialize footprint grid. 
			m_FootPrintGrid.Initialize(m_size.width * 3, m_size.height * 3);

			// initialize bounding box grid
			m_BoundingBoxGrid.Initialize(m_size.width, m_size.height);

			InitializeEvent(m_position, m_size, m_tileSize);

			return true;
		}

		PositionF ScreenToWorldPosition(const PositionF& screenPos) const
		{
			return screenPos - m_position;
		}

		PositionF ScreenToTilePosition(const PositionF& screenPos) const
		{
			PositionF mouseWorldPos = screenPos - m_position;
			Coord coord = PositionToCoord(mouseWorldPos, m_tileSize);
			PositionF coordTopLeftWorldPos = CoordToPosition(coord, m_tileSize);
			return mouseWorldPos - coordTopLeftWorldPos;
		}

		Size<size_t> GetSize() const
		{
			return m_size;
		}

		SizeF GetTileSize() const
		{
			return m_tileSize;
		}

		SizeF GetRefinedTileSize() const
		{
			return m_tileSize / 3.0f;
		}

		Size<size_t> GetRefinedSize() const
		{
			return m_FootPrintGrid.GetSize();
		}

		bool IsInBounds(int row, int col) const
		{
			return
				row >= 0 && col >= 0 &&		// make sure rows and columns are not negatives.
				col < m_size.width &&		// make sure column is within the grid's width
				row < m_size.height;	// make sure row is within the grid's height
		}

		bool IsInBounds(const Coord& coord) const
		{
			return IsInBounds(coord.row, coord.col);
		}

		bool IsInBounds(const PositionF& worldPosition) const
		{
			Coord coord = PositionToCoord(worldPosition, m_tileSize);
			return IsInBounds(coord);
		}

		//std::vector<Coord> QueryTilesOverlappedByFootprint(const RectF& worldRect) const
		//{
		//	// get all the subtiles the footprint overlapped
		//	return GetOverlappedTiles(worldRect, m_tileSize / 3.0f, m_FootPrintGrid.GetSize());
		//}

		//std::vector<Coord> QueryTilesOverlappedByBoundingBox(const RectF& worldRect) const
		//{
		//	return GetOverlappedTiles(worldRect, m_tileSize, m_BoundingBoxGrid.GetSize());
		//}

		// removes a given prop from the map. this is basically the reverse of PlaceProp.
		// it just blindly remove prop from all systems without checking for anything. 
		// make sure to call this method only when you know exactly which prop you want to remove.
		// this should be called only by editing tools or placement system
		void Remove(Prop* prop)
		{
			// 2. remove this object from navigation grid
			std::vector<Coord> subTilesOccupiedByPropToRemove = m_FootPrintGrid.GetOccupiedTiles(prop);
			for (const Coord& coord : subTilesOccupiedByPropToRemove)
			{
				// coord is a subtile coord in world (map). get the parent tile coord
				Coord tileCoord = { coord.row / 3, coord.col / 3 };

				// calculate the sub tile coord relative to this tile
				Coord subCoord = { coord.row % 3, coord.col % 3 };

				TileConstraint bit = SubCellToConstraint(subCoord.row, subCoord.col);

				m_navGrid.RemoveFlag(tileCoord, bit);
			}

			// 3. remove this object from spatial occupancy grid
			m_FootPrintGrid.Vacate(prop);

			// 4. remove this object from bounding box occupancy grid
			m_BoundingBoxGrid.Vacate(prop);

			// 1. remove this object from object layer
			m_objectLayer.Remove(prop);

		}

		// places a given prop into the map at given position. 
		// it takes context which contains all necessary information for placing the prop into different systems.
		// this should be called only by editing tools or placement system.
		bool Place(std::unique_ptr<Prop> object, const PropPlacementContext& ctx)
		{
			// set the tile constraints of navigation grid where the new object is going to be placed
			for (const Coord& subCoordWorld : ctx.footprintTiles)
			{
				// calculate the actual tile coord in map
				Coord tileCoord = { subCoordWorld.row / 3, subCoordWorld.col / 3 };

				// calculate the sub tile coord relative to this tile
				Coord subCoord = { subCoordWorld.row % 3, subCoordWorld.col % 3 };

				TileConstraint bit = SubCellToConstraint(subCoord.row, subCoord.col);

				// then apply it
				m_navGrid.AddFlag(tileCoord, bit);
			}

			// add the new object into footprint grid
			m_FootPrintGrid.Occupy(object.get(), ctx.footprintTiles);

			// add the new object into bounding box occupancy grid
			m_BoundingBoxGrid.Add(object.get(), ctx.boundingBoxTiles);

			// add the new object into object layer (bucket grid)
			Coord coord = PositionToCoord(ctx.worldPosition, m_tileSize);
			m_objectLayer.Add(coord, std::move(object));

			return true;
		}

		std::vector<Prop*> QueryBoundingBoxOverlaps(const Coord& coord) const
		{
			return m_BoundingBoxGrid.Get(coord);
		}

		std::vector<Prop*> QueryFootprintOverlaps(const std::vector<Coord>& coord) const
		{
			// if there are objects the new footprint overlaps, get their references
			return m_FootPrintGrid.Get(coord);
		}

		// try to get tile coordinate of a given prop. if prop is invalid, 
		bool TryGetCoord(Prop* prop, spatial::Coord& coord) const
		{
			return m_objectLayer.TryGetCoord(prop, coord);
		}

		void Validate() const
		{
			m_navGrid.Validate();
			m_FootPrintGrid.Validate();
			m_BoundingBoxGrid.Validate();
		}

		template<typename Predicate>
		void ForEachProp(int row, int col, Predicate func)
		{
			if (!IsInBounds(row, col)) return;

			m_objectLayer.ForEach(row, col, func);
		}

		template<typename Predicate>
		void ForEachTileConstraint(int row, int col, Predicate func) const
		{
			if (!IsInBounds(row, col)) return;

			m_navGrid.ForEach(row, col, func);
		}

	};

#pragma endregion

#pragma region // PropSelectionSystem
	class PropSelectionSystem
	{
	public:
		static std::vector<Prop*> SelectAtPoint(PropMap& map, const PositionF& worldPosition)
		{
			// is the position within bounds of the world? if not, bail out
			Coord coord = PositionToCoord(worldPosition, map.GetTileSize());

			// get props that occupies the tile coord where the point is. bail out when there is no selected props
			std::vector<Prop*> candidates = map.QueryBoundingBoxOverlaps(coord);
			if (candidates.empty()) return {};

			// if there are objects, iterate through them and find the ones whose bounding box intersects with the cursor position (in world coordinate). return all intersecting objects. 
			std::vector<Prop*> selected{};
			for (Prop* candidate : candidates)
			{
				// find the coordinate of the cell that owns this prop 
				Coord propCoordOwner;
				if (!map.TryGetCoord(candidate, propCoordOwner))
				{
					// if this prop does not exist in the map, throw. this must be a bug. 
					// how did we find pointer to this object in bounding box grid? where is the actual object stored?
					throw std::out_of_range("PropSelectionSystem::SelectAtPoint() - prop does not exist in the map but we have pointer to it. where is it stored?!");
				}

				// get the object's position. the object's position is relative to the cell in the map it is stored. 
				PositionF propPosInTileOwner = candidate->GetPosition(true);

				// get the world position of this object by adding the object's position relative to tile with the top-left position of the tile in world coordinate
				PositionF propPosInWorld = propPosInTileOwner + CoordToPosition(propCoordOwner, map.GetTileSize());

				// get this bounding box of the object in world coordinates
				RectF objectBoundingBox = candidate->GetScaledBoundingBoxWorld(propPosInWorld, true);

				// broad phase collision check: check if the mouse cursor (in world coordinate) is intersecting with the bounding box already in world coordinate. 
				if (!objectBoundingBox.Contains(worldPosition)) continue;

				// TODO:implement narrow phase collision check and execute here

				selected.push_back(candidate);
			}

			return selected;
		}

		static Prop* SelectTopMostAtPoint(PropMap& map, const PositionF& worldPosition)
		{
			// is the position within bounds of the world? if not, bail out
			Coord coord = PositionToCoord(worldPosition, map.GetTileSize());

			// get props that occupies the tile coord where the point is. bail out when there is no selected props
			std::vector<Prop*> candidates = map.QueryBoundingBoxOverlaps(coord);
			if (candidates.empty()) return {};

			// if there are objects, iterate through them and find the ones whose bounding box intersects with the cursor position (in world coordinate). return all intersecting objects. 
			Prop* topMostProp = nullptr;
			RectF topMostPropBoundingBox{};
			for (Prop* candidate : candidates)
			{
				// find the coordinate of the cell that owns this prop 
				Coord propCoordOwner;
				if (!map.TryGetCoord(candidate, propCoordOwner))
				{
					// if this prop does not exist in the map, throw. this must be a bug. 
					// how did we find pointer to this object in bounding box grid? where is the actual object stored?
					throw std::out_of_range("PropSelectionSystem::SelectAtPoint() - prop does not exist in the map but we have pointer to it. where is it stored?!");
				}

				// get the object's position. the object's position is relative to the cell in the map it is stored. 
				PositionF propPosInTileOwner = candidate->GetPosition(true);

				// get the world position of this object by adding the object's position relative to tile with the top-left position of the tile in world coordinate
				PositionF propPosInWorld = propPosInTileOwner + CoordToPosition(propCoordOwner, map.GetTileSize());

				// get this bounding box of the object in world coordinates
				RectF objectBoundingBox = candidate->GetScaledBoundingBoxWorld(propPosInWorld, true);

				// broad phase collision check: check if the mouse cursor (in world coordinate) is intersecting with the bounding box already in world coordinate. 
				if (!objectBoundingBox.Contains(worldPosition)) continue;

				// TODO:implement narrow phase collision check and execute here

				// if so, compare its depth with current candidate for top most and replace candidate if this one is higher (lower on the screen)
				if (topMostProp == nullptr || objectBoundingBox.bottom >= topMostPropBoundingBox.bottom)
				{
					topMostProp = candidate;
					topMostPropBoundingBox = objectBoundingBox;
				}
			}

			return topMostProp;
		}
	};
#pragma endregion

#pragma region // NavigationSystem
	class NavigationSystem
	{

	};
#pragma endregion

#pragma region // WorldMap
	class WorldMap
	{
	private:
		class PropMapView
		{
		private:
			View<PropMap> m_view;

		public:
			PropMapView(PropMap& map) :
				m_view(&map)
			{
			}
		};

	private:
		// top-left position of the world in screen space. this is basically the camera position.
		PositionF m_position;

		// size of the world in tiles
		Size<size_t> m_size;

		// size of each tile in pixels. 
		SizeF m_tileSize;

		PropMap m_propMap;

	public:
		WorldMap() :
			m_position(0, 0),
			m_size(0, 0),
			m_tileSize(0, 0)
		{
		}


		bool Initialize(const PositionF& position, const Size<size_t>& size, const SizeF& tilesize)
		{
			m_position = position;
			m_size = size;
			m_tileSize = tilesize;

			return m_propMap.Initialize(position, size, tilesize);
		}	

		Size<size_t> GetSize() const
		{
			return m_size;
		}

		SizeF GetTileSize() const
		{
			return m_tileSize;
		}

		PositionF GetPosition() const
		{
			return m_position;
		}

		bool IsInBounds(int row, int col) const
		{
			return
				row >= 0 && col >= 0 &&		// make sure rows and columns are not negatives.
				col < m_size.width &&		// make sure column is within the grid's width
				row < m_size.height;	// make sure row is within the grid's height
		}

		bool IsInBounds(const Coord& coord) const
		{
			return IsInBounds(coord.row, coord.col);
		}

		bool IsInBounds(const PositionF& worldPosition) const
		{
			Coord coord = PositionToCoord(worldPosition, m_tileSize);
			return IsInBounds(coord);
		}

		void Validate() const
		{
			m_propMap.Validate();
		}

		template<typename Predicate>
		void ForEachProp(int row, int col, Predicate func)
		{
			m_propMap.ForEachProp(row, col, func);
		}

		template<typename Predicate>
		void ForEachTileConstraint(int row, int col, Predicate func)
		{
			m_propMap.ForEachTileConstraint(row, col, func);
		}

		template<typename Predicate>
		void ForEachTileConstraint(int row, int col, Predicate func) const
		{
			m_propMap.ForEachTileConstraint(row, col, func);
		}

		std::vector<Prop*> QueryFootprintOverlaps(const PositionF& worldPosition) const
		{
			// get the tile coordinate this world position is in
			Coord coord = PositionToCoord(worldPosition, m_tileSize);
		}
	};
#pragma endregion

#pragma region // PropPlacementSystem
	class PropPlacementSystem
	{
	private:
	public:
		PropPlacementSystem()
		{
		}

		static bool Place(WorldMap& map, std::unique_ptr<Prop> prop, const PositionF& worldPosition)
		{
			// validate the position is within bounds of the world. if not, bail out
			if (!map.IsInBounds(worldPosition))
			{
				return false;
			}

			// get prop's footprint in world coordinate
			RectF footprint = prop->GetScaledFootprintWorld(worldPosition);

			// get all the props that overlapped with the footprint

			// if there are any, remove them from the map (for now we just remove everything. later we can have more complex rules/policies about what to remove and what to keep)

			// place the new prop into the map (this will update all necessary systems in the map like navigation grid and occupancy grid)


			return true;
		}

		static bool Remove(WorldMap& map, const PositionF& worldPosition)
		{
			return true;

		}

		void CalculateTileConstraint(const RectF& footprint, const RectF& tile)
		{

		}

		static bool Place(PropMap& map, std::unique_ptr<Prop> prop, const PositionF& worldPosition)
		{
			// 1. build context
			PropPlacementContext ctx;

			ctx.worldPosition = worldPosition;

			// footprint
			RectF footprint = prop->GetScaledFootprintWorld(worldPosition);
			ctx.footprintTiles = GridQuery::QueryCells(footprint, map.GetRefinedTileSize(), map.GetRefinedSize());

			// bounding box
			RectF boundingBox = prop->GetScaledBoundingBoxWorld(worldPosition, true);
			ctx.boundingBoxTiles = GridQuery::QueryCells(boundingBox, map.GetTileSize(), map.GetSize());

			// overlaps
			std::vector<Prop*> overlappedProps = map.FootprintLayer.Get(ctx.footprintTiles); //map.QueryPropsOverlappingTiles(ctx.footprintTiles);

			// get footprint
			// get tile coords that intersects with footprint
			// 
			// for each tile coord...
			// calculate corresponding tile constraint for each tile coord
			// 
			// 

			// 2. validation before we make any changes to the world
			if (!map.IsInBounds(worldPosition))
			{
				return false;
			}

			// 3. remove overlapped objects. this is a game rule. does this decision belong in this class? decide later
			// TODO: turn this into a policy
			for (Prop* p : overlappedProps)
			{
				map.Remove(p);
			}

			// IMPORTANT:
			// after removal, context remains valid because it was precomputed
			// (this is why ctx is immutable snapshot)

			// 4. apply placement (delegate to PropMap)
			map.Place(std::move(prop), ctx);

			return true;
		}

		static bool Remove(PropMap& propmap, const PositionF& worldPosition)
		{
			// Bounds check (early)
			if (!propmap.IsInBounds(worldPosition))
			{
				return false;
			}

			// query top most prop that intersects with this position. if there is none, bail out.
			// TODO: this should be a policy. but for now we implement it here for simplicity. just find the top most prop and remove it.
			Prop* topMostProp = PropSelectionSystem::SelectTopMostAtPoint(propmap, worldPosition);
			if (!topMostProp)
			{
				return false;
			}

			// 4. Apply removal (delegate to PropMap / WorldState)
			propmap.Remove(topMostProp);

			return true;
		}

	};
#pragma endregion

#pragma region // debug scene
	class DebugScene : public Scene
	{
		PositionF m_mousePos;
		SizeF m_footprint;
		RectF m_tile;


	public:
		void OnEnter() override
		{
			m_tile.left = 400;
			m_tile.top = 400;
			m_tile.right = 500;
			m_tile.bottom = 480;

			m_footprint.width = 240;
			m_footprint.height = 160;
		}

		void OnMouseMove(int x, int y) override
		{
			m_mousePos = PositionF((float)x, (float)y);

			//m_tile.left += m_mousePos.x;
			//m_tile.top += m_mousePos.y;
			//m_tile.right += m_mousePos.x;
			//m_tile.bottom += m_mousePos.y;
		}

		void OnUpdate(double dt) override
		{

		}

		void OnRender() override
		{
			AssetManager assets;
			IRenderer& renderer = assets.Get<IRenderer>("renderer");

			renderer.Draw(m_tile.GetTopLeft(), m_tile.GetSize(), { 1,1,1,0.5 }, 0.0f);

			renderer.Draw(m_mousePos, m_footprint, { 1,0,0,0.5 }, 0.0f);

			RectF fp{};
			fp.left = m_mousePos.x - m_tile.left;
			fp.top = m_mousePos.y - m_tile.top;
			fp.right = fp.left + m_footprint.width;
			fp.bottom = fp.top + m_footprint.height;

			SizeF cellsize(m_tile.GetWidth() / 3.0f, m_tile.GetHeight() / 3.0f);
			std::vector<Coord> coords = GridQuery::QueryCells(fp, cellsize, Size<size_t>(3,3));

			SizeF tilesize(m_tile.GetSize());
			for (Coord coord : coords)
			{
				PositionF pos(tilesize.width / 3.0f * coord.col, tilesize.height / 3.0f * coord.row);
				pos += PositionF(m_tile.GetWidth() / 3.0f * 0.125f, m_tile.GetHeight() / 3.0f * 0.125f);
				pos += m_tile.GetTopLeft();

				SizeF size(m_tile.GetWidth() / 3.0f * 0.75f, m_tile.GetHeight() / 3.0f * 0.75f);

				renderer.Draw(pos, size, { 0,0,0,0.5 }, 0.0f);
			}
		}

	};
#pragma endregion

#pragma region // editor scene
	class EditorScene : public Scene
	{
	private:
		StateMachine m_stateMachine;
		AssetManager m_assets;

		Floor m_floor;

		TileLayer m_grassTileLayer;
		TileLayer m_splashTileLayer;
		TileLayer m_gridTileLayer;
		TileLayer m_fineGridTileLayer;
		WorldMap m_worldMap;
		PropMap m_propMap;

		PropBrushTool m_placementTool;
		PropPlacementSystem m_propPlacer;

		PositionF m_mousePos;

		bool m_showDebug = true;


		PropBrush LargePineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush SmallPineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{0.5f, 0.5f}, ColorF{1,1,1,1}, RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush NormalPineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1},RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.2f, 0.2f, 0.8f, 0.92f} };
		PropBrush LargeBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush SmallBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{0.5f, 0.5f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush NormalBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.27f, 0.12f, 0.73f, 0.87f} };
		PropBrush LargeWaterRocks{ "water_rocks_anim_set",	"water_rocks_idle",	VecF{2.0f, 2.0f}, ColorF{1,1,1,1}, RectF{0.1f,0.4f,0.9f,0.8f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };

		PropBrush NormalCastle{ "castle_anim_set", "castle_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1}, RectF{0.05f, 0.6f, 0.95f, 0.90f}, RectF{0.05f, 0.2f, 0.95f, 0.9f} };
		PropBrush LargeCastle { "castle_anim_set", "castle_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.05f, 0.6f, 0.95f, 0.90f}, RectF{0.05f, 0.2f, 0.95f, 0.9f} };

	public:
		void OnEnter() override
		{
			// initialize grass tile layer. fill it with invalid tiles for now so they have empty tiles
			auto& grassTileset = m_assets.Get<Tileset<IRenderable>>("grass_tileset");
			auto& mapSize = m_assets.Get<Size<size_t>>("map_size");
			m_grassTileLayer.tileset = &grassTileset;
			m_grassTileLayer.tilegrid.Initialize(mapSize, grassTileset.MakeInvalidTile());

			// initialize water splash tile layer. also fill with invalid tiles for now
			auto& splashTileset = m_assets.Get<Tileset<IRenderable>>("splash_tileset");
			m_splashTileLayer.tileset = &splashTileset;
			m_splashTileLayer.tilegrid.Initialize(mapSize, splashTileset.MakeInvalidTile());

			// initialize grid tile layer. fill it with its only tile
			m_gridTileLayer.tileset = &grassTileset;
			m_gridTileLayer.tilegrid.Initialize(mapSize, grassTileset.MakeTile(13));

			// initialize fine grid tile layer. fill it with its only tile
			m_fineGridTileLayer.tileset = &grassTileset;
			m_fineGridTileLayer.tilegrid.Initialize(mapSize, grassTileset.MakeTile(22));

			// set placement tool default placement
			m_placementTool.SetBrush(NormalPineTree, m_assets);

			//m_worldMap.Initialize(m_assets.Get<PositionF>("map_position"), m_assets.Get<Size<size_t>>("map_size"), m_assets.Get<SizeF>("tile_size"));
			m_propMap.Initialize(m_assets.Get<PositionF>("map_position"), m_assets.Get<Size<size_t>>("map_size"), m_assets.Get<SizeF>("tile_size"));

		}

		void OnUpdate(double dt) override
		{
			m_stateMachine.OnUpdate(dt);

			// this is for debugging only. validate every frame to ensure our containers are in good state
			m_worldMap.Validate();
		}

		void OnKeyDown(int key) override
		{
			m_stateMachine.OnKeyDown(key);

			switch (key)
			{
			case 27: // ESC
				break;
			case 32: // SPACE
				m_showDebug = !m_showDebug;
				break;
			case 49: // 1
				m_placementTool.SetBrush(NormalPineTree, m_assets);
				break;
			case 50: // 2
				m_placementTool.SetBrush(NormalBirchTree, m_assets);
				break;
			case 51: // 3 
				m_placementTool.SetBrush(SmallPineTree, m_assets);
				break;
			case 52: // 4
				m_placementTool.SetBrush(SmallBirchTree, m_assets);
				break;
			case 53: // 5
				m_placementTool.SetBrush(LargePineTree, m_assets);
				break;
			case 54: // 6
				m_placementTool.SetBrush(LargeBirchTree, m_assets);
				break;
			case 55: // 7
			{
				m_placementTool.SetBrush(NormalCastle, m_assets);
				break;
			}
			case 56: // 8
			{
				m_placementTool.SetBrush(LargeCastle, m_assets);
				break;
			}
			case 57: // 9
			{
				m_placementTool.SetBrush(LargeWaterRocks, m_assets);
				break;
			}

			default:
				break;
			}
		}

		void OnMouseMove(int x, int y) override 
		{
			m_mousePos = PositionF((float)x, (float)y);

			return;

			// is mouse left button is held while moving...
			if (Input::Instance().IsMouseHeld(1))
			{
				// calculate the coord in map the mouse cursor intersect with
				auto& mapPos = m_assets.Get<PositionF>("map_position");
				auto& tilesize = m_assets.Get<SizeF>("tile_size");
				Coord coord = PositionToCoord(m_mousePos - mapPos, tilesize);

				auto& config = m_assets.Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");
				auto& splashAnimLookup = m_assets.Get<Dictionary<TileVariant, int>>("splash_anim_tile_lookup");

				// place grass tile
				TileLayerEditor tle;
				tle.LinkLayers(m_grassTileLayer, m_splashTileLayer, splashAnimLookup);
				tle.Paint(m_grassTileLayer, config, coord);
			}
		}

		void OnMouseUp(int btn, int x, int y) override 
		{
		}

		void OnMouseDown(int btn, int x, int y) override
		{
			m_stateMachine.OnMouseDown(btn, x, y);

			m_mousePos = PositionF((float)x, (float)y);

			// calculate the coord in map the mouse cursor intersect with
			auto& mapPos = m_assets.Get<PositionF>("map_position");
			auto& tilesize = m_assets.Get<SizeF>("tile_size");

			PositionF pos = m_mousePos;
			Coord coord = PositionToCoord(pos - mapPos, tilesize);

			auto& config = m_assets.Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");
			auto& splashAnimLookup = m_assets.Get<Dictionary<TileVariant, int>>("splash_anim_tile_lookup");

			// left click to place grass tile
			if (btn == 1)
			{
				PositionF worldPos = ScreenToWorld(m_mousePos, m_worldMap.GetPosition());
				Coord coord = ScreenToTileCoord(m_mousePos, m_worldMap.GetPosition(), m_worldMap.GetTileSize());
				PositionF tilePos = CoordToPosition(coord, m_worldMap.GetTileSize());

				//PropPlacementSystem::Place(m_worldMap, m_placementTool.CreateAnimatedProp(tilePos, m_assets), worldPos);

				PropPlacementSystem::Place(m_propMap, m_placementTool.CreateAnimatedProp(m_propMap.ScreenToTilePosition(m_mousePos), m_assets), m_propMap.ScreenToWorldPosition(m_mousePos));

				return;

				//// place grass tile
				//TileLayerEditor tle;
				//tle.LinkLayers(m_grassTileLayer, m_splashTileLayer, splashAnimLookup);
				//tle.Paint(m_grassTileLayer, config, coord);
			}
			// right click to remove tile
			else if (btn == 2)
			{
				// immediate goal is to find the top-most object that intersects with the mouse cursor in this cell and remove it.
				PropPlacementSystem::Remove(m_propMap, m_mousePos - mapPos);
				//PropPlacementSystem::Remove(m_worldMap, m_mousePos - mapPos);

				return;

				//// remove grass tile
				//TileLayerEditor tle;
				//tle.LinkLayers(m_grassTileLayer, m_splashTileLayer, splashAnimLookup);
				//tle.Erase(m_grassTileLayer, config, coord);
			}
		}

		void OnRender() override
		{
			m_stateMachine.OnRender();

			// draw tiles in order of their depth (Y) so that tiles with higher Y (lower on the screen) are drawn after 
			// tiles with lower Y (higher on the screen) to create proper overlapping. props will be drawn in between floor 
			// and edge tiles based on their tile constraint, so we draw all floor and edge tiles first, then props, 
			// then debug constraint indicators
			auto& mapLayerRenderer = m_assets.Get<MapLayerRenderer>("renderer");

			// draw water splash tile
			mapLayerRenderer.Clear();
			mapLayerRenderer.QueueAllTilesForDraw(m_splashTileLayer.tilegrid, m_splashTileLayer.tilegrid.GetSize(), 1, {3,3});
			mapLayerRenderer.Sort();
			mapLayerRenderer.Draw();

			// draw grass tile
			mapLayerRenderer.Clear();
			mapLayerRenderer.QueueAllTilesForDraw(m_grassTileLayer.tilegrid, m_grassTileLayer.tilegrid.GetSize(), 1, {1,1});
			mapLayerRenderer.Sort();
			mapLayerRenderer.Draw();

			DrawSortedSpritesCommand& drawCommand = m_assets.Get<DrawSortedSpritesCommand>("drawCommand");
			drawCommand.Clear();
			auto& tilesize = m_assets.Get<SizeF>("tile_size");
			auto& mapPos = m_assets.Get<PositionF>("map_position");
			//for (int row = 0; row < (int)m_worldMap.GetSize().height; row++)
			//{
			//	for (int col = 0; col < (int)m_worldMap.GetSize().width; col++)
			//	{
			//		Coord coord(row, col);
			//		PositionF tileScreenPos = CoordToPosition(coord, tilesize) + mapPos;

			//		m_worldMap.ForEachProp(row, col, [&drawCommand, &tileScreenPos](Prop* prop)
			//			{
			//				prop->QueueForDraw(drawCommand, tileScreenPos, 1);
			//			});
			//	}
			//}

			for (int row = 0; row < (int)m_propMap.m_objectLayer.GetHeight(); row++)
			{
				for (int col = 0; col < (int)m_propMap.m_objectLayer.GetWidth(); col++)
				{
					Coord coord(row, col);
					PositionF tileScreenPos = CoordToPosition(coord, tilesize) + mapPos;

					m_propMap.ForEachProp(row, col, [&drawCommand, &tileScreenPos](Prop* prop)
						{
							prop->QueueForDraw(drawCommand, tileScreenPos, 1);
						});
				}
			}


			m_placementTool.QueuePreviewForDraw(drawCommand, m_mousePos, 1);
			drawCommand.Sort();
			drawCommand.Execute();

			if (m_showDebug)
			{
				RectF fp = m_placementTool.GetPreviewFootprintAt(m_mousePos);
				IRenderer& renderer = m_assets.Get<IRenderer>("renderer");
				DrawQuadCommand cmd(renderer, fp.GetTopLeft(), fp.GetSize(), { 1,1,1,0.5f }, 0.0f);
				cmd.Execute();

				RectF hb = m_placementTool.GetPreviewBoundingBoxAt(m_mousePos);
				DrawQuadCommand cmdBoundingBox(renderer, hb.GetTopLeft(), hb.GetSize(), { 1,0,1,0.5f }, 0.0f);
				cmdBoundingBox.Execute();

				// draw grid tile
				mapLayerRenderer.Clear();
				ColorF color = mapLayerRenderer.GetColor();
				mapLayerRenderer.SetColor({ 0,0,0,0.2f });
				mapLayerRenderer.QueueAllTilesForDraw(m_gridTileLayer.tilegrid, m_gridTileLayer.tilegrid.GetSize(), 1, { 1,1 });
				mapLayerRenderer.Sort();
				mapLayerRenderer.Draw();
				mapLayerRenderer.SetColor(color);

				//// draw fine grid tile
				//mapLayerRenderer.Clear();
				//ColorF color = mapLayerRenderer.GetColor();
				//mapLayerRenderer.SetColor({ 0,0,0,0.2f });
				//mapLayerRenderer.QueueAllTilesForDraw(m_fineGridTileLayer.tilegrid, m_fineGridTileLayer.tilegrid.GetSize(), 1, { 1,1 });
				//mapLayerRenderer.Sort();
				//mapLayerRenderer.Draw();
				//mapLayerRenderer.SetColor(color);

				//DrawNavigationGridOverlay(renderer, m_worldMap);
				DrawNavigationGridOverlay(renderer, m_propMap.m_navGrid, mapPos, m_propMap.GetTileSize());
			}

		}

		void DrawNavigationGridOverlay(
			IRenderer& renderer,
			const WorldMap& map
		)
		{
			struct SubCellOffset
			{
				int row;
				int col;
				TileConstraint bit;
			};

			static const SubCellOffset offsets[9] =
			{
				{ 0, 0, TileConstraint::NW },
				{ 0, 1, TileConstraint::N  },
				{ 0, 2, TileConstraint::NE },

				{ 1, 0, TileConstraint::W  },
				{ 1, 1, TileConstraint::CENTER },
				{ 1, 2, TileConstraint::E  },

				{ 2, 0, TileConstraint::SW },
				{ 2, 1, TileConstraint::S  },
				{ 2, 2, TileConstraint::SE }
			};

			SizeF subTileSize(map.GetTileSize().width / 3.0f, m_worldMap.GetTileSize().height / 3.0f);

			// visual tweak (same as yours)
			VecF shift(subTileSize.width * 0.25f, subTileSize.height * 0.25f);
			SizeF overlaySize(subTileSize.width / 2.0f, subTileSize.height / 2.0f);

			for (int row = 0; row < (int)map.GetSize().height; ++row)
			{
				for (int col = 0; col < (int)map.GetSize().width; ++col)
				{
					map.ForEachTileConstraint(row, col, [row, col, &map, subTileSize, shift, overlaySize, &renderer](TileConstraint constraint)
						{ 
							// skip empty tiles early (fast path)
							if (constraint == TileConstraint::NONE) return;

							// top-left of this tile in world space
							PositionF tileWorldPos = CoordToPosition({ row, col }, map.GetTileSize()) + map.GetPosition();

							// iterate 3x3 subcells
							for (const auto& offset : offsets)
							{
								// for this subcell, check if its corresponding constraint bit is set. if not, skip						
								if (!HasFlag(constraint, offset.bit))
								{
									continue;
								}

								// compute subcell position
								PositionF subCellPos = tileWorldPos;
								subCellPos.x += offset.col * subTileSize.width;
								subCellPos.y += offset.row * subTileSize.height;

								// apply shift. we're rendering a rectangle smaller than the actual size of the subcell so we shift it a bit to center it
								subCellPos += shift;

								// draw
								renderer.Draw(
									subCellPos,
									overlaySize,
									{ 0, 0, 0, 0.5f },
									0.0f
								);
							}
						}
					);
				}
			}
		}

		void DrawNavigationGridOverlay(
			IRenderer& renderer,
			const NavigationGrid& navGrid,
			const PositionF& mapPos,
			const SizeF& tileSize)
		{
			struct SubCellOffset
			{
				int row;
				int col;
				TileConstraint bit;
			};

			static const SubCellOffset offsets[9] =
			{
				{ 0, 0, TileConstraint::NW },
				{ 0, 1, TileConstraint::N  },
				{ 0, 2, TileConstraint::NE },

				{ 1, 0, TileConstraint::W  },
				{ 1, 1, TileConstraint::CENTER },
				{ 1, 2, TileConstraint::E  },

				{ 2, 0, TileConstraint::SW },
				{ 2, 1, TileConstraint::S  },
				{ 2, 2, TileConstraint::SE }
			};

			SizeF subTileSize(tileSize.width / 3.0f, tileSize.height / 3.0f);

			// visual tweak (same as yours)
			VecF shift(subTileSize.width * 0.25f, subTileSize.height * 0.25f);
			SizeF overlaySize(subTileSize.width / 2.0f, subTileSize.height / 2.0f);

			for (int row = 0; row < (int)navGrid.GetHeight(); ++row)
			{
				for (int col = 0; col < (int)navGrid.GetWidth(); ++col)
				{
					TileConstraint constraint = navGrid.Get(row, col);

					// skip empty tiles early (fast path)
					if (constraint == TileConstraint::NONE)
					{
						continue;
					}

					// top-left of this tile in world space
					PositionF tileWorldPos = CoordToPosition({ row, col }, tileSize) + mapPos;

					// iterate 3x3 subcells
					for (const auto& offset : offsets)
					{
						// for this subcell, check if its corresponding constraint bit is set. if not, skip						
						if (!HasFlag(constraint, offset.bit))
						{
							continue;
						}

						// compute subcell position
						PositionF subCellPos = tileWorldPos;
						subCellPos.x += offset.col * subTileSize.width;
						subCellPos.y += offset.row * subTileSize.height;

						// apply shift. we're rendering a rectangle smaller than the actual size of the subcell so we shift it a bit to center it
						subCellPos += shift;

						// draw
						renderer.Draw(
							subCellPos,
							overlaySize,
							{ 0, 0, 0, 0.5f },
							0.0f
						);
					}
				}
			}
		}
	};
#pragma endregion
	
#pragma region // another editor scene
	class AnotherEditorScene : public Scene
	{
	private:
		StateMachine m_stateMachine;
		AssetManager m_assets;
		TileLayer m_gridTileLayer;
		TileLayer m_fineGridTileLayer;
		PropMap1 m_propMap;

		PropBrushTool m_placementTool;
		PropPlacementSystem m_propPlacer;

		PositionF m_mousePos;

		bool m_showDebug = true;


		PropBrush LargePineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush SmallPineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{0.5f, 0.5f}, ColorF{1,1,1,1}, RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush NormalPineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1},RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.2f, 0.2f, 0.8f, 0.92f} };
		PropBrush LargeBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush SmallBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{0.5f, 0.5f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush NormalBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.27f, 0.12f, 0.73f, 0.87f} };
		PropBrush LargeWaterRocks{ "water_rocks_anim_set",	"water_rocks_idle",	VecF{2.0f, 2.0f}, ColorF{1,1,1,1}, RectF{0.1f,0.4f,0.9f,0.8f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };

		PropBrush NormalCastle{ "castle_anim_set", "castle_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1}, RectF{0.05f, 0.6f, 0.95f, 0.90f}, RectF{0.05f, 0.2f, 0.95f, 0.9f} };
		PropBrush LargeCastle{ "castle_anim_set", "castle_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.05f, 0.6f, 0.95f, 0.90f}, RectF{0.05f, 0.2f, 0.95f, 0.9f} };

	public:
		void OnEnter() override
		{
			// set placement tool default placement
			m_placementTool.SetBrush(NormalPineTree, m_assets);

			//m_worldMap.Initialize(m_assets.Get<PositionF>("map_position"), m_assets.Get<Size<size_t>>("map_size"), m_assets.Get<SizeF>("tile_size"));
			m_propMap.Initialize(m_assets.Get<PositionF>("map_position"), m_assets.Get<Size<size_t>>("map_size"), m_assets.Get<SizeF>("tile_size"));

			// initialize grid tile layer. fill it with its only tile
			auto& grassTileset = m_assets.Get<Tileset<IRenderable>>("grass_tileset");
			auto& mapSize = m_assets.Get<Size<size_t>>("map_size");
			m_gridTileLayer.tileset = &grassTileset;
			m_gridTileLayer.tilegrid.Initialize(mapSize, grassTileset.MakeTile(13));

			// initialize fine grid tile layer. fill it with its only tile
			m_fineGridTileLayer.tileset = &grassTileset;
			m_fineGridTileLayer.tilegrid.Initialize(mapSize, grassTileset.MakeTile(22));

		}

		void OnUpdate(double dt) override
		{
			m_stateMachine.OnUpdate(dt);

			// this is for debugging only. validate every frame to ensure our containers are in good state
			m_propMap.Validate();
		}

		void OnKeyDown(int key) override
		{
			m_stateMachine.OnKeyDown(key);

			switch (key)
			{
			case 27: // ESC
				break;
			case 32: // SPACE
				m_showDebug = !m_showDebug;
				break;
			case 49: // 1
				m_placementTool.SetBrush(NormalPineTree, m_assets);
				break;
			case 50: // 2
				m_placementTool.SetBrush(NormalBirchTree, m_assets);
				break;
			case 51: // 3 
				m_placementTool.SetBrush(SmallPineTree, m_assets);
				break;
			case 52: // 4
				m_placementTool.SetBrush(SmallBirchTree, m_assets);
				break;
			case 53: // 5
				m_placementTool.SetBrush(LargePineTree, m_assets);
				break;
			case 54: // 6
				m_placementTool.SetBrush(LargeBirchTree, m_assets);
				break;
			case 55: // 7
			{
				m_placementTool.SetBrush(NormalCastle, m_assets);
				break;
			}
			case 56: // 8
			{
				m_placementTool.SetBrush(LargeCastle, m_assets);
				break;
			}
			case 57: // 9
			{
				m_placementTool.SetBrush(LargeWaterRocks, m_assets);
				break;
			}

			default:
				break;
			}
		}

		void OnMouseMove(int x, int y) override
		{
			m_mousePos = PositionF((float)x, (float)y);

			return;

			// is mouse left button is held while moving...
			if (Input::Instance().IsMouseHeld(1))
			{
				// calculate the coord in map the mouse cursor intersect with
				auto& mapPos = m_assets.Get<PositionF>("map_position");
				auto& tilesize = m_assets.Get<SizeF>("tile_size");
				Coord coord = PositionToCoord(m_mousePos - mapPos, tilesize);

				auto& config = m_assets.Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");
				auto& splashAnimLookup = m_assets.Get<Dictionary<TileVariant, int>>("splash_anim_tile_lookup");
			}
		}

		void OnMouseUp(int btn, int x, int y) override
		{
		}

		void OnMouseDown(int btn, int x, int y) override
		{
			m_stateMachine.OnMouseDown(btn, x, y);

			m_mousePos = PositionF((float)x, (float)y);

			// calculate the coord in map the mouse cursor intersect with
			auto& mapPos = m_assets.Get<PositionF>("map_position");
			auto& tilesize = m_assets.Get<SizeF>("tile_size");

			PositionF pos = m_mousePos;
			Coord coord = PositionToCoord(pos - mapPos, tilesize);

			auto& config = m_assets.Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");
			auto& splashAnimLookup = m_assets.Get<Dictionary<TileVariant, int>>("splash_anim_tile_lookup");

			// left click to place grass tile
			if (btn == 1)
			{
				PositionF worldPos = ScreenToWorld(m_mousePos, m_propMap.GetPosition());
				m_propMap.Place(m_placementTool.CreateAnimatedProp({}, m_assets), worldPos);

				return;
			}
			// right click to remove tile
			else if (btn == 2)
			{
				// immediate goal is to find the top-most object that intersects with the mouse cursor in this cell and remove it.
				PositionF worldPos = ScreenToWorld(m_mousePos, m_propMap.GetPosition());
				m_propMap.Remove(worldPos);

				return;
			}
		}

		void OnRender() override
		{
			m_stateMachine.OnRender();

			// draw tiles in order of their depth (Y) so that tiles with higher Y (lower on the screen) are drawn after 
			// tiles with lower Y (higher on the screen) to create proper overlapping. props will be drawn in between floor 
			// and edge tiles based on their tile constraint, so we draw all floor and edge tiles first, then props, 
			// then debug constraint indicators
			auto& mapLayerRenderer = m_assets.Get<MapLayerRenderer>("renderer");

			DrawSortedSpritesCommand& drawCommand = m_assets.Get<DrawSortedSpritesCommand>("drawCommand");
			drawCommand.Clear();
			auto& tilesize = m_assets.Get<SizeF>("tile_size");
			auto& mapPos = m_assets.Get<PositionF>("map_position");

			for (int row = 0; row < (int)m_propMap.m_objectLayer.GetHeight(); row++)
			{
				for (int col = 0; col < (int)m_propMap.m_objectLayer.GetWidth(); col++)
				{
					Coord coord(row, col);
					PositionF tileScreenPos = CoordToPosition(coord, tilesize) + mapPos;

					m_propMap.ForEachProp(row, col, [&drawCommand, &tileScreenPos](Prop* prop)
						{
							prop->QueueForDraw(drawCommand, tileScreenPos, 1);
						});
				}
			}


			m_placementTool.QueuePreviewForDraw(drawCommand, m_mousePos, 1);
			drawCommand.Sort();
			drawCommand.Execute();

			if (m_showDebug)
			{
				RectF fp = m_placementTool.GetPreviewFootprintAt(m_mousePos);
				IRenderer& renderer = m_assets.Get<IRenderer>("renderer");
				DrawQuadCommand cmd(renderer, fp.GetTopLeft(), fp.GetSize(), { 1,1,1,0.5f }, 0.0f);
				cmd.Execute();

				RectF hb = m_placementTool.GetPreviewBoundingBoxAt(m_mousePos);
				DrawQuadCommand cmdBoundingBox(renderer, hb.GetTopLeft(), hb.GetSize(), { 1,0,1,0.5f }, 0.0f);
				cmdBoundingBox.Execute();

				// draw grid tile
				mapLayerRenderer.Clear();
				ColorF color = mapLayerRenderer.GetColor();
				mapLayerRenderer.SetColor({ 0,0,0,0.2f });
				mapLayerRenderer.QueueAllTilesForDraw(m_gridTileLayer.tilegrid, m_gridTileLayer.tilegrid.GetSize(), 1, { 1,1 });
				mapLayerRenderer.Sort();
				mapLayerRenderer.Draw();
				mapLayerRenderer.SetColor(color);

				//// draw fine grid tile
				//mapLayerRenderer.Clear();
				//ColorF color = mapLayerRenderer.GetColor();
				//mapLayerRenderer.SetColor({ 0,0,0,0.2f });
				//mapLayerRenderer.QueueAllTilesForDraw(m_fineGridTileLayer.tilegrid, m_fineGridTileLayer.tilegrid.GetSize(), 1, { 1,1 });
				//mapLayerRenderer.Sort();
				//mapLayerRenderer.Draw();
				//mapLayerRenderer.SetColor(color);

				//DrawNavigationGridOverlay(renderer, m_worldMap);
				DrawNavigationGridOverlay(renderer, m_propMap.m_navGrid, mapPos, m_propMap.GetTileSize());

				std::string msg = m_propMap.GetDebugInfo();

				renderer.Draw(m_assets.Get<IFontAtlas>("font"), msg, { 400, 5 }, { 1,1,1,1 });

			}

		}

		void DrawNavigationGridOverlay(
			IRenderer& renderer,
			const WorldMap& map
		)
		{
			struct SubCellOffset
			{
				int row;
				int col;
				TileConstraint bit;
			};

			static const SubCellOffset offsets[9] =
			{
				{ 0, 0, TileConstraint::NW },
				{ 0, 1, TileConstraint::N  },
				{ 0, 2, TileConstraint::NE },

				{ 1, 0, TileConstraint::W  },
				{ 1, 1, TileConstraint::CENTER },
				{ 1, 2, TileConstraint::E  },

				{ 2, 0, TileConstraint::SW },
				{ 2, 1, TileConstraint::S  },
				{ 2, 2, TileConstraint::SE }
			};

			SizeF subTileSize(map.GetTileSize().width / 3.0f, map.GetTileSize().height / 3.0f);

			// visual tweak (same as yours)
			VecF shift(subTileSize.width * 0.25f, subTileSize.height * 0.25f);
			SizeF overlaySize(subTileSize.width / 2.0f, subTileSize.height / 2.0f);

			for (int row = 0; row < (int)map.GetSize().height; ++row)
			{
				for (int col = 0; col < (int)map.GetSize().width; ++col)
				{
					map.ForEachTileConstraint(row, col, [row, col, &map, subTileSize, shift, overlaySize, &renderer](TileConstraint constraint)
						{
							// skip empty tiles early (fast path)
							if (constraint == TileConstraint::NONE) return;

							// top-left of this tile in world space
							PositionF tileWorldPos = CoordToPosition({ row, col }, map.GetTileSize()) + map.GetPosition();

							// iterate 3x3 subcells
							for (const auto& offset : offsets)
							{
								// for this subcell, check if its corresponding constraint bit is set. if not, skip						
								if (!HasFlag(constraint, offset.bit))
								{
									continue;
								}

								// compute subcell position
								PositionF subCellPos = tileWorldPos;
								subCellPos.x += offset.col * subTileSize.width;
								subCellPos.y += offset.row * subTileSize.height;

								// apply shift. we're rendering a rectangle smaller than the actual size of the subcell so we shift it a bit to center it
								subCellPos += shift;

								// draw
								renderer.Draw(
									subCellPos,
									overlaySize,
									{ 0, 0, 0, 0.5f },
									0.0f
								);
							}
						}
					);
				}
			}
		}

		void DrawNavigationGridOverlay(
			IRenderer& renderer,
			const NavigationGrid& navGrid,
			const PositionF& mapPos,
			const SizeF& tileSize)
		{
			struct SubCellOffset
			{
				int row;
				int col;
				TileConstraint bit;
			};

			static const SubCellOffset offsets[9] =
			{
				{ 0, 0, TileConstraint::NW },
				{ 0, 1, TileConstraint::N  },
				{ 0, 2, TileConstraint::NE },

				{ 1, 0, TileConstraint::W  },
				{ 1, 1, TileConstraint::CENTER },
				{ 1, 2, TileConstraint::E  },

				{ 2, 0, TileConstraint::SW },
				{ 2, 1, TileConstraint::S  },
				{ 2, 2, TileConstraint::SE }
			};

			SizeF subTileSize(tileSize.width / 3.0f, tileSize.height / 3.0f);

			// visual tweak (same as yours)
			VecF shift(subTileSize.width * 0.25f, subTileSize.height * 0.25f);
			SizeF overlaySize(subTileSize.width / 2.0f, subTileSize.height / 2.0f);

			for (int row = 0; row < (int)navGrid.GetHeight(); ++row)
			{
				for (int col = 0; col < (int)navGrid.GetWidth(); ++col)
				{
					TileConstraint constraint = navGrid.Get(row, col);

					// skip empty tiles early (fast path)
					if (constraint == TileConstraint::NONE)
					{
						continue;
					}

					// top-left of this tile in world space
					PositionF tileWorldPos = CoordToPosition({ row, col }, tileSize) + mapPos;

					// iterate 3x3 subcells
					for (const auto& offset : offsets)
					{
						// for this subcell, check if its corresponding constraint bit is set. if not, skip						
						if (!HasFlag(constraint, offset.bit))
						{
							continue;
						}

						// compute subcell position
						PositionF subCellPos = tileWorldPos;
						subCellPos.x += offset.col * subTileSize.width;
						subCellPos.y += offset.row * subTileSize.height;

						// apply shift. we're rendering a rectangle smaller than the actual size of the subcell so we shift it a bit to center it
						subCellPos += shift;

						// draw
						renderer.Draw(
							subCellPos,
							overlaySize,
							{ 0, 0, 0, 0.5f },
							0.0f
						);
					}
				}
			}
		}
	};
#pragma endregion

	class Test
	{
	private:
		SceneManager m_sceneManager;
		PositionF m_mousePos;

	public:
		Test()
		{
			Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			Window::OnExit += event::Handler(this, &Test::OnExit);
			Window::OnIdle += event::Handler(this, &Test::OnIdle);
			Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create window object
			Registry<IWindow>::Instance().Register("window", std::make_unique<win32::Window>());
			IWindow& window = Registry<IWindow>::Instance().Get("window");

			// subcribe to window events
			window.OnClose += event::Handler(this, &Test::OnWindowClose);
			window.OnCreate += event::Handler(this, &Test::OnWindowCreate);
			window.OnSize += event::Handler(this, &Test::OnWindowSize);
			window.OnWindowMessage += event::Handler(&Input::Instance(), &Input::ProcessWin32Message);
			window.Create(L"Test Map Editor", 1400, 900);

			// subscribe to input events
			Input::Instance().KeyDownEvent += event::Handler(this, &Test::OnKeyDown);
			Input::Instance().MouseDownEvent += event::Handler(this, &Test::OnMouseDown);
			Input::Instance().MouseMoveEvent += event::Handler(this, &Test::OnMouseMove);

			// scene manager to subscribe to input events
			Input::Instance().KeyDownEvent += event::Handler(&m_sceneManager, &SceneManager::OnKeyDown);
			Input::Instance().KeyUpEvent += event::Handler(&m_sceneManager, &SceneManager::OnKeyUp);
			Input::Instance().MouseDownEvent += event::Handler(&m_sceneManager, &SceneManager::OnMouseDown);
			Input::Instance().MouseUpEvent += event::Handler(&m_sceneManager, &SceneManager::OnMouseUp);
			Input::Instance().MouseMoveEvent += event::Handler(&m_sceneManager, &SceneManager::OnMouseMove);

			// hide cursor
			window.ShowCursor(true);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			// without config cache, this defaults to DX11 canvas
			CanvasFactory::Create("canvas");
			ICanvas& canvas = Registry<ICanvas>::Instance().Get("canvas");
			canvas.Initialize(hWnd);
			canvas.SetViewPort();

			// without config cache, this defaults to DX11 renderer batch
			RendererFactory::Create("renderer");
			IRenderer& renderer = Registry<IRenderer>::Instance().Get("renderer");
			renderer.Initialize();

			// create our utility font atlas for drawing text
			FontFactory::Create("font", "Terminal", 12);

			// setup stopwatch to manage timing and start it
			Registry<StopWatch>::Instance().Register("stopwatch", std::make_unique<StopWatch>());
			Registry<StopWatch>::Instance().Get("stopwatch").OnLap += event::Handler(this, &Test::OnLap);
			Registry<StopWatch>::Instance().Get("stopwatch").Start();

			// set map parameters
			{
				Registry<SizeF>::Instance().Register("tile_size", std::make_unique<SizeF>(64.0f, 64.0f));
				Registry<PositionF>::Instance().Register("map_position", std::make_unique<PositionF>(50.0f, 50.0f));
				Registry<Size<size_t>>::Instance().Register("map_size", std::make_unique<Size<size_t>>(20, 12));
				Registry<PositionF>::Instance().Register("depth", std::make_unique<PositionF>(0.0f, 64.0f));
			}

			// create all resources here 
			{
				AssetManager assets;

				// load sprite atlas for baselayer tiles
				SpriteAtlasLoader::Load("grass_tile_sprites", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9);

				// create our tileset and load all sprites from sprite atlas
				TilesetLoader::Load("grass_tileset", "grass_tile_sprites");

				// create sprite atlas for the water splash animation
				SpriteAtlasFactory::Create("splash_anim_sprites", L"../Assets/3072x192px_1x17tile_waterfoam.png", 1, 16);

				// create animation set where we will store animated tiles like splash animation
				Registry<AnimationSet<Sprite>>::Instance().Register("splash_anim_set", std::make_unique<AnimationSet<Sprite>>());

				// create animation for splash and store in animation set
				ISpriteAtlas& splashAnimSprites = assets.Get<ISpriteAtlas>("splash_anim_sprites");
				AnimationSet<Sprite>& splashAnimSet = assets.Get<AnimationSet<Sprite>>("splash_anim_set");
				splashAnimSet.Register("splash_anim", AnimationFactory::Create(splashAnimSprites, 100.0f, true, { .33f, .355f }));

				// create tileset that stores water splash animation
				Registry<Tileset<IRenderable>>::Instance().Register("splash_tileset", std::make_unique<Tileset<IRenderable>>());
				Tileset<IRenderable>& splashTileset = assets.Get<Tileset<IRenderable>>("splash_tileset");
				splashTileset.Register(0, std::make_unique<Animated>(splashAnimSet, "splash_anim"));

				// create auto tile config for base grass tile
				Registry<AutoTileSystem::AutoTileConfig>::Instance().Register("grass_tile_auto_config", std::make_unique<AutoTileSystem::AutoTileConfig>());
				AutoTileSystem::AutoTileConfig& config = assets.Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");

				// configure base layer auto-tile mapping
				config.Register(4, TileVariant::Empty);
				config.Register(30, TileVariant::Island);
				config.Register(10, TileVariant::Full);
				config.Register(21, TileVariant::NorthEdge);
				config.Register(3, TileVariant::SouthEdge);
				config.Register(29, TileVariant::EastEdge);
				config.Register(27, TileVariant::WestEdge);
				config.Register(0, TileVariant::NECorner);
				config.Register(2, TileVariant::NWCorner);
				config.Register(18, TileVariant::SECorner);
				config.Register(20, TileVariant::SWCorner);
				config.Register(12, TileVariant::Vertical);
				config.Register(28, TileVariant::Horizontal);
				config.Register(1, TileVariant::TNorth);
				config.Register(19, TileVariant::TSouth);
				config.Register(9, TileVariant::TEast);
				config.Register(11, TileVariant::TWest);

				Registry<Dictionary<TileVariant, int>>::Instance().Register("splash_anim_tile_lookup", std::make_unique<Dictionary<TileVariant, int>>());
				Dictionary<TileVariant, int>& lookup = assets.Get<Dictionary<TileVariant, int>>("splash_anim_tile_lookup");

				// map the lookup for splash tile layer
				lookup.Register(TileVariant::Empty, -1);
				lookup.Register(TileVariant::Island, 0);
				lookup.Register(TileVariant::Full, -1);
				lookup.Register(TileVariant::NorthEdge, 0);
				lookup.Register(TileVariant::SouthEdge, 0);
				lookup.Register(TileVariant::EastEdge, 0);
				lookup.Register(TileVariant::WestEdge, 0);
				lookup.Register(TileVariant::NECorner, 0);
				lookup.Register(TileVariant::NWCorner, 0);
				lookup.Register(TileVariant::SECorner, 0);
				lookup.Register(TileVariant::SWCorner, 0);
				lookup.Register(TileVariant::Vertical, 0);
				lookup.Register(TileVariant::Horizontal, 0);
				lookup.Register(TileVariant::TNorth, 0);
				lookup.Register(TileVariant::TSouth, 0);
				lookup.Register(TileVariant::TEast, 0);
				lookup.Register(TileVariant::TWest, 0);

				// create sprites for tree props
				SpriteAtlasFactory::Create("birch_tree", L"../Assets/tree_1x8_1536x192.png", 1, 8); // birch tree
				SpriteAtlasFactory::Create("pine_tree", L"../Assets/tree_1x8_1536x256.png", 1, 8); // pine tree

				// create animation set for birch tree
				Registry<AnimationSet<Sprite>>::Instance().Register("birchtree_anim_set", std::make_unique<AnimationSet<Sprite>>());
				auto& birchTreeAnimSet = assets.Get<AnimationSet<Sprite>>("birchtree_anim_set");

				// create "idle" animation for birch tree and register in its animation set
				auto& birchTreeAtlas = assets.Get<ISpriteAtlas>("birch_tree");
				birchTreeAnimSet.Register("birch_tree_idle", AnimationFactory::Create(birchTreeAtlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.85f }));

				// create animation set for pine tree
				Registry<AnimationSet<Sprite>>::Instance().Register("pinetree_anim_set", std::make_unique<AnimationSet<Sprite>>());
				auto& pineTreeAnimSet = assets.Get<AnimationSet<Sprite>>("pinetree_anim_set");

				// create "idle" animation for pine tree and register in its animation set
				auto& pineTreeAtlas = assets.Get<ISpriteAtlas>("pine_tree");
				pineTreeAnimSet.Register("pine_tree_idle", AnimationFactory::Create(pineTreeAtlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.91f }));

				SpriteAtlasFactory::Create("birch_tree", L"../Assets/tree_1x8_1536x192.png", 1, 8); // birch tree

				// setup castle
				SpriteAtlasFactory::Create("castle", L"../Assets/Castle.png", 1, 1); // castle
				Registry<AnimationSet<Sprite>>::Instance().Register("castle_anim_set", std::make_unique<AnimationSet<Sprite>>());
				auto& castleAnimSet = assets.Get<AnimationSet<Sprite>>("castle_anim_set");
				auto& castleAtlas = assets.Get<ISpriteAtlas>("castle");
				castleAnimSet.Register("castle_idle", AnimationFactory::Create(castleAtlas, std::vector<int>{ 0 }, 200.0f, true, PositionF{ 0.5f, 0.95f }));

				// setup water rocks
				SpriteAtlasFactory::Create("water_rocks", L"../Assets/Water_Rocks.png", 1, 16); // castle
				Registry<AnimationSet<Sprite>>::Instance().Register("water_rocks_anim_set", std::make_unique<AnimationSet<Sprite>>());
				auto& waterRocksAnimSet = assets.Get<AnimationSet<Sprite>>("water_rocks_anim_set");
				auto& waterRockAtlas = assets.Get<ISpriteAtlas>("water_rocks");
				waterRocksAnimSet.Register("water_rocks_idle", AnimationFactory::Create(waterRockAtlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 ,14 ,15 }, 100.0f, true, PositionF{ 0.5f, 0.8f }));

				
			}

			// initialize scenes
			{
				m_sceneManager.CreateScene<EditorScene>("Edit");
				m_sceneManager.CreateScene<AnotherEditorScene>("AnotherEdit");
				m_sceneManager.CreateScene<DebugScene>("Debug");
				m_sceneManager.SetActive("AnotherEdit");
			}

			// create draw command for drawing sorted sprites. we will use this for drawing the base layer tiles.
			// we set its capacity to be same as number of tiles in map for now, but we can increase this if we want to draw other stuff with it
			{
				IRenderer& renderer = Registry<IRenderer>::Instance().Get("renderer");
				Registry<DrawSortedSpritesCommand>::Instance().Register("drawCommand", std::make_unique<DrawSortedSpritesCommand>(renderer, 256));
				DrawSortedSpritesCommand& drawCommand = Registry<DrawSortedSpritesCommand>::Instance().Get("drawCommand");

				
				// get map parameters
				SizeF tileSize = Registry<SizeF>::Instance().Get("tile_size");
				Size<size_t> mapSize = Registry<Size<size_t>>::Instance().Get("map_size");
				PositionF pos = Registry<PositionF>::Instance().Get("map_position");

				Registry<MapLayerRenderer>::Instance().Register("renderer", std::make_unique<MapLayerRenderer>(tileSize, pos, drawCommand, ColorF{ 1,1,1,1 }));
			}
		}

		void OnKeyDown(int key)
		{ 
		//	m_sceneManager.OnKeyDown(key);
			return;

			//BaseLayer& baseLayer = Registry<BaseLayer>::Instance().Get("baselayer");
			TileLayer& layer = Registry<TileLayer>::Instance().Get("grass_layer");
			Size<size_t> mapSize = Registry<Size<size_t>>::Instance().Get("map_size");
			AutoTileSystem::AutoTileConfig config = Registry<AutoTileSystem::AutoTileConfig>::Instance().Get("grass_tile");

			AssetManager assets;
			auto& splashAnimLookup = assets.Get<Dictionary<TileVariant, int>>("splash");
			auto& splashLayer = assets.Get<TileLayer>("splash_layer");
			auto& grassLayer = assets.Get<TileLayer>("grass_layer");

			switch (key)
			{
			case 27: // ESC
				break;
			case 32: // SPACE
				break;
			case 49: // 1
				// modify map state 

				break;
			case 50: // 2
				break;
			case 51: // 3 
				break;
			case 52: // 4
				break;
			case 53: // 5
				break;
			case 54: // 6
				break;
			case 55: // 7
			{
				TileLayerEditor tle;
				tle.LinkLayers(grassLayer, splashLayer, splashAnimLookup);
				tle.Fill(grassLayer, config);
				break;
			}

			case 56: // 8
			{
				TileLayerEditor tle;
				tle.LinkLayers(grassLayer, splashLayer, splashAnimLookup);
				tle.Clear(grassLayer, config);
				break;
			}

			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
		//	m_sceneManager.OnMouseDown(btn, x, y);
			return;

			// get coordinate of the tile we clicked on
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
			PositionF mousePos(static_cast<float>(x), static_cast<float>(y));
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(mousePos - mapPos, tilesize);

			//BaseLayer& baseLayer = Registry<BaseLayer>::Instance().Get("baselayer");
			TileLayer& grassLayer = Registry<TileLayer>::Instance().Get("grass_layer");
			AutoTileSystem::AutoTileConfig config = Registry<AutoTileSystem::AutoTileConfig>::Instance().Get("grass_tile");
			AssetManager assets;

			Dictionary<TileVariant, int>& splashAnimLookup = assets.Get<Dictionary<TileVariant, int>>("splash");
			TileLayer& splashLayer = assets.Get<TileLayer>("splash_layer");

			// left click to place tile
			if (btn == 1)
			{
				// place grass tile
				TileLayerEditor tle;
				tle.LinkLayers(grassLayer, splashLayer, splashAnimLookup);
				tle.Paint(grassLayer, config, coord);
			}
			// right click to remove tile
			else if (btn == 2)
			{
				// remove grass tile
				TileLayerEditor tle;
				tle.LinkLayers(grassLayer, splashLayer, splashAnimLookup);
				tle.Erase(grassLayer, config, coord);
			}
		}


		void OnMouseMove(int x, int y)
		{
		//	m_sceneManager.OnMouseMove(x, y);
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			AnimationSystemCache<Sprite>::Instance().Update(time);

			m_sceneManager.OnUpdate(time);
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			Registry<StopWatch>::Instance().Get("stopwatch").Lap<engine::timer::milliseconds>();

			// update input to trigger input events
			Input::Instance().Update();

			// get canvas and clear it with a nice color
			ICanvas& canvas = Registry<ICanvas>::Instance().Get("canvas");
			canvas.Clear({ 0.5f, 0.5f, 0.7f, 1.0f });

			// draw within canvas context
			canvas.Begin();
			{
				IRenderer& renderer = Registry<IRenderer>::Instance().Get("renderer");
				renderer.Begin();
				{
					m_sceneManager.OnRender();

					//// get map parameters
					//SizeF tileSize = Registry<SizeF>::Instance().Get("tile_size");
					//Size<size_t> mapSize = Registry<Size<size_t>>::Instance().Get("map_size");

					//// draw tiles in order of their depth (Y) so that tiles with higher Y (lower on the screen) are drawn after 
					//// tiles with lower Y (higher on the screen) to create proper overlapping. props will be drawn in between floor 
					//// and edge tiles based on their tile constraint, so we draw all floor and edge tiles first, then props, 
					//// then debug constraint indicators
					//MapLayerRenderer& mapLayerRenderer = Registry<MapLayerRenderer>::Instance().Get("renderer");

					//{
					//	mapLayerRenderer.Clear();
					//	TileLayer& layer = Registry<TileLayer>::Instance().Get("splash_layer");
					//	mapLayerRenderer.QueueAllTilesForDraw(layer.tilegrid, mapSize, 1, { 3,3 });
					//	mapLayerRenderer.Sort();
					//	mapLayerRenderer.Draw();
					//}

					//{
					//	mapLayerRenderer.Clear();
					//	TileLayer& layer = Registry<TileLayer>::Instance().Get("grass_layer");
					//	mapLayerRenderer.QueueAllTilesForDraw(layer.tilegrid, mapSize, 1, { 1,1 });
					//	mapLayerRenderer.Sort();
					//	mapLayerRenderer.Draw();
					//}

				}
				renderer.End();
			}
			canvas.End();	
		}

		void OnExit()
		{

		}

		void OnWindowClose()
		{
		}

		void OnWindowSize(size_t nWidth, size_t nHeight)
		{

		}




	};
}