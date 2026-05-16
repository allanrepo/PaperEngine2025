#pragma once
#include <IO/CSVFileParser.h>
#include <Containers/InstanceGrid.h>
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
#include <Engine/Factory/SpriteAnimationFactory.h>
#include <memory>
#include <functional>
#include <Spatial/ObjectGrid.h>
#include <Algorithm/Pathfinding.h>
#include <Core/View.h>
#include <Engine/Loader/SpriteAtlasLoader.h>
#include <Spatial/SpatialOccupancyGrid.h>
#include <Scene/Scene.h>

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
	using SpriteAnimationFactory = engine::graphics::factory::SpriteAnimationFactory;
	using VecF = engine::math::VecF;
	using InputEvent = engine::input::InputEvent;
	using TileConstraint = engine::navigation::tile::TileConstraint;
	using RectF = engine::math::geometry::RectF;
	using CSVFileParser = engine::io::CSVFileParser;
	using NavigationGrid = engine::navigation::tile::NavigationGrid;
	using SpriteAtlasLoader = engine::graphics::loader::SpriteAtlasLoader;
	using Scene = engine::scene::Scene;
	using SceneManager = engine::scene::SceneManager;

	template<typename T, typename U>
	using SpatialOccupancyGrid = engine::spatial::SpatialOccupancyGrid<T, U>;

	template<typename T>
	using View = engine::core::View<T>;

	template<typename T>
	using Grid = engine::container::Grid<T>;

	template<typename T>
	using InstanceGrid = engine::container::InstanceGrid<T>;

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

#pragma region // TileDefinition
	struct TileDefinition
	{
		std::unique_ptr<IRenderable> renderable = nullptr;
	};
#pragma endregion

#pragma region // Tile
	class TileHandle
	{
	private:
		TileDefinition* m_tileDefinition;
		int m_index;

	public:
		TileHandle(int index, TileDefinition* td = nullptr) :
			m_index(index),
			m_tileDefinition(td)
		{
		}

		Sprite GetSprite() const noexcept
		{
			return IsValid()? m_tileDefinition->renderable->GetSprite() : Sprite::MakeInvalidSprite();
		}

		bool IsValid() const noexcept
		{
			return m_tileDefinition != nullptr && m_tileDefinition->renderable != nullptr;
		}


		const int GetIndex() const
		{
			return m_index;
		}
	};
#pragma endregion

#pragma region // TerrainGrid
	class TerrainGrid
	{
	private:
#pragma region // parameters
		engine::container::Grid<TileHandle> m_map;
#pragma endregion

	public:
#pragma region // constructor/destructor
		TerrainGrid() :
			m_map(0)
		{
		}
#pragma endregion

#pragma region // non copyable, non movable
		TerrainGrid(const TerrainGrid&) = delete;
		TerrainGrid& operator=(const TerrainGrid&) = delete;
		TerrainGrid(TerrainGrid&&) = delete;
		TerrainGrid& operator=(TerrainGrid&&) = delete;
#pragma endregion

#pragma region // size query
		// returns grid width
		size_t GetWidth() const
		{
			return m_map.GetWidth();
		}

		// returns grid height. includes last row even if it is incomplete
		size_t GetHeight() const
		{
			return m_map.GetHeight();
		}

		spatial::Size<size_t> GetSize() const
		{
			return m_map.GetSize();
		}

		size_t GetElementCount() const
		{
			return m_map.GetElementCount();
		}

		bool IsEmpty() const
		{
			return m_map.IsEmpty();
		}
#pragma endregion

#pragma region // bound checks
		bool IsInBounds(int row, int col) const
		{
			return m_map.IsInBounds(row, col);
		}

		// overload for Coord input
		bool IsInBounds(const engine::spatial::Coord& Coord) const
		{
			return m_map.IsInBounds(Coord.row, Coord.col);
		}
#pragma endregion

#pragma region // accessors. return by value to be consistent. tile is a lightweight view to an object, similar to Sprite where GetSprite() returns value
		TileHandle Get(int row, int col)
		{
			return m_map.Get(row, col);
		}

		const TileHandle Get(int row, int col) const
		{
			return m_map.Get(row, col);
		}

		// retrieves the data at Coord
		TileHandle Get(const engine::spatial::Coord& coord)
		{
			return m_map.Get(coord.row, coord.col);
		}

		// retrieves the data at Coord
		const TileHandle Get(const engine::spatial::Coord& coord) const
		{
			return m_map.Get(coord.row, coord.col);
		}
#pragma endregion

#pragma region // replace value
		void Set(int row, int col, const TileHandle& data)
		{
			m_map.Set(row, col, data);
		}

		void Set(int row, int col, TileHandle&& data)
		{
			m_map.Set(row, col, std::move(data));
		}

		void Set(const engine::spatial::Coord& coord, const TileHandle& data)
		{
			m_map.Set(coord, data);
		}

		void Set(const engine::spatial::Coord& coord, TileHandle&& data)
		{
			m_map.Set(coord, std::move(data));
		}
#pragma endregion

#pragma region // content management
		void Reserve(const spatial::Size<size_t>& size)
		{
			m_map.Reserve(size);
		}

		void Clear()
		{
			m_map.Clear();
		}

		// copy only, no move option. expects T to be copyable or else
		void Initialize(size_t width, size_t height, const TileHandle& data)
		{
			m_map.Clear();
			m_map.SetWidth(width);
			m_map.Reserve({ width, height });

			for (size_t i = 0; i < width * height; ++i)
			{
				m_map.Add(data);
			}
		}

		void Initialize(engine::spatial::Size<size_t> size, const TileHandle& data)
		{
			Initialize(size.width, size.height, data);
		}
#pragma endregion

#pragma region // iteration
		template<typename Func>
		void ForEach(const Func& func)
		{
			for (int row = 0; row < (int)GetHeight(); ++row)
			{
				for (int col = 0; col < (int)GetWidth(); ++col)
				{
					func(row, col, Get(row, col));
				}
			}
		}

		template<typename Func>
		void ForEach(const Func& func) const
		{
			for (int row = 0; row < (int)GetHeight(); ++row)
			{
				for (int col = 0; col < (int)GetWidth(); ++col)
				{
					func(row, col, Get(row, col));
				}
			}
		}
#pragma endregion

	};
#pragma endregion

#pragma region // TerrainSet
	class TerrainSet
	{
	protected:
		container::Dictionary<int, std::unique_ptr<TileDefinition>> m_tiles;

		int m_invalidTileIndex;

		TileDefinition* GetInvalidTileDefinition() const
		{
			static TileDefinition s_invalidTileDefinition;
			return &s_invalidTileDefinition;
		}

	public:
		TerrainSet(int invalidTileIndex = -0xFFFF) :
			m_invalidTileIndex(invalidTileIndex)
		{
		}

		~TerrainSet() = default;

		// non copyable, non movable
		TerrainSet(const TerrainSet&) = delete;
		TerrainSet& operator=(const TerrainSet&) = delete;
		TerrainSet(TerrainSet&&) = delete;
		TerrainSet& operator=(TerrainSet&&) = delete;

		bool Register(int id, std::unique_ptr<TileDefinition> data)
		{
			return m_tiles.Register(id, std::move(data));
		}

		bool IsValid(int id) const noexcept
		{
			return m_tiles.Has(id);
		}

		// raw access
		const TileDefinition* Get(int id) const
		{
			return m_tiles.Has(id) ? m_tiles.Get(id).get() : nullptr;
		}

		// creates a tile instance for the given id. returns invalid tile if id not found
		TileHandle MakeTile(int id) const
		{
			return TileHandle(
				m_tiles.Has(id)? id: m_invalidTileIndex,
				m_tiles.Has(id)?
				m_tiles.Get(id).get():		// if we have valid tile definition use it
				GetInvalidTileDefinition()	// otherwise, use invalid one
			);
		}
	};



#pragma endregion

#pragma region // TerrainAutoTileAdapter
	struct TerrainAutoTileAdapter
	{
		TerrainGrid& grid;
		TerrainSet& set;

		bool IsInBounds(const Coord& c) const
		{
			return grid.IsInBounds(c);
		}

		int GetIndex(const Coord& c) const
		{
			return grid.Get(c).GetIndex();
		}

		void Set(const Coord& c, int index)
		{
			grid.Set(c, set.MakeTile(index));
		}

		TerrainAutoTileAdapter(TerrainGrid& g, TerrainSet& s) : 
			grid(g), 
			set(s)
		{
		}
	};

	struct TileGridAutoTileAdapter
	{
		TileGrid<IRenderable>& grid;
		Tileset<IRenderable>& set;

		bool IsInBounds(const Coord& c) const
		{
			return grid.IsInBounds(c);
		}

		int GetIndex(const Coord& c) const
		{
			return grid.Get(c).GetIndex();
		}

		void Set(const Coord& c, int index)
		{
			grid.Set(c, set.MakeTile(index));
		}
	};

#pragma endregion

#pragma region // IAutoTileAdapter
	class IAutoTileAdapter
	{
	public:
		virtual bool IsInBounds(const Coord& c) const = 0;
		virtual int GetIndex(const Coord& c) const = 0;
		virtual void Set(const Coord& c, int index) = 0;
	};
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

		// design note:
		// context ctx is not a reference or pointer. we expect T to be a stateless object
		template<typename T>
		struct AutoTileContext
		{
			T context;

			bool isInBounds(const Coord& c) const
			{
				return context.IsInBounds(c);
			}

			int getIndex(const Coord& c) const
			{
				return context.GetIndex(c);
			}

			void applyTile(const Coord& c, int index)
			{
				context.Set(c, index);
			}

			AutoTileContext(T ctx) :
				context(ctx)
			{
			}
		};

		struct TileChangeEventArgs
		{
			Coord coord;
			int index;
			TileVariant variant;
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

		template<typename T>
		void UpdateMask(
			AutoTileContext<T>& ctx,
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

		template<typename T>
		unsigned int ComputeMask(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord)
		{
			unsigned int mask = 0;

			UpdateMask(ctx, autoTileConfig, { coord.row - 1, coord.col }, mask, 8);	// N
			UpdateMask(ctx, autoTileConfig, { coord.row + 1, coord.col }, mask, 2);	// S
			UpdateMask(ctx, autoTileConfig, { coord.row, coord.col + 1 }, mask, 4);	// E
			UpdateMask(ctx, autoTileConfig, { coord.row, coord.col - 1 }, mask, 1);	// W

			return mask;
		}

		template<typename T>
		void PlaceTile(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord, const TileVariant type)
		{
			// Set the selected tile
			ctx.applyTile(coord, autoTileConfig.ToIndex(type));

			// fire event so listeners know when tile has changed
			TileChangeEventArgs args{};
			args.coord = coord;
			args.index = autoTileConfig.ToIndex(type);
			args.variant = type;
			TileChangedEvent(args);
		}

		template<typename T>
		void ResolveNeighbors(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord)
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
		AutoTileSystem() = default;

		AutoTileSystem(const AutoTileSystem&) = delete;
		AutoTileSystem& operator=(const AutoTileSystem&) = delete;
		AutoTileSystem(AutoTileSystem&&) = delete;
		AutoTileSystem& operator=(AutoTileSystem&&) = delete;

		virtual ~AutoTileSystem()
		{
		}

		template<typename T>
		void Set(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord, bool force = false)
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

		template<typename T>
		void Remove(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, const engine::spatial::Coord& coord, bool force = false)
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

		template<typename T>
		void Set(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, engine::spatial::Size<size_t> size)
		{
			for (int row = 0; row < size.height; row++)
			{
				for (int col = 0; col < size.width; col++)
				{
					Set(ctx, autoTileConfig, engine::spatial::Coord(row, col));
				}
			}
		}

		template<typename T>
		void Remove(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, engine::spatial::Size<size_t> size)
		{
			for (int row = 0; row < size.height; row++)
			{
				for (int col = 0; col < size.width; col++)
				{
					Remove(ctx, autoTileConfig, Coord(row, col));
				}
			}
		}

		event::Event<const TileChangeEventArgs&> TileChangedEvent;
	};
#pragma endregion

#pragma region // TerrainLayer
	class TerrainLayer
	{
	private:
		TerrainGrid m_grid;
		const TerrainSet* m_set;

	public:
		TerrainLayer() :
			m_set(nullptr)
		{
		}

		TileHandle Get(const Coord& coord) const
		{
			return m_grid.Get(coord);
		}

		TileHandle Get(int row, int col) const
		{
			return m_grid.Get(row, col);
		}

		Size<size_t> GetSize() const
		{
			return m_grid.GetSize();
		}

		void Initialize(size_t width, size_t height, const TerrainSet* set, int tileIndex)
		{
			Initialize({ width, height }, set, tileIndex);
		}

		void Initialize(const Size<size_t> size, const TerrainSet* set, int tileIndex)
		{
			m_set = set;
			m_grid.Initialize(size, m_set->MakeTile(tileIndex));
		}

		void Set(const Coord& c, int index)
		{
			m_grid.Set(c, m_set->MakeTile(index));
		}

		bool IsInBounds(const Coord& c) const
		{
			return m_grid.IsInBounds(c);
		}

		int GetIndex(const Coord& c) const
		{
			return m_grid.Get(c).GetIndex();
		}

		template<typename Func>
		void ForEach(const Func& func)
		{
			Size<size_t> size = GetSize();
			m_grid.ForEach(func);
		}

		template<typename Func>
		void ForEach(const Func& func) const
		{
			Size<size_t> size = GetSize();
			m_grid.ForEach(func);
		}
	};
#pragma endregion

#pragma region // TerrainLayerAutoTileAdapter
	class TerrainLayerAutoTileAdapter//: public IAutoTileAdapter
	{
	private:
		TerrainLayer& layer;

	public:
		TerrainLayerAutoTileAdapter(TerrainLayer& l) :
			layer(l)
		{
		}

		bool IsInBounds(const Coord& c) const //override
		{
			return layer.IsInBounds(c);
		}

		int GetIndex(const Coord& c) const// override
		{
			return layer.GetIndex(c);
		}

		void Set(const Coord& c, int index)// override
		{
			layer.Set(c, index);
		}
	};
#pragma endregion

#pragma region // TerrainBrush
	struct TerrainBrush
	{
		std::string layer;
		const AutoTileSystem::AutoTileConfig* config;

		bool operator == (const TerrainBrush& rhs) const
		{
			return layer == rhs.layer && config == rhs.config;
		}

		bool operator != (const TerrainBrush& rhs) const
		{
			return layer != rhs.layer || config != rhs.config;
		}
	};
#pragma endregion

#pragma region // TerrainMap
	class TerrainMap
	{
	private:
		Dictionary<std::string, std::unique_ptr<TerrainLayer>> m_layers;

	public:
		TerrainMap() = default;

		void Add(const std::string& key, size_t width, size_t height, const TerrainSet& set, int tileIndex)
		{
			Add(key, { width, height }, set, tileIndex);
		}

		void Add(const std::string& key, const Size<size_t> size, const TerrainSet& set, int tileIndex)
		{
			if (!Has(key))
			{
				if (!m_layers.Register(key, std::make_unique<TerrainLayer>()))
				{
					throw std::runtime_error("failed to create new terrain layer");
				}
			}

			m_layers[key]->Initialize(size, &set, tileIndex);
		}

		bool Has(const std::string& key) const
		{
			return m_layers.Has(key);
		}

		TerrainLayerAutoTileAdapter GetAutoTileAdapter(const std::string& key) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			return TerrainLayerAutoTileAdapter(*m_layers.Get(key));
		}

		TileHandle Get(const std::string& key, const Coord& coord) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			return  m_layers[key]->Get(coord);
		}

		TileHandle Get(const std::string& key, int row, int col) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			return  m_layers[key]->Get(row, col);
		}

		template<typename Func>
		void ForEach(const std::string& key, const Func& func)
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			m_layers[key]->ForEach(func);
		}

		template<typename Func>
		void ForEach(const std::string& key, const Func& func) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			m_layers[key]->ForEach(func);
		}
	};
#pragma endregion

#pragma region // TerrainBrushLink
	struct TerrainBrushLink
	{
		// source brush that triggers propagation
		TerrainBrush sourceBrush;

		// target brush to execute
		TerrainBrush targetBrush;

		// maps source tile index -> target tile index
		Dictionary<int, int> sourceToTarget;

		bool Has(int sourceIndex) const
		{
			return sourceToTarget.Has(sourceIndex);
		}

		int ToTarget(int sourceIndex) const
		{
			return sourceToTarget[sourceIndex];
		}

		bool Involves(TerrainBrush brush) const
		{
			return sourceBrush == brush || targetBrush == brush;
		}
	};
#pragma endregion

#pragma region // TerrainLinkLibrary
	class TerrainLinkLibrary
	{
	private:
		std::vector<TerrainBrushLink> m_links;

	public:

		void Link(
			const TerrainBrush& source,
			const TerrainBrush& target,
			const Dictionary<int, int>& map)
		{
			TerrainBrushLink link;

			link.sourceBrush = source;
			link.targetBrush = target;
			link.sourceToTarget = map;

			m_links.push_back(std::move(link));
		}

		void Add(const TerrainBrushLink& link)
		{
			m_links.push_back(link);
		}

		void RemoveLinksFor(const TerrainBrush& brush)
		{
			m_links.erase(
				std::remove_if(
					m_links.begin(),
					m_links.end(),
					[&](const TerrainBrushLink& link)
					{
						return link.Involves(brush);
					}),
				m_links.end());
		}

		template<typename Func>
		void ForEach(const Func& func)
		{
			for (TerrainBrushLink& link : m_links)
			{
				func(link);
			}
		}

		template<typename Func>
		void ForEach(const Func& func) const
		{
			for (const TerrainBrushLink& link : m_links)
			{
				func(link);
			}
		}

		void Clear()
		{
			m_links.clear();
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
		// fine if we ensure cells occupied by an object is usually small.
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
		std::unique_ptr<InstanceGrid<IRenderable>> objectLayer;
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
		AutoTileSystem::AutoTileContext<TileGridAutoTileAdapter> GetAutoTileContext(TileLayer& layer)
		{
			TileGridAutoTileAdapter tgata{ layer.tilegrid, *layer.tileset };

			return AutoTileSystem::AutoTileContext<TileGridAutoTileAdapter>
			{
				tgata
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

		static TerrainSet& LoadTerrainSet(
			const std::string& name, // key for storing in cache
			const std::string& atlasName // key of the sprite atlas to get sprites to
		)
		{
			// if this tileset already exist, just return its reference
			auto& registry = Registry<TerrainSet>::Instance();
			if (registry.Has(name))
			{
				return registry.Get(name);
			}
			else
			{
				// create tileset object
				std::unique_ptr<TerrainSet> tileset = std::make_unique<TerrainSet>();

				// if we this atlas exist, get a reference
				// TODO: a bit of a problem. if the sprite atlas does not exist, then we will have an empty tileset. this happens silently
				if (Registry<ISpriteAtlas>::Instance().Has(atlasName))
				{
					ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get(atlasName);

					// in this method, we are loading all the sprite atlas' sprites
					for (int i = 0; i < atlas.GetUVRectCount(); i++)
					{
						std::unique_ptr<TileDefinition> tiledef = std::make_unique<TileDefinition>();

						tiledef->renderable = std::make_unique<Renderable>(atlas.MakeSprite(i));

						tileset->Register(i, std::move(tiledef));
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

#pragma region // helper methods

	//// TODO: put on navigation namespace along with NavigationGrid and TileConstraint but as a static helper
	//inline static TileConstraint SubCellToConstraint(int r, int c)
	//{
	//	static const TileConstraint table[9] =
	//	{
	//		TileConstraint::NW, TileConstraint::N,  TileConstraint::NE,
	//		TileConstraint::W,  TileConstraint::CENTER, TileConstraint::E,
	//		TileConstraint::SW, TileConstraint::S,  TileConstraint::SE
	//	};

	//	if (r < 0 || r >= 3 || c < 0 || c >= 3)
	//	{
	//		return TileConstraint::NONE;
	//	}

	//	return table[r * 3 + c];
	//}
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

		// TODO: for debug only quick instancing to test. remove this in production code
		Prop()
		{

		}

		Prop(std::unique_ptr<IRenderable> r, const ColorF& c, const VecF& s, const RectF& fp, const RectF& bb) :
			renderable(std::move(r)),
			position({}),
			scale(s),
			color(c),
			footprint(fp),
			boundingBox(bb),
			collisionShape({})
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

#pragma region // NavigationSystem
	class NavigationSystem
	{

	};
#pragma endregion

#pragma region // PropMap
	class PropMap
	{
	private:
		struct Dummy {};

		InstanceGrid<Prop> m_objectLayer;
		NavigationGrid m_navGrid;
		SpatialOccupancyGrid<Prop, TileConstraint> m_FootPrintGrid;
		SpatialOccupancyGrid<Prop, Dummy> m_BoundingBoxGrid;

	public:
		engine::event::Event<const PositionF&, const Size<size_t>&, const SizeF&> InitializeEvent;

		PropMap()
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


		bool Initialize(const PositionF& position, const Size<size_t>& size, const SizeF& tilesize)
		{
			// initialize bucket grid
			m_objectLayer.Initialize(size);

			// initialize navigation grid. fill it with none meaning all tiles are walkable (no constraints)
			m_navGrid.Initialize(size, TileConstraint::NONE);

			// initialize footprint grid. 
			m_FootPrintGrid.Initialize(size.width, size.height);

			// initialize bounding box grid
			m_BoundingBoxGrid.Initialize(size.width, size.height);

			InitializeEvent(position, size, tilesize);

			return true;
		}

		void Remove(Prop* prop)
		{
			// iterate through tiles occupied by prop
			m_FootPrintGrid.ForEachCell(prop, [this, &prop](Coord coord) 
				{
					// get the constraint of the prop in the given coord.
					// this is a strict method. it's gonna throw error if there is no prop in this coord
					TileConstraint constraint = m_FootPrintGrid.Get(prop, coord);

					// remove that constraint for this coord in navigation grid
					m_navGrid.RemoveFlag(coord, constraint);
				}
			);

			// remove this object from spatial occupancy grid
			m_FootPrintGrid.Remove(prop);

			// remove this object from bounding box occupancy grid
			m_BoundingBoxGrid.Remove(prop);

			// remove this object from object layer
			m_objectLayer.Remove(prop);
		}

		// try to get tile coordinate of a given prop. if prop is invalid, return false
		bool TryGetCoord(Prop* prop, spatial::Coord& coord) const
		{
			return m_objectLayer.TryGetCoord(prop, coord);
		}

		bool Has(Prop* prop) const
		{
			return m_objectLayer.Has(prop);
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
			m_objectLayer.ForEach(row, col, func);
		}

		template<typename Predicate>
		void ForEachTileConstraint(int row, int col, Predicate func) const
		{
			m_navGrid.ForEach(row, col, func);
		}

		template<typename Func>
		void ForEachFootprint(const Coord& coord, Func func)
		{
			m_FootPrintGrid.ForEach(coord, func);
		}

		template<typename Func>
		void ForEachPropInBoundingBox(const Coord& coord, Func func)
		{
			m_BoundingBoxGrid.ForEachObject(coord, func);
		}

		void AddProp(const Coord& coord, std::unique_ptr<Prop> prop)
		{
			m_objectLayer.Add(coord, std::move(prop));
		}

		void AddFootprint(Prop* prop, const Coord& coord, TileConstraint constraint)
		{
			m_FootPrintGrid.Add(prop, coord, constraint);
			m_navGrid.AddFlag(coord, constraint);
		}

		void AddBoundingBox(Prop* prop, const Coord& coord)
		{
			m_BoundingBoxGrid.Add(prop, coord, {});
		}

	};

#pragma endregion

#pragma region // WorldTransform
	class WorldTransform
	{
	private:
		// top-left position of the world in screen space. this is basically the camera position.
		PositionF m_position;

		// size of the world in tiles
		Size<size_t> m_size;

		// size of each tile in pixels. 
		SizeF m_tilesize;

	public:
		WorldTransform():
			m_position(0, 0),
			m_size(0, 0),
			m_tilesize(0, 0)
		{
		}

		void SetSize(const Size<size_t>& size)
		{
			m_size = size;
		}

		void SetTileSize(const SizeF& tilesize)
		{
			m_tilesize = tilesize;
		}

		void SetPosition(const PositionF& position)
		{
			m_position = position;
		}

		Size<size_t> GetSize() const
		{
			return m_size;
		}

		SizeF GetTileSize() const
		{
			return m_tilesize;
		}

		PositionF GetPosition() const
		{
			return m_position;
		}

		size_t GetWidth() const
		{
			return m_size.width;
		}

		size_t GetHeight() const
		{
			return m_size.height;
		}

		PositionF WorldToTileSpace(const PositionF& worldPosition) const
		{
			// get the tile coord where the world position intersects
			Coord coord = PositionToCoord(worldPosition, m_tilesize);

			// get the tile's top-left position in the world
			PositionF coordTopLeftWorldPos = CoordToPosition(coord, m_tilesize);

			// translate the world position into this tile's local coordinate
			return worldPosition - coordTopLeftWorldPos;
		}

		Coord WorldToTileCoord(const PositionF& worldPosition) const
		{
			return PositionToCoord(worldPosition, m_tilesize);
		}

		// given tile coord and a position local to this tile, translate the local position into world position
		PositionF TileToWorldPosition(const Coord& coord, const PositionF& localPosition) const
		{
			return localPosition + CoordToPosition(coord, m_tilesize);
		}

		PositionF ScreenToWorld(const PositionF& screen) const
		{
			return screen - m_position;
		}

	};
#pragma endregion

#pragma region // WorldMap
	class WorldMap
	{
	private:
		WorldTransform m_worldTransform;		
		PropMap m_propMap;
		TerrainMap m_terrainMap;

	public:
		WorldMap()
		{
		}

		const WorldTransform& GetTransform() const
		{
			return m_worldTransform;
		}

		bool Initialize(const PositionF& position, const Size<size_t>& size, const SizeF& tilesize)
		{
			m_worldTransform.SetPosition(position);
			m_worldTransform.SetSize(size);
			m_worldTransform.SetTileSize(tilesize);

			if (!m_propMap.Initialize(m_worldTransform.GetPosition(), m_worldTransform.GetSize(), m_worldTransform.GetTileSize())) return false;

			return true;
		}	

		bool IsInBounds(int row, int col) const
		{
			return
				row >= 0 && col >= 0 &&		// make sure rows and columns are not negatives.
				col < m_worldTransform.GetWidth() &&		// make sure column is within the grid's width
				row < m_worldTransform.GetHeight();	// make sure row is within the grid's height
		}

		bool IsInBounds(const Coord& coord) const
		{
			return IsInBounds(coord.row, coord.col);
		}

		bool IsInBounds(const PositionF& worldPosition) const
		{
			return IsInBounds(m_worldTransform.WorldToTileCoord(worldPosition));
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

		std::string GetDebugInfo() const
		{
			return m_propMap.GetDebugInfo();
		}
		
		bool Has(Prop* prop) const
		{
			return m_propMap.Has(prop);
		}

		template<typename Func>
		void ForEachFootprint(const Coord& coord, Func func)
		{
			m_propMap.ForEachFootprint(coord, func);
		}

		template<typename Func>
		void ForEachPropInBoundingBox(const Coord& coord, Func func)
		{
			m_propMap.ForEachPropInBoundingBox(coord, func);
		}

		void InsertProp(
			std::unique_ptr<Prop> prop, 
			const Coord& coord, 
			const std::vector<Coord>& boundingBoxCells,
			const Dictionary<Coord, TileConstraint>& coordToConstraints
		)
		{
			// this prop to occupy respective coord in footprint layer in propmap
			for (auto& constraintsPerCell : coordToConstraints)
			{
				// place the new prop into navigation grid
				Coord coord = constraintsPerCell.first;
				TileConstraint constraint = constraintsPerCell.second;

				// place in footprint and update navigation grid with this constraint
				m_propMap.AddFootprint(prop.get(), coord, constraint);
			}

			// this prop to occupy respective coord in bounding box layer in propmap
			for (Coord coord : boundingBoxCells)
			{
				if (!IsInBounds(coord)) continue;

				m_propMap.AddBoundingBox(prop.get(), coord);
			}

			// add the actual prop object into prop map
			m_propMap.AddProp(coord, std::move(prop));
		}

		void RemoveProp(Prop* prop)
		{
			m_propMap.Remove(prop);
		}

		// try to get tile coordinate of a given prop. if prop is invalid, return false
		bool TryGetCoord(Prop* prop, spatial::Coord& coord) const
		{
			return m_propMap.TryGetCoord(prop, coord);
		}

		bool TryGetWorldPosition(Prop* prop, PositionF& position) const
		{
			Coord coord;
			if (!TryGetCoord(prop, coord))
			{
				return false;
			}

			position = m_worldTransform.TileToWorldPosition(coord, prop->GetPosition(true));
			return true;
		}

		void AddTerrain(const std::string& key, const TerrainSet& set, int tileIndex)
		{
			m_terrainMap.Add(key, GetTransform().GetSize(), set, tileIndex);
		}

		bool HasTerrain(const std::string& key) const
		{
			return m_terrainMap.Has(key);
		}

		TerrainLayerAutoTileAdapter GetAutoTileAdapter(const std::string& key) const
		{
			return m_terrainMap.GetAutoTileAdapter(key);
		}

		template<typename Func>
		void ForEachTerrainTile(const std::string& key, const Func& func)
		{
			m_terrainMap.ForEach(key, func);
		}

		template<typename Func>
		void ForEachTerrainTile(const std::string& key, const Func& func) const
		{
			m_terrainMap.ForEach(key, func);
		}
	};
#pragma endregion

#pragma region // TerrainBrushTool
	class TerrainBrushTool
	{
	private:
		TerrainBrush m_currentBrush;

	public:
		void Set(const TerrainBrush& brush)
		{
			m_currentBrush = brush;
		}

		TerrainBrush Get() const
		{
			return m_currentBrush;
		}


		bool Paint(const WorldMap& world, const Coord& coord)
		{
			// if no valid terrain, bail out
			if (!world.HasTerrain(m_currentBrush.layer))
			{
				// should we throw?
				return false;
			}
			if (m_currentBrush.config != nullptr)
			{
				AutoTileSystem::AutoTileContext<TerrainLayerAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };
				AutoTileSystem ats;
				ats.Set(context, *m_currentBrush.config, coord);
			}
			else
			{
				// what are we supposed to do if there is no config?? we don't know which index to set...
				//m_currentBrush.layer->Set(coord, )
				return false;
			}

			return true;
		}
		bool Paint(const WorldMap& world, const PositionF& worldPos)
		{
			Coord coord = world.GetTransform().WorldToTileCoord(worldPos);
			return Paint(world, coord);
		}

		bool Erase(const WorldMap& world, const Coord& coord)
		{
			// if no valid terrain, bail out
			if (!world.HasTerrain(m_currentBrush.layer))
			{
				// should we throw?
				return false;
			}

			if (m_currentBrush.config != nullptr)
			{
				AutoTileSystem::AutoTileContext<TerrainLayerAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };
				AutoTileSystem ats;
				ats.Remove(context, *m_currentBrush.config, coord);
			}
			else
			{
				// what are we supposed to do if there is no config?? we don't know which index to set...
				//m_currentBrush.layer->Set(coord, )
				return false;
			}

			return true;
		}

		bool Erase(const WorldMap& world, const PositionF& worldPos)
		{
			Coord coord = world.GetTransform().WorldToTileCoord(worldPos);
			return Erase(world, coord);
		}

	};
#pragma endregion

#pragma region // TerrainLinkTool
	class TerrainLinkTool
	{
	private:
		const WorldMap& m_world;
		AutoTileSystem& m_autoTile;
		TerrainLinkLibrary& m_links;
		TerrainBrush& m_source;

	public:
		TerrainLinkTool(
			const WorldMap& world,
			AutoTileSystem& autoTile,
			TerrainLinkLibrary& links,
			TerrainBrush& source
		) :
			m_world(world),
			m_autoTile(autoTile),
			m_links(links),
			m_source(source)
		{
			m_autoTile.TileChangedEvent += event::Handler(this, &TerrainLinkTool::OnTileChanged);
		}

		~TerrainLinkTool()
		{
			m_autoTile.TileChangedEvent -= event::Handler(this, &TerrainLinkTool::OnTileChanged);
		}

		void OnTileChanged(const AutoTileSystem::TileChangeEventArgs& e)
		{
			m_links.ForEach(
				[&](const TerrainBrushLink& link)
				{
					if (link.sourceBrush != m_source)
						return;

					if (!link.Has(e.index))
						return;

					int targetIndex = link.ToTarget(e.index);

					TerrainLayerAutoTileAdapter adapter = m_world.GetAutoTileAdapter(link.targetBrush.layer);
					AutoTileSystem::AutoTileContext<TerrainLayerAutoTileAdapter> context{ adapter };
					context.applyTile(e.coord, targetIndex);
				});
		}
	};
#pragma endregion

#pragma region // TerrainEditor
	class TerrainEditor
	{
	private:
		TerrainBrush m_currentBrush;
		TerrainLinkLibrary m_links;

	public:
		void Set(const TerrainBrush& brush)
		{
			m_currentBrush = brush;
		}

		TerrainBrush Get() const
		{
			return m_currentBrush;
		}

		void Link(
			const TerrainBrush& source,
			const TerrainBrush& target,
			const Dictionary<int, int>& map)
		{
			m_links.Link(source, target, map);
		}

		void Add(const TerrainBrushLink& link)
		{
			m_links.Add(link);
		}

		void RemoveLinksFor(const TerrainBrush& brush)
		{
			m_links.RemoveLinksFor(brush);
		}

		bool Paint(const WorldMap& world, const Coord& coord)
		{
			// if no valid terrain, bail out
			if (!world.HasTerrain(m_currentBrush.layer))
			{
				// should we throw?
				return false;
			}

			if (m_currentBrush.config == nullptr)
			{
				// no config, no way to know which tile to set
				return false;
			}

			// set adapter for current layer
			AutoTileSystem::AutoTileContext<TerrainLayerAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };

			// create autotilesystem
			AutoTileSystem ats;

			// create our link tool. this will subscribe to our auto tile system and will update all linked layers when our source layer change tiles
			TerrainLinkTool tlt(world, ats, m_links, m_currentBrush);

			// perform the paint and auto paint neighbors if needed
			ats.Set(context, *m_currentBrush.config, coord);

			return true;
		}
		bool Paint(const WorldMap& world, const PositionF& worldPos)
		{
			Coord coord = world.GetTransform().WorldToTileCoord(worldPos);
			return Paint(world, coord);
		}

		bool Erase(const WorldMap& world, const Coord& coord)
		{
			// if no valid terrain, bail out
			if (!world.HasTerrain(m_currentBrush.layer))
			{
				// should we throw?
				return false;
			}

			if (m_currentBrush.config == nullptr)
			{
				// no config, no way to know which tile to set
				return false;
			}

			// set adapter for current layer
			AutoTileSystem::AutoTileContext<TerrainLayerAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };

			// create autotilesystem
			AutoTileSystem ats;

			// create our link tool. this will subscribe to our auto tile system and will update all linked layers when our source layer change tiles
			TerrainLinkTool tlt(world, ats, m_links, m_currentBrush);

			// perform the remove and auto remove neighbors if needed
			ats.Remove(context, *m_currentBrush.config, coord);

			return true;
		}

		bool Erase(const WorldMap& world, const PositionF& worldPos)
		{
			Coord coord = world.GetTransform().WorldToTileCoord(worldPos);
			return Erase(world, coord);
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

#pragma region // PropFactory
	// design note: 
	// a bit of an issue where brush already knows animation set key to look for in cache but we still 
	// pass the actual animation set here, making the animation set key useless in brush. the caller 
	// though used the brush's animation set key outside to determine the animation set. is this a smell?
	// i don't think so. i think factory does not need to fully depend on brush information to build brush
	// it can also take information somewhere else like we did here. 
	class PropFactory
	{
	public:
		static std::unique_ptr<Prop> Create(const PropBrush& brush, const AnimationSet<Sprite>& animationSet)
		{
			return std::make_unique<Prop>(
				std::make_unique<Animated>(animationSet, brush.animation),
				brush.color,
				brush.scale,
				brush.footprint,
				brush.boundingBox
			);
		}
	};
#pragma endregion

#pragma region // PropSelectionSystem
	class PropSelectionSystem
	{
	public:
		static std::vector<Prop*> SelectAtPoint(WorldMap& world, const PositionF& worldPosition)
		{
			// get the tile coord in map where world position intersect
			Coord coord = world.GetTransform().WorldToTileCoord(worldPosition);

			// Bounds check (early)
			if (!world.IsInBounds(worldPosition))
			{
				return {};
			}

			// given the tile coord in worldmap where the specified world position intersects, iterate through the props where their bounding box overlaps this tile coord
			std::vector<Prop*> selected{};
			world.ForEachPropInBoundingBox(coord, [&world, &selected, worldPosition](Prop* candidate)
				{
					// get the world position of this prop
					PositionF propPosInWorld; 
					if(!world.TryGetWorldPosition(candidate, propPosInWorld))
					{ 
						// if this prop does not exist in the map, throw. this must be a bug. 
						// how did we find pointer to this object in bounding box grid? where is the actual object stored?
						throw std::out_of_range("PropSelectionSystem::SelectAtPoint() - prop does not exist in the map but we have pointer to it. where is it stored?!");
					}						

					// get this bounding box of this prop in world coordinates
					RectF objectBoundingBox = candidate->GetScaledBoundingBoxWorld(propPosInWorld, true);

					// broad phase collision check: check if the mouse cursor (in world coordinate) is intersecting with the bounding box already in world coordinate. 
					if (!objectBoundingBox.Contains(worldPosition)) return;

					// TODO:implement narrow phase collision check and execute here

					selected.push_back(candidate);
				}
			);

			return selected;
		}

		static Prop* SelectTopMostAtPoint(WorldMap& world, const PositionF& worldPosition)
		{
			// get the tile coord in map where world position intersect
			Coord coord = world.GetTransform().WorldToTileCoord(worldPosition);

			// Bounds check (early)
			if (!world.IsInBounds(worldPosition))
			{
				return nullptr;
			}
			// if there are props, iterate through them and find the ones whose bounding box intersects with the cursor position (in world coordinate). return all intersecting objects. 
			Prop* topMostProp = nullptr;
			RectF topMostPropBoundingBox{};
			world.ForEachPropInBoundingBox(coord, [&world, &topMostProp, &topMostPropBoundingBox, worldPosition](Prop* candidate)
				{
					// get the world position of this prop
					PositionF propPosInWorld;
					if (!world.TryGetWorldPosition(candidate, propPosInWorld))
					{
						// if this prop does not exist in the map, throw. this must be a bug. 
						// how did we find pointer to this object in bounding box grid? where is the actual object stored?
						throw std::out_of_range("PropSelectionSystem::SelectAtPoint() - prop does not exist in the map but we have pointer to it. where is it stored?!");
					}

					// get this bounding box of the object in world coordinates
					RectF objectBoundingBox = candidate->GetScaledBoundingBoxWorld(propPosInWorld, true);

					// broad phase collision check: check if the mouse cursor (in world coordinate) is intersecting with the bounding box already in world coordinate. 
					if (!objectBoundingBox.Contains(worldPosition)) return;

					// TODO:implement narrow phase collision check and execute here

					// if so, compare its depth with current candidate for top most and replace candidate if this one is higher (lower on the screen)
					if (topMostProp == nullptr || objectBoundingBox.bottom >= topMostPropBoundingBox.bottom)
					{
						topMostProp = candidate;
						topMostPropBoundingBox = objectBoundingBox;
					}
				}
			);

			// it is possible that there is no candidate prop that actually was selected. note that candidates are only based on props that belongs to 
			// the tiles. it does not mean that any of them will intersect with the position. that is why the candidates need to do a broad phase check 
			// via their bounding box (and later narrow phase check with their collision shape).
			return topMostProp;
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

		// this sets the rule on how to interpret map's spatial data into tile constraint
		// placement system defines this. right now, it interprets that a tile is composed of 3x3 subcells
		// each cell corresponds to a tile constraint bit. there are 9 bits - center, corner, edges
		// this methods determines which subcells in the map the footprint overlaps with.
		// these subcells are translated into tile constraint bits. all bits found per coord is merged
		// to form the tile's tile constraint.
		static Dictionary<Coord, TileConstraint> BuildConstraints(const WorldMap& world, const RectF& footprint)
		{
			// get list of subcells this footprint overlaps
			std::vector<Coord> subcells = QueryCoords(footprint, world.GetTransform().GetTileSize() / 3.0f);

			Dictionary<Coord, TileConstraint> constraintsPerCoord;
			for (Coord& subcell : subcells)
			{
				// calculate the actual tile coord in map
				Coord tileCoord = { subcell.row / 3, subcell.col / 3 };

				// QueryCoords does not clamp collection of tile coords within grid size, so there might be coords that are invalid. skip it
				if (!world.IsInBounds(tileCoord)) continue;

				// calculate the sub tile coord relative to this tile
				Coord subCoord = { subcell.row % 3, subcell.col % 3 };

				TileConstraint bit = engine::navigation::tile::SubCellToConstraint(subCoord.row, subCoord.col);

				if (!constraintsPerCoord.Has(tileCoord))
				{
					constraintsPerCoord.Register(tileCoord, TileConstraint::NONE);
				}
				constraintsPerCoord[tileCoord] |= bit;
			}
			return constraintsPerCoord;
		}

		static bool Place(WorldMap& world, std::unique_ptr<Prop> prop, const PositionF& worldPosition)
		{
			// --------------------------------------------------------------------------------
			// VALIDATION
			// --------------------------------------------------------------------------------

			// validate the position is within bounds of the world. if not, bail out
			if (!world.IsInBounds(worldPosition))
			{
				return false;
			}

			// sanity check. we already have this prop. why are we placing again. how is this even possible!
			if (world.Has(prop.get()))
			{
				throw std::runtime_error("why do we already have this object???");
			}

			// --------------------------------------------------------------------------------
			// COMPUTE DATA
			// --------------------------------------------------------------------------------

			// get prop's footprint in world coordinate
			RectF footprint = prop->GetScaledFootprintWorld(worldPosition);

			// given the footprint, extract all the coords that intersects with footprint and their corresponding tile constraint value after footprint is placed
			Dictionary<Coord, TileConstraint> constraintsPerCoord = BuildConstraints(world, footprint);

			// iterate through each cells the prop overlapped. we will check if the props in these cells are overlapped by the new prop
			std::unordered_set<Prop*> toEvict;
			for (auto& constraintsPerCell : constraintsPerCoord)
			{
				Coord coord = constraintsPerCell.first;

				// what we're doing here is we are comparing each of the coord in map that the footprint intersected. 
				// each coord is occupied by existing prop, and these props have their corresponding tile constraint in this coord
				// we then compare the tile constraint of existing prop in this coord to the new tile constraint when footprint is applied
				// if these tile constraints overlaps (any constraint bits are both high), the prop
				world.ForEachFootprint(coord, [constraintsPerCell, &toEvict](Prop* prop, TileConstraint constraint)
					{
						// if they overlap, we will remove this prop
						TileConstraint tc = constraintsPerCell.second & constraint;
						if (tc != TileConstraint::NONE)
						{
							toEvict.insert(prop);
						}
					});
			}

			// get the cells in the map where the bounding box of this prop overlaps
			RectF boundingBox = prop->GetScaledBoundingBoxWorld(worldPosition, true);
			std::vector<Coord> boundingBoxTiles = QueryCoords(boundingBox, world.GetTransform().GetTileSize());

			// --------------------------------------------------------------------------------
			// REMOVE OVERLAPS (RULE)
			// --------------------------------------------------------------------------------

			// remove props overlapped by new prop
			for (Prop* prop : toEvict)
			{
				world.RemoveProp(prop);
			}

			// --------------------------------------------------------------------------------
			// APPLY PLACEMENT
			// --------------------------------------------------------------------------------

			// we set it as position of this prop
			prop->position = world.GetTransform().WorldToTileSpace(worldPosition);

			// add the actual object into our object layer
			world.InsertProp(std::move(prop), world.GetTransform().WorldToTileCoord(worldPosition), boundingBoxTiles, constraintsPerCoord);

			return true;
		}

		static bool Remove(WorldMap& world, const PositionF& worldPosition)
		{
			// --------------------------------------------------------------------------------
			// VALIDATION AND SELECTION
			// --------------------------------------------------------------------------------
			
			Prop* topMostProp = PropSelectionSystem::SelectTopMostAtPoint(world, worldPosition);

			// it is possible that there is no candidate prop that actually was selected. note that candidates are only based on props that belongs to 
			// the tiles. it does not mean that any of them will intersect with the position. that is why the candidates need to do a broad phase check 
			// via their bounding box (and later narrow phase check with their collision shape).
			if (topMostProp == nullptr) return false;

			// --------------------------------------------------------------------------------
			// APPLY REMOVE
			// --------------------------------------------------------------------------------

			// Apply removal (delegate to PropMap / WorldState)
			world.RemoveProp(topMostProp);

			return true;
		}
		
	};
#pragma endregion

#pragma region // PropBrushTool
	class PropBrushTool
	{
	private:
		// design note:
		// by right this does not belong here. it is the brush library. 
		// tool should only be responsible for deciding which brush to be active
		// but we decided to place this here because it's a simple dictionary only.
		// it kinda feels inconvenient defining a "BrushLibrary" just to separate this.
		// however doing this breaks separation of responsibilities.
		// this class now does 2 jobs - paint and store brushes. 
		// is this wrong? maybe not now because this brush library is so simple
		// maybe in future we will separate it.
		Dictionary<std::string, PropBrush> m_brushes;
		PropBrush m_currentBrush;

	public:
		PropBrushTool()
		{
		}

		void Register(const std::string& key, const PropBrush& brush)
		{
			m_brushes.Register(key, brush);
		}

		void Set(const std::string& key)
		{
			m_currentBrush = m_brushes.Get(key);
		}

		// brush tool places the current active prop in the world at given position in the world.
		bool Paint(WorldMap& world, const PositionF& worldPosition) const
		{
			// gets the animation set from asset based on the brush
			// this is strict. if animation set does not exist in cache, this will throw
			// design note: maybe it's a bit complicated to create fallback of animation set so we leave it like this 
			AssetManager assets;
			auto& animSet = assets.Get<AnimationSet<Sprite>>(m_currentBrush.animationSet);

			// create the prop based on the brush
			std::unique_ptr<Prop> prop = PropFactory::Create(m_currentBrush, animSet);

			// delegate placement on placement system
			return PropPlacementSystem::Place(world, std::move(prop), worldPosition);
		}

		// erase top-most prop from the world at given position in the world
		bool Erase(WorldMap& world, const PositionF& worldPosition)
		{
			return PropPlacementSystem::Remove(world, worldPosition);
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
			std::vector<Coord> coords = QueryCoords(fp, cellsize);

			SizeF tilesize(m_tile.GetSize());
			for (Coord coord : coords)
			{
				if (coord.col < 0 || coord.col >= 3 || coord.row < 0 || coord.row >= 3) continue;

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
		TileLayer m_gridTileLayer;
		TileLayer m_fineGridTileLayer;
		WorldMap m_worldMap;

		PropBrushTool m_propBrushTool;
		PropPlacementSystem m_propPlacer;

		PositionF m_mousePos;

		bool m_showDebug = true;

		PropBrush NormalBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.27f, 0.12f, 0.73f, 0.87f} };
		PropBrush NormalPineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1},RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.2f, 0.2f, 0.8f, 0.92f} };
		PropBrush NormalCastle{ "castle_anim_set", "castle_idle",	VecF{1.0f, 1.0f}, ColorF{1,1,1,1}, RectF{0.05f, 0.6f, 0.95f, 0.90f}, RectF{0.05f, 0.2f, 0.95f, 0.9f} };
		PropBrush LargePineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush LargeBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush LargeCastle{ "castle_anim_set", "castle_idle",	VecF{1.5f, 1.5f}, ColorF{1,1,1,1}, RectF{0.05f, 0.6f, 0.95f, 0.90f}, RectF{0.05f, 0.2f, 0.95f, 0.9f} };
		PropBrush LargeWaterRocks{ "water_rocks_anim_set",	"water_rocks_idle",	VecF{2.0f, 2.0f}, ColorF{1,1,1,1}, RectF{0.1f,0.4f,0.9f,0.8f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush SmallPineTree{ "pinetree_anim_set",	"pine_tree_idle",	VecF{0.5f, 0.5f}, ColorF{1,1,1,1}, RectF{0.38f, 0.8f, 0.62f, 0.92f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };
		PropBrush SmallBirchTree{ "birchtree_anim_set",	"birch_tree_idle",	VecF{0.5f, 0.5f}, ColorF{1,1,1,1}, RectF{0.47f, 0.8f, 0.53f, 0.85f}, RectF{0.1f, 0.1f, 0.9f, 0.9f} };

	public:
		void OnEnter() override
		{
			// register our brushes
			m_propBrushTool.Register("normal_pine_tree", NormalPineTree);
			m_propBrushTool.Register("normal_birch_tree", NormalBirchTree);
			m_propBrushTool.Register("normal_castle", NormalCastle);
			m_propBrushTool.Register("large_pine_tree", LargePineTree);
			m_propBrushTool.Register("large_birch_tree", LargeBirchTree);
			m_propBrushTool.Register("large_castle", LargeCastle);
			m_propBrushTool.Register("small_pine_tree", SmallPineTree);
			m_propBrushTool.Register("small_birch_tree", SmallBirchTree);
			m_propBrushTool.Register("large_water_rocks", LargeWaterRocks);

			// set placement tool default placement
			m_propBrushTool.Set("normal_pine_tree");

			m_worldMap.Initialize(m_assets.Get<PositionF>("map_position"), m_assets.Get<Size<size_t>>("map_size"), m_assets.Get<SizeF>("tile_size"));

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
				m_propBrushTool.Set("normal_pine_tree");
				break;
			case 50: // 2
				m_propBrushTool.Set("normal_birch_tree");
				break;
			case 51: // 3 
				m_propBrushTool.Set("normal_castle");
				break;
			case 52: // 4
				break;
			case 53: // 5
				break;
			case 54: // 6
				break;
			case 55: // 7
			{
				break;
			}
			case 56: // 8
			{
				break;
			}
			case 57: // 9
			{
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
				m_propBrushTool.Paint(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));

				return;
			}
			// right click to remove tile
			else if (btn == 2)
			{
				// immediate goal is to find the top-most object that intersects with the mouse cursor in this cell and remove it.
				m_propBrushTool.Erase(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));

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

			Size<size_t> mapSize = m_worldMap.GetTransform().GetSize();
			for (int row = 0; row < (int)mapSize.height; row++)
			{
				for (int col = 0; col < (int)mapSize.width; col++)
				{
					Coord coord(row, col);
					PositionF tileScreenPos = CoordToPosition(coord, tilesize) + mapPos;

					m_worldMap.ForEachProp(row, col, [&drawCommand, &tileScreenPos](Prop* prop)
						{
							prop->QueueForDraw(drawCommand, tileScreenPos, 1);
						});
				}
			}
			//m_placementTool.QueuePreviewForDraw(drawCommand, m_mousePos, 1);
			drawCommand.Sort();
			drawCommand.Execute();



			if (m_showDebug)
			{
				IRenderer& renderer = m_assets.Get<IRenderer>("renderer");
				//RectF fp = m_placementTool.GetPreviewFootprintAt(m_mousePos);
				//DrawQuadCommand cmd(renderer, fp.GetTopLeft(), fp.GetSize(), { 1,1,1,0.5f }, 0.0f);
				//cmd.Execute();

				//RectF hb = m_placementTool.GetPreviewBoundingBoxAt(m_mousePos);
				//DrawQuadCommand cmdBoundingBox(renderer, hb.GetTopLeft(), hb.GetSize(), { 1,0,1,0.5f }, 0.0f);
				//cmdBoundingBox.Execute();

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

				DrawNavigationGridOverlay(renderer, m_worldMap);

				std::string msg = m_worldMap.GetDebugInfo();

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

			SizeF subTileSize(map.GetTransform().GetTileSize().width / 3.0f, map.GetTransform().GetTileSize().height / 3.0f);

			// visual tweak (same as yours)
			VecF shift(subTileSize.width * 0.25f, subTileSize.height * 0.25f);
			SizeF overlaySize(subTileSize.width / 2.0f, subTileSize.height / 2.0f);

			for (int row = 0; row < (int)map.GetTransform().GetSize().height; ++row)
			{
				for (int col = 0; col < (int)map.GetTransform().GetSize().width; ++col)
				{
					map.ForEachTileConstraint(row, col, [row, col, &map, subTileSize, shift, overlaySize, &renderer](TileConstraint constraint)
						{
							// skip empty tiles early (fast path)
							if (constraint == TileConstraint::NONE) return;

							// top-left of this tile in world space
							PositionF tileWorldPos = CoordToPosition({ row, col }, map.GetTransform().GetTileSize()) + map.GetTransform().GetPosition();

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

#pragma region // TerrainEditScene scene
	class TerrainEditScene : public Scene
	{
	private:
		StateMachine m_stateMachine;
		AssetManager m_assets;
		WorldMap m_worldMap;

		TileLayer m_grassTileLayer;
		TileLayer m_splashTileLayer;

		PositionF m_mousePos;

		TerrainGrid m_tilegrid;
		TerrainGrid m_finegrid;

		TerrainBrush m_grassTerrainBrush;
		TerrainBrush m_splashTerrainBrush;

		TerrainBrushLink m_grassToSplashBrushLink;
		TerrainEditor m_terrainEditor;

		bool m_showDebug = true;

	public:
		void OnEnter() override
		{
			m_worldMap.Initialize(m_assets.Get<PositionF>("map_position"), m_assets.Get<Size<size_t>>("map_size"), m_assets.Get<SizeF>("tile_size"));

			// initialize grid tile layer. fill it with its only tile
			auto& grassTileset = m_assets.Get<Tileset<IRenderable>>("grass_tileset");
			auto& mapSize = m_assets.Get<Size<size_t>>("map_size");

			// initialize grass tile layer. fill it with invalid tiles for now so they have empty tiles
			m_grassTileLayer.tileset = &grassTileset;
			m_grassTileLayer.tilegrid.Initialize(mapSize, grassTileset.MakeInvalidTile());

			// initialize water splash tile layer. also fill with invalid tiles for now
			auto& splashTileset = m_assets.Get<Tileset<IRenderable>>("splash_tileset");
			m_splashTileLayer.tileset = &splashTileset;
			m_splashTileLayer.tilegrid.Initialize(mapSize, splashTileset.MakeInvalidTile());


			// initialize grids
			auto& terrainSet = m_assets.Get<TerrainSet>("grass_tileset");
			m_tilegrid.Initialize(m_worldMap.GetTransform().GetSize(), terrainSet.MakeTile(13));
			m_finegrid.Initialize(m_worldMap.GetTransform().GetSize(), terrainSet.MakeTile(22));

			auto& splashSet = m_assets.Get<TerrainSet>("splash_tileset");

			// add layers to terrain map
			m_worldMap.AddTerrain("grass", terrainSet, 4);
			m_worldMap.AddTerrain("splash", splashSet, -1);

			// initialize terrain brushes
			m_grassTerrainBrush.layer = "grass";
			m_grassTerrainBrush.config = &m_assets.Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");

			m_splashTerrainBrush.layer = "splash";
			m_splashTerrainBrush.config = &m_assets.Get<AutoTileSystem::AutoTileConfig>("splash_tile_auto_config");

			m_grassToSplashBrushLink.sourceBrush = m_grassTerrainBrush;
			m_grassToSplashBrushLink.targetBrush = m_splashTerrainBrush;
			m_grassToSplashBrushLink.sourceToTarget.Register(4, -1);
			m_grassToSplashBrushLink.sourceToTarget.Register(30, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(10,-1);
			m_grassToSplashBrushLink.sourceToTarget.Register(21, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(3, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(29, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(27,0);
			m_grassToSplashBrushLink.sourceToTarget.Register(0, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(2, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(18, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(20, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(12, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(28, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(1, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(19, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(9, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(11, 0);

			m_terrainEditor.Set(m_grassTerrainBrush);
			m_terrainEditor.Add(m_grassToSplashBrushLink);
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
				break;
			case 50: // 2
				break;
			case 51: // 3 
				break;


			default:
				break;
			}
		}

		void OnMouseMove(int x, int y) override
		{
			m_mousePos = PositionF((float)x, (float)y);

			// is mouse left button is held while moving...
			if (Input::Instance().IsMouseHeld(1))
			{
				//m_terrainBrushTool.Paint(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));
				m_terrainEditor.Paint(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));

			}
			else if (Input::Instance().IsMouseHeld(2))
			{
				m_terrainEditor.Erase(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));

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
			auto& grassTileset = m_assets.Get<TerrainSet>("grass_tileset");

			// left click to place grass tile
			if (btn == 1)
			{
				//// place grass tile
				//TileLayerEditor tle;
				//tle.LinkLayers(m_grassTileLayer, m_splashTileLayer, splashAnimLookup);
				//tle.Paint(m_grassTileLayer, config, coord);
				//return;

				//m_terrainBrushTool.Paint(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));

				m_terrainEditor.Paint(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));


				//AutoTileSystem ats;
				//TerrainAutoTileAdapter tata{ m_terrain, grassTileset };
				//AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> ctx{tata};
				//ats.Set(ctx, config, coord);
			}
			// right click to remove tile
			else if (btn == 2)
			{
				//// remove grass tile
				//TileLayerEditor tle;
				//tle.LinkLayers(m_grassTileLayer, m_splashTileLayer, splashAnimLookup);
				//tle.Erase(m_grassTileLayer, config, coord);

				//m_terrainBrushTool.Erase(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));
				m_terrainEditor.Erase(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));


			}
		}

		void OnRender() override
		{
			m_stateMachine.OnRender();

			// get resources
			IRenderer& renderer = m_assets.Get<IRenderer>("renderer");
			DrawSortedSpritesCommand& drawCommand = m_assets.Get<DrawSortedSpritesCommand>("drawCommand");

			// draw the terrain
			drawCommand.Clear();
			//DrawTerrainGrid(renderer, drawCommand, m_terrain, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 1,1,1,1 });
			//DrawTerrainLayer(renderer, drawCommand, m_grassTerrainLayer, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 1,1,1,1 });
			DrawTerrainLayer(renderer, drawCommand, m_worldMap, "splash", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 3,3 }, { 0, 0 });
			drawCommand.Sort();
			drawCommand.Execute();

			drawCommand.Clear();
			DrawTerrainLayer(renderer, drawCommand, m_worldMap, "grass", m_worldMap.GetTransform().GetPosition(),  m_worldMap.GetTransform().GetTileSize());
			drawCommand.Sort();
			drawCommand.Execute();
			
			if (m_showDebug)
			{
				// draw the tile grid
				drawCommand.Clear();
				DrawTerrainGrid(renderer, drawCommand, m_tilegrid, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 0,0,0,0.2f });
				DrawTerrainGrid(renderer, drawCommand, m_finegrid, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 0,0,0,0.05f });
				drawCommand.Sort();
				drawCommand.Execute();
			}

			if (m_showDebug)
			{
				IRenderer& renderer = m_assets.Get<IRenderer>("renderer");

				DrawNavigationGridOverlay(renderer, m_worldMap);


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

			SizeF subTileSize(map.GetTransform().GetTileSize().width / 3.0f, map.GetTransform().GetTileSize().height / 3.0f);

			// visual tweak (same as yours)
			VecF shift(subTileSize.width * 0.25f, subTileSize.height * 0.25f);
			SizeF overlaySize(subTileSize.width / 2.0f, subTileSize.height / 2.0f);

			for (int row = 0; row < (int)map.GetTransform().GetSize().height; ++row)
			{
				for (int col = 0; col < (int)map.GetTransform().GetSize().width; ++col)
				{
					map.ForEachTileConstraint(row, col, [row, col, &map, subTileSize, shift, overlaySize, &renderer](TileConstraint constraint)
						{
							// skip empty tiles early (fast path)
							if (constraint == TileConstraint::NONE) return;

							// top-left of this tile in world space
							PositionF tileWorldPos = CoordToPosition({ row, col }, map.GetTransform().GetTileSize()) + map.GetTransform().GetPosition();

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

		void DrawTerrainGrid(
			IRenderer& renderer,
			DrawSortedSpritesCommand& command,
			const TerrainGrid& grid,
			const PositionF& worldPos,
			const SizeF& tileSize,
			const ColorF& color,
			VecF scale = { 1,1 }
		)
		{
			for (int row = 0; row < (int)grid.GetHeight(); ++row)
			{
				for (int col = 0; col < (int)grid.GetWidth(); ++col)
				{
					// get the tile from main layer.
					const TileHandle& tile = grid.Get(row, col);
					
					// if tile is valid, we can queue it for draw. otherwise, we skip it
					if (tile.IsValid())
					{
						// find the top-left position of the tile in map space.
						engine::spatial::PositionF tilePosFromMap =
						{
							col * tileSize.width,
							row * tileSize.height
						};

						// apply scale to tile size in case we want to draw the tile at different size. 
						// note that only size change. position is still based on original tile size 
						engine::spatial::SizeF scaledtilesize
						{
							tileSize.width * scale.x,
							tileSize.height * scale.y
						};

						command.Add({
							tile.GetSprite(),					// sprite object to draw
							worldPos + tilePosFromMap,		// shift tile position from map space to world space
							scaledtilesize,						// size to draw the tile at
							color,								// color
							0.0f,								// no rotation for tile
							1								// depth value for sorting
							});
					}
				}
			}
		}


		void DrawTerrainLayer(
			IRenderer& renderer,
			DrawSortedSpritesCommand& command,
			const TerrainLayer& layer,
			const PositionF& worldPos,
			const SizeF& tileSize,
			const ColorF& color,
			VecF scale = { 1,1 }
		)
		{
			for (int row = 0; row < (int)layer.GetSize().height; ++row)
			{
				for (int col = 0; col < (int)layer.GetSize().width; ++col)
				{
					// get the tile from main layer.
					const TileHandle tile = layer.Get(row, col);

					// if tile is valid, we can queue it for draw. otherwise, we skip it
					if (tile.IsValid())
					{
						// find the top-left position of the tile in map space.
						engine::spatial::PositionF tilePosFromMap =
						{
							col * tileSize.width,
							row * tileSize.height
						};

						// apply scale to tile size in case we want to draw the tile at different size. 
						// note that only size change. position is still based on original tile size 
						engine::spatial::SizeF scaledtilesize
						{
							tileSize.width * scale.x,
							tileSize.height * scale.y
						};

						command.Add({
							tile.GetSprite(),					// sprite object to draw
							worldPos + tilePosFromMap,		// shift tile position from map space to world space
							scaledtilesize,						// size to draw the tile at
							color,								// color
							0.0f,								// no rotation for tile
							1								// depth value for sorting
							});
					}
				}
			}
		}

		void DrawTerrainLayer(
			IRenderer& renderer,
			DrawSortedSpritesCommand& command,
			const WorldMap& world,
			const std::string& layer,
			const PositionF& worldPos,
			const SizeF& tileSize,
			VecF scale = { 1,1 },
			const PositionF& offset = {0,0},
			const ColorF& color = {1,1,1,1}
		)
		{
			world.ForEachTerrainTile(layer, [tileSize, scale, &command, worldPos, color, offset](int row, int col, TileHandle tile)
				{
					// skip invalid tiles.
					if (!tile.GetSprite().IsValid()) return;

					// find the top-left position of the tile in map space.
					engine::spatial::PositionF tilePosFromMap =
					{
						col * tileSize.width,
						row * tileSize.height
					};

					// apply scale to tile size in case we want to draw the tile at different size. 
					// note that only size change. position is still based on original tile size 
					engine::spatial::SizeF scaledtilesize
					{
						tileSize.width * scale.x,
						tileSize.height * scale.y
					};

					tilePosFromMap += offset;

					command.Add({
						tile.GetSprite(),					// sprite object to draw
						worldPos + tilePosFromMap,		// shift tile position from map space to world space
						scaledtilesize,						// size to draw the tile at
						color,								// color
						0.0f,								// no rotation for tile
						1								// depth value for sorting
						});
				});	
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
				splashAnimSet.Register("splash_anim", SpriteAnimationFactory::Create(splashAnimSprites, 100.0f, true, { .33f, .355f }));

				// create tileset that stores water splash animation
				Registry<Tileset<IRenderable>>::Instance().Register("splash_tileset", std::make_unique<Tileset<IRenderable>>());
				Tileset<IRenderable>& splashTileset = assets.Get<Tileset<IRenderable>>("splash_tileset");
				splashTileset.Register(0, std::make_unique<Animated>(splashAnimSet, "splash_anim"));


				{
					// create auto tile config for base grass tile
					Registry<AutoTileSystem::AutoTileConfig>::Instance().Register("splash_tile_auto_config", std::make_unique<AutoTileSystem::AutoTileConfig>());
					AutoTileSystem::AutoTileConfig& config = assets.Get<AutoTileSystem::AutoTileConfig>("splash_tile_auto_config");

					// configure base layer auto-tile mapping
					config.Register(-1, TileVariant::Empty);
					config.Register(0, TileVariant::Island);
					config.Register(-1, TileVariant::Full);
					config.Register(0, TileVariant::NorthEdge);
					config.Register(0, TileVariant::SouthEdge);
					config.Register(0, TileVariant::EastEdge);
					config.Register(0, TileVariant::WestEdge);
					config.Register(0, TileVariant::NECorner);
					config.Register(0, TileVariant::NWCorner);
					config.Register(0, TileVariant::SECorner);
					config.Register(0, TileVariant::SWCorner);
					config.Register(0, TileVariant::Vertical);
					config.Register(0, TileVariant::Horizontal);
					config.Register(0, TileVariant::TNorth);
					config.Register(0, TileVariant::TSouth);
					config.Register(0, TileVariant::TEast);
					config.Register(0, TileVariant::TWest);
				}

				{
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
				}


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
				birchTreeAnimSet.Register("birch_tree_idle", SpriteAnimationFactory::Create(birchTreeAtlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.85f }));

				// create animation set for pine tree
				Registry<AnimationSet<Sprite>>::Instance().Register("pinetree_anim_set", std::make_unique<AnimationSet<Sprite>>());
				auto& pineTreeAnimSet = assets.Get<AnimationSet<Sprite>>("pinetree_anim_set");

				// create "idle" animation for pine tree and register in its animation set
				auto& pineTreeAtlas = assets.Get<ISpriteAtlas>("pine_tree");
				pineTreeAnimSet.Register("pine_tree_idle", SpriteAnimationFactory::Create(pineTreeAtlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.91f }));

				SpriteAtlasFactory::Create("birch_tree", L"../Assets/tree_1x8_1536x192.png", 1, 8); // birch tree

				// setup castle
				SpriteAtlasFactory::Create("castle", L"../Assets/Castle.png", 1, 1); // castle
				Registry<AnimationSet<Sprite>>::Instance().Register("castle_anim_set", std::make_unique<AnimationSet<Sprite>>());
				auto& castleAnimSet = assets.Get<AnimationSet<Sprite>>("castle_anim_set");
				auto& castleAtlas = assets.Get<ISpriteAtlas>("castle");
				castleAnimSet.Register("castle_idle", SpriteAnimationFactory::Create(castleAtlas, std::vector<int>{ 0 }, 200.0f, true, PositionF{ 0.5f, 0.95f }));

				// setup water rocks
				SpriteAtlasFactory::Create("water_rocks", L"../Assets/Water_Rocks.png", 1, 16); // castle
				Registry<AnimationSet<Sprite>>::Instance().Register("water_rocks_anim_set", std::make_unique<AnimationSet<Sprite>>());
				auto& waterRocksAnimSet = assets.Get<AnimationSet<Sprite>>("water_rocks_anim_set");
				auto& waterRockAtlas = assets.Get<ISpriteAtlas>("water_rocks");
				waterRocksAnimSet.Register("water_rocks_idle", SpriteAnimationFactory::Create(waterRockAtlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 ,14 ,15 }, 100.0f, true, PositionF{ 0.5f, 0.8f }));




				// create our tileset and load all sprites from sprite atlas
				{
					TilesetLoader::LoadTerrainSet("grass_tileset", "grass_tile_sprites");

					Registry<TerrainSet>::Instance().Register("splash_tileset", std::make_unique<TerrainSet>());
					TerrainSet& splashTileset = assets.Get<TerrainSet>("splash_tileset");
					std::unique_ptr<TileDefinition> tiledef = std::make_unique<TileDefinition>();
					tiledef->renderable = std::make_unique<Animated>(splashAnimSet, "splash_anim");
					splashTileset.Register(0, std::move(tiledef));
				}
			}

			// initialize scenes
			{
				m_sceneManager.CreateScene<EditorScene>("Edit");
				m_sceneManager.CreateScene<DebugScene>("Debug");
				m_sceneManager.CreateScene<TerrainEditScene>("Terrain");
				m_sceneManager.SetActive("Terrain");
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