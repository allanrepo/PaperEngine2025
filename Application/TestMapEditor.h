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
#include <Math/Size.h>
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
#include <Spatial/Camera.h>

namespace TestMapEditor
{
#pragma region // forward declaration
	class Test;
	struct PropPlacementContext;
	class PropPlacementTool;
	class PropMap;
	class WorldMap;

	class OverlayTrigger;
	class UISystem;
	class LayerStack;
	class Widget;
	struct UIDrawContext;
	class Layer;
	class Frame;
	class Tooltip;
	class Draggable;
	class MenuButton;
	class SubMenuButton;
	class MenuItem;
	class Thumb;
	class Slider;
	class CheckBox;
	class RadioButton;
	class ScrollBar;
	class ResizeableFrame;
	class Grip;
	class ViewPort;
	class Content;
#pragma endregion

#pragma region // namespaces
	using namespace engine;
	using IWindow = win32::Window;
	using Window = win32::Window;
	using ICanvas = engine::graphics::ICanvas;
	using IRenderer = engine::graphics::renderer::IRenderer;
	using StopWatch = timer::StopWatch;
	using PositionF = spatial::PositionF;
	using SizeF = math::SizeF;
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
	using RectF = engine::math::RectF;
	using CSVFileParser = engine::io::CSVFileParser;
	using NavigationGrid = engine::navigation::tile::NavigationGrid;
	using SpriteAtlasLoader = engine::graphics::loader::SpriteAtlasLoader;
	using Scene = engine::scene::Scene;
	using SceneManager = engine::scene::SceneManager;
	using Camera = engine::spatial::CameraF;

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

	template<typename K, typename T>
	using Dictionary = engine::container::Dictionary<K, T>;

	template<typename T>
	using Rect = engine::math::Rect<T>;

	template<typename T>
	using Size = engine::math::Size<T>;

#pragma endregion

#pragma region // TileDefinition
	struct TileDefinition
	{
		std::unique_ptr<IRenderable> renderable = nullptr;
		TileConstraint constraint = TileConstraint::NONE;
	};
#pragma endregion

#pragma region // Tile
	class Tile
	{
	private:
		TileDefinition* m_tileDefinition;
		int m_index;

	public:
		Tile(int index, TileDefinition* td = nullptr) :
			m_index(index),
			m_tileDefinition(td)
		{
		}

		Sprite GetSprite() const noexcept
		{
			return IsValid() ? m_tileDefinition->renderable->GetSprite() : Sprite::MakeInvalidSprite();
		}

		bool IsValid() const noexcept
		{
			return m_tileDefinition != nullptr && m_tileDefinition->renderable != nullptr;
		}


		const int GetIndex() const
		{
			return m_index;
		}

		const TileConstraint GetConstraint() const
		{
			return IsValid() ? m_tileDefinition->constraint : TileConstraint::NONE;
		}
	};
#pragma endregion

#pragma region // TerrainGrid
	class TerrainGrid
	{
	private:
#pragma region // parameters
		engine::container::Grid<Tile> m_map;
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

		math::Size<size_t> GetSize() const
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
		Tile Get(int row, int col)
		{
			return m_map.Get(row, col);
		}

		Tile Get(int row, int col) const
		{
			return m_map.Get(row, col);
		}

		// retrieves the data at Coord
		Tile Get(const engine::spatial::Coord& coord)
		{
			return m_map.Get(coord.row, coord.col);
		}

		// retrieves the data at Coord
		Tile Get(const engine::spatial::Coord& coord) const
		{
			return m_map.Get(coord.row, coord.col);
		}
#pragma endregion

#pragma region // replace value
		void Set(int row, int col, const Tile& data)
		{
			m_map.Set(row, col, data);
		}

		void Set(int row, int col, Tile&& data)
		{
			m_map.Set(row, col, std::move(data));
		}

		void Set(const engine::spatial::Coord& coord, const Tile& data)
		{
			m_map.Set(coord, data);
		}

		void Set(const engine::spatial::Coord& coord, Tile&& data)
		{
			m_map.Set(coord, std::move(data));
		}
#pragma endregion

#pragma region // content management
		void Reserve(const Size<size_t>& size)
		{
			m_map.Reserve(size);
		}

		// remove all tiles, reducing the grid's size (width and height) to 0,0
		void Clear()
		{
			m_map.Clear();
		}

		// copy only, no move option. expects T to be copyable or else
		void Initialize(size_t width, size_t height, const Tile& data)
		{
			m_map.Clear();
			m_map.SetWidth(width);
			m_map.Reserve({ width, height });

			for (size_t i = 0; i < width * height; ++i)
			{
				m_map.Add(data);
			}
		}

		void Initialize(engine::math::Size<size_t> size, const Tile& data)
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

		template<typename Func>
		void ForEach(const Coord& tl, const Coord& br, const Func& func)
		{
			// make sure to clamp to min/max actual value
			int r0 = std::clamp<int>(tl.row, 0, (int)GetHeight());
			int r1 = std::clamp<int>(br.row, 0, (int)GetHeight());
			int c0 = std::clamp<int>(tl.col, 0, (int)GetWidth());
			int c1 = std::clamp<int>(br.col, 0, (int)GetWidth());

			for (int r = r0; r < r1; ++r)
			{
				for (int c = c0; c < c1; ++c)
				{
					func(r, c, Get(r, c));
				}
			}
		}

		template<typename Func>
		void ForEach(const Coord& tl, const Coord& br, const Func& func) const
		{
			// make sure to clamp to min/max actual value
			int r0 = std::clamp<int>(tl.row, 0, (int)GetHeight());
			int r1 = std::clamp<int>(br.row, 0, (int)GetHeight());
			int c0 = std::clamp<int>(tl.col, 0, (int)GetWidth());
			int c1 = std::clamp<int>(br.col, 0, (int)GetWidth());

			for (int r = r0; r < r1; ++r)
			{
				for (int c = c0; c < c1; ++c)
				{
					func(r, c, Get(r, c));
				}
			}
		}
#pragma endregion

	};
#pragma endregion

#pragma region // TerrainSet
	class TerrainSet
	{
	private:
		container::Dictionary<int, std::unique_ptr<TileDefinition>> m_tiles;
		int m_invalidTileIndex;
		const std::string m_name;

		TileDefinition* GetInvalidTileDefinition() const
		{
			static TileDefinition s_invalidTileDefinition;
			return &s_invalidTileDefinition;
		}

	public:
		TerrainSet(const std::string& name, int invalidTileIndex = -0xFFFF) :
			m_invalidTileIndex(invalidTileIndex),
			m_name(name)
		{
		}

		~TerrainSet() = default;

		// non copyable, non movable
		TerrainSet(const TerrainSet&) = delete;
		TerrainSet& operator=(const TerrainSet&) = delete;
		TerrainSet(TerrainSet&&) = delete;
		TerrainSet& operator=(TerrainSet&&) = delete;

		const std::string& GetName() const
		{
			return m_name;
		}

		const size_t Size() const
		{
			return m_tiles.Size();
		}

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
		Tile MakeTile(int id) const
		{
			return Tile(
				m_tiles.Has(id) ? id : m_invalidTileIndex,
				m_tiles.Has(id) ?
				m_tiles.Get(id).get() :		// if we have valid tile definition use it
				GetInvalidTileDefinition()	// otherwise, use invalid one
			);
		}
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
		void Set(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, engine::math::Size<size_t> size)
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
		void Remove(AutoTileContext<T>& ctx, const AutoTileConfig& autoTileConfig, engine::math::Size<size_t> size)
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
		std::string m_name;
		TerrainGrid m_grid;
		const TerrainSet* m_set;

	public:
		TerrainLayer(const std::string& name) :
			m_name(name),
			m_set(nullptr)
		{
		}

		// TODO:
		// parameters are pass by value because they are lightweight. but better to test if this is true
		event::Event<const TerrainLayer&, Coord, Tile> TileChangeEvent;
		event::Event<const TerrainLayer&, Size<size_t>, Tile> InitializeEvent;

		const std::string& GetSetName() const
		{
			return m_set->GetName();
		}

		// TODO: 
		// justify in documentation why identities can be std::string and not faster ones like uint ID's. 
		// we're going to pass reference so it should be just as fast
		const std::string& GetName() const
		{
			return m_name;
		}

		Tile Get(const Coord& coord) const
		{
			return m_grid.Get(coord);
		}

		Tile Get(int row, int col) const
		{
			return m_grid.Get(row, col);
		}

		Size<size_t> GetSize() const
		{
			return m_grid.GetSize();
		}

		void Initialize(size_t width, size_t height, const TerrainSet* set, int index)
		{
			Initialize({ width, height }, set, index);
		}

		void Initialize(const Size<size_t> size, const TerrainSet* set, int index)
		{
			m_set = set;
			Tile tile = m_set->MakeTile(index);

			m_grid.Initialize(size, tile);

			InitializeEvent(*this, size, tile);
		}

		void Clear()
		{
			m_grid.Clear();
			m_set = nullptr;
		}

		void Set(const Coord& c, int index)
		{
			Tile tile = m_set->MakeTile(index);
			m_grid.Set(c, tile);

			TileChangeEvent(*this, c, tile);
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
			m_grid.ForEach(func);
		}

		template<typename Func>
		void ForEach(const Func& func) const
		{
			m_grid.ForEach(func);
		}

		template<typename Func>
		void ForEach(const Coord& tl, const Coord& br, const Func& func)
		{
			m_grid.ForEach(tl, br, func);
		}

		template<typename Func>
		void ForEach(const Coord& tl, const Coord& br, const Func& func) const
		{
			m_grid.ForEach(tl, br, func);
		}
	};
#pragma endregion

#pragma region // TerrainAutoTileAdapter
	class TerrainAutoTileAdapter//: public IAutoTileAdapter
	{
	private:
		TerrainLayer& layer;

	public:
		TerrainAutoTileAdapter(TerrainLayer& l) :
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

		void OnTileChange(const TerrainLayer& layer, Coord coord, Tile tile)
		{
			TileChangeEvent(layer.GetName(), coord, tile);
		}

		void OnInitialize(const TerrainLayer& layer, Size<size_t> size, Tile tile)
		{
			InitializeEvent(layer.GetName(), size, tile);
		}

	public:
		TerrainMap() = default;

		event::Event<const std::string&, Coord, Tile> TileChangeEvent;
		event::Event<const std::string&, Size<size_t>, Tile> InitializeEvent;

		void Add(const std::string& key, size_t width, size_t height, const TerrainSet& set, int tileIndex)
		{
			Add(key, { width, height }, set, tileIndex);
		}

		void Add(const std::string& key, const Size<size_t> size, const TerrainSet& set, int tileIndex)
		{
			if (!Has(key))
			{
				if (!m_layers.Register(key, std::make_unique<TerrainLayer>(key)))
				{
					throw std::runtime_error("failed to create new terrain layer");
				}

				m_layers[key]->TileChangeEvent += event::Handler(this, &TerrainMap::OnTileChange);
				m_layers[key]->InitializeEvent += event::Handler(this, &TerrainMap::OnInitialize);
			}

			m_layers[key]->Initialize(size, &set, tileIndex);
		}

		// TODO: test this
		void Remove(const std::string& key)
		{
			if (!Has(key))
			{
				return;
			}

			m_layers[key]->TileChangeEvent -= event::Handler(this, &TerrainMap::OnTileChange);
			m_layers[key]->InitializeEvent -= event::Handler(this, &TerrainMap::OnInitialize);

			m_layers.Unregister("key");
		}

		bool Has(const std::string& key) const
		{
			return m_layers.Has(key);
		}

		TerrainAutoTileAdapter GetAutoTileAdapter(const std::string& key) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			return TerrainAutoTileAdapter(*m_layers.Get(key));
		}

		Tile Get(const std::string& key, const Coord& coord) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			return  m_layers[key]->Get(coord);
		}

		Tile Get(const std::string& key, int row, int col) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			return  m_layers[key]->Get(row, col);
		}

		void Clear()
		{
			m_layers.Clear();
		}

		template<typename Func>
		void ForEach(const Func& func)
		{
			for (const auto& layer : m_layers)
			{
				func(layer.first, *layer.second.get());
			}
		}

		template<typename Func>
		void ForEach(const Func& func) const
		{
			for (const auto& layer : m_layers)
			{
				func(layer.first, *layer.second.get());
			}
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

		template<typename Func>
		void ForEach(const std::string& key, const Coord& tl, const Coord& br, const Func& func)
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			m_layers[key]->ForEach(tl, br, func);
		}

		template<typename Func>
		void ForEach(const std::string& key, const Coord& tl, const Coord& br, const Func& func) const
		{
			if (!Has(key))
			{
				throw std::runtime_error("no layer found");
			}

			m_layers[key]->ForEach(tl, br, func);
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
			if (!cellsToOccupy.empty())
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
		static TerrainSet& LoadTerrainSet(
			const std::string& name, // key for storing in cache
			const std::string& atlasName, // key of the sprite atlas to get sprites to
			const Dictionary<int, TileConstraint>& indexToConstraintMap,
			const TileConstraint defaultConstraint
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
				std::unique_ptr<TerrainSet> tileset = std::make_unique<TerrainSet>(name);

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

						tiledef->constraint = indexToConstraintMap.Has(i) ? indexToConstraintMap.Get(i) : defaultConstraint;

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

#pragma region // Prop
	struct CollisionShape
	{
		// can change this later for polygon class implementation for more accurate shape
		RectF shape;

		bool Contains(const PositionF& position) const
		{
			return false; // TODO: for now we are not using this yet...
			return shape.Contains(position);
		}
	};

	struct Prop
	{
		std::unique_ptr<IRenderable> renderable;
		//PositionF position; // position of this object relative to its owner
		PositionF worldPosition; // position of this object in the world
		// TODO:
		// we are now using worldPosition when locating Prop. before that, we use this, which is Prop's position in its owning tile's local space.
		// with this information alone, there is an issue when rendering props while culling tiles not visible in camera viewport
		// this prop's owning tile may not be visible in camera viewport but since this prop can be taller than the tile, it can still 
		// occupy tiles above its owning tile, making this prop still visible in screen even when its owning tile is not.
		// in bounding box map, this prop's reference is included in tiles still visible. but to render it, it needs the owning tile coordinate
		// so it can calculate its world position. THAT IS A PROBLEM. so, from now on we are storing Prop's world position and use it for locating
		// the prop. 
		// right now this property is useless but we're keeping it for now. later will decide to remove or not
		PositionF localPosition; // position of this object relative to its owning tile local space
		VecF scale;
		ColorF color;
		RectF footprint;
		RectF boundingBox;
		CollisionShape collisionShape;

		// this is not really used by Prop. this is only used when instancing the Prop to setup its renderable 
		// we are including these here because they are needed when saving prop into PropData 
		std::string animationSet;
		std::string animation;

		inline const std::string& GetAnimationSet() const
		{
			return animationSet;
		}

		inline const std::string& GetAnimation() const
		{
			return animation;
		}

		inline VecF GetScale() const
		{
			return scale;
		}

		inline RectF GetFootprint() const
		{
			return footprint;
		}

		inline RectF GetBoundingBox() const
		{
			return boundingBox;
		}

		inline Sprite GetSprite() const
		{
			return renderable->GetSprite();
		}

		inline ColorF GetColor() const
		{
			return color;
		}

		// TODO: for debug only quick instancing to test. remove this in production code
		Prop()
		{

		}

		Prop(std::unique_ptr<IRenderable> r, const ColorF& c, const VecF& s, const RectF& fp, const RectF& bb, const std::string& animset, const std::string& anim) :
			renderable(std::move(r)),
			localPosition({}),
			worldPosition({}),
			scale(s),
			color(c),
			footprint(fp),
			boundingBox(bb),
			collisionShape({}),
			animationSet(animset),
			animation(anim)
		{
		}

		PositionF GetWorldPosition() const
		{
			return worldPosition;
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

			return true;
		}

		// difference with Initialize() is that this retains world transform of the map
		void Reset()
		{
			m_objectLayer.Reset();

			m_navGrid.Fill(TileConstraint::NONE);

			m_FootPrintGrid.Reset();

			m_BoundingBoxGrid.Reset();
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
				});

			// remove this object from spatial occupancy grid
			m_FootPrintGrid.Remove(prop);

			// remove this object from bounding box occupancy grid
			m_BoundingBoxGrid.Remove(prop);

			// remove this object from object layer
			m_objectLayer.Remove(prop);
		}

		void Add(
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

				// it's possible for footprint to be out of bounds. 
				// a prop can be placed at the edge of a corner or side of the map so itfootprint can spill out of the map. 
				// that is ok so we just skip it
				if (!m_FootPrintGrid.IsInBounds(coord)) continue;

				m_FootPrintGrid.Add(prop.get(), coord, constraint);
				m_navGrid.AddFlag(coord, constraint);
			}

			// this prop to occupy respective coord in bounding box layer in propmap
			for (Coord coord : boundingBoxCells)
			{
				// it's possible for bounding box to be out of bounds. 
				// a prop can be placed at the edge of a corner or side of the map so its bounding box can spill out of the map. 
				// that is ok so we just skip it
				if (!m_BoundingBoxGrid.IsInBounds(coord)) continue;
				m_BoundingBoxGrid.Add(prop.get(), coord, {});
			}

			// add the actual prop object into prop map
			m_objectLayer.Add(coord, std::move(prop));
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

		template<typename Func>
		void ForEachProp(int row, int col, const Func& func)
		{
			m_objectLayer.ForEach(row, col, func);
		}

		template<typename Func>
		void ForEachProp(int row, int col, const Func& func) const
		{
			m_objectLayer.ForEach(row, col, func);
		}

		template<typename Func>
		void ForEachProp(const Func& func)
		{
			m_objectLayer.ForEach(func);
		}

		template<typename Func>
		void ForEachProp(const Func& func) const
		{
			m_objectLayer.ForEach(func);
		}

		template<typename Func>
		void ForEachTileConstraint(int row, int col, const Func& func) const
		{
			m_navGrid.ForEach(row, col, func);
		}

		template<typename Func>
		void ForEachPropInFootPrint(const Coord& coord, const Func& func)
		{
			m_FootPrintGrid.ForEach(coord, func);
		}

		template<typename Func>
		void ForEachPropInBoundingBox(const Coord& coord, const Func& func)
		{
			m_BoundingBoxGrid.ForEachObject(coord, func);
		}

		template<typename Func>
		void ForEachPropInBoundingBox(const Coord& coord, const Func& func) const
		{
			m_BoundingBoxGrid.ForEachObject(coord, func);
		}

		template<typename Func>
		void ForEachFootprintTile(const Prop* prop, const Func& func) const
		{
			m_FootPrintGrid.ForEachCell(prop, func);
		}

		std::vector<Coord> GetOccupiedFootprintTiles(const Prop* prop) const
		{
			return m_FootPrintGrid.GetOccupiedCells(prop);
		}

		std::vector<Coord> GetOccupiedFootprintTiles() const
		{
			return m_FootPrintGrid.GetOccupiedCells();
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
		WorldTransform() :
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

		SizeF GetWorldSize() const
		{
			return SizeF
			{
				m_size.As<float>().width * m_tilesize.As<float>().width,
				m_size.As<float>().height * m_tilesize.As<float>().height,
			};
		}

	};
#pragma endregion

#pragma region // WorldMap
	class WorldMap
	{
	private:
#pragma region // parameters
		WorldTransform m_worldTransform;
		PropMap m_propMap;
		TerrainMap m_terrainMap;
		NavigationGrid m_navigationGrid;
#pragma endregion

#pragma region // event handlers for updating navigation grid

		// this method will resolve the cumulative tile constraint of given coord
		// it will combine the tile constraints of all active terrain layer and props in the coord 
		void RefreshNavigationGrid(Coord coord)
		{
			// sanity check
			if (!m_navigationGrid.IsInBounds(coord))
			{
				throw std::runtime_error("out of bounds navigation grid");
			}

			// reset the navgrid's coord first. 
			// we will fill it with all constraints that exists in this coord from terrain and props that occupies it
			m_navigationGrid.Set(coord, TileConstraint::NONE);

			// iterate through each layer in terrain map
			m_terrainMap.ForEach([&](const std::string& key, const TerrainLayer& layer)
				{
					// OR constraint of each layer's coord to our navigation grid coord
					m_navigationGrid.AddFlag(coord, layer.Get(coord).GetConstraint());
				});

			// get all props occupying the footprint grid's coord. get their corresponding tile constraint. OR them all together into navgrid's coord
			m_propMap.ForEachPropInFootPrint(coord, [&](const Prop* prop, TileConstraint c)
				{
					m_navigationGrid.AddFlag(coord, c);
				});
		}

		// this is the event handler for event when a terrain layer changes a tile in a given coord
		void OnTerrainTileChange(const std::string& layer, Coord coord, Tile tile)
		{
			RefreshNavigationGrid(coord);
		}

		// this is the event handler for event when terrain layer is initialized. 
		// it will iterate through each tile and refresh the navigation grid's tile constraint
		void OnTerrainInitialize(const std::string& layer, Size<size_t> size, Tile tile)
		{
			// iterate through each tile coord in world map
			for (int row = 0; row < GetTransform().GetSize().height; row++)
			{
				for (int col = 0; col < GetTransform().GetSize().width; col++)
				{
					// given this coord, refresh its constraint value on navigation grid
					RefreshNavigationGrid({ row, col });
				}
			}
		}
#pragma endregion

	public:

#pragma region // constructors
		WorldMap()
		{
			m_terrainMap.TileChangeEvent += event::Handler(this, &WorldMap::OnTerrainTileChange);
			m_terrainMap.InitializeEvent += event::Handler(this, &WorldMap::OnTerrainInitialize);
		}
#pragma endregion

#pragma region // accessors and queries
		const WorldTransform& GetTransform() const
		{
			return m_worldTransform;
		}

		bool Has(Prop* prop) const
		{
			return m_propMap.Has(prop);
		}

		std::string GetDebugInfo() const
		{
			return m_propMap.GetDebugInfo();
		}
#pragma endregion

#pragma region // initialize
		// clears all the components of world map. all resources are removed - terrains, props, and internal systems like footprint grid
		// all grids are cleared, meaning their grid sizes are reset to 0,0 and recreated with new size
		// does not bother to refresh navigation grid because it removes all terrains and props. so it assumes there is no constraints
		// therefore resetting all to TileConstraint::NONE
		bool Initialize(const PositionF& position, const Size<size_t>& size, const SizeF& tilesize)
		{
			// set world transform
			m_worldTransform.SetPosition(position);
			m_worldTransform.SetSize(size);
			m_worldTransform.SetTileSize(tilesize);

			// initialized prop map. if world map already exists, this will totally wipe the prop map, removing all props, as well as all its cells.
			// the containers of prop map (footprint grid, boundingbox grid, etc...) will be cleared and grid size be 0,0 then initialized to new
			// grid size
			if (!m_propMap.Initialize(m_worldTransform.GetPosition(), m_worldTransform.GetSize(), m_worldTransform.GetTileSize())) return false;

			// remove all existing terrain layers
			m_terrainMap.Clear();

			// initialize our navigation grid. since there are currently no terrains or props, fill it with all walkable as default
			m_navigationGrid.Initialize(m_worldTransform.GetSize(), TileConstraint::NONE);

			return true;
		}
#pragma endregion

#pragma region // mutations
		void InsertProp(
			std::unique_ptr<Prop> prop,
			const Coord& coord,
			const std::vector<Coord>& boundingBoxCells,
			const Dictionary<Coord, TileConstraint>& coordToConstraints
		)
		{
			// let's add the prop into prop map
			m_propMap.Add(std::move(prop), coord, boundingBoxCells, coordToConstraints);

			// now that the new prop is added, let's refresh the navigation grid's coords this prop's footprint occupied
			// these coords should have new constraints now that the new prop is placed
			for (auto& kvp : coordToConstraints)
			{
				RefreshNavigationGrid(kvp.first);
			}
		}

		void RemoveProp(Prop* prop)
		{
			// find cells in footprint grid this prop occupies
			std::vector<Coord> coords = m_propMap.GetOccupiedFootprintTiles(prop);

			// remove the prop
			m_propMap.Remove(prop);

			// now the prop is gone from the map, but we know which cells in footprint grid it occupied before...
			// so we refresh them
			for (const Coord& coord : coords)
			{
				RefreshNavigationGrid(coord);
			}
		}

		// WARNING: this is an expensive method. 
		// removes all the props in the map. it will also refresh the navigation grid 
		void RemoveAllProps()
		{
			// find cells in footprint grid of prop map where all props occupies
			std::vector<Coord> coords = m_propMap.GetOccupiedFootprintTiles();

			// reset prop map. this will remove all the props
			m_propMap.Reset();

			// now that all the props are gone from the map, we need to refresh the navigation grid so that tiles that does not have props
			// anymore will be updated with new constraints. we collected occupied tiles by props before so we refresh those tiles
			for (const Coord& coord : coords)
			{
				RefreshNavigationGrid(coord);
			}
		}
#pragma endregion

#pragma region // bounds check
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
#pragma endregion

#pragma region // validate
		void Validate() const
		{
			m_propMap.Validate();
		}
#pragma endregion

#pragma region // iteration with props
		template<typename Func>
		void ForEachProp(const Func& func)
		{
			m_propMap.ForEachProp(func);
		}

		template<typename Func>
		void ForEachProp(const Func& func) const
		{
			m_propMap.ForEachProp(func);
		}

		template<typename Func>
		void ForEachProp(int row, int col, const Func& func)
		{
			m_propMap.ForEachProp(row, col, func);
		}

		template<typename Func>
		void ForEachProp(int row, int col, const Func& func) const
		{
			m_propMap.ForEachProp(row, col, func);
		}
#pragma endregion

#pragma region // iteration with navigation grid
		template<typename Func>
		void ForEachNavigationTile(int row, int col, const Func& func)
		{
			m_navigationGrid.ForEach(row, col, func);
		}

		template<typename Func>
		void ForEachNavigationTile(int row, int col, const Func& func) const
		{
			m_navigationGrid.ForEach(row, col, func);
		}
#pragma endregion

#pragma region // iteration with footprint grid
		// access props occupying a given coord in footprint grid. 
		// used in placement tool where it finds props occupying a given coord to check if 
		// a new prop to place overlaps existing props
		template<typename Func>
		void ForEachPropInFootPrint(const Coord& coord, const Func& func)
		{
			m_propMap.ForEachPropInFootPrint(coord, func);
		}
#pragma endregion

#pragma region // iteration with bounding box grid
		// access props occupying a given coord in bounding box grid. 
		// used in selection tool where it finds props occupying a given coord to check 
		// props for selection or top selection
		template<typename Func>
		void ForEachPropInBoundingBox(const Coord& coord, const Func& func)
		{
			m_propMap.ForEachPropInBoundingBox(coord, func);
		}

		template<typename Func>
		void ForEachPropInBoundingBox(const Coord& coord, const Func& func) const
		{
			m_propMap.ForEachPropInBoundingBox(coord, func);
		}
#pragma endregion

#pragma region // safe query of position of a prop in the world, if this prop exists
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

			position = prop->GetWorldPosition();
			return true;
		}
#pragma endregion

#pragma region // terrain
		// mutate terrain map 
		void AddTerrain(const std::string& key, const TerrainSet& set, int tileIndex)
		{
			m_terrainMap.Add(key, GetTransform().GetSize(), set, tileIndex);
		}

		// check if this terrain exists
		bool HasTerrain(const std::string& key) const
		{
			return m_terrainMap.Has(key);
		}

		// create adapter for autotiling. this is simply a getter for terrain layer reference
		TerrainAutoTileAdapter GetAutoTileAdapter(const std::string& key) const
		{
			return m_terrainMap.GetAutoTileAdapter(key);
		}

		// iterates through each terrain layer in terrain map
		// function signature -> (const std::string& key, const TerrainLayer& layer)
		template<typename Func>
		void ForEachTerrain(const Func& func)
		{
			m_terrainMap.ForEach(func);
		}

		// iterates through each terrain layer in terrain map
		// function signature -> (const std::string& key, const TerrainLayer& layer)
		template<typename Func>
		void ForEachTerrain(const Func& func) const
		{
			m_terrainMap.ForEach(func);
		}

		// queries tile on all coords of a terrain layer specified with key
		// function signature -> (int row, int col, Tile tile)
		template<typename Func>
		void ForEachTileInTerrain(const std::string& key, const Func& func) const
		{
			m_terrainMap.ForEach(key, func);
		}

		// queries tile on all coords of a terrain layer specified with key
		// function signature -> (int row, int col, Tile tile)
		template<typename Func>
		void ForEachTileInTerrain(const std::string& key, const Func& func)
		{
			m_terrainMap.ForEach(key, func);
		}

		// queries tile for each coord given range of coord tl (top-left) and br (bottom-right)
		// function signature -> (int row, int col, Tile tile)
		template<typename Func>
		void ForEachTileInTerrain(const std::string& key, const Coord& tl, const Coord& br, const Func& func)
		{
			m_terrainMap.ForEach(key, tl, br, func);
		}

		// queries tile for each coord given range of coord tl (top-left) and br (bottom-right)
		// function signature -> (int row, int col, Tile tile)
		template<typename Func>
		void ForEachTileInTerrain(const std::string& key, const Coord& tl, const Coord& br, const Func& func) const
		{
			m_terrainMap.ForEach(key, tl, br, func);
		}

#pragma endregion

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
				AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };
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
				AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };
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

					TerrainAutoTileAdapter adapter = m_world.GetAutoTileAdapter(link.targetBrush.layer);
					AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> context{ adapter };
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
			AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };

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
			AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };

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

		bool Fill(const WorldMap& world)
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
			AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };

			// create autotilesystem
			AutoTileSystem ats;

			// create our link tool. this will subscribe to our auto tile system and will update all linked layers when our source layer change tiles
			TerrainLinkTool tlt(world, ats, m_links, m_currentBrush);

			// perform the paint and auto paint neighbors if needed
			ats.Set(context, *m_currentBrush.config, world.GetTransform().GetSize());

			return true;
		}

		bool Clear(const WorldMap& world)
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
			AutoTileSystem::AutoTileContext<TerrainAutoTileAdapter> context{ world.GetAutoTileAdapter(m_currentBrush.layer) };

			// create autotilesystem
			AutoTileSystem ats;

			// create our link tool. this will subscribe to our auto tile system and will update all linked layers when our source layer change tiles
			TerrainLinkTool tlt(world, ats, m_links, m_currentBrush);

			// perform the remove and auto remove neighbors if needed
			ats.Remove(context, *m_currentBrush.config, world.GetTransform().GetSize());

			return true;
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
				brush.boundingBox,
				brush.animationSet,
				brush.animation
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
					if (!world.TryGetWorldPosition(candidate, propPosInWorld))
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

#pragma region // PropPlacementTool
	class PropPlacementTool
	{
	private:
	public:
		struct Result
		{
			bool success = false;
			std::vector<Coord> occupiedTiles;
			std::vector<Coord> boundingTiles;
		};

		PropPlacementTool()
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

		static Result Place(WorldMap& world, std::unique_ptr<Prop> prop, const PositionF& worldPosition, bool evictOverlaps = true)
		{
			Result result;

			// --------------------------------------------------------------------------------
			// VALIDATION
			// --------------------------------------------------------------------------------

			// validate the position is within bounds of the world. if not, bail out
			if (!world.IsInBounds(worldPosition))
			{
				result.success = false;
				return result;
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
			Dictionary<Coord, TileConstraint> footprintTiles = BuildConstraints(world, footprint);

			// get the cells in the map where the bounding box of this prop overlaps
			RectF boundingBox = prop->GetScaledBoundingBoxWorld(worldPosition, true);
			std::vector<Coord> boundingBoxTiles = QueryCoords(boundingBox, world.GetTransform().GetTileSize());

			// store in result
			result.boundingTiles = boundingBoxTiles;

			// --------------------------------------------------------------------------------
			// REMOVE OVERLAPS (RULE)
			// --------------------------------------------------------------------------------
			if (evictOverlaps)
			{

				// iterate through each cells the prop overlapped. we will check if the props in these cells are overlapped by the new prop
				std::unordered_set<Prop*> toEvict;
				for (auto& tileToConstraint : footprintTiles)
				{
					Coord coord = tileToConstraint.first;

					// store in result
					result.occupiedTiles.push_back(coord);

					// what we're doing here is we are comparing each of the coord in map that the footprint intersected. 
					// each coord is occupied by existing prop, and these props have their corresponding tile constraint in this coord
					// we then compare the tile constraint of existing prop in this coord to the new tile constraint when footprint is applied
					// if these tile constraints overlaps (any constraint bits are both high), the prop
					world.ForEachPropInFootPrint(coord, [tileToConstraint, &toEvict](Prop* prop, TileConstraint constraint)
						{
							// if they overlap, we will remove this prop
							TileConstraint tc = tileToConstraint.second & constraint;
							if (tc != TileConstraint::NONE)
							{
								toEvict.insert(prop);
							}
						});
				}

				// remove props overlapped by new prop
				for (Prop* prop : toEvict)
				{
					world.RemoveProp(prop);
				}
			}

			// --------------------------------------------------------------------------------
			// APPLY PLACEMENT
			// --------------------------------------------------------------------------------

			// TODO: we probably be removing this soon...
			// we set it as position of this prop
			prop->localPosition = world.GetTransform().WorldToTileSpace(worldPosition);

			// remember its world position
			prop->worldPosition = worldPosition;

			// add the actual object into our object layer
			world.InsertProp(std::move(prop), world.GetTransform().WorldToTileCoord(worldPosition), boundingBoxTiles, footprintTiles);

			result.success = true;
			return result;
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
		PropPlacementTool::Result Paint(WorldMap& world, const PositionF& worldPosition) const
		{
			// gets the animation set from asset based on the brush
			// this is strict. if animation set does not exist in cache, this will throw
			// design note: maybe it's a bit complicated to create fallback of animation set so we leave it like this 
			AssetManager assets;
			auto& animSet = assets.Get<AnimationSet<Sprite>>(m_currentBrush.animationSet);

			// create the prop based on the brush
			std::unique_ptr<Prop> prop = PropFactory::Create(m_currentBrush, animSet);

			// delegate placement on placement system
			return PropPlacementTool::Place(world, std::move(prop), worldPosition);
		}

		// erase top-most prop from the world at given position in the world
		bool Erase(WorldMap& world, const PositionF& worldPosition)
		{
			return PropPlacementTool::Remove(world, worldPosition);
		}
	};

#pragma endregion

#pragma region // Draw function utilities
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
				const Tile& tile = grid.Get(row, col);

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
					engine::math::SizeF scaledtilesize
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
				const Tile tile = layer.Get(row, col);

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
					engine::math::SizeF scaledtilesize
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
		const PositionF& offset = { 0,0 },
		const ColorF& color = { 1,1,1,1 }
	)
	{
		world.ForEachTileInTerrain(layer, [tileSize, scale, &command, worldPos, color, offset](int row, int col, Tile tile)
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
				engine::math::SizeF scaledtilesize
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

	void DrawTerrainLayer(
		IRenderer& renderer,
		DrawSortedSpritesCommand& command,
		const Camera& camera,
		const WorldMap& world,
		const std::string& layer,
		const PositionF& worldPos,
		const SizeF& tileSize,
		bool drawAllTiles = false,
		VecF scale = { 1,1 },
		const PositionF& offset = { 0,0 },
		const ColorF& color = { 1,1,1,1 }
	)
	{
		if (!world.HasTerrain(layer))
		{
			return;
		}

		Coord topLeft;
		Coord bottomRight;

		if (!drawAllTiles)
		{
			RectF vp = camera.GetViewport();
			PositionF pos = camera.GetPosition();
			float zoom = camera.GetZoom();

			topLeft =
			{
				(int)(pos.y / tileSize.height),
				(int)(pos.x / tileSize.width),
			};

			bottomRight =
			{
				(int)((pos.y + vp.GetHeight() / zoom) / tileSize.height) + 1,
				(int)((pos.x + vp.GetWidth() / zoom) / tileSize.width) + 1,
			};
		}
		else
		{
			topLeft = { 0,0 };
			bottomRight = { world.GetTransform().GetSize().As<int>().height, world.GetTransform().GetSize().As<int>().width };
		}

		world.ForEachTileInTerrain(layer, topLeft, bottomRight, [tileSize, scale, &command, worldPos, color, offset, camera, &renderer](int row, int col, Tile tile)
			{
				// skip invalid tiles.
				if (!tile.GetSprite().IsValid()) return;

				// find the top-left position of the tile in map space.
				engine::spatial::PositionF tilePosFromWorld =
				{
					col * tileSize.width,
					row * tileSize.height
				};

				// apply scale to tile size in case we want to draw the tile at different size. 
				// note that only size change. position is still based on original tile size 
				engine::math::SizeF scaledtilesize
				{
					tileSize.width * scale.x,
					tileSize.height * scale.y
				};

				scaledtilesize *= camera.GetZoom();

				tilePosFromWorld += offset;

				tilePosFromWorld += worldPos;

				PositionF tilePosInScreen = camera.WorldToScreen(tilePosFromWorld);

				//renderer.Draw(tile.GetSprite(), tilePosInScreen, scaledtilesize, color, 0.0f);
				//renderer.Draw(tilePosInScreen, scaledtilesize, color, 0.0f);
				command.Add({
					tile.GetSprite(),					// sprite object to draw
					tilePosInScreen,		// shift tile position from map space to world space
					scaledtilesize,						// size to draw the tile at
					color,								// color
					0.0f,								// no rotation for tile
					1								// depth value for sorting
					});
			});
	}

	void DrawTerrainGrid(
		IRenderer& renderer,
		DrawSortedSpritesCommand& command,
		const Camera& camera,
		const TerrainGrid& grid,
		const PositionF& worldPos,
		const SizeF& tileSize,
		bool drawAllTiles = false,
		VecF scale = { 1,1 },
		const PositionF& offset = { 0,0 },
		const ColorF& color = { 1,1,1,1 }
	)
	{
		Coord topLeft;
		Coord bottomRight;

		if (!drawAllTiles)
		{
			RectF vp = camera.GetViewport();
			PositionF pos = camera.GetPosition();
			float zoom = camera.GetZoom();

			topLeft =
			{
				(int)(pos.y / tileSize.height),
				(int)(pos.x / tileSize.width),
			};

			bottomRight =
			{
				(int)((pos.y + vp.GetHeight() / zoom) / tileSize.height) + 1,
				(int)((pos.x + vp.GetWidth() / zoom) / tileSize.width) + 1,
			};
		}
		else
		{
			topLeft = { 0,0 };
			bottomRight = { grid.GetSize().As<int>().height, grid.GetSize().As<int>().width };
		}

		grid.ForEach(topLeft, bottomRight, [tileSize, scale, &command, worldPos, color, offset, camera, &renderer](int row, int col, Tile tile)
			{
				// skip invalid tiles.
				if (!tile.GetSprite().IsValid()) return;

				// find the top-left position of the tile in map space.
				engine::spatial::PositionF tilePosFromWorld =
				{
					col * tileSize.width,
					row * tileSize.height
				};

				// apply scale to tile size in case we want to draw the tile at different size. 
				// note that only size change. position is still based on original tile size 
				engine::math::SizeF scaledtilesize
				{
					tileSize.width * scale.x,
					tileSize.height * scale.y
				};

				scaledtilesize *= camera.GetZoom();

				tilePosFromWorld += offset;

				tilePosFromWorld += worldPos;

				PositionF tilePosInScreen = camera.WorldToScreen(tilePosFromWorld);

				//renderer.Draw(tile.GetSprite(), tilePosInScreen, scaledtilesize, color, 0.0f);
				//renderer.Draw(tilePosInScreen, scaledtilesize, color, 0.0f);
				command.Add({
					tile.GetSprite(),					// sprite object to draw
					tilePosInScreen,		// shift tile position from map space to world space
					scaledtilesize,						// size to draw the tile at
					color,								// color
					0.0f,								// no rotation for tile
					1								// depth value for sorting
					});
			});
	}


	void DrawProps(
		IRenderer& renderer,
		DrawSortedSpritesCommand& command,
		const Camera& camera,
		const WorldMap& world,
		float depth,
		bool drawAllTiles = false
	)
	{
		Coord topLeft;
		Coord bottomRight;
		const SizeF& tilesize = world.GetTransform().GetTileSize();

		if (!drawAllTiles)
		{
			RectF vp = camera.GetViewport();
			PositionF pos = camera.GetPosition();
			float zoom = camera.GetZoom();


			topLeft =
			{
				(int)(pos.y / tilesize.height),
				(int)(pos.x / tilesize.width),
			};

			bottomRight =
			{
				(int)((pos.y + vp.GetHeight() / zoom) / tilesize.height) + 1,
				(int)((pos.x + vp.GetWidth() / zoom) / tilesize.width) + 1,
			};


			Size<size_t> worldSize = world.GetTransform().GetSize();

			bottomRight.col = std::min<int>(bottomRight.col, static_cast<int>(worldSize.width));
			bottomRight.row = std::min<int>(bottomRight.row, static_cast<int>(worldSize.height));

			topLeft.col = std::max<int>(0, topLeft.col);
			topLeft.row = std::max<int>(0, topLeft.row);
		}
		else
		{
			topLeft = { 0,0 };
			bottomRight = { world.GetTransform().GetSize().As<int>().height, world.GetTransform().GetSize().As<int>().width };
		}

		std::unordered_set<Prop*> props;
		for (int row = topLeft.row; row < bottomRight.row; row++)
		{
			for (int col = topLeft.col; col < bottomRight.col; col++)
			{
				// get tile position from world (top-left)
				engine::spatial::PositionF tilePosFromWorld =
				{
					col * tilesize.width,
					row * tilesize.height
				};

				Coord coord;
				coord.col = col;
				coord.row = row;

				world.ForEachPropInBoundingBox(coord, [&props](Prop* prop)
					{
						props.insert(prop);
					});
			}
		}

		for (Prop* prop : props)
		{
			// scale the sprite
			SizeF size = prop->renderable->GetSprite().GetSize();
			size.width *= prop->scale.x;
			size.height *= prop->scale.y;

			size *= camera.GetZoom();

			PositionF propWorldPos = prop->GetWorldPosition();

			PositionF propScreenPos = camera.WorldToScreen(propWorldPos);

			command.Add({
				prop->renderable->GetSprite(),		// sprite object to draw
				propScreenPos,			// position
				size,							// scaled size
				prop->color,							// color
				0.0f,							// no rotation
				depth							// depth value for sorting
				});
		}
	}

	void DrawNavigationOverlay(
		IRenderer& renderer,
		DrawSortedSpritesCommand& command,
		const Camera& camera,
		const WorldMap& world,
		bool drawAllTiles = false
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

		// get sub tile size. 
		SizeF subTileSize(world.GetTransform().GetTileSize().width / 3.0f, world.GetTransform().GetTileSize().height / 3.0f);

		// information used to define the size of the rect to render per sub tile
		VecF shift(subTileSize.width * 0.25f, subTileSize.height * 0.25f);
		SizeF overlaySize(subTileSize.width / 2.0f, subTileSize.height / 2.0f);

		// calculate visible coord in viewport
		Coord topLeft;
		Coord bottomRight;
		const SizeF& tilesize = world.GetTransform().GetTileSize();
		if (!drawAllTiles)
		{
			RectF vp = camera.GetViewport();
			PositionF pos = camera.GetPosition();
			float zoom = camera.GetZoom();


			topLeft =
			{
				(int)(pos.y / tilesize.height),
				(int)(pos.x / tilesize.width),
			};

			bottomRight =
			{
				(int)((pos.y + vp.GetHeight() / zoom) / tilesize.height) + 1,
				(int)((pos.x + vp.GetWidth() / zoom) / tilesize.width) + 1,
			};


			Size<size_t> worldSize = world.GetTransform().GetSize();

			bottomRight.col = std::min<int>(bottomRight.col, static_cast<int>(worldSize.width));
			bottomRight.row = std::min<int>(bottomRight.row, static_cast<int>(worldSize.height));

			topLeft.col = std::max<int>(0, topLeft.col);
			topLeft.row = std::max<int>(0, topLeft.row);
		}
		else
		{
			topLeft = { 0,0 };
			bottomRight = { world.GetTransform().GetSize().As<int>().height, world.GetTransform().GetSize().As<int>().width };
		}

		// iterate through each row and col coord in navigation grid
		for (int row = topLeft.row; row < bottomRight.row; row++)
		{
			for (int col = topLeft.col; col < bottomRight.col; col++)
			{
				// access tile constraint value of each tile in grid
				world.ForEachNavigationTile(row, col, [row, col, &subTileSize, &shift, &overlaySize, &renderer, &camera, &world](TileConstraint constraint)
					{
						// skip empty tiles early (fast path)
						if (constraint == TileConstraint::NONE) return;

						// find the top-left position of the tile in world space.
						engine::spatial::PositionF tilePosFromWorld =
						{
							col * world.GetTransform().GetTileSize().width,
							row * world.GetTransform().GetTileSize().height
						};

						// apply zoom from camera
						tilePosFromWorld *= camera.GetZoom();

						// translate to tile position to worldmap position in world. it's possible worldmap's top-left position in world is not 0,0
						tilePosFromWorld += world.GetTransform().GetPosition();

						// finally translate it to screen position
						PositionF tilePosInScreen = camera.WorldToScreen(tilePosFromWorld);

						// iterate 3x3 subcells
						for (const auto& offset : offsets)
						{
							// for this subcell, check if its corresponding constraint bit is set. if not, skip						
							if (!HasFlag(constraint, offset.bit))
							{
								continue;
							}

							// compute subcell position
							PositionF subCellPos = tilePosInScreen;
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

					});
			}
		}
	}

#pragma endregion

#pragma region // WorldMapFile
	// TODO:
	// BIG ONE. this works. but its a big pile of soup. needs refactoring. but will do it later.
	// this is a self contained system that does one thing (or 2) so i can leave it alone for now.
	// what i can do now is observe for unexpected behaviors and errors
	class WorldMapFile
	{
	private:
		// prop data structure for serializing
		struct PropData
		{
			// Identity / reconstruction
			std::string animationSet;
			std::string animation;

			// Transform / placement
			PositionF worldPosition;
			VecF scale;
			ColorF color;

			// Spatial definition (important for rebuild consistency)
			RectF footprint;
			RectF boundingBox;
		};

		// terrain layer data structure for serializing
		struct TerrainLayerData
		{
			std::string name;
			std::string tileset;
			std::vector<int32_t> tiles;
		};

		// worldmap structure for serializing
		struct WorldMapData
		{
			PositionF position;
			Size<size_t> size;
			SizeF tilesize;
			std::vector<TerrainLayerData> terrains;
			std::vector<PropData> props;
		};

		// this method will parse the data from line. since we expect data from CSV file, we assume delimiter is a ','
		// in this parser, we take whatever data between delimiter as the data and store it as std::string
		static std::vector<std::string> ParseLine(const std::string& line, char delimiter)
		{
			std::vector<std::string> tokens;
			std::string token;
			std::istringstream ss(line);
			// std::getline reads from the stream ss into token, stopping whenever it encounters the delimiter ','
			// Each time it succeeds, the extracted substring is appended to the tokens vector
			while (std::getline(ss, token, delimiter))
			{
				tokens.push_back(token);
			}
			return tokens;
		}

		static std::string Trim(const std::string& str)
		{
			size_t start = str.find_first_not_of(" \t\r\n");

			if (start == std::string::npos)
			{
				return "";
			}

			size_t end = str.find_last_not_of(" \t\r\n");

			return str.substr(start, end - start + 1);
		}

		static PropBrush ToPropBrush(const PropData& data)
		{
			PropBrush brush;
			brush.animationSet = data.animationSet;
			brush.animation = data.animation;
			brush.scale = data.scale;
			brush.color = data.color;
			brush.footprint = data.footprint;
			brush.boundingBox = data.boundingBox;
			return brush;
		}

	public:
		static void Save(
			const std::string& path,
			const WorldMap& world)
		{
			// ------------------------------------------------------------
			// Build serializable data
			// ------------------------------------------------------------

			// world data
			WorldMapData data;
			data.position = world.GetTransform().GetPosition();
			data.size = world.GetTransform().GetSize();
			data.tilesize = world.GetTransform().GetTileSize();

			// terrain data 
			world.ForEachTerrain(
				[&data](
					const std::string& key,
					const TerrainLayer& layer)
				{
					TerrainLayerData terrain;

					terrain.name = key;
					terrain.tileset = layer.GetSetName();

					layer.ForEach(
						[&terrain](
							int row,
							int col,
							const Tile& tile)
						{
							terrain.tiles.push_back(
								tile.GetIndex());
						});

					data.terrains.push_back(
						std::move(terrain));
				});


			// prop data
			world.ForEachProp(
				[&data](size_t row, size_t col, const Prop* prop)
				{
					PropData pd;

					pd.animationSet = prop->GetAnimationSet();
					pd.animation = prop->GetAnimation();
					pd.worldPosition = prop->GetWorldPosition();
					pd.scale = prop->GetScale();
					pd.color = prop->GetColor();
					pd.footprint = prop->GetFootprint();
					pd.boundingBox = prop->GetBoundingBox();
					data.props.push_back(std::move(pd));
				});

			// ------------------------------------------------------------
			// Write file
			// ------------------------------------------------------------
			std::ofstream file(path);

			if (!file.is_open())
			{
				throw std::runtime_error(
					"WorldMapFile::Save() - failed to open file");
			}

			// World header
			file << "worldmap, begin\n";

			file << "position, "
				<< data.position.x
				<< ", "
				<< data.position.y
				<< "\n";

			file << "size, "
				<< data.size.width
				<< ", "
				<< data.size.height
				<< "\n\n";

			file << "tilesize, "
				<< data.tilesize.width
				<< ", "
				<< data.tilesize.height
				<< "\n\n";

			// Terrain layers
			for (const auto& terrain : data.terrains)
			{
				file << "terrainlayer, begin\n";

				file << "name, "
					<< terrain.name
					<< "\n";

				file << "tileset, "
					<< terrain.tileset
					<< "\n";

				file << "tiles, begin\n";

				for (size_t row = 0;
					row < data.size.height;
					++row)
				{
					for (size_t col = 0;
						col < data.size.width;
						++col)
					{
						size_t index =
							row * data.size.width + col;

						file << terrain.tiles[index];

						if (col + 1 < data.size.width)
						{
							file << ", ";
						}
					}

					file << "\n";
				}

				file << "tiles, end\n";

				file << "terrainlayer, end\n\n";
			}

			// Props
			for (const PropData& prop : data.props)
			{
				file << "prop, begin\n";

				file << "animationset, " << prop.animationSet << "\n";
				file << "animation, " << prop.animation << "\n";

				file << "worldposition, "
					<< prop.worldPosition.x << ", "
					<< prop.worldPosition.y << "\n";

				file << "scale, "
					<< prop.scale.x << ", "
					<< prop.scale.y << "\n";

				file << "color, "
					<< prop.color.red << ", "
					<< prop.color.green << ", "
					<< prop.color.blue << ", "
					<< prop.color.alpha << "\n";

				file << "footprint, "
					<< prop.footprint.left << ", "
					<< prop.footprint.top << ", "
					<< prop.footprint.right << ", "
					<< prop.footprint.bottom << "\n";

				file << "boundingbox, "
					<< prop.boundingBox.left << ", "
					<< prop.boundingBox.top << ", "
					<< prop.boundingBox.right << ", "
					<< prop.boundingBox.bottom << "\n";

				file << "prop, end\n\n";
			}

			file << "worldmap, end\n";
		}


		static bool Load(
			const std::string& path,
			WorldMap& world,
			AssetManager& assets)
		{
			// open file
			std::ifstream file(path);
			if (!file.is_open())
			{
				return false;
			}

			// data 
			WorldMapData data;

			// trackers
			std::string line;
			TerrainLayerData* currentTerrain = nullptr;
			bool readingTiles = false;
			PropData* currentProp = nullptr;

			// ------------------------------------------------------------
			// Parse file
			// ------------------------------------------------------------
			while (std::getline(file, line))
			{
				if (line.empty())
				{
					continue;
				}

				// parse line into tokens
				std::vector<std::string> tokens = ParseLine(line, ',');

				// trim whitespaces if any
				for (std::string& token : tokens)
				{
					token = Trim(token);
				}

				// bail out if there are no tokens. how is this ever possible? no clue
				if (tokens.empty())
				{
					continue;
				}

				// World begin/end
				if (tokens[0] == "worldmap")
				{
					continue;
				}

				// World position
				else if (tokens[0] == "position")
				{
					data.position.x = std::stof(tokens[1]);
					data.position.y = std::stof(tokens[2]);
				}

				// World size
				else if (tokens[0] == "size")
				{
					data.size.width = static_cast<size_t>(std::stoul(tokens[1]));
					data.size.height = static_cast<size_t>(std::stoul(tokens[2]));
				}

				// tile size
				else if (tokens[0] == "tilesize")
				{
					data.tilesize.width = std::stof(tokens[1]);
					data.tilesize.height = std::stof(tokens[2]);
				}

				// Terrain layer begin/end
				else if (tokens[0] == "terrainlayer")
				{
					if (tokens[1] == "begin")
					{
						data.terrains.emplace_back();

						currentTerrain =
							&data.terrains.back();

						readingTiles = false;
					}
					else if (tokens[1] == "end")
					{
						currentTerrain = nullptr;
						readingTiles = false;
					}
				}

				// Terrain properties
				else if (currentTerrain)
				{
					if (tokens[0] == "name")
					{
						currentTerrain->name = tokens[1];
					}
					else if (tokens[0] == "tileset")
					{
						currentTerrain->tileset = tokens[1];
					}
					else if (tokens[0] == "tiles")
					{
						if (tokens[1] == "begin")
						{
							readingTiles = true;
						}
						else if (tokens[1] == "end")
						{
							readingTiles = false;
						}
					}
					else if (readingTiles)
					{
						for (const std::string& token : tokens)
						{
							if (!token.empty())
							{
								currentTerrain->tiles.push_back(std::stoi(token));
							}
						}
					}
				}

				// Prop
				else if (tokens[0] == "prop")
				{
					if (tokens[1] == "begin")
					{
						data.props.emplace_back();
						currentProp = &data.props.back();
					}
					else if (tokens[1] == "end")
					{
						currentProp = nullptr;
					}
				}

				// Prop properties
				else if (currentProp)
				{
					if (tokens[0] == "animationset")
					{
						currentProp->animationSet = tokens[1];
					}
					else if (tokens[0] == "animation")
					{
						currentProp->animation = tokens[1];
					}
					else if (tokens[0] == "worldposition")
					{
						currentProp->worldPosition.x = std::stof(tokens[1]);
						currentProp->worldPosition.y = std::stof(tokens[2]);
					}
					else if (tokens[0] == "scale")
					{
						currentProp->scale.x = std::stof(tokens[1]);
						currentProp->scale.y = std::stof(tokens[2]);
					}
					else if (tokens[0] == "color")
					{
						currentProp->color.red = std::stof(tokens[1]);
						currentProp->color.green = std::stof(tokens[2]);
						currentProp->color.blue = std::stof(tokens[3]);
						currentProp->color.alpha = std::stof(tokens[4]);
					}
					else if (tokens[0] == "footprint")
					{
						currentProp->footprint.left = std::stof(tokens[1]);
						currentProp->footprint.top = std::stof(tokens[2]);
						currentProp->footprint.right = std::stof(tokens[3]);
						currentProp->footprint.bottom = std::stof(tokens[4]);
					}
					else if (tokens[0] == "boundingbox")
					{
						currentProp->boundingBox.left = std::stof(tokens[1]);
						currentProp->boundingBox.top = std::stof(tokens[2]);
						currentProp->boundingBox.right = std::stof(tokens[3]);
						currentProp->boundingBox.bottom = std::stof(tokens[4]);
					}
				}
			}

			// ------------------------------------------------------------
			// Rebuild world
			// ------------------------------------------------------------

			// NOTE:
			// tile size must come from somewhere.
			// For now we preserve existing world tile size.
			SizeF tilesize = world.GetTransform().GetTileSize();

			if (!world.Initialize(
				data.position,
				data.size,
				tilesize))
			{
				return false;
			}

			// ------------------------------------------------------------
			// Load terrain layers
			// ------------------------------------------------------------
			for (const TerrainLayerData& terrain : data.terrains)
			{
				if (!assets.Has<TerrainSet>(terrain.tileset))
				{
					continue;
				}

				// lookup terrain set
				const TerrainSet& set = assets.Get<TerrainSet>(terrain.tileset);

				// create terrain layer. note -0xFFFF is just place holder. we will fill the actual data from file next
				world.AddTerrain(terrain.name, set, -0xFFFF);
			}

			for (const TerrainLayerData& terrain : data.terrains)
			{
				world.ForEachTerrain([&](const std::string& name, TerrainLayer& layer)
					{
						if (name == terrain.name)
						{
							size_t index = 0;
							for (int row = 0; row < data.size.height; row++)
							{
								for (int col = 0; col < data.size.width; col++)
								{
									if (index >= terrain.tiles.size())
									{
										throw std::runtime_error("WorldMapFile::Load() - tile count mismatch");
									}

									layer.Set({ row, col }, terrain.tiles[index]);

									index++;
								}
							}

						}
					});
			}

			// ------------------------------------------------------------
			// Load props
			// ------------------------------------------------------------

			for (const PropData& p : data.props)
			{
				// 1. build brush from data
				PropBrush brush = ToPropBrush(p);

				// 2. get animation set from assets
				if (!assets.Has<AnimationSet<Sprite>>(p.animationSet))
				{
					// let's be strict here. if animation set does not exist. throw
					throw std::runtime_error("animation set not available");
				}
				const auto& animSet = assets.Get<AnimationSet<Sprite>>(p.animationSet);

				// 3. create prop using factory (single source of truth)
				std::unique_ptr<Prop> prop = PropFactory::Create(brush, animSet);

				// 4. place using EXISTING placement system. do not remove overlap as floating point inaccuracy can cause props to overlap
				PropPlacementTool::Place(world, std::move(prop), p.worldPosition, false);
			}

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
			// update input to trigger input events
			Input::Instance().Update();
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

#pragma region // TerrainEditScene scene
	class TerrainEditScene : public Scene
	{
	private:
		StateMachine m_stateMachine;
		AssetManager m_assets;
		WorldMap m_worldMap;

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
			auto& mapSize = m_assets.Get<Size<size_t>>("map_size");

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
			m_grassToSplashBrushLink.sourceToTarget.Register(10, -1);
			m_grassToSplashBrushLink.sourceToTarget.Register(21, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(3, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(29, 0);
			m_grassToSplashBrushLink.sourceToTarget.Register(27, 0);
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
			// update input to trigger input events
			Input::Instance().Update();

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
				m_terrainEditor.Paint(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));
			}
			// right click to remove tile
			else if (btn == 2)
			{
				m_terrainEditor.Erase(m_worldMap, m_worldMap.GetTransform().ScreenToWorld(m_mousePos));

			}
		}

		void OnRender() override
		{
			m_stateMachine.OnRender();

			// get resources
			IRenderer& renderer = m_assets.Get<IRenderer>("renderer");
			DrawSortedSpritesCommand& drawCommand = m_assets.Get<DrawSortedSpritesCommand>("drawCommand");

			// draw the terrain's background shoreline splash
			drawCommand.Clear();
			//DrawTerrainGrid(renderer, drawCommand, m_terrain, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 1,1,1,1 });
			//DrawTerrainLayer(renderer, drawCommand, m_grassTerrainLayer, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 1,1,1,1 });
			DrawTerrainLayer(renderer, drawCommand, m_worldMap, "splash", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), { 3,3 }, { 0, 0 });
			drawCommand.Sort();
			drawCommand.Execute();

			// draw the grass terrain
			drawCommand.Clear();
			DrawTerrainLayer(renderer, drawCommand, m_worldMap, "grass", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize());
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

		}
	};
#pragma endregion

#pragma region // CameraScene scene
	class CameraScene : public Scene
	{
	private:
		WorldMap m_worldMap;
		int m_debugState = 2;
		Camera m_camera;
		PositionF m_lastMousePos;
		bool m_isPanning;

		PositionF m_currMousePos;

	public:
		CameraScene() :
			m_camera({ 400, 400, 600, 600 }),
			m_lastMousePos(0, 0),
			m_isPanning(false)
		{

		}

		void OnEnter() override
		{
			auto& terrainSet = AssetManager().Get<TerrainSet>("grass_tileset");

			m_worldMap.Initialize({ 0,0 }, { 1, 1 }, { 300, 300 });

			m_worldMap.AddTerrain("fine_grid", terrainSet, 22);
			m_worldMap.AddTerrain("tile_grid", terrainSet, 13);

			m_camera.SetWorldSize(m_worldMap.GetTransform().GetWorldSize());

		}

		void OnKeyDown(int key) override
		{
			switch (key)
			{
			case 27: // ESC
				break;
			case 32: // SPACE
				m_debugState++;
				if (m_debugState > 3) m_debugState = 0;
				break;
			case 49: // 1
				m_camera.SetZoom(1.0f);
				break;
			case 50: // 2
				m_camera.SetZoom(1.5f);
				break;
			case 51: // 3 
				m_camera.SetZoom(0.5f);
				break;
			case 52: // 4
			{
				m_worldMap.Initialize({ 0,0 }, { 12, 8 }, { 64, 64 });

				auto& terrainSet = AssetManager().Get<TerrainSet>("grass_tileset");
				m_worldMap.AddTerrain("fine_grid", terrainSet, 22);
				m_worldMap.AddTerrain("tile_grid", terrainSet, 13);

				m_camera.SetViewport({ 300, 300, 800, 600 });
				m_camera.SetWorldSize(m_worldMap.GetTransform().GetWorldSize());
				m_camera.SetPosition({ 0,0 });
				m_camera.SetZoom(1.0f);

				break;
			}
			case 53: // 5 
			{
				m_worldMap.Initialize({ 0,0 }, { 1, 1 }, { 300, 300 });

				auto& terrainSet = AssetManager().Get<TerrainSet>("grass_tileset");
				m_worldMap.AddTerrain("fine_grid", terrainSet, 22);
				m_worldMap.AddTerrain("tile_grid", terrainSet, 13);

				m_camera.SetViewport({ 400, 400, 600, 600 });
				m_camera.SetWorldSize(m_worldMap.GetTransform().GetWorldSize());
				m_camera.SetPosition({ 0,0 });
				m_camera.SetZoom(1.0f);

				break;
			}
			default:
				break;
			}
		}

		void OnMouseMove(int x, int y)
		{
			m_currMousePos = { (float)x, (float)y };

			// is we're holding down left mouse button and dragging it, pan the map
			if (m_isPanning)
			{
				// get the change in position and move camera position by that
				PositionF currMousePos = engine::math::VecF((float)x, (float)y);
				engine::math::VecF delta = currMousePos - m_lastMousePos;
				m_camera.MoveBy(delta);

				LOG(std::to_string(delta.x) << ", " << std::to_string(currMousePos.x));

				// remember the last mouse position
				m_lastMousePos = { (float)x, (float)y };
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			m_lastMousePos = { (float)x, (float)y };

			// this button is for panning the camera
			if (btn == 1)
			{
				m_isPanning = true;
			}
			// if this button is clicked, move our focus in this position
			if (btn == 2)
			{
				// this is screen position and convert it to world position
				PositionF pos = m_camera.ScreenToWorld(m_lastMousePos);

				// pan the camera such that the focus is at center of the viewport, if possible
				m_camera.CenterOn(pos);
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
			m_isPanning = false;
		}

		void OnUpdate(double dt) override
		{
			// update input to trigger input events
			Input::Instance().Update();
		}

		void OnRender() override
		{
			AssetManager assets;

			// get resources
			IRenderer& renderer = assets.Get<IRenderer>("renderer");
			DrawSortedSpritesCommand& drawCommand = assets.Get<DrawSortedSpritesCommand>("drawCommand");


			renderer.SetClipRegion(m_camera.GetViewport());
			renderer.EnableClipping(m_debugState > 2);

			if (m_debugState > 0)
			{
				// draw the tile grid
				drawCommand.Clear();
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "tile_grid", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_debugState == 2, { 1, 1 }, { 0, 0 }, { 0,0,0,0.2f });
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "fine_grid", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_debugState == 2, { 1, 1 }, { 0, 0 }, { 0,0,0,0.05f });
				drawCommand.Sort();
				drawCommand.Execute();

				RectF vp = m_camera.GetViewport();

				renderer.Draw(vp.GetTopLeft(), vp.GetSize(), { 1,1,1,0.1f }, 0.0f);

				std::string msg;
				msg += std::to_string(m_camera.GetPosition().x) + ", " + std::to_string(m_camera.GetPosition().y);
				renderer.Draw(assets.Get<IFontAtlas>("font"), msg, { 400, 5 }, { 1,1,1,1 });

				SizeF worldSize
				{
					m_worldMap.GetTransform().GetSize().As<float>().width * m_worldMap.GetTransform().GetTileSize().As<float>().width,
					m_worldMap.GetTransform().GetSize().As<float>().height * m_worldMap.GetTransform().GetTileSize().As<float>().height,
				};

				worldSize *= m_camera.GetZoom();

				msg.clear();
				msg += std::to_string(worldSize.width) + ", " + std::to_string(worldSize.height);
				renderer.Draw(assets.Get<IFontAtlas>("font"), msg, { 400, 25 }, { 1,1,1,1 });

				msg.clear();
				msg += std::to_string(m_currMousePos.x) + ", " + std::to_string(m_currMousePos.y);
				renderer.Draw(assets.Get<IFontAtlas>("font"), msg, { 400, 45 }, { 1,1,1,1 });



			}
		}

	};
#pragma endregion

#pragma region // WorldCameraScene scene
	class EditWorldCameraScene : public Scene
	{
	private:

		enum class EditMode : unsigned int
		{
			Tile = 0,
			PineTree = 1,
			BirchTree = 2,
			Castle = 3,
			WaterRock = 4,
			Size
		};

		int m_debugState = 2;
		PositionF m_lastMousePos;
		bool m_isPanning;
		PositionF m_currMousePos;
		bool m_simulation = false;
		EditMode m_editMode = EditMode::Tile;
		bool m_grid = true;
		bool m_clipping = true;
		bool m_alltiles = false;

		// world data
		WorldMap m_worldMap;

		// map editing tools
		TerrainEditor m_terrainEditor;
		PropBrushTool m_propBrushTool;

		// spatial control
		Camera m_camera;


	public:
		EditWorldCameraScene() :
			m_camera({ 100, 100, 900, 700 }),
			m_lastMousePos(0, 0),
			m_isPanning(false)
		{
		}

		void OnEnter() override
		{
			IWindow& window = AssetManager().Get<IWindow>("window");
			int width = 0, height = 0;
			window.GetClientSize(width, height);
			m_camera.SetViewport({ 0, 0, static_cast<float>(width), static_cast<float>(height) });

			// initialize our world map
			m_worldMap.Initialize({ 0,0 }, { 1024, 1024 }, { 64, 64 });

			// add terrain grids
			auto& terrainSet = AssetManager().Get<TerrainSet>("grass_tileset");
			m_worldMap.AddTerrain("fine_grid", terrainSet, 22);
			m_worldMap.AddTerrain("tile_grid", terrainSet, 13);

			// add grass layer in terrain
			m_worldMap.AddTerrain("grass", terrainSet, 4);

			// add shoreline water splash layer in terrain. 
			auto& splashSet = AssetManager().Get<TerrainSet>("splash_tileset");
			m_worldMap.AddTerrain("splash", splashSet, -1);

			// tell camera about world map size
			m_camera.SetWorldSize(m_worldMap.GetTransform().GetWorldSize());

			// create our grass terrain brush
			TerrainBrush grassTerrainBrush;
			grassTerrainBrush.layer = "grass";
			grassTerrainBrush.config = &AssetManager().Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");

			// create our shoreline splash terrain brush. this will be linked into grass terrain
			// when grass terrain is brushed and the tile type is a shoreline, this will be brushed too
			TerrainBrush splashTerrainBrush;
			splashTerrainBrush.layer = "splash";
			splashTerrainBrush.config = &AssetManager().Get<AutoTileSystem::AutoTileConfig>("splash_tile_auto_config");

			// create brushlink between grass and splash brushes
			TerrainBrushLink grassToSplashBrushLink;
			grassToSplashBrushLink.sourceBrush = grassTerrainBrush;
			grassToSplashBrushLink.targetBrush = splashTerrainBrush;

			// map the tile indices between grass and splash terrain
			grassToSplashBrushLink.sourceToTarget.Register(4, -1);
			grassToSplashBrushLink.sourceToTarget.Register(30, 0);
			grassToSplashBrushLink.sourceToTarget.Register(10, -1);
			grassToSplashBrushLink.sourceToTarget.Register(21, 0);
			grassToSplashBrushLink.sourceToTarget.Register(3, 0);
			grassToSplashBrushLink.sourceToTarget.Register(29, 0);
			grassToSplashBrushLink.sourceToTarget.Register(27, 0);
			grassToSplashBrushLink.sourceToTarget.Register(0, 0);
			grassToSplashBrushLink.sourceToTarget.Register(2, 0);
			grassToSplashBrushLink.sourceToTarget.Register(18, 0);
			grassToSplashBrushLink.sourceToTarget.Register(20, 0);
			grassToSplashBrushLink.sourceToTarget.Register(12, 0);
			grassToSplashBrushLink.sourceToTarget.Register(28, 0);
			grassToSplashBrushLink.sourceToTarget.Register(1, 0);
			grassToSplashBrushLink.sourceToTarget.Register(19, 0);
			grassToSplashBrushLink.sourceToTarget.Register(9, 0);
			grassToSplashBrushLink.sourceToTarget.Register(11, 0);

			// set the grass brush as terrain editor's default brush
			m_terrainEditor.Set(grassTerrainBrush);

			// add the grass to splash brush link in terrain editor
			m_terrainEditor.Add(grassToSplashBrushLink);

			// create prop brushes		- animation set				- default animation - scale				- color				- footprint area (normal)			- bounding box area (normal)
			PropBrush NormalBirchTree{ "birchtree_anim_set",		"birch_tree_idle",	VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.47f, 0.8f, 0.53f, 0.85f},	RectF{0.27f, 0.12f, 0.73f, 0.87f} };
			PropBrush NormalPineTree{ "pinetree_anim_set",		"pine_tree_idle",	VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.38f, 0.8f, 0.62f, 0.92f},	RectF{0.2f, 0.2f, 0.8f, 0.92f} };
			PropBrush NormalCastle{ "castle_anim_set",		"castle_idle",		VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.05f, 0.6f, 0.95f, 0.90f},	RectF{0.05f, 0.2f, 0.95f, 0.9f} };
			PropBrush NormalWaterRocks{ "water_rocks_anim_set",	"water_rocks_idle",	VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.1f,0.4f,0.9f,0.8f},			RectF{0.1f, 0.1f, 0.9f, 0.9f} };

			// register our brushes to our brush tool
			m_propBrushTool.Register("normal_pine_tree", NormalPineTree);
			m_propBrushTool.Register("normal_birch_tree", NormalBirchTree);
			m_propBrushTool.Register("normal_castle", NormalCastle);
			m_propBrushTool.Register("normal_water_rocks", NormalWaterRocks);

			// set default prop on our prop brush tool
			m_propBrushTool.Set("normal_pine_tree");


		}

		void OnKeyDown(int key) override
		{
			switch (key)
			{
			case 9: // TAB
				m_editMode = (EditMode)(((int)m_editMode + 1) % (int)EditMode::Size);
				break;
			case 27: // ESC
				break;
			case 32: // SPACE
				m_debugState++;
				if (m_debugState > 3) m_debugState = 0;
				break;
			case 49: // 1
				m_simulation = !m_simulation;
				break;
			case 50: // 2
				m_camera.SetZoom(1.5f);
				break;
			case 51: // 3 
				m_camera.SetZoom(1.0f);
				break;
			case 52: // 4
				m_camera.SetZoom(0.4f);
				break;
			case 53: // 5 
			{
				break;
			}
			case 103: // g
			case 71: // G
			{
				m_grid = !m_grid;
				break;
			}
			case 67: // C
			case 99: // c
			{
				m_clipping = !m_clipping;
				break;
			}
			case 84: // T
			case 116: // t
			{
				m_alltiles = !m_alltiles;
				break;
			}
			default:
				break;
			}
		}

		void OnMouseMove(int x, int y)
		{
			m_currMousePos = { (float)x, (float)y };

			if (m_simulation)
			{
				// is we're holding down left mouse button and dragging it, pan the map
				if (m_isPanning)
				{
					// get the change in position and move camera position by that
					engine::math::VecF delta = engine::math::VecF((float)x, (float)y) - m_lastMousePos;
					m_camera.MoveBy(delta);

					// remember the last mouse position
					m_lastMousePos = { (float)x, (float)y };
				}
			}
			else
			{
				switch (m_editMode)
				{
				case EditMode::Tile:
				{
					// is mouse left button is held while moving...
					if (Input::Instance().IsMouseHeld(1))
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_currMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Paint(m_worldMap, coord);

					}
					else if (Input::Instance().IsMouseHeld(2))
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_currMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Erase(m_worldMap, coord);
					}
					break;
				}
				default:
					break;
				}
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			m_lastMousePos = { (float)x, (float)y };

			// this button is for panning the camera
			if (btn == 1)
			{
				if (m_simulation)
				{
					m_isPanning = true;
				}
				else
				{
					switch (m_editMode)
					{
					case EditMode::Tile:
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Paint(m_worldMap, coord);
						break;
					}
					case EditMode::PineTree:
					{
						m_propBrushTool.Set("normal_pine_tree");
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						PropPlacementTool::Result result = m_propBrushTool.Paint(m_worldMap, worldPos);

						// what tiles did this tree occupied? we should place grass tiles. 
						// get the tiles this tree occupied
						// place grass tiles on all the tiles it occupied
						for (const Coord& coord : result.occupiedTiles)
						{
							m_terrainEditor.Paint(m_worldMap, coord);
						}

						break;
					}
					case EditMode::Castle:
					{
						m_propBrushTool.Set("normal_castle");
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						PropPlacementTool::Result result = m_propBrushTool.Paint(m_worldMap, worldPos);

						for (const Coord& coord : result.occupiedTiles)
						{
							m_terrainEditor.Paint(m_worldMap, coord);
						}

						break;
					}
					case EditMode::WaterRock:
					{
						m_propBrushTool.Set("normal_water_rocks");
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						PropPlacementTool::Result result = m_propBrushTool.Paint(m_worldMap, worldPos);

						for (const Coord& coord : result.occupiedTiles)
						{
							m_terrainEditor.Erase(m_worldMap, coord);
						}

						break;
					}
					default:
						break;
					}
				}
			}
			// if this button is clicked, move our focus in this position
			if (btn == 2)
			{
				if (m_simulation)
				{
					// this is screen position and convert it to world position
					PositionF pos = m_camera.ScreenToWorld(m_lastMousePos);

					// pan the camera such that the focus is at center of the viewport, if possible
					m_camera.CenterOn(pos);
				}
				else
				{
					switch (m_editMode)
					{
					case EditMode::Tile:
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Erase(m_worldMap, coord);
						break;
					}
					case EditMode::PineTree:
					case EditMode::Castle:
					case EditMode::WaterRock:
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						m_propBrushTool.Erase(m_worldMap, worldPos);
						break;
					}
					default:
						break;
					}

				}
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
			m_isPanning = false;
		}


		void OnUpdate(double dt) override
		{
			// update input to trigger input events
			Input::Instance().Update();

			// this is for debugging only. validate every frame to ensure our containers are in good state
			m_worldMap.Validate();
		}

		void OnRender() override
		{
			AssetManager assets;

			// get resources
			IRenderer& renderer = assets.Get<IRenderer>("renderer");
			DrawSortedSpritesCommand& drawCommand = assets.Get<DrawSortedSpritesCommand>("drawCommand");

			renderer.SetClipRegion(m_camera.GetViewport());
			renderer.EnableClipping(m_clipping);

			// draw the terrain's background shoreline splash
			{
				drawCommand.Clear();
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "splash", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles, { 3,3 }, { 0, 0 }, { 1,1,1,0.5f });
				drawCommand.Sort();
				drawCommand.Execute();
			}

			// draw the grass terrain
			{
				drawCommand.Clear();
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "grass", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles);
				drawCommand.Sort();
				drawCommand.Execute();
			}

			// draw props
			{
				drawCommand.Clear();
				DrawProps(renderer, drawCommand, m_camera, m_worldMap, 1, m_alltiles);
				drawCommand.Sort();
				drawCommand.Execute();
			}

			if (m_grid)
			{
				// draw the tile grid
				drawCommand.Clear();
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "tile_grid", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles, { 1, 1 }, { 0, 0 }, { 0,0,0,0.2f });
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "fine_grid", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles, { 1, 1 }, { 0, 0 }, { 0,0,0,0.05f });
				drawCommand.Sort();
				drawCommand.Execute();

				RectF vp = m_camera.GetViewport();

				renderer.Draw(vp.GetTopLeft(), vp.GetSize(), { 1,1,1,0.1f }, 0.0f);
			}

			//if (!m_simulation)
			{
				std::string msg;
				switch (m_editMode)
				{
				case EditMode::Tile: msg += "Place Tile|"; break;
				case EditMode::BirchTree: msg += "Place Birch Tree|"; break;
				case EditMode::PineTree: msg += "Place Pine Tree|"; break;
				case EditMode::Castle: msg += "Place Castle|"; break;
				case EditMode::WaterRock: msg += "Place Water Rock|"; break;
				default: break;
				}
				msg += " ";
				msg += m_simulation ? "simulation|" : "edit|";
				msg += m_grid ? "grid enabled|" : "grid disabled|";
				msg += m_clipping ? "clipping enabled|" : "clipping disabled|";
				msg += m_alltiles ? "all tiles|" : "visible tiles|";
				renderer.Draw(assets.Get<IFontAtlas>("font"), msg, { 600, 5 }, { 1,1,1,1 });
			}
		}

		void OnResize(size_t width, size_t height) override
		{
			m_camera.SetViewport({ 0,0, static_cast<float>(width), static_cast<float>(height) });
		}
	};
#pragma endregion

#pragma region // LoadSaveWorldScene
	class LoadSaveWorldScene : public Scene
	{
	private:
		enum class EditMode : unsigned int
		{
			Tile = 0,
			PineTree = 1,
			BirchTree = 2,
			Castle = 3,
			WaterRock = 4,
			Size
		};

		PositionF m_lastMousePos;
		bool m_isPanning;
		PositionF m_currMousePos;
		bool m_simulation = false;
		EditMode m_editMode = EditMode::Tile;
		bool m_grid = true;
		bool m_clipping = true;
		bool m_alltiles = false;
		bool m_navigation = true;

		// world data
		WorldMap m_worldMap;

		// grids
		TerrainGrid m_tilegrid;
		TerrainGrid m_finegrid;

		// map editing tools
		TerrainEditor m_terrainEditor;
		PropBrushTool m_propBrushTool;

		// spatial control
		Camera m_camera;


	public:
		LoadSaveWorldScene() :
			m_camera({ 100, 100, 1000, 800 }),
			m_lastMousePos(0, 0),
			m_isPanning(false)
		{
		}

		void OnEnter() override
		{
			// initialize our world map
			m_worldMap.Initialize({ 0,0 }, { 20, 12 }, { 64, 64 });

			// add terrain grids
			auto& terrainSet = AssetManager().Get<TerrainSet>("grass_tileset");
			m_tilegrid.Initialize(m_worldMap.GetTransform().GetSize(), terrainSet.MakeTile(13));
			m_finegrid.Initialize(m_worldMap.GetTransform().GetSize(), terrainSet.MakeTile(22));

			// add grass layer in terrain
			m_worldMap.AddTerrain("grass", terrainSet, 4);

			// add shoreline water splash layer in terrain. 
			auto& splashSet = AssetManager().Get<TerrainSet>("splash_tileset");
			m_worldMap.AddTerrain("splash", splashSet, -1);

			// tell camera about world map size
			m_camera.SetWorldSize(m_worldMap.GetTransform().GetWorldSize());

			// create our grass terrain brush
			TerrainBrush grassTerrainBrush;
			grassTerrainBrush.layer = "grass";
			grassTerrainBrush.config = &AssetManager().Get<AutoTileSystem::AutoTileConfig>("grass_tile_auto_config");

			// create our shoreline splash terrain brush. this will be linked into grass terrain
			// when grass terrain is brushed and the tile type is a shoreline, this will be brushed too
			TerrainBrush splashTerrainBrush;
			splashTerrainBrush.layer = "splash";
			splashTerrainBrush.config = &AssetManager().Get<AutoTileSystem::AutoTileConfig>("splash_tile_auto_config");

			// create brushlink between grass and splash brushes
			TerrainBrushLink grassToSplashBrushLink;
			grassToSplashBrushLink.sourceBrush = grassTerrainBrush;
			grassToSplashBrushLink.targetBrush = splashTerrainBrush;

			// map the tile indices between grass and splash terrain
			grassToSplashBrushLink.sourceToTarget.Register(4, -1);
			grassToSplashBrushLink.sourceToTarget.Register(30, 0);
			grassToSplashBrushLink.sourceToTarget.Register(10, -1);
			grassToSplashBrushLink.sourceToTarget.Register(21, 0);
			grassToSplashBrushLink.sourceToTarget.Register(3, 0);
			grassToSplashBrushLink.sourceToTarget.Register(29, 0);
			grassToSplashBrushLink.sourceToTarget.Register(27, 0);
			grassToSplashBrushLink.sourceToTarget.Register(0, 0);
			grassToSplashBrushLink.sourceToTarget.Register(2, 0);
			grassToSplashBrushLink.sourceToTarget.Register(18, 0);
			grassToSplashBrushLink.sourceToTarget.Register(20, 0);
			grassToSplashBrushLink.sourceToTarget.Register(12, 0);
			grassToSplashBrushLink.sourceToTarget.Register(28, 0);
			grassToSplashBrushLink.sourceToTarget.Register(1, 0);
			grassToSplashBrushLink.sourceToTarget.Register(19, 0);
			grassToSplashBrushLink.sourceToTarget.Register(9, 0);
			grassToSplashBrushLink.sourceToTarget.Register(11, 0);

			// set the grass brush as terrain editor's default brush
			m_terrainEditor.Set(grassTerrainBrush);

			// add the grass to splash brush link in terrain editor
			m_terrainEditor.Add(grassToSplashBrushLink);

			// create prop brushes		- animation set				- default animation - scale				- color				- footprint area (normal)			- bounding box area (normal)
			PropBrush NormalBirchTree{ "birchtree_anim_set",		"birch_tree_idle",	VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.47f, 0.8f, 0.53f, 0.85f},	RectF{0.27f, 0.12f, 0.73f, 0.87f} };
			PropBrush NormalPineTree{ "pinetree_anim_set",		"pine_tree_idle",	VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.38f, 0.8f, 0.62f, 0.92f},	RectF{0.2f, 0.2f, 0.8f, 0.92f} };
			PropBrush NormalCastle{ "castle_anim_set",		"castle_idle",		VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.05f, 0.6f, 0.95f, 0.90f},	RectF{0.05f, 0.2f, 0.95f, 0.9f} };
			PropBrush NormalWaterRocks{ "water_rocks_anim_set",	"water_rocks_idle",	VecF{1.0f, 1.0f},	ColorF{1,1,1,1},	RectF{0.1f,0.4f,0.9f,0.8f},			RectF{0.1f, 0.1f, 0.9f, 0.9f} };

			// register our brushes to our brush tool
			m_propBrushTool.Register("normal_pine_tree", NormalPineTree);
			m_propBrushTool.Register("normal_birch_tree", NormalBirchTree);
			m_propBrushTool.Register("normal_castle", NormalCastle);
			m_propBrushTool.Register("normal_water_rocks", NormalWaterRocks);

			// set default prop on our prop brush tool
			m_propBrushTool.Set("normal_pine_tree");

		}

		void OnKeyDown(int key) override
		{
			switch (key)
			{
			case 9: // TAB
				m_editMode = (EditMode)(((int)m_editMode + 1) % (int)EditMode::Size);
				break;
			case 27: // ESC
				m_terrainEditor.Clear(m_worldMap);
				m_worldMap.RemoveAllProps();
				break;
			case 32: // SPACE
				break;
			case 49: // 1
				m_simulation = !m_simulation;
				break;
			case 50: // 2
				WorldMapFile::Save("..\\Assets\\worldmap.csv", m_worldMap);
				break;
			case 51: // 3 
				AssetManager assets;
				WorldMapFile::Load("..\\Assets\\worldmap.csv", m_worldMap, assets);
				break;
			case 52: // 4
				m_camera.SetZoom(1.5f);
				break;
			case 53: // 5 
			{
				m_worldMap.RemoveAllProps();
				break;
			}
			case 103: // g
			case 71: // G
			{
				m_grid = !m_grid;
				break;
			}
			case 67: // C
			case 99: // c
			{
				m_clipping = !m_clipping;
				break;
			}
			case 84: // T
			case 116: // t
			{
				m_alltiles = !m_alltiles;
				break;
			}
			case 78: // N
			case 110: // n
			{
				m_navigation = !m_navigation;
				break;
			}
			default:
				break;
			}
		}

		void OnMouseMove(int x, int y)
		{
			m_currMousePos = { (float)x, (float)y };

			if (m_simulation)
			{
				// is we're holding down left mouse button and dragging it, pan the map
				if (m_isPanning)
				{
					// get the change in position and move camera position by that
					engine::math::VecF delta = engine::math::VecF((float)x, (float)y) - m_lastMousePos;
					m_camera.MoveBy(delta);

					// remember the last mouse position
					m_lastMousePos = { (float)x, (float)y };
				}
			}
			else
			{
				switch (m_editMode)
				{
				case EditMode::Tile:
				{
					// is mouse left button is held while moving...
					if (Input::Instance().IsMouseHeld(1))
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_currMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Paint(m_worldMap, coord);

					}
					else if (Input::Instance().IsMouseHeld(2))
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_currMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Erase(m_worldMap, coord);
					}
					break;
				}
				default:
					break;
				}
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			m_lastMousePos = { (float)x, (float)y };

			// this button is for panning the camera
			if (btn == 1)
			{
				if (m_simulation)
				{
					m_isPanning = true;
				}
				else
				{
					switch (m_editMode)
					{
					case EditMode::Tile:
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Paint(m_worldMap, worldPos);
						break;
					}
					case EditMode::PineTree:
					{
						m_propBrushTool.Set("normal_pine_tree");
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						PropPlacementTool::Result result = m_propBrushTool.Paint(m_worldMap, worldPos);

						// what tiles did this tree occupied? we should place grass tiles. 
						// get the tiles this tree occupied
						// place grass tiles on all the tiles it occupied
						for (const Coord& coord : result.occupiedTiles)
						{
							m_terrainEditor.Paint(m_worldMap, coord);
						}

						break;
					}
					case EditMode::Castle:
					{
						m_propBrushTool.Set("normal_castle");
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						PropPlacementTool::Result result = m_propBrushTool.Paint(m_worldMap, worldPos);

						for (const Coord& coord : result.occupiedTiles)
						{
							m_terrainEditor.Paint(m_worldMap, coord);
						}

						break;
					}
					case EditMode::WaterRock:
					{
						m_propBrushTool.Set("normal_water_rocks");
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						PropPlacementTool::Result result = m_propBrushTool.Paint(m_worldMap, worldPos);

						for (const Coord& coord : result.occupiedTiles)
						{
							m_terrainEditor.Erase(m_worldMap, coord);
						}

						break;
					}
					default:
						break;
					}
				}
			}
			// if this button is clicked, move our focus in this position
			if (btn == 2)
			{
				if (m_simulation)
				{
					// this is screen position and convert it to world position
					PositionF pos = m_camera.ScreenToWorld(m_lastMousePos);

					// pan the camera such that the focus is at center of the viewport, if possible
					m_camera.CenterOn(pos);
				}
				else
				{
					switch (m_editMode)
					{
					case EditMode::Tile:
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						Coord coord = m_worldMap.GetTransform().WorldToTileCoord(worldPos);
						m_terrainEditor.Erase(m_worldMap, coord);
						break;
					}
					case EditMode::PineTree:
					case EditMode::Castle:
					case EditMode::WaterRock:
					{
						PositionF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
						m_propBrushTool.Erase(m_worldMap, worldPos);
						break;
					}
					default:
						break;
					}

				}
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
			m_isPanning = false;
		}

		void OnUpdate(double dt) override
		{
			// update input to trigger input events
			Input::Instance().Update();

			// this is for debugging only. validate every frame to ensure our containers are in good state
			m_worldMap.Validate();
		}

		void OnRender() override
		{
			AssetManager assets;

			// get resources
			IRenderer& renderer = assets.Get<IRenderer>("renderer");
			DrawSortedSpritesCommand& drawCommand = assets.Get<DrawSortedSpritesCommand>("drawCommand");

			renderer.SetClipRegion(m_camera.GetViewport());
			renderer.EnableClipping(m_clipping);

			// draw the terrain's background shoreline splash
			{
				drawCommand.Clear();
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "splash", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles, { 3,3 }, { 0, 0 }, { 1,1,1,0.5f });
				drawCommand.Sort();
				drawCommand.Execute();
			}

			// draw the grass terrain
			{
				drawCommand.Clear();
				DrawTerrainLayer(renderer, drawCommand, m_camera, m_worldMap, "grass", m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles);
				drawCommand.Sort();
				drawCommand.Execute();
			}

			// draw props
			{
				drawCommand.Clear();
				DrawProps(renderer, drawCommand, m_camera, m_worldMap, 1, m_alltiles);
				drawCommand.Sort();
				drawCommand.Execute();
			}

			if (m_grid)
			{
				// draw the tile grid
				drawCommand.Clear();
				DrawTerrainGrid(renderer, drawCommand, m_camera, m_tilegrid, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles, { 1, 1 }, { 0, 0 }, { 0,0,0,0.2f });
				DrawTerrainGrid(renderer, drawCommand, m_camera, m_tilegrid, m_worldMap.GetTransform().GetPosition(), m_worldMap.GetTransform().GetTileSize(), m_alltiles, { 1, 1 }, { 0, 0 }, { 0,0,0,0.05f });
				drawCommand.Sort();
				drawCommand.Execute();

				RectF vp = m_camera.GetViewport();

				renderer.Draw(vp.GetTopLeft(), vp.GetSize(), { 1,1,1,0.1f }, 0.0f);
			}

			// render navigation overlay
			if (m_navigation)
			{
				DrawNavigationOverlay(renderer, drawCommand, m_camera, m_worldMap, m_alltiles);
			}

			//if (!m_simulation)
			{
				renderer.EnableClipping(false);

				std::string msg;
				switch (m_editMode)
				{
				case EditMode::Tile: msg += "Place Tile|"; break;
				case EditMode::BirchTree: msg += "Place Birch Tree|"; break;
				case EditMode::PineTree: msg += "Place Pine Tree|"; break;
				case EditMode::Castle: msg += "Place Castle|"; break;
				case EditMode::WaterRock: msg += "Place Water Rock|"; break;
				default: break;
				}
				msg += " ";
				msg += m_simulation ? "simulation|" : "edit|";
				msg += m_grid ? "grid enabled|" : "grid disabled|";
				msg += m_clipping ? "clipping enabled|" : "clipping disabled|";
				msg += m_alltiles ? "all tiles|" : "visible tiles|";
				msg += m_navigation ? "navigation enabled|" : "navigation disabled|";
				renderer.Draw(assets.Get<IFontAtlas>("font"), msg, { 300, 5 }, { 1,1,1,1 });
			}
		}

		void OnResize(size_t width, size_t height) override
		{
		}
	};
#pragma endregion

#pragma region // UISkin
	class UISkin
	{
	public:
		virtual ~UISkin() = default;

		virtual void DrawButton(const class Button& button, const UIDrawContext& ctx) const = 0;
		virtual void DrawLayer(const class Layer& overlay, const UIDrawContext& ctx) const = 0;
		virtual void DrawFrame(const class Frame& frame, const UIDrawContext& ctx) const = 0;
		virtual void DrawTooltip(const class Tooltip& tooltip, const UIDrawContext& ctx) const = 0;
		virtual void DrawLabel(const class Label& label, const UIDrawContext& ctx) const = 0;
		virtual void DrawImage(const class Image& image, const UIDrawContext& ctx) const = 0;
		virtual void DrawDraggable(const Draggable& draggable, const UIDrawContext& context) const = 0;
		virtual void DrawMenuButton(const MenuButton& menuButton, const UIDrawContext& context) const = 0;
		virtual void DrawMenuItem(const MenuItem& menuItem, const UIDrawContext& context) const = 0;
		virtual void DrawSubMenuButton(const SubMenuButton& subMenuButton, const UIDrawContext& context) const = 0;
		virtual void DrawSlider(const Slider& slider, const UIDrawContext& context) const = 0;
		virtual void DrawScrollBar(const ScrollBar& scrollbar, const UIDrawContext& context) const = 0;
		virtual void DrawThumb(const Thumb& thumb, const UIDrawContext& context) const = 0;
		virtual void DrawCheckBox(const CheckBox& checkbox, const UIDrawContext& context) const = 0;
		virtual void DrawRadioButton(const RadioButton& radiobutton, const UIDrawContext& context) const = 0;
		virtual void DrawGrip(const Grip& radiobutton, const UIDrawContext& context) const = 0;
		virtual void DrawResizeableFrame(const ResizeableFrame& radiobutton, const UIDrawContext& context) const = 0;
		virtual void DrawViewPort(const ViewPort& vp, const UIDrawContext& context) const = 0;
		virtual void DrawContent(const Content& content, const UIDrawContext& context) const = 0;

	};
#pragma endregion

#pragma region // UIDrawContext
	struct UIDrawContext
	{
		IRenderer& renderer;
		UISystem& system;
		UISkin* skin = nullptr;
		Widget* hover = nullptr;
		Widget* focus = nullptr;
		Widget* capture = nullptr;
	};
#pragma endregion

#pragma region // Widget
	class Widget
	{
	private:
#pragma region // DragController
		// --------------------------------------------------------------------------------
		// DRAG MANAGEMENT
		// --------------------------------------------------------------------------------
		friend class DragHandler;
		class DragHandler
		{
		private:
			// widget dragging trackers
			PositionF m_beginMousePosition;
			PositionF m_beginMovePosition;
			bool m_isMoving = false;

		public:
			void Begin(const PositionF& position, Widget* widget)
			{
				// if not movable, bail out
				if (widget->m_moveBehavior == MoveBehavior::None) return;

				// remember this mouse position. this will be the pivot position as this widget gets dragged around by mouse
				m_beginMousePosition = position;

				// remember the widget's position now. this will be the reference position as it gets dragged around by mouse
				m_beginMovePosition = widget->GetPosition();

				// this widget is now moving
				m_isMoving = true;

				widget->OnDragBegin(DragEventArgs{ GetBeginPosition(), position });
			}

			void Update(const PositionF& position, Widget* widget)
			{
				if (m_isMoving)
				{
					// calculate the mouse movement delta between its position at start of mouse drag and its position now
					// factor in the move state - free? horizontal? vertical?
					VecF delta =
					{
						// if we can move horizontally, use the mouse position. otherwise, use begin position
						(widget->m_moveBehavior & MoveBehavior::Horizontal) ? position.x - m_beginMousePosition.x : 0.0f,

						// if we can move vertically, use the mouse position. otherwise, use begin position
						(widget->m_moveBehavior & MoveBehavior::Vertical) ? position.y - m_beginMousePosition.y : 0.0f
					};

					// transform widget's position based on mouse movement delta
					widget->SetPosition(m_beginMovePosition + delta);

					widget->OnDragMove(DragEventArgs{ GetBeginPosition(), position });
				}
			}

			void End(const PositionF& position, Widget* widget)
			{
				m_isMoving = false;

				widget->OnDragEnd(DragEventArgs{ GetBeginPosition(), position });
			}

			bool IsDragging() const
			{
				return m_isMoving;
			}

			PositionF GetBeginPosition() const
			{
				return m_beginMousePosition;
			}
		};
#pragma endregion

		bool UnregisterToSystem();

		bool RegisterToSystem()
		{
			return OnRegisterToSystem();
		}

	protected:
		// is just enum (not class) because it needs logical operations
		enum MoveBehavior
		{
			None = 0,
			Horizontal = 1 << 0,
			Vertical = 1 << 1,
			Free = Horizontal | Vertical,
		};

		enum class HitTestBehavior
		{
			Normal,
			AlwaysPass,
			AlwaysFail
		};

		// tree
		Widget* m_parent = nullptr;
		std::vector<std::unique_ptr<Widget>> m_children;

		// transform
		PositionF m_position;
		SizeF m_size;

		// states
		bool m_visible = true;
		bool m_enabled = true;

		// behavior
		bool m_focusable = true;
		bool m_droppable = false;
		MoveBehavior m_moveBehavior = MoveBehavior::Free;
		HitTestBehavior m_hitTestBehavior = HitTestBehavior::Normal;

		// tooltip support
		std::function<void(Widget&, Widget&)> m_tooltipBuilder;

		// widget dragging tracker
		DragHandler m_dragHandler;

		// --------------------------------------------------------------------------------
		// SYSTEM 
		// --------------------------------------------------------------------------------
		virtual UISystem* GetSystem() const
		{
			if (m_parent)
			{
				return m_parent->GetSystem();
			}

			return nullptr;
		}

		virtual bool OnRegisterToSystem()
		{
			return true;
		}

		virtual bool OnUnregisterToSystem()
		{
			return true;
		}

		// --------------------------------------------------------------------------------
		// CHANGE PARAMETER HANDLERS
		// --------------------------------------------------------------------------------
		virtual void OnPositionChanged(const PositionF& oldPos, const PositionF& newPos)
		{
			// default implementation does nothing. derived class can override this to react to position change
		}

		virtual void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)
		{
			// default implementation does nothing. derived class can override this to react to size change
		}

		virtual void OnResourceChange()
		{
			// default implementation does nothing. derived class can override this to react to resource change
		}

		// --------------------------------------------------------------------------------
		// DRAG AND DROP EVENT HANDLERS
		// --------------------------------------------------------------------------------
		virtual void OnDrop(Widget* dragged)
		{

		}


		// --------------------------------------------------------------------------------
		// INPUT HANDLERS
		// --------------------------------------------------------------------------------
		virtual void OnMouseDown(const PositionF& position)
		{

		}

		virtual void OnMouseUp(const PositionF& position)
		{

		}

		virtual void OnMouseMove(const PositionF& position)
		{

		}

		virtual void OnMouseEnter()
		{
		}

		virtual void OnMouseLeave()
		{
		}

		virtual void OnKeyDown(int key)
		{
		}

		virtual void OnKeyUp(int key)
		{
		}

	public:
		struct DragEventArgs
		{
			PositionF beginPosition;
			PositionF currentPosition;

			VecF Delta() const
			{
				return currentPosition - beginPosition;
			}
		};

		virtual ~Widget() = default;

		enum class HorizontalAlignment
		{
			Left,
			Right,
			Center
		};
		enum class VerticalAlignment
		{
			Top,
			Bottom,
			Center
		};

		// --------------------------------------------------------------------------------
		// DRAG AND DROP
		// --------------------------------------------------------------------------------
		void DropAccepted(Widget* widget)
		{
			OnDrop(widget);
		}

		// --------------------------------------------------------------------------------
		// HIERARCHY
		// --------------------------------------------------------------------------------
		void AddChild(std::unique_ptr<Widget> child)
		{
			child->m_parent = this;

			Widget* c = child.get();
			m_children.push_back(std::move(child));

			// traverse through this widget's whole tree including itself and register them to system
			c->ForEachWidget([&](Widget* widget)
				{
					widget->RegisterToSystem();
					return true;
				});
		}

		void RemoveChild(Widget* widget)
		{
			// is this widget our child?
			auto it = std::find_if(
				m_children.begin(),
				m_children.end(),
				[&](const auto& ptr)
				{
					return ptr.get() == widget;
				});

			// unregister this widget's whole tree including itself. then remove this widget
			if (it != m_children.end())
			{
				widget->ForEachWidget([&](Widget* w)
					{
						w->UnregisterToSystem();
						return true;
					});

				m_children.erase(it);
			}
		}

		void RemoveChildren()
		{
			// remove all children and unregister their trees from system
			while (m_children.size())
			{
				m_children.back()->ForEachWidget([&](Widget* w)
					{
						w->UnregisterToSystem();
						return true;
					});

				m_children.pop_back();
			}
		}

		Widget* GetParent() const
		{
			return m_parent;
		}

		// remove a widget in this widget tree. this will traverse through this widget's tree to find the widget
		// if found, removes it as well as its tree. returns true if successfully found and removed
		bool Remove(Widget* widget)
		{
			Widget* found = nullptr;

			//bool result = false;
			// do not remove self, so just start searching from children onwards
			for (const std::unique_ptr<Widget>& child : m_children)
			{
				// no need to continue searching children if we already found the widget we're looking for
				if (found) break;

				child->ForEachWidget([&](Widget* w)
					{
						if (w == widget)
						{
							// found the widget we're looking for
							found = w;

							// return false to tell foreach to stop traversing now
							return false;
						}

						// tell foreach to continue traversing
						return true;
					});
			}

			// if we didn't find widget...
			if (!found) return false;

			// be strict here. ensure this widget has parent so we can remove it
			if (!found->m_parent)
			{
				throw std::runtime_error("how come this widget has no parent and is getting remove?");
			}

			// time to safely remove the widget
			found->m_parent->RemoveChild(found);
			return true;
		}

		// checks if this widget is descendant of given widget
		bool IsDescendantOf(const Widget* ancestor) const
		{
			if (!ancestor)
			{
				return false;
			}

			const Widget* current = m_parent;

			// traverse through the parents until given widget is found or root is reached
			while (current)
			{
				if (current == ancestor)
				{
					return true;
				}

				current = current->m_parent;
			}

			// if you reached this point, this widget is not descendant of given widget
			return false;
		}

		bool HasChildren() const
		{
			return m_children.size() > 0;
		}

		void MoveChildTo(Widget* child, Widget* newParent)
		{
			// we're a bit strict here
			if (!child)
			{
				throw std::invalid_argument("MoveChildTo() - child is null");
			}

			// we're a bit strict here
			if (!newParent)
			{
				throw std::invalid_argument("MoveChildTo() - newParent is null");
			}

			// make sure child is not same as new parent
			if (child == newParent)
			{
				throw std::invalid_argument("MoveChildTo() - newParent is same as child");
			}

			// be more strict. new parent cannot be descendant of child
			if (newParent->IsDescendantOf(child))
			{
				throw std::invalid_argument("MoveChildTo() - newParent is descendant of child");
			}

			// child must belong to this parent
			auto it = std::find_if(
				m_children.begin(),
				m_children.end(),
				[child](const std::unique_ptr<Widget>& ptr)
				{
					return ptr.get() == child;
				});

			// we're a bit strict here
			if (it == m_children.end())
			{
				throw std::runtime_error("MoveChildTo() - child not found");
			}

			// transfer ownership out of current parent
			std::unique_ptr<Widget> movedChild = std::move(*it);

			// remove empty slot
			m_children.erase(it);

			// add to new parent
			newParent->AddChild(std::move(movedChild));
		}

		// --------------------------------------------------------------------------------
		// Z ORDER
		// --------------------------------------------------------------------------------
		void BringChildToFront(Widget* child)
		{
			// use find_if better than for loop because you iterator on erase()
			auto it = std::find_if(
				m_children.begin(),
				m_children.end(),
				[&](const auto& ptr)
				{
					return ptr.get() == child;
				});

			// no child? bail out
			if (it == m_children.end())
				return;

			// move this widget out of the children's list
			std::unique_ptr<Widget> node = std::move(*it);
			m_children.erase(it);

			// put it back at the end of the children's list so it will be at the front
			m_children.push_back(std::move(node));
		}

		void BringToFront()
		{
			// if this has no parent, then it has no siblings. then it does not have z order
			if (!m_parent) return;

			m_parent->BringChildToFront(this);
		}

		// --------------------------------------------------------------------------------
		// STATE
		// --------------------------------------------------------------------------------
		void Show()
		{
			m_visible = true;
		}

		void Hide()
		{
			m_visible = false;
		}

		bool IsVisible() const
		{
			return m_visible;
		}

		void Enable()
		{
			m_enabled = true;
		}

		void Disable()
		{
			m_enabled = false;
		}

		bool IsEnabled() const
		{
			// if this widget is disabled, can return now
			if (!m_enabled) return false;

			// widgets has dependency on their parents/ascendants when it comes to enable state
			// if parent is disabled, then this must be disabled too.
			if (m_parent) return m_parent->IsEnabled();

			// if this is enabled as well as its ascendants, then this is enabled
			return true;
		}



		// --------------------------------------------------------------------------------
		// BEHAVIOR
		// --------------------------------------------------------------------------------
		bool IsFocusable() const
		{
			return m_focusable;
		}

		bool IsDroppable() const
		{
			return m_droppable;
		}

		// --------------------------------------------------------------------------------
		// TRANSFORM
		// --------------------------------------------------------------------------------
		float GetWidth() const
		{
			return m_size.width;
		}

		float GetHeight() const
		{
			return m_size.height;
		}

		SizeF GetSize() const
		{
			return m_size;
		}

		engine::event::Event<const SizeF&> OnResize;

		void SetSize(const SizeF& size)
		{
			SizeF oldSize = m_size;
			m_size = size;
			OnResize(size);
			OnSizeChanged(oldSize, size);
		}

		PositionF GetAbsolutePosition() const
		{
			PositionF position = m_position;
			if (m_parent)
			{
				position += m_parent->GetAbsolutePosition();
			}
			return position;
		}

		engine::event::Event<const PositionF&> OnMove;
		void SetPosition(const PositionF& pos)
		{
			PositionF oldPos = m_position;
			m_position = pos;
			OnMove(pos);
			OnPositionChanged(oldPos, m_position);
		}

		PositionF GetPosition() const
		{
			return m_position;
		}

		RectF GetAbsoluteRect() const
		{
			PositionF absPos = GetAbsolutePosition();
			SizeF size = GetSize();
			return RectF
			{
				absPos.x,
				absPos.y,
				absPos.x + size.width,
				absPos.y + size.height
			};
		}

		// --------------------------------------------------------------------------------
		// HIT TEST
		// --------------------------------------------------------------------------------

		bool Contains(const PositionF& position) const
		{
			// always pass 
			if (m_hitTestBehavior == HitTestBehavior::AlwaysPass) return true;

			// always fail
			if (m_hitTestBehavior == HitTestBehavior::AlwaysFail) return false;

			// translate the point (assume to be absolute position) into this widget's local space
			PositionF local = position - GetAbsolutePosition();

			// convert our size into rect. 
			RectF rect{ 0, 0, m_size.width, m_size.height };

			// since point is now in widget's local space, we can check if its inside it
			return rect.Contains(local);
		}

		// --------------------------------------------------------------------------------
		// INPUT
		// --------------------------------------------------------------------------------

		engine::event::Event<const DragEventArgs&> OnDragBegin;
		engine::event::Event<const DragEventArgs&> OnDragMove;
		engine::event::Event<const DragEventArgs&> OnDragEnd;

		void MouseDown(const PositionF& position)
		{
			// let derived widget handle mouse down event first
			OnMouseDown(position);

			m_dragHandler.Begin(position, this);

		}

		void MouseUp(const PositionF& position)
		{
			m_dragHandler.End(position, this);

			// now we handle mouse up event after we set its to state to NOT moving
			OnMouseUp(position);
		}

		void MouseMove(const PositionF& position)
		{
			m_dragHandler.Update(position, this);

			// handle this mouse event after this widget updates its position from mouse move
			OnMouseMove(position);
		}

		void MouseEnter()
		{
			OnMouseEnter();
		}

		void MouseLeave()
		{
			OnMouseLeave();
		}

		void KeyDown(int key)
		{
			OnKeyDown(key);
		}

		void KeyUp(int key)
		{
			OnKeyUp(key);
		}

		// --------------------------------------------------------------------------------
		// FOCUS
		// --------------------------------------------------------------------------------
		virtual void OnGotFocus()
		{
		}

		virtual void OnLostFocus()
		{
		}

		// --------------------------------------------------------------------------------
		// TREE TRAVERSAL
		// --------------------------------------------------------------------------------
		enum SearchFlags
		{
			Visible = 1 << 0,
			Enabled = 1 << 1,
			Focusable = 1 << 2,
		};

		// traverse through the tree and find the top-most widget that intersects with point
		Widget* FindTopWidgetAt(const PositionF& position, unsigned int flag)
		{
			// if widget is hidden, bail out
			if (!IsVisible() && (flag & SearchFlags::Visible))
			{
				return nullptr;
			}

			// if widget is disabled, bail out
			if (!IsEnabled() && (flag & SearchFlags::Enabled))
			{
				return nullptr;
			}

			// if widget is not focusable, bail out
			if (!IsFocusable() && (flag & SearchFlags::Focusable))
			{
				return nullptr;
			}

			// do self test first. if this widget did not intersect with point, none of the children can. bail out
			if (!Contains(position))
			{
				return nullptr;
			}

			for (std::vector<std::unique_ptr<Widget>>::reverse_iterator it = m_children.rbegin(); it != m_children.rend(); it++)
			{
				// find the top widget at this child. this call will also check this child for intersect
				Widget* hit = it->get()->FindTopWidgetAt(position, flag);
				if (hit) return hit;
			}

			// if none of this widget's children intersect with point, then this widget does
			return this;
		}

		// find the top child that is visible, enabled, and intersects with given point
		Widget* FindTopChildAt(const PositionF& position, int flag)
		{
			for (std::vector<std::unique_ptr<Widget>>::reverse_iterator it = m_children.rbegin(); it != m_children.rend(); it++)
			{
				// if widget is hidden, bail out
				if (!(*it)->IsVisible() && (flag & SearchFlags::Visible))
				{
					return nullptr;
				}

				// if widget is disabled, bail out
				if (!(*it)->IsEnabled() && (flag & SearchFlags::Enabled))
				{
					return nullptr;
				}

				// if widget is not focusable, bail out
				if (!(*it)->IsFocusable() && (flag & SearchFlags::Focusable))
				{
					return nullptr;
				}

				// if this widget intersects with point..
				if ((*it)->Contains(position))
				{
					// note we're returning this child, not this child's possible descendants that might have intersected with the point 
					return it->get();
				}
			}

			// returns nullptr if none of this widget's children intersects with point
			return nullptr;
		}

		Widget* FindAndResolveZOrderAt(const PositionF& position, int flag)
		{
			Widget* widget = this;

			// if widget is hidden, bail out
			if (!widget->IsVisible() && (flag & SearchFlags::Visible))
			{
				return nullptr;
			}

			// if widget is disabled, bail out
			if (!widget->IsEnabled() && (flag & SearchFlags::Enabled))
			{
				return nullptr;
			}

			// if widget is not focusable, bail out
			if (!widget->IsFocusable() && (flag & SearchFlags::Focusable))
			{
				return nullptr;
			}

			// check first if point is inside the root. bail out if not.
			if (!widget->Contains(position))
			{
				return nullptr;
			}

			while (true)
			{
				// returns nullptr if none of the widget's child intersects with p
				Widget* child = widget->FindTopChildAt(position, flag);

				// bring the child to front is not really part of routing. this is z order handling
				// but its convenient here. the right way architecturally is to collect route path 
				// then process the route path outside of routing. however, that may introduce unnecessary
				// performance impact so doing z order handling here is the best.				
				if (child)
				{
					widget->BringChildToFront(child);
				}
				else
				{
					break;
				}

				widget = child;
			}

			return widget;
		}

		template<typename Func>
		void ForEachChild(const Func& func)
		{
			for (const std::unique_ptr<Widget>& child : m_children)
			{
				func(child.get());
			}
		}

		template<typename Func>
		void ForEachChild(const Func& func) const
		{
			for (const std::unique_ptr<Widget>& child : m_children)
			{
				func(child.get());
			}
		}

		template<typename Func>
		bool ForEachWidget(const Func& func)
		{
			if (!func(this)) return false;

			for (const std::unique_ptr<Widget>& child : m_children)
			{
				if (!child->ForEachWidget(func)) return false;
			}

			return true;
		}

		// --------------------------------------------------------------------------------
		// TOOLTIP
		// --------------------------------------------------------------------------------

		bool HasTooltip() const
		{
			return m_tooltipBuilder != nullptr;
		}

		void BuildTooltip(Widget& tooltip)
		{
			if (m_tooltipBuilder)
			{
				m_tooltipBuilder(*this, tooltip);
			}
		}

		void SetTooltip(std::function<void(Widget&, Widget&)> builder)
		{
			m_tooltipBuilder = std::move(builder);
		}

		// --------------------------------------------------------------------------------
		// Draw
		// --------------------------------------------------------------------------------
		virtual void Draw(const UIDrawContext& context) const
		{
			// default implementation does nothing. derived class can override this to draw itself
		}

		// --------------------------------------------------------------------------------
		// RESOURCE
		// --------------------------------------------------------------------------------
		virtual void ResourceChange()
		{
			OnResourceChange();
		}

	};
#pragma endregion

#pragma region // Layer
	class Layer : public Widget
	{
	public:
		enum Type
		{
			Popup,
			Modal,
			Menu,
			SubMenu
		};

	private:
		friend class LayerStack;

		Widget* m_owner;
		UISystem* m_system;
		Type m_type;

	protected:
		UISystem* GetSystem() const override final
		{
			return m_system;
		}

	public:
		struct BuildDescription
		{
			PositionF position = {};
			SizeF size = {};
			std::function<void(Widget*)> builder = nullptr;
			Type type = Type::Popup;
			bool movable = false;
		};

		Layer(UISystem* system, Widget* owner, const PositionF& pos, const SizeF& size, const Type& type, bool movable) :
			m_owner(owner),
			m_system(system),
			m_type(type)		
		{
			m_moveBehavior = movable? Widget::MoveBehavior::Free : Widget::MoveBehavior::None;
			SetPosition(pos);
			SetSize(size);
			m_focusable = false;
		}

		Widget* GetOwner() const
		{
			return m_owner;
		}

		bool IsModal() const
		{
			return m_type == Type::Modal;
		}

		bool IsMenu() const
		{
			return m_type == Type::Menu;
		}

		bool IsPopup() const
		{
			return m_type == Type::Popup;
		}

		Type GetType() const
		{
			return m_type;
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawLayer(*this, context);
		}
	};

	// design consideration
	// - enforce a policy where in finding route, search stops once a modal overlay did not intersect with input point
	class LayerStack
	{
	private:
		std::vector<std::unique_ptr<Layer>> m_layers;

	public:
		struct Route
		{
			Widget* overlay = nullptr;
			Widget* target = nullptr;
			int index = -1;
			bool isBlockedByModal = false;
		};

		LayerStack()
		{
		}

		size_t Size() const
		{
			return m_layers.size();
		}

		// Collapses the overlay stack starting at the specified overlay index.
		//
		// ---------------------------------------------------------------------------------
		// DESIGN NOTES
		// ---------------------------------------------------------------------------------
		// Layer collapse is always performed from a overlay downward toward the top
		// of the overlay stack.
		//
		// Example:
		//
		//	Stack:
		//		[Layer A]
		//		[Layer B]
		//		[Layer C]
		//
		//	CollapseAt(B)
		//
		//	Result:
		//		[Layer A]
		//
		// Layer B and all overlays above it are removed.
		//
		// ---------------------------------------------------------------------------------
		// CASCADED OVERLAY COLLAPSE
		// ---------------------------------------------------------------------------------
		//
		// Overlays may contain OverlayTriggers that own child overlays higher in the stack.
		//
		// Example:
		//
		//	Layer A
		//		contains Trigger B
		//
		//	Layer B
		//		contains Trigger C
		//
		//	Layer C
		//
		// During collapse, overlay widgets are first unregistered from the UISystem
		// before the overlay itself is erased from the overlay stack.
		//
		// While unregistering:
		//
		//	OverlayTrigger::OnUnregisterToSystem()
		//		-> UISystem::UnregisterLayer()
		//			-> CollapseByOwner()
		//
		// may recursively request collapse of child overlays higher in the stack.
		//
		// This is safe because:
		//	- overlay ownership is acyclic
		//	- overlay stack destruction only proceeds upward
		//	- overlay mutations only remove suffixes of the overlay stack
		//	- traversal is index-based (not iterator-based)
		//	- the overlay stack is never reordered during collapse
		//
		// Example:
		//
		//	Initial stack:
		//		[A][B][C]
		//
		//	CollapseAt(A)
		//
		//	1. A unregisters Trigger B
		//	2. Trigger B collapses B
		//	3. B unregisters Trigger C
		//	4. Trigger C collapses C
		//
		// Each nested collapse only removes overlays above the current overlay.
		//
		// Because nested collapses only shrink the end of the overlay stack,
		// the outer forward traversal remains valid and will naturally terminate
		// once the overlay stack size becomes smaller than the current traversal index.
		//
		// ---------------------------------------------------------------------------------
		// IMPORTANT INVARIANT
		// ---------------------------------------------------------------------------------
		//
		// This method is only safe because overlay collapse semantics are strictly:
		//
		//	- synchronous
		//	- upward-only
		//	- suffix-removing
		//
		// Future changes such as below may invalidate these assumptions and require a deferred mutation model.
		//	- arbitrary overlay removal
		//	- overlay insertion during collapse
		//	- overlay reordering
		//	- deferred destruction
		//	- async/evented mutation
		void CollapseAt(const Route& result)
		{
			int index = result.index < 0 ? 0 : result.index;

			// index can be out of bounds. if there are no active overlays, and this is called, if index = 0, then this condition is valid
			if (index >= (int)m_layers.size()) return;

			// since we're removing overlays, their children must unregister to system.
			for (size_t i = index; i < m_layers.size(); i++)
			{
				m_layers[i]->RemoveChildren();
				m_layers[i]->OnUnregisterToSystem();
			}

			// after unregistering overlays' tree, remove them 
			m_layers.erase(m_layers.begin() + index, m_layers.end());
		}

		void CollapseAbove(const Route& route)
		{
			Route routeAbove = route;
			routeAbove.index++;
			CollapseAt(routeAbove);
		}

		void Collapse()
		{
			Route route;
			route.index = 0;
			CollapseAt(route);
		}

		// this is the only way to add a new overlay in the stack and it will always end it at the end of the stack
		void Add(std::unique_ptr<Layer> overlay)
		{
			m_layers.push_back(std::move(overlay));
		}

		Route FindRouteByOwner(Widget* owner)
		{
			Route result{ nullptr, nullptr, -1 };

			// check if any active overlay is owned by given owner
			for (int i = 0; i < m_layers.size(); i++)
			{
				// if this widget is an owner of existing overlay, then overlay is active. collapse overlay stack on it
				if (m_layers[i].get()->GetOwner() == owner)
				{
					result.overlay = m_layers[i].get();
					result.target = m_layers[i].get();
					result.index = i;
					break;
				}
			}

			return result;
		}

		// collapses overlay stack on overlay with the specified owner widget
		void CollapseByOwner(Widget* owner)
		{
			// find the active overlay that is owned by given owner, if any
			Route result = FindRouteByOwner(owner);
			if (!result.overlay) return;

			// if found, since you get the index, create Route and set the index. collapse on it
			CollapseAt(result);
		}

		// traverse through the overlay stack from bottom to top
		template<typename Func>
		void ForEach(const Func& func)
		{
			for (std::vector<std::unique_ptr<Layer>>::iterator it = m_layers.begin(); it != m_layers.end(); it++)
			{
				func(it->get());
			}
		}

		Layer& Bottom() const
		{
			if (m_layers.empty())
			{
				throw std::runtime_error("Querying an empty stack is wrong.");
			}

			return *m_layers.front().get();
		}

		Layer& Top() const
		{
			if (m_layers.empty())
			{
				throw std::runtime_error("Querying an empty stack is wrong.");
			}

			return *m_layers.back().get();
		}

		// find which top-most active overlay that intersects with given point
		Route FindRouteFromTopAt(const PositionF& position, int flags)
		{
			Route result;

			for (int i = (int)m_layers.size() - 1; i >= 0; i--)
			{
				Widget* widget = m_layers[i]->FindTopWidgetAt(position, flags);
				if (widget)
				{
					result.target = widget;
					result.index = i;
					result.overlay = m_layers[i].get();
					result.isBlockedByModal = false;
					break;
				}
				// if this overlay did not intersect with point, check if it's modal
				else
				{
					// is this overlay a modal? if yes, stop right here. modal overlays when active is the only widget that can absorb user input
					if (m_layers[i]->IsModal())
					{
						result.index = i;
						result.isBlockedByModal = true;
						break;
					}
				}
			}

			return result;
		}

		bool IsExpanded(const Widget* owner) const
		{
			// check if any active overlay is owned by given owner
			for (int i = 0; i < m_layers.size(); i++)
			{
				// if this widget is an owner of existing overlay, then overlay is active. 
				if (m_layers[i].get()->GetOwner() == owner)
				{
					return true;
				}
			}

			return false;
		}

	};

	class LayerManager
	{
	private:
		// internal data structure to store command request 
		struct Command
		{
			enum Type
			{
				Add,
				Remove,
				Collapse,
			};

			Type command;
			Widget* owner = nullptr;
			int index;
			PositionF position;
			SizeF size;
			std::function<void(Widget*)> builder;
			Layer::Type type;
			bool movable;
		};

		LayerStack m_stack;
		UISystem* m_system;
		Dictionary<Widget*, Layer::BuildDescription> m_buildDescriptions;
		std::vector<Command> m_commands;

	public:
		LayerManager(UISystem* system) :
			m_system(system)
		{
		}

		void CollapseAbove(const LayerStack::Route& route)
		{
			m_stack.CollapseAbove(route);
		}

		// finds the top-most layer that intersects with given position and valid with given flags
		LayerStack::Route FindRouteFromTopAt(const PositionF& position, int flags)
		{
			return m_stack.FindRouteFromTopAt(position, flags);
		}

		void Collapse()
		{
			m_stack.Collapse();
		}

		void FlushCommands()
		{
			m_commands.clear();
		}

		// given a overlay stack route result, let overlay tree handle mouse down by performing overlay stack collapse if needed, 
		// and process on queue overlay command requests e.g. toggle up/down a overlay
		void ProcessCommandRequests()
		{
			// handle overlay add/remove queue requests
			for (Command& cmd : m_commands)
			{
				switch (cmd.command)
				{
				// remove/toggle off the overlay that is owned by widget from overlay request
				case Command::Remove:
				{
					// we already have the index of the overlay stack that we want to collapsed at. just validate and collapse with it
					if (cmd.index >= 0 && cmd.index < m_stack.Size())
					{
						LayerStack::Route route{};
						route.index = cmd.index;
						m_stack.CollapseAt(route);
					}
					break;
				}
				// add this overlay on top of stack
				case Command::Add:
				{
					// create the overlay
					std::unique_ptr<Layer> overlay = std::make_unique<Layer>(m_system, cmd.owner, cmd.position, cmd.size, cmd.type, cmd.movable);

					// if it has a payload, build it and add to overlay as child
					if (cmd.builder)
					{
						cmd.builder(overlay.get());
					}

					// finally, add overlay to top of stack
					m_stack.Add(std::move(overlay));
					break;
				}
				case Command::Collapse:
				{
					m_stack.Collapse();
					break;
				}
				default:
					break;
				}
			}

			// flush the commands after consuming them
			m_commands.clear();
		}

		// toggle the overlay
		void QueueToggle(Widget* owner)
		{
			// check if there is an active overlay that is owned by given owner
			LayerStack::Route result = m_stack.FindRouteByOwner(owner);

			// if the owner's overlay is already active, queue it for removal/collapse
			if (result.overlay)
			{
				Command cmd{};
				cmd.command = Command::Remove;
				cmd.index = result.index;
				cmd.owner = owner;
				m_commands.push_back(cmd);
				return;
			}

			// this widget's overlay does not exist in overlay stack. create it and add into top of the stack. but first, check if this widget has registered overlay build command
			if (!m_buildDescriptions.Has(owner))
			{
				throw std::runtime_error("command for this owner does not exist");
			}

			// get the popu build command 
			Layer::BuildDescription& desc = m_buildDescriptions.Get(owner);

			// create overlay build request
			Command cmd{};
			cmd.command = Command::Add;
			cmd.owner = owner;
			cmd.position = owner->GetAbsolutePosition() + desc.position;
			cmd.size = desc.size;
			cmd.builder = desc.builder;
			cmd.type = desc.type;
			cmd.movable = desc.movable;
			m_commands.push_back(cmd);
		}

		// register a overlay build description owned by given widget
		bool Register(Widget* widget, const Layer::BuildDescription& desc)
		{
			return m_buildDescriptions.Register(widget, desc);
		}

		// unregister a overlay build description owned by given widget
		bool Unregister(Widget* owner)
		{
			// collapse overlay stack at the overlay of this owner, if any
			m_stack.CollapseByOwner(owner);

			// then we unregister it from our overlay layer
			return m_buildDescriptions.Unregister(owner);
		}

		// traverse through the overlay stack from bottom to top
		template<typename Func>
		void ForEach(const Func& func)
		{
			m_stack.ForEach(func);
		}

		// queue add overlay based on build description as this has no owner
		void QueueAdd(const Layer::BuildDescription& desc)
		{
			// create overlay build command on top of stack based on build description
			Command cmd{};
			cmd.command = Command::Add;
			cmd.owner = nullptr;
			cmd.position = desc.position;
			cmd.size = desc.size;
			cmd.builder = desc.builder;
			cmd.type = desc.type;
			cmd.movable = desc.movable;
			m_commands.push_back(cmd);
		}

		void QueueCollapse(int index = 0)
		{
			Command cmd{};
			cmd.command = Command::Remove;
			cmd.index = index;
			m_commands.push_back(cmd);
		}

		Layer& Bottom() const
		{
			if (!m_stack.Size())
			{
				throw std::runtime_error("stack is empty. querying for first is wrong");
			}

			return m_stack.Bottom();
		}

		bool IsExpanded(const Widget* owner) const
		{
			return m_stack.IsExpanded(owner);
		}
	};

#pragma endregion

#pragma region // UIRenderer
	class UIRenderer
	{
	private:
	public:
		static void Draw(const UIDrawContext& context, const Widget& widget)
		{
			// if widget is hidden, its whole tree is also hidden. bail out
			if (!widget.IsVisible()) return;

			// draw this widget
			widget.Draw(context);

			// get current clip region from renderer. intersect with this widget's rect to get effective clip region. 
			RectF orig = context.renderer.GetClipRegion();
			RectF effective = widget.GetAbsoluteRect().Intersect(orig);

			// apply effective clip region to renderer. this will make sure this widget's tree will be clipped by this widget's rect
			context.renderer.SetClipRegion(effective);

			// draw children
			widget.ForEachChild([&](Widget* widget)
				{
					Draw(context, *widget);
				});

			// restore previous clip region after drawing this widget's tree
			context.renderer.SetClipRegion(orig);
		}
	};
#pragma endregion

#pragma region // Tooltip
	class Tooltip : public Widget
	{
	private:
	public:
		Tooltip()
		{
			m_moveBehavior = MoveBehavior::None;
			m_focusable = false;
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawTooltip(*this, context);
		}
	};

	// design consideration:
	// - only one tooltip exists
	// - tooltip information belongs to a widget
	// - not all widgets has tooltip
	// - tooltip should not show during capture/drag
	// - tooltip position is relative to owner widget
	// - delayed appearance is a system timing concern, not widget concern
	// design consideration:
	// - "it does not decide whether to show or hide a tooltip, but if it's told to show a tooltip, it will show it his way"
	//		- what it means is that UISystem decides if a tooltip is shown or hidden and requests ToolTipManager to do it
	//		- but ToolTipManager decides how tooltip is shown e.g. TooltipManager will apply delay before showing tooltip
	// - the following policies are enforced by UISystem regarding showing or hiding of tooltip
	//		- if mouse is captured by widget, tooltip must be hidden
	//		- if mouse hovers over a widget while mouse is not captured and widget has tooltip, tooltip is displayed
	class TooltipManager
	{
	private:
		Tooltip m_tooltip;
		Widget* m_owner;

	public:
		TooltipManager() :
			m_owner(nullptr)
		{
			m_tooltip.SetPosition({ 0,0 });
			m_tooltip.SetSize({ 0,0 });
		}

		void Hide()
		{
			m_tooltip.RemoveChildren();
			m_tooltip.SetPosition({ 0,0 });
			m_tooltip.SetSize({ 0,0 });
			m_tooltip.Hide();
			m_owner = nullptr;
		}

		// toggle the overlay
		void Show(Widget* hover)
		{
			if (!hover || !hover->HasTooltip())
			{
				Hide();
				return;
			}

			if (hover != m_owner)
			{
				// do this first to flush the old tooltip
				Hide();

				// rebuild tooltip for new owner
				hover->BuildTooltip(m_tooltip);

				// this is new tooltip owner now
				m_owner = hover;
			}

			// show tooltip
			m_tooltip.Show();
		}

		const Widget* Get() const
		{
			return &m_tooltip;
		}
	};
#pragma endregion

#pragma region // DragDropLayer
	// this class is a widget layer in UI system. there should be only one of this in a UI system
	// it's purpose is to store current widgets that are in drag/drop state.
	// at the beginning of drag state, it adopts the dragged widget as its child and manages its movement
	// at the end of drag state (drop), it release the dragged widget into appropriate droppable target widget
	// if there is no appropriate droppable target widget, it returns it to original parent
	class DragDropLayer : public Widget
	{
		// context to remember information about a widget being dragged
		struct DragDropContext
		{
			Widget* originalParent = nullptr;
			PositionF originalPosition;
		};

	private:
		UISystem* m_system;
		Dictionary<Widget*, DragDropContext> m_draggables;

	protected:
		UISystem* GetSystem() const override final
		{
			return m_system;
		}

	public:
		DragDropLayer(UISystem* system):
			m_system(system)
		{
			// this layer should not be movable at all. it should remain and behave like a root widget
			m_moveBehavior = MoveBehavior::None;

			// not focusable, not droppable
			m_focusable = false;
			m_droppable = false;

			// not necessary but just making it explicit to tell that this layer is root like
			SetPosition({ 0,0 });
		}

		// this method is called when a drag/drop state on a given widget is about to begin
		void Begin(Widget* draggable)
		{
			// only reason why our draggables contain something is if we previously started dragging a draggable and has not dropped it yet.
			// starting another drag while in this state is unacceptable. i should not happen
			if (m_draggables.Size())
			{
				throw std::runtime_error("we're about to start dragging something, why are we already in dragging state?");
			}

			// just to be sure, layer should not have any children before we begin a drag. if it does, it means it is dragging something already
			// so that is not possible. 
			if (HasChildren())
			{
				throw std::runtime_error("we're about to start dragging something, why do we already dragging something?");
			}

			// it's not possible to drag an invalid draggable widget. 
			if (!draggable)
			{
				throw std::runtime_error("draggable widget cannot be invalid");
			}

			// let's be strict here. if dragged widget has no parent, that is not acceptable!
			if(!draggable->GetParent())
			{
				throw std::runtime_error("widget is not attached to any parent");
			}

			// save the absolute position of the drag widget. we need to translate its position once we move it to the layer 
			PositionF pos = draggable->GetAbsolutePosition();

			// save reference to original parent. in case target drop is not a valid droppable widget, this widget returns to its original parent
			DragDropContext context{};
			context.originalParent = draggable->GetParent();

			// also save reference to drag widget's original position relative to its original parent. 
			// in case target drop is not a valid droppable widget, drag widget remains in original parent and in original position
			context.originalPosition = draggable->GetPosition();

			// let's register this drag widget and its information so we remember later once we drop it
			m_draggables.Register(draggable, context);

			// let's now move the drag widget into the drag layer
			draggable->GetParent()->MoveChildTo(draggable, this);

			// we're policing very strictly here. may not be necessary, but good to have. also, we don't execute this every frame so i think it's ok to be strict.
			if (draggable->GetParent() != this)
			{
				throw std::runtime_error("failed to move draggable into dragdrop layer");
			}

			// since the drag widget is now a child of this layer, let's translate its position from absolute to relative to this layer
			// note that dragdrop layer is root like and its position is 0,0 so translating does not do anything. but for now we do this to be explicit
			pos = pos - GetAbsolutePosition();
			draggable->SetPosition(pos);
		}

		void End(Widget* draggable, Widget* newParent)
		{
			// we're about to end dragging but if there is no draggable, how is this possible? this cannot happen
			if (!m_draggables.Size())
			{
				throw std::runtime_error("we're about to end dragging something, where are the draggables to drop?");
			}

			// our layer has no draggable to drop while trying to end a drag state? that is not possible
			if (!HasChildren())
			{
				throw std::runtime_error("we're about to end dragging something, why do we not contain a draggable?");
			}

			// it's not possible to drop an invalid draggable widget. 
			if (!draggable)
			{
				throw std::runtime_error("draggable widget cannot be invalid");
			}

			// if we're trying to drop a draggable that is not being dragged, something is wrong
			if (!m_draggables.Has(draggable))
			{
				throw std::runtime_error("trying to drop a draggable that is not tracked");
			}

			DragDropContext& context = m_draggables.Get(draggable);

			// check what's gonna be the parent - original or new?
			Widget* parent = newParent ?	// is new parent valid?
				newParent->IsDroppable() ?	// is new parent droppable?
				newParent:					// new parent is valid, set it
				context.originalParent:		// new parent is not droppable, so using the original parent
				context.originalParent;		// new parent is invalid, so using the original parent 

			// check what will be the position of the drag widget once it is dropped - is it back to original position or now in the new parent?
			// NOTE: we calculate position here before moving child to new parent because we refer to drag widget's absolute position here prior to being moved to new parent
			PositionF pos = newParent ?												// is new parent valid?
				newParent->IsDroppable() ?											// is new parent droppable?
				draggable->GetAbsolutePosition() - parent->GetAbsolutePosition() :	// new parent is valid, so position is now relative to new parent
				context.originalPosition :												// new parent is not droppable, so using original position
				context.originalPosition;												// new parent is invalid, so using original position

			// move the drag widget to new parent. either drop it on new parent, or return it back to original parent
			MoveChildTo(draggable, parent);

			// we're policing very strictly here. may not be necessary, but good to have. also, we don't execute this every frame so i think it's ok to be strict.
			if (draggable->GetParent() != parent)
			{
				throw std::runtime_error("failed to move draggable into a droppable parent");
			}

			// move position of the drag widget now relative to new parent
			draggable->SetPosition(pos);

			// TODO: for now, let's just always call this when drop happens. we don't know what use cases are for handling this yet. let's deal with it once we hit them use cases
			// let parent invoke drop acceptance event
			parent->DropAccepted(draggable);

			// clear our draggables list
			m_draggables.Clear();
		}
	};	
#pragma endregion

#pragma region // UIResources
	struct UIResources
	{
		IFontAtlas* defaultFont = nullptr;
		IFontAtlas* highlightFont = nullptr;
		IFontAtlas* titleFont = nullptr;

		enum class FontType
		{
			Default,
			Highlight,
			Title
		};
	};
#pragma endregion

#pragma region // UISystem
	class UISystem
	{
	private:
		LayerManager m_layerManager;
		TooltipManager m_tooltipManager;
		DragDropLayer	m_DragDropLayer;

		Widget* m_mouseCapture = nullptr;
		Widget* m_mouseOver = nullptr;
		Widget* m_focus = nullptr;

		UIResources m_resources;

		void SetFocus(Widget* widget)
		{
			// if we're setting the same widget that is already in focus, do nothing
			if (m_focus == widget) return;

			// since we're changing focus, notify current focus it's about to lose focus
			if (m_focus)
			{
				m_focus->OnLostFocus();
				m_focus = nullptr;
			}

			// if new focus widget does not exist, bail out
			if (!widget) return;

			// if this widget is not focusable, bail out
			if (!widget->IsFocusable()) return;

			// this new widget is valid to be new focus, notify it
			m_focus = widget;
			if (m_focus)
			{
				m_focus->OnGotFocus();
			}
		}

		void SetCapture(Widget* widget)
		{
			m_mouseCapture = widget;
		}

		Widget& Root() const
		{
			return m_layerManager.Bottom();
		}

	public:
		void SetFont(IFontAtlas* font, UIResources::FontType type)
		{
			bool fontChanged = false;
			switch (type)
			{
			case UIResources::FontType::Default:
				if (m_resources.defaultFont != font) fontChanged = true;
				m_resources.defaultFont = font;				
				break;
			case UIResources::FontType::Highlight:
				if (m_resources.highlightFont != font) fontChanged = true;
				m_resources.highlightFont = font;
				break;
			case UIResources::FontType::Title:
				if (m_resources.titleFont != font) fontChanged = true;
				m_resources.titleFont = font;
				break;
			default:
				break;
			}

			// update all widgets if font changed as they may need to recalculate their layout based on new font
			if (fontChanged)
			{
				m_layerManager.ForEach([](Widget* widget)
					{
						widget->ForEachWidget([](Widget* widget)
							{
								widget->ResourceChange();
								return true;
							});
					});
			}
		}

		IFontAtlas* GetFont(UIResources::FontType type) const
		{
			switch (type)
			{
			case UIResources::FontType::Default:
				return m_resources.defaultFont;
			case UIResources::FontType::Highlight:
				return m_resources.highlightFont;
			case UIResources::FontType::Title:
				return m_resources.titleFont;
			default:
				return nullptr;
			}
		}

		UISystem() :
			//m_layoutTree(this),
			m_layerManager(this),
			m_DragDropLayer(this)
		{
			// define build for root layer and queue on layer manager
			Layer::BuildDescription root
			{
				PositionF{0,0},
				SizeF{0, 0},
				nullptr,
				Layer::Modal,
				false
			};
			m_layerManager.QueueAdd(root);

			// build the root layer
			m_layerManager.ProcessCommandRequests();
		}

		void SetSize(const SizeF& size)
		{
			Root().SetSize(size);
		}

		void SetPosition(const PositionF& pos)
		{
			Root().SetPosition(pos);
		}

		void Show()
		{
			Root().Show();
		}

		void Draw(UIDrawContext& context)
		{
			// set the input state in context so that widgets can use it when drawing themselves
			context.capture = m_mouseCapture;
			context.hover = m_mouseOver;
			context.focus = m_focus;

			// draw overlays
			m_layerManager.ForEach([&](Widget* widget)
				{
					UIRenderer::Draw(context, *widget);
				});

			// draw tooltip
			UIRenderer::Draw(context, *m_tooltipManager.Get());

			// draw draggable
			m_DragDropLayer.ForEachChild([&](Widget* widget)
				{
					UIRenderer::Draw(context, *widget);
				});
		}

		// this "detaches" the widget from system. if widget is mouse capture, hover, or focus, these states will be reset to null
		void Detach(Widget* widget)
		{
			if (m_mouseCapture == widget) SetCapture(nullptr);
			if (m_focus == widget) SetFocus(nullptr);
			if (m_mouseOver == widget) m_mouseOver = nullptr;
		}

		// scenario 1 - no overlay exists, overlay trigger is clicked
		//		- system does not check overlay tree for hit, as it is empty
		//		- overlay trigger requests system to toggle its overlay
		//		- system does not have its overlay yet so queue it to add
		//		- system does not remove any overlay in tree. does nothing
		//		- system handles all queued overlay requests
		// 
		// scenario 2 - overlays exists, overlay trigger is clicked, and its overlay already exists
		//		- none of the overlays in overlay tree is hit, so all is queued for removal
		//		- overlay trigger requests system to toggle its overlay
		//		- system have its overlay so queue it to remove
		//		- system removes all existing overlay in overlay tree
		//		- system handles all queued overlay requests
		// 
		// scenario 3 - overlays exists, overlay trigger is clicked
		// 		- none of the overlays in overlay tree is hit, so all is queued for removal
		//		- overlay trigger requests system to toggle its overlay
		//		- system does not have its overlay yet so queue it to add
		//		- system removes all existing overlay in overlay tree
		//		- system handles all queued overlay requests
		// 
		// scenario 4 - overlays exists, overlay trigger's overlay is active, mouse clicked somewhere not in any overlay nor in overlay trigger
		// 		- none of the overlays in overlay tree is hit, so all is queued for removal
		//		- overlay trigger does nothing. it did not get hit.
		//		- system removes all existing overlay in overlay tree
		//		- system has no pop requests to handle, does nothing
		// 
		// scenario 5 - overlay exists, overlay trigger's overlay is active, mouse clicked in one of the existing overlays
		//		- system finds overlay that got hit in stack. queue overlays above it for removal
		//		- overlay trigger does nothing. it did not get hit.
		//		- system removes all overlays on queue for removal
		//		- system has no pop requests to handle, does nothing
		// 
		// scenario 6 - no overlay exists, mouse clicked somewhere not in any overlay nor in overlay trigger
		//		- system does not check overlay tree for hit, as it is empty
		//		- overlay trigger does nothing. it did not get hit.
		//		- system does not remove any overlay in tree. does nothing
		//		- system has no pop requests to handle, does nothing
		// 
		// scenario 7 - overlay exists, overlay trigger a's overlay is active, but overlay trigger b is clicked
		// 		- none of the overlays in overlay tree is hit, so all is queued for removal
		//		- overlay trigger b requests system to toggle its overlay
		//		- system checks for popbutton b's overlay. if it exists, queue it for removal. otherwise, queue it for add
		//		- overlay trigger a does nothing. it did not get hit
		//		- system removes all existing overlay in overlay tree
		//		- system handles all queued overlay requests
		// 
		// scenario 8 - overlay opens a child overlay. this only happens if overlay contains a overlay trigger as child (only overlay trigger can request to spawn a overlay, as of now)
		//		- system finds overlay that got hit in stack. queue overlays above it for removal
		//		- overlay trigger clicked requests system to toggle its overlay
		//		- system checks for popbutton's overlay. if it exists, queue it for removal. otherwise, queue it for add
		//		- system removes all overlays on queue for removal
		//		- system handles all queued overlay requests
		// 
		// scenario 9 - modal overlay exists
		//		- THIS IS PROBLEM FOR ANOTHER DAY. WE DON'T HAVE MODAL YET
		//
		void MouseDown(const PositionF& p)
		{
			m_layerManager.FlushCommands();

			// find top overlay that intersects with point. overlay must be visible and enabled
			LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// check if result says we're block by modal. this means that a modal layer exist and did not intersect with point and this blocks search to succeeding layer stack
			if (result.isBlockedByModal)
			{
				// if block by modal, collapse above it. we should not collapse modals. it should only be collapsed via command
				m_layerManager.CollapseAbove(result);

				// in case focus, hover and capture are set to widgets that belong to overlay that collapsed, they are reset safely via UnregisterToSystem>Detach
				return;
			}			

			// if we reach this point, we should be able to find the top widget that intersects with point. simultaneously we can resolve Z order as we traverse to find the top widget
			// since bottom layer is a modal (root), it should always exist therefore we should always expect a valid layer at this point
			// if not, then we must throw exception as this should not happen
			if (!result.overlay)
			{
				throw std::runtime_error("impossible not to find an overlay. why is this so???");
			}

			Widget* widget = result.overlay->FindAndResolveZOrderAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// at this point, we should have the top-most widget and Z order is resolved. it's impossible to not find top-most widget, we already have the layer.
			if (!widget)
			{
				throw std::runtime_error("why no top-most widget when we already found the layer??");
			}

			// now we are ready to execute MouseDown event on the clicked widget, if there is one. by right there should be one by this time. 
			widget->MouseDown(p);

			// collapse the overlay stack above the clicked overlay. we do this because:
			// - if none of the overlays were clicked, all active overlay stacks will be collapsed 
			// - if a overlay is clicked, all active overlays on top of it will be collapsed
			m_layerManager.CollapseAbove(result);

			// set capture
			SetCapture(widget);

			// set focus
			SetFocus(widget);

			// hide tooltip. if mouse is down, tooltip should be hidden regardless of where the mouse is clicked
			m_tooltipManager.Hide();
		}

		void MouseUp(const PositionF& p)
		{
			if (!m_mouseCapture) return;
			m_mouseCapture->MouseUp(p);
			m_mouseCapture = nullptr;

			// by right, tooltip of the widget (if it has tooltip) the mouse hovers now should appear... 
			// but after mouse up, we don't have mouse over widget yet, so we don't bother showing tooltip now
		}

		void MouseMove(const PositionF& p)
		{
			// prioritize captured widget to handle mouse move 
			if (m_mouseCapture)
			{
				m_mouseCapture->MouseMove(p);

				// since mouse is captured, tooltip should be hidden
				m_tooltipManager.Hide();

				return;
			}

			// check first if mouse hovers over a overlay
			LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// if mouse hovers outside of the top overlay in the stack and down to top-most modal overlay, the route result will be "blocked by modal"
			// this is because when one or more modal overlay exists, the top-most modal overlay and succeeding overlays on top of it are the only ones 
			// allowed to receive mouse event or user input in general. if mouse cursor did not hover over any of them overlays, then mouse move is ignored. 
			if (result.isBlockedByModal)
			{
				// just in case there is a mouse over widget somewhere, let's handle its mouse leave
				if (m_mouseOver)
				{
					m_mouseOver->MouseLeave();
					m_mouseOver = nullptr;
				}

				// make sure to hide any active tooltip as well
				m_tooltipManager.Hide();

				return;
			}

			// if there is no overlay found yet we were not blocked by modal, something is wrong. this cannot happen
			if (!result.overlay)
			{
				throw std::runtime_error("impossible not to find an overlay. why is this so???");
			}

			// find the top widget in this layer's tree that is hovered. we also include disabled widgets in hover check.
			// reason is so that even disable widgets can still have tooltip shown if they have it
			Widget* hover = result.overlay->FindTopWidgetAt(p, Widget::SearchFlags::Visible);

			// let's resolve which widget is mouse over now, if any
			if (hover != m_mouseOver)
			{
				// invoke mouse leave on current mouse hover widget
				if (m_mouseOver)
				{
					m_mouseOver->MouseLeave();
				}

				// just in case we hover outside of root, assuming root is not desktop, hover will be nullptr
				m_mouseOver = hover;
				if (m_mouseOver)
				{
					m_mouseOver->MouseEnter();
				}
			}

			// finally if there is a mouse over widget, let it handle mouse move event
			if (m_mouseOver)
			{
				m_mouseOver->MouseMove(p);
			}

			// if you reach this point, then mouse hovers a widget that might have a tooltip. show it.
			m_tooltipManager.Show(m_mouseOver);
		}

		void KeyDown(int key)
		{
			if (m_focus)
			{
				m_focus->KeyDown(key);
			}
		}

		void KeyUp(int key)
		{
			if (m_focus)
			{
				m_focus->KeyUp(key);
			}
		}

		bool RegisterLayer(Widget* widget, const Layer::BuildDescription& desc)
		{
			return m_layerManager.Register(widget, desc);
		}

		bool UnregisterLayer(Widget* owner)
		{
			return m_layerManager.Unregister(owner);
		}

		void ToggleLayer(Widget* owner)
		{
			m_layerManager.QueueToggle(owner);
		}

		void AddWidget(std::unique_ptr<Widget> widget)
		{
			Root().AddChild(std::move(widget));
		}

		void RemoveWidget(Widget* widget)
		{
			// bail out if invalid
			if (!widget) return;

			// we can now remove this widget. this will remove the widget's whole tree. 
			//if (!m_layoutTree.Remove(widget))
			if (!Root().Remove(widget))
			{
				// let's be strict for now to catch any silent error
				throw std::runtime_error("failed to remove a widget from root");
			}
		}

		void Collapse()
		{
			m_layerManager.QueueCollapse(1);
		}

		void AddLayer(const Layer::BuildDescription& desc)
		{
			m_layerManager.QueueAdd(desc);
		}

		bool IsLayerExpanded(const Widget* owner) const
		{
			return m_layerManager.IsExpanded(owner);
		}

		void Begin()
		{
			m_layerManager.FlushCommands();
		}

		void End()
		{
			// if a overlay trigger is clicked, it might have requested to toggle its overlay. process those requests here
			m_layerManager.ProcessCommandRequests();
		}

		void BeginDrag(Widget* source)
		{
			// for now we just end drag immediately. we can implement this later when we have drag drop scenario
			// but we want to have this method here as placeholder to show where drag drop manager will be used in
			m_DragDropLayer.Begin(source);
		}

		void EndDrag(Widget* draggable,const PositionF& p)
		{
			// 1. find the top-most widget that intersects with given point
			LayerStack::Route result = m_layerManager.FindRouteFromTopAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);

			// if route result is blocked by modal, it means we intersect outside of existing modal layer and there are no other widgets that can be found to drop current dragged widget
			// but if not modal, we must have found the layer that intersects with  point
			Widget* target = nullptr;
			if (!result.isBlockedByModal)
			{
				// but check first if layer is really valid. it must.
				// if there is no overlay found yet we were not blocked by modal, something is wrong. this cannot happen
				if (!result.overlay)
				{
					throw std::runtime_error("impossible not to find an overlay. why is this so???");
				}

				// let's now find the top widget in this layer's tree that intersects with the point
				target = result.overlay->FindAndResolveZOrderAt(p, Widget::SearchFlags::Visible | Widget::SearchFlags::Enabled);
			}


			// 2. pass that widget to dragdrop layer so it will attemp to drop the widget being drag into it
			m_DragDropLayer.End(draggable, target);
		}
	};

	bool Widget::UnregisterToSystem()
	{
		OnUnregisterToSystem();

		UISystem* system = GetSystem();
		if (system) system->Detach(this);
		return true;
	}
#pragma endregion

#pragma region // OverlayTrigger
	class OverlayTrigger : public Widget
	{
	protected:
		Layer::BuildDescription m_buildDesc;

		// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
		bool OnRegisterToSystem() override final
		{
			UISystem* system = GetSystem();
			if (system)
			{
				// be strict for now
				if (!system->RegisterLayer(this, m_buildDesc))
				{
					throw std::runtime_error("failed to register layer");
				}
			}
			return true;
		}

		// this is fired up when this widget is removed from a widget tree with a UI system. it will remove its layer descriptor into the system
		bool OnUnregisterToSystem() override final
		{
			UISystem* system = GetSystem();
			if (system)
			{
				// be strict for now
				if (!system->UnregisterLayer(this))
				{
					throw std::runtime_error("failed to unregister layer");
				}
			}

			return true;
		}

		// requests system to toggle this widget's overlay
		void Toggle()
		{
			UISystem* system = GetSystem();
			if (system)
			{
				system->ToggleLayer(this);
			}
		}

		void OnMouseDown(const PositionF& position) override final
		{
			Toggle();
		}

	public:
		OverlayTrigger(const Layer::BuildDescription& buildDesc) :
			m_buildDesc(buildDesc)
		{
			m_moveBehavior = MoveBehavior::None;
		}

		virtual bool HasTooltip() const
		{
			return true;
		}

		virtual void BuildTooltip(Widget& tooltip)
		{
			// this is just for debug purposes. can formalize this later
			tooltip.SetSize({ 80,30 });
			tooltip.SetPosition(GetAbsolutePosition() + PositionF{ GetSize().width + 5, 0 });
		}
	};
#pragma endregion

#pragma region // Draggable
	// a widget that can be dragged from one droppable widget into another
	// it's used for inventory systems, skill bars, customizable menus, etc...
	class Draggable : public Widget
	{
	private:
	protected:
		// this widget is draggable via mouse move so we handle start of dragging through mouse down
		virtual void OnMouseDown(const PositionF& position)
		{
			UISystem* system = GetSystem();
			if (!system)
			{
				throw std::runtime_error("widget is not attached to any UISystem");
			}

			// let system know we want to drag this widget
			system->BeginDrag(this);
		}

		// this widget drops on mouse up
		virtual void OnMouseUp(const PositionF& position)
		{
			UISystem* system = GetSystem();
			if (!system)
			{
				throw std::runtime_error("widget is not attached to any UISystem");
			}

			// let system know we want this widget to drop
			system->EndDrag(this, position);
		}

	public:
		Draggable()
		{
			m_moveBehavior = Widget::MoveBehavior::Free;
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawDraggable(*this, context);
		}
	};
#pragma endregion

#pragma region // BoundRef

	template<typename T>
	class BoundRef
	{
	private:
		T m_internal{};   // fallback storage
		T* m_ptr = nullptr; // external binding

		std::function<T()> m_getter;
		std::function<void(const T&)> m_setter;

	public:
		void Bind(T& external)
		{
			m_ptr = &external;
		}

		void Bind(
			std::function<T()> getter,
			std::function<void(const T&)> setter
		)
		{
			m_ptr = nullptr;
			m_getter = std::move(getter);
			m_setter = std::move(setter);
		}

		void Unbind()
		{
			m_ptr = nullptr;
		}

		T Get() const
		{
			if (m_ptr) return *m_ptr;

			if (m_getter) return m_getter();

			return m_internal;
		}

		void Set(const T& value)
		{
			if (m_ptr)
			{
				*m_ptr = value;
				return;
			}

			if (m_setter)
			{
				m_setter(value);
				return;
			}

			m_internal = value;
		}

		bool IsBound() const { return m_ptr != nullptr; }

		void operator = (const T& v)
		{
			Set(v);
		}

		operator T() const
		{
			return Get();
		}

		BoundRef& operator = (const BoundRef& other)
		{
			Set(other.Get());
			return *this;
		}

	};
#pragma endregion

#pragma region // gui controls
	class Image : public Widget
	{
	private:
		std::unique_ptr<IRenderable> m_image;
		Widget::VerticalAlignment m_vAlign;
		Widget::HorizontalAlignment m_hAlign;
		PositionF m_imagePosition;
		bool m_stretch = false;

	protected:
		// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
		bool OnRegisterToSystem() override final
		{
			RefreshLayout();
			return true;
		}

		void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)override final
		{
			// refresh cached information about text with new font type
			RefreshLayout();
		}

		bool RefreshLayout()
		{
			if (!m_image)
			{
				m_imagePosition = {};
				return false;
			}

			if (m_stretch)
			{
				m_imagePosition = { 0,0 };
			}
			else
			{
				switch (m_vAlign)
				{
				case Widget::VerticalAlignment::Center:
					m_imagePosition.y = (GetSize().height - m_image->GetSprite().GetHeight()) / 2.0f;
					break;
				case Widget::VerticalAlignment::Top:
					m_imagePosition.y = 0;
					break;
				case Widget::VerticalAlignment::Bottom:
					m_imagePosition.y = GetSize().height - m_image->GetSprite().GetHeight();
					break;
				default:
					break;
				}

				switch (m_hAlign)
				{
				case Widget::HorizontalAlignment::Center:
					m_imagePosition.x = (GetSize().width - m_image->GetSprite().GetWidth()) / 2.0f;
					break;
				case Widget::HorizontalAlignment::Left:
					m_imagePosition.x = 0;
					break;
				case Widget::HorizontalAlignment::Right:
					m_imagePosition.x = GetSize().width - m_image->GetSprite().GetWidth();
					break;
				default:
					break;
				}
			}

			return true;
		}

	public:
		Image(std::unique_ptr<IRenderable> renderable) :
			m_image(std::move(renderable)),
			m_vAlign(Widget::VerticalAlignment::Center),
			m_hAlign(Widget::HorizontalAlignment::Center),
			m_imagePosition({ 0,0 })
		{
			m_moveBehavior = MoveBehavior::None;
			RefreshLayout();
			m_hitTestBehavior = HitTestBehavior::AlwaysFail;
		}

		void EnableStretch(bool stretch)
		{
			m_stretch = stretch;
			RefreshLayout();
		}

		bool IsStretched() const
		{
			return m_stretch;
		}

		PositionF GetImageAbsolutePosition() const
		{
			return GetAbsolutePosition() + m_imagePosition;
		}

		Sprite Get() const
		{
			return m_image->GetSprite();
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawImage(*this, context);
		}

		void SetAlignment(Widget::VerticalAlignment vAlign, Widget::HorizontalAlignment hAlign)
		{
			m_vAlign = vAlign;
			m_hAlign = hAlign;
			RefreshLayout();
		}
	};

	class Label : public Widget
	{
	private:
		std::string m_text;
		UIResources::FontType m_fontType;
		Widget::VerticalAlignment m_vAlign;
		Widget::HorizontalAlignment m_hAlign;
		SizeF m_textSize;
		PositionF m_textPosition;

	protected:
		// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
		bool OnRegisterToSystem() override final
		{
			// refresh cached information about text with new font type
			return RefreshLayout();
		}

		// this is fired up when this widget is removed from a widget tree with a UI system. it will remove its layer descriptor into the system
		bool OnUnregisterToSystem() override final
		{
			// refresh cached information about text with new font type
			return RefreshLayout();
		}

		void OnResourceChange() override final
		{
			// refresh cached information about text with new font type
			RefreshLayout();
		}

		void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize)override final
		{
			// refresh cached information about text with new font type
			RefreshLayout();
		}

		bool RefreshLayout()
		{
			UISystem* system = GetSystem();
			if (!system)
			{
				m_textSize = {};
				m_textPosition = {};
				return false;
			}

			IFontAtlas* font = system->GetFont(m_fontType);
			if (!font)
			{
				m_textSize = {};
				m_textPosition = {};
				return false;
			}

			m_textSize = font->GetSize(m_text);

			switch (m_vAlign)
			{
			case Widget::VerticalAlignment::Center:
				m_textPosition.y = (GetSize().height - m_textSize.height) / 2.0f;
				break;
			case Widget::VerticalAlignment::Top:
				m_textPosition.y = 0;
				break;
			case Widget::VerticalAlignment::Bottom:
				m_textPosition.y = GetSize().height - m_textSize.height;
				break;
			default:
				break;
			}

			switch (m_hAlign)
			{
			case Widget::HorizontalAlignment::Center:
				m_textPosition.x = (GetSize().width - m_textSize.width) / 2.0f;
				break;
			case Widget::HorizontalAlignment::Left:
				m_textPosition.x = 0;
				break;
			case Widget::HorizontalAlignment::Right:
				m_textPosition.x = GetSize().width - m_textSize.width;
				break;
			default:
				break;
			}

			return true;
		}

	public:
		Label(const std::string& text, UIResources::FontType fontType = UIResources::FontType::Default) :
			m_text(text),
			m_fontType(fontType),
			m_vAlign(Widget::VerticalAlignment::Center),
			m_hAlign(Widget::HorizontalAlignment::Center),
			m_textSize({0,0}),
			m_textPosition({0,0})
		{
			m_moveBehavior = MoveBehavior::None;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail;
		}

		PositionF GetTextAbsolutePosition() const
		{
			return GetAbsolutePosition() + m_textPosition;
		}

		void SetFontType(UIResources::FontType type)
		{
			m_fontType = type;

			// refresh cached information about text with new font type
			RefreshLayout();	
		}

		UIResources::FontType GetFontType() const
		{
			return m_fontType;
		}

		std::string Get() const
		{
			return m_text;
		}

		void Set(const std::string& text)
		{
			m_text = text;

			RefreshLayout();
		}

		void SetAlignment(Widget::VerticalAlignment vAlign, Widget::HorizontalAlignment hAlign)
		{
			m_vAlign = vAlign;
			m_hAlign = hAlign;
			RefreshLayout();
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawLabel(*this, context);
		}
	};

	class Frame : public Widget
	{
	private:

	public:
		Frame(bool movable = true, bool droppable = true)
		{
			m_moveBehavior = movable? MoveBehavior::Free : MoveBehavior::None;
			m_droppable = droppable;
			m_focusable = false;
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawFrame(*this, context);
		}
	};
	
	class Button : public Widget
	{
	private:
	public:
		Button()
		{
			m_moveBehavior = MoveBehavior::None;
		}

		event::Event<> OnClick;

		void OnMouseUp(const PositionF& position) override
		{
			// did the mouse release occur over this button? if not, then this mouse up is not for us. ignore
			if (!Contains(position)) return;

			// handle click event
			OnClick();
		}

		void Draw(const UIDrawContext& context) const override
		{
			if(context.skin) context.skin->DrawButton(*this, context);
		}
	};

	class MenuButton : public Button
	{
	protected:
		Layer::BuildDescription m_buildDesc;

		// this is fired up when this widget is added to a widget tree with a UI system. it will register its layer descriptor into the system
		bool OnRegisterToSystem() override final
		{
			UISystem* system = GetSystem();
			if (system)
			{
				// be strict for now
				if (!system->RegisterLayer(this, m_buildDesc))
				{
					throw std::runtime_error("failed to register layer");
				}
			}
			return true;
		}

		// this is fired up when this widget is removed from a widget tree with a UI system. it will remove its layer descriptor into the system
		bool OnUnregisterToSystem() override final
		{
			UISystem* system = GetSystem();
			if (system)
			{
				// be strict for now
				if (!system->UnregisterLayer(this))
				{
					throw std::runtime_error("failed to unregister layer");
				}
			}

			return true;
		}

		// requests system to toggle this widget's overlay
		void Toggle()
		{
			UISystem* system = GetSystem();
			if (system)
			{
				system->ToggleLayer(this);
			}
		}

		void OnMouseDown(const PositionF& position) override final
		{
			Toggle();
		}

	public:
		MenuButton(const Layer::BuildDescription& buildDesc) :
			m_buildDesc(buildDesc)
		{
			m_moveBehavior = MoveBehavior::None;
			m_buildDesc.type = Layer::Menu;
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawMenuButton(*this, context);
		}
	};

	class SubMenuButton : public MenuButton
	{
	protected:
	public:
		SubMenuButton(const Layer::BuildDescription& buildDesc) :
			MenuButton(buildDesc)
		{
			m_buildDesc.type = Layer::SubMenu;
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawSubMenuButton(*this, context);
		}
	};

	class MenuItem : public Button
	{
	public:
		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawMenuItem(*this, context);
		}
	};

	class Thumb : public Widget
	{
	private:
	public:
		Thumb()
		{
			m_moveBehavior = MoveBehavior::None;
			m_hitTestBehavior = HitTestBehavior::AlwaysFail; // non interactive
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawThumb(*this, context);
		}
	};

	class Slider: public Widget
	{
	private:
		float m_min;
		float m_max;
		float m_value;

		bool m_horizontal;
		Widget* m_thumb;
		float m_thumbLength;
		bool m_isDragging;
		int m_steps;

	protected:
		virtual void OnMouseDown(const PositionF& position)
		{
			m_isDragging = true;
			UpdateValueFromPosition(position);
		}

		void OnMouseUp(const PositionF& position) override
		{
			m_isDragging = false;
		}

		void OnMouseMove(const PositionF& position) override
		{
			if (m_isDragging)
			{
				UpdateValueFromPosition(position);
			}
		}

		void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
		{
			UpdateThumbSize();

			UpdateThumbPosition();
		}

		void UpdateThumbPosition()
		{
			// value is in range between min and max. normalize it. if max < min, set normalize value to 0
			float range = m_max - m_min;
			float nvalue = range > 0.0f ? (m_value - m_min) / range : 0.0f;

			SizeF thumbSize = m_thumb->GetSize();

			// horizontal orientation
			if (m_horizontal)
			{
				float x = (GetSize().width - thumbSize.width) * nvalue;
				m_thumb->SetPosition({ x, 0 });
			}
			// vertical orientation
			else
			{
				float y = (GetSize().height - thumbSize.height) * nvalue;
				m_thumb->SetPosition({ 0, y });
			}
		}

		void UpdateThumbSize()
		{
			// if horizontal orientation, get slider height. otherwise, get width
			float thickness = m_horizontal ? GetHeight() : GetWidth();

			// get the slider length. 
			float sliderLength = m_horizontal ? GetWidth() : GetHeight();

			// if clamp thumb length within slider length, if needed
			float length = m_thumbLength < sliderLength ? m_thumbLength : sliderLength;

			SizeF thumbSize
			{
				m_horizontal? length : thickness,
				m_horizontal ? thickness : length,
			};

			m_thumb->SetSize(thumbSize);
		}

		void UpdateValueFromPosition(const PositionF& position)
		{
			// translate the clicked position (world position) to slider's local coordinate
			PositionF local = position - GetAbsolutePosition();

			float thumbLength = m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;

			// get length of the slider. this is the length it can move, so subtract thumb length
			float length = m_horizontal ? GetSize().width - m_thumb->GetSize().width : GetSize().height - m_thumb->GetSize().height;

			// normalize the value of the position based on slider length
			float nvalue = length <= 0.0f ? 0.0f : (m_horizontal? local.x - thumbLength / 2.0f : local.y - thumbLength / 2.0f) / length;

			// if in case position is outside slider extents, clamp it
			nvalue = std::clamp<float>(nvalue, 0.0f, 1.0f);

			// convert it into value based on range 
			float value = m_min + (m_max - m_min) * nvalue;

			value = Snap(value);

			// finally we set value
			Value(value);
		}

		float Snap(float value) const
		{
			if (m_steps <= 1) return m_min;

			int steps = m_steps - 1;

			float stepSize = (m_max - m_min) / steps;	

			value = std::round((value - m_min) / stepSize);
			value *= stepSize;
			value += m_min;

			return value;
		}

	public:
		engine::event::Event<float> OnChange;

		Slider(float min, float max, float thumbLength) :
			m_min(min),
			m_max(max),
			m_thumbLength(thumbLength),
			m_value(min),
			m_horizontal(true),
			m_isDragging(false),
			m_steps(0)
		{
			m_moveBehavior = MoveBehavior::None;

			std::unique_ptr<Thumb> thumb = std::make_unique<Thumb>();
			m_thumb = thumb.get();

			AddChild(std::move(thumb));

		}

		void Horizontal(bool enable)
		{
			if (m_horizontal != enable)
			{
				m_horizontal = enable;

				UpdateThumbSize();

				UpdateThumbPosition();
			}
		}
		
		bool Horizontal() const
		{
			return m_horizontal;
		}

		void SetThumbLength(float length)
		{
			m_thumbLength = length;

			UpdateThumbSize();

			UpdateThumbPosition();
		}

		void Min(float min)
		{
			m_min = min;

			// range changed, we might need to update value if it gets clamped
			Value(m_value);

			// value might not have changed, but thumb position might change with new range
			UpdateThumbPosition();
		}

		void Max(float max)
		{
			m_max = max;

			// range changed, we might need to update value if it gets clamped
			Value(m_value);

			// value might not have changed, but thumb position might change with new range
			UpdateThumbPosition();
		}

		float Min() const
		{
			return m_min;
		}

		float Max() const
		{
			return m_max;
		}

		void Value(float value)
		{
			// always clamp to min if min happens to be larger than max
			if (m_min > m_max)
			{
				value = m_min;
			}
			// otherwise make sure to clamp within range
			else
			{
				value = std::clamp<float>(value, m_min, m_max);
			}

			// if new value same as current, no change, no update, no notification needed
			if (m_value == value)
			{
				return;
			}

			// we're ready to set new value
			m_value = value;

			// update thumb position
			UpdateThumbPosition();

			// fire up event
			OnChange(m_value);
		}

		float Value() const
		{
			return m_value;
		}		

		void SetStepSize(float size)
		{
			m_steps = size > 0 ?
				static_cast<int>(std::round((m_max - m_min) / size)) + 1 :
				1;

			// snap the current value and update value. this will also possibly update thumb position if needed
			Value(Snap(m_value));
		}

		void SetStepCount(int count)
		{
			m_steps = count;

			// snap the current value and update value. this will also possibly update thumb position if needed
			Value(Snap(m_value));
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawSlider(*this, context);
		}
	};

	class Switch: public Widget
	{
	private:
		BoundRef<bool> m_checked;
		bool m_pressed;

	public:
		Switch():
			m_pressed(false)
		{
			m_moveBehavior = MoveBehavior::None;
		}

		event::Event<bool> OnClick;

		void Bind(bool& checked)
		{
			m_checked.Bind(checked);
		}

		void Bind(
			std::function<bool()> getter,
			std::function<void(const bool&)> setter
		)
		{
			m_checked.Bind(getter, setter);
		}

		void Toggle()
		{
			m_checked = !m_checked;
		}

		void TurnOn()
		{
			if (!m_checked) m_checked = true;
		}

		void TurnOff()
		{
			if (m_checked) m_checked = false;
		}

		void OnMouseDown(const PositionF& position) override
		{
			m_pressed = true;
		}

		bool IsOn() const
		{
			return m_checked;
		}

		void OnMouseUp(const PositionF& position) override
		{
			if (!m_pressed) return;

			m_pressed = false;

			// did the mouse release occur over this button? if not, then this mouse up is not for us. ignore
			if (!Contains(position)) return;

			Toggle();

			// handle click event
			OnClick(m_checked);
		}
	};

	class CheckBox : public Switch
	{
	private:

	public:
		CheckBox()
		{
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawCheckBox(*this, context);
		}
	};

	class RadioButton : public Switch
	{
	private:

	public:
		RadioButton()
		{
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawRadioButton(*this, context);
		}
	};

	class ScrollBar : public Widget
	{
	private:
		float m_contentLength;
		float m_viewportLength;
		float m_offset; // current scroll position
		bool m_horizontal;
		Widget* m_thumb;
		bool m_isDragging;

	protected:
		void OnMouseDown(const PositionF& pos) override
		{
			m_isDragging = true;
			UpdateOffsetFromPosition(pos);
		}

		void OnMouseUp(const PositionF&) override { m_isDragging = false; }

		void OnMouseMove(const PositionF& pos) override
		{
			if (m_isDragging) UpdateOffsetFromPosition(pos);
		}

		void OnSizeChanged(const SizeF&, const SizeF&) override
		{
			UpdateThumbSize();
			UpdateThumbPosition();
		}

		void UpdateThumbSize()
		{
			float trackLength = m_horizontal ? GetWidth() : GetHeight();
			float thickness = m_horizontal ? GetHeight() : GetWidth();

			float ratio = m_contentLength > 0 ? m_viewportLength / m_contentLength : 1.0f;
			float length = std::clamp(ratio * trackLength, 10.0f, trackLength); // clamp min size

			SizeF thumbSize{
				m_horizontal ? length : thickness,
				m_horizontal ? thickness : length
			};
			m_thumb->SetSize(thumbSize);
		}

		void UpdateThumbPosition()
		{
			float trackLength = m_horizontal ? GetWidth() : GetHeight();
			float thumbLength = m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;
			float maxOffset = std::max<float>(0.0f, m_contentLength - m_viewportLength);

			float nvalue = maxOffset > 0 ? m_offset / maxOffset : 0.0f;
			float pos = nvalue * (trackLength - thumbLength);

			if (m_horizontal) m_thumb->SetPosition({ pos, 0 });
			else              m_thumb->SetPosition({ 0, pos });
		}

		void UpdateOffsetFromPosition(const PositionF& pos)
		{
			PositionF local = pos - GetAbsolutePosition();
			float trackLength = m_horizontal ? GetWidth() : GetHeight();
			float thumbLength = m_horizontal ? m_thumb->GetSize().width : m_thumb->GetSize().height;
			float length = trackLength - thumbLength;

			float nvalue = length > 0 ? (m_horizontal ? local.x - thumbLength / 2 : local.y - thumbLength / 2) / length : 0;
			nvalue = std::clamp(nvalue, 0.0f, 1.0f);

			float maxOffset = std::max<float>(0.0f, m_contentLength - m_viewportLength);
			float newOffset = nvalue * maxOffset;

			SetOffset(newOffset);
		}

		void ClampOffset()
		{
			float maxOffset = std::max<float>(0.0f, m_contentLength - m_viewportLength);
			m_offset = std::clamp(m_offset, 0.0f, maxOffset);
		}

	public:
		engine::event::Event<float> OnScroll;

		ScrollBar(float contentLength, float viewportLength): 
			m_contentLength(contentLength), 
			m_viewportLength(viewportLength),
			m_offset(0), 
			m_horizontal(true), 
			m_isDragging(false)
		{
			m_moveBehavior = MoveBehavior::None;

			std::unique_ptr<Thumb> thumb = std::make_unique<Thumb>();
			m_thumb = thumb.get();
			AddChild(std::move(thumb));
		}

		void SetOffset(float offset)
		{
			offset = std::clamp(offset, 0.0f, std::max<float>(0.0f, m_contentLength - m_viewportLength));
			if (m_offset == offset) return;
			m_offset = offset;
			UpdateThumbPosition();
			OnScroll(m_offset);
		}

		float Offset() const { return m_offset; }

		void SetContentLength(float length)
		{
			m_contentLength = std::max<float>(0.0f, length);
			ClampOffset();
			UpdateThumbSize();
			UpdateThumbPosition();
		}

		void SetViewportLength(float length)
		{
			m_viewportLength = std::max<float>(0.0f, length);
			ClampOffset();
			UpdateThumbSize();
			UpdateThumbPosition();
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawScrollBar(*this, context);
		}


	};

	class Content : public Widget
	{
	private:
	public:
		Content()
		{
			m_moveBehavior = MoveBehavior::Free;
			m_droppable = false;
			m_focusable = false;
		}

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawContent(*this, context);
		}
	};

	class Grip : public Widget
	{
	protected: 

	public:
		Grip(bool MoveHorizontal, bool MoveVertical)
		{
			m_moveBehavior = (MoveHorizontal && MoveVertical) ? MoveBehavior::Free :
				(MoveHorizontal && !MoveVertical) ? MoveBehavior::Horizontal :
				(!MoveHorizontal && MoveVertical) ? MoveBehavior::Vertical :
				MoveBehavior::None;

			m_droppable = false;
			m_focusable = false;
		}

		void Draw(const UIDrawContext& context) const override
		{
		//	if (context.skin) context.skin->DrawGrip(*this, context);
		}
	};

	// a frame the can be resized when dragging its edge/corner grips
	// it has a min size that clamps to it when resizing the frame via grips
	class ResizeableFrame : public Widget
	{
	private:
		// resize grip components
		Grip* m_leftResizeGrip = nullptr;
		Grip* m_rightResizeGrip = nullptr;
		Grip* m_topResizeGrip = nullptr;
		Grip* m_bottomResizeGrip = nullptr;
		Grip* m_topLeftResizeGrip = nullptr;
		Grip* m_topRightResizeGrip = nullptr;
		Grip* m_bottomLeftResizeGrip = nullptr;
		Grip* m_bottomRightResizeGrip = nullptr;

		Widget* m_content = nullptr;

		// resize grip thickness
		float m_borderSize;

		// min size when resizing through grips
		SizeF m_minResize;

		// resizing trackers
		PositionF m_beginPosition;
		SizeF m_beginSize;

	protected:
		void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
		{
			UpdateLayout();
		}

		void UpdateLayout()
		{
			// resize and reposition right grip control to occupy right edge of the frame with border size as thickness
			m_rightResizeGrip->SetPosition({ m_size.width - m_borderSize, m_borderSize });
			m_rightResizeGrip->SetSize({ m_borderSize, m_size.height - m_borderSize * 2 });

			// resize and reposition right grip control to occupy left edge of the frame with border size as thickness
			m_leftResizeGrip->SetPosition({ 0, m_borderSize });
			m_leftResizeGrip->SetSize({ m_borderSize, m_size.height - m_borderSize * 2 });

			// resize and reposition right grip control to occupy bottom edge of the frame with border size as thickness
			m_bottomResizeGrip->SetPosition({ m_borderSize, m_size.height - m_borderSize });
			m_bottomResizeGrip->SetSize({ m_size.width - m_borderSize * 2, m_borderSize });

			// resize and reposition right grip control to occupy top edge of the frame with border size as thickness
			m_topResizeGrip->SetPosition({ m_borderSize, 0 });
			m_topResizeGrip->SetSize({ m_size.width - m_borderSize * 2, m_borderSize });

			// resize and reposition right grip control to occupy top-left corner of the frame with border size as thickness
			m_topLeftResizeGrip->SetPosition({ 0, 0 });
			m_topLeftResizeGrip->SetSize({ m_borderSize, m_borderSize });

			// resize and reposition right grip control to occupy top-right corner of the frame with border size as thickness
			m_topRightResizeGrip->SetPosition({ m_size.width - m_borderSize, 0 });
			m_topRightResizeGrip->SetSize({ m_borderSize, m_borderSize });

			// resize and reposition right grip control to occupy bottom-left corner of the frame with border size as thickness
			m_bottomLeftResizeGrip->SetPosition({ 0, m_size.height - m_borderSize });
			m_bottomLeftResizeGrip->SetSize({ m_borderSize, m_borderSize });

			// resize and reposition right grip control to occupy bottom-right corner of the frame with border size as thickness
			m_bottomRightResizeGrip->SetPosition({ m_size.width - m_borderSize, m_size.height - m_borderSize });
			m_bottomRightResizeGrip->SetSize({ m_borderSize, m_borderSize });

			m_content->SetPosition({ m_borderSize, m_borderSize });
			m_content->SetSize({ GetSize().width - m_borderSize * 2, GetSize().height - m_borderSize * 2 });

			OnContentAreaChanged(m_content->GetSize());
		}

		SizeF ClampSize(const SizeF& size) const
		{
			return
			{
				std::max<float>(size.width,  m_minResize.width),
				std::max<float>(size.height, m_minResize.height)
			};
		}

	public:
		ResizeableFrame(float borderSize = 20.0f, const SizeF& minSize = {200, 200}) :
			m_borderSize(borderSize),
			m_minResize(minSize)
		{
			// left grip
			std::unique_ptr<Grip> widget = std::make_unique<Grip>(true, false);
			m_leftResizeGrip = widget.get();
			AddChild(std::move(widget));

			// right grip
			widget = std::make_unique<Grip>(true, false);
			m_rightResizeGrip = widget.get();
			AddChild(std::move(widget));

			// top grip
			widget = std::make_unique<Grip>(false, true);
			m_topResizeGrip = widget.get();
			AddChild(std::move(widget));

			// bottom grip
			widget = std::make_unique<Grip>(false, true);
			m_bottomResizeGrip = widget.get();
			AddChild(std::move(widget));

			// top-left grip
			widget = std::make_unique<Grip>(true, true);
			m_topLeftResizeGrip = widget.get();
			AddChild(std::move(widget));

			// top-right grip
			widget = std::make_unique<Grip>(true, true);
			m_topRightResizeGrip = widget.get();
			AddChild(std::move(widget));

			// bottom-left grip
			widget = std::make_unique<Grip>(true, true);
			m_bottomLeftResizeGrip = widget.get();
			AddChild(std::move(widget));

			// bottom-right grip
			widget = std::make_unique<Grip>(true, true);
			m_bottomRightResizeGrip = widget.get();
			AddChild(std::move(widget));

			std::unique_ptr<Widget> client = std::make_unique<Content>();
			m_content = client.get();
			AddChild(std::move(client));

			// begin drag lambda is same for all grips, so we define one here and assign to all grips
			auto capture = [&](const Widget::DragEventArgs&)
				{
					m_beginPosition = GetPosition();
					m_beginSize = GetSize();
				};
			m_bottomRightResizeGrip->OnDragBegin += capture;
			m_topLeftResizeGrip->OnDragBegin += capture;
			m_bottomResizeGrip->OnDragBegin += capture;
			m_bottomLeftResizeGrip->OnDragBegin += capture;
			m_topRightResizeGrip->OnDragBegin += capture;
			m_topResizeGrip->OnDragBegin += capture;
			m_leftResizeGrip->OnDragBegin += capture;
			m_rightResizeGrip->OnDragBegin += capture;

			// we also track content drag in case content is draggable, we bubble up movement to the frame and make content stationary
			m_content->OnDragBegin += capture;
			m_content->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					VecF delta = args.Delta();

					SetPosition(m_beginPosition + delta);

					UpdateLayout();
				};

			//  bottom-right grip handlers
			m_bottomRightResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					VecF delta = args.Delta();

					SetSize(ClampSize(
						{
							m_beginSize.width + delta.x,
							m_beginSize.height + delta.y
						}));
				};

			//  top-left grip handlers
			m_topLeftResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					VecF delta = args.Delta();

					// when dragging left grip, if moving towards right, we are reducing the width of the frame. we might hit min size
					// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
					float maxDeltaX = m_beginSize.width - m_minResize.width;

					// the delta width will be clamped to max allowed width
					delta.x = std::min<float>(delta.x, maxDeltaX);

					// when dragging top grip, if moving downwards, we are reducing the height of the frame. we might hit min size
					// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
					float maxDeltaY = m_beginSize.height - m_minResize.height;

					// the delta width will be clamped to max allowed width
					delta.y = std::min<float>(delta.y, maxDeltaY);

					SetPosition(m_beginPosition + delta);

					SetSize(
						{
							m_beginSize.width - delta.x,
							m_beginSize.height - delta.y,
						});
				};

			//  bottom-left grip handlers
			m_bottomLeftResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					VecF delta = args.Delta();

					// when dragging left grip, if moving towards right, we are reducing the width of the frame. we might hit min size
					// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
					float maxDeltaX = m_beginSize.width - m_minResize.width;

					// the delta width will be clamped to max allowed width
					delta.x = std::min<float>(delta.x, maxDeltaX);

					SetPosition(
						{
							m_beginPosition.x + delta.x,
							m_beginPosition.y
						});

					SetSize(ClampSize(
						{
							m_beginSize.width - delta.x,
							m_beginSize.height + delta.y,
						}));
				};

			//  top-right grip handlers
			m_topRightResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					SetPosition(
						{
							m_beginPosition.x,
							m_beginPosition.y + args.Delta().y
						});

					SetSize(
						{
							m_beginSize.width + args.Delta().x,
							m_beginSize.height - args.Delta().y,
						});
				};

			//  top grip handlers
			m_topResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					VecF delta = args.Delta();

					// when dragging top grip, if moving downwards, we are reducing the height of the frame. we might hit min size
					// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
					float maxDeltaY = m_beginSize.height - m_minResize.height;

					// the delta width will be clamped to max allowed width
					delta.y = std::min<float>(delta.y, maxDeltaY);

					SetPosition(
						{
							m_beginPosition.x,
							m_beginPosition.y + delta.y
						});

					SetSize(
						{
							m_beginSize.width,
							m_beginSize.height - delta.y,
						});
				};

			//  left grip handlers
			m_leftResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					VecF delta = args.Delta();

					// when dragging left grip, if moving towards right, we are reducing the width of the frame. we might hit min size
					// so the larger the delta, the likely we hit min size. so we calculate max delta allowed before hitting min size
					float maxDeltaX = m_beginSize.width - m_minResize.width;

					// the delta width will be clamped to max allowed width
					delta.x = std::min<float>(delta.x, maxDeltaX);

					SetPosition(
						{
							m_beginPosition.x + delta.x,
							m_beginPosition.y
						});

					SetSize(
						{
							m_beginSize.width - delta.x,
							m_beginSize.height,
						});
				};

			//  right grip handlers
			m_rightResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					SetSize(ClampSize(
						{
							m_beginSize.width + args.Delta().x,
							m_beginSize.height,
						}));
				};

			//  bottom grip handlers
			m_bottomResizeGrip->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					SetSize(ClampSize(
						{
							m_beginSize.width,
							m_beginSize.height + args.Delta().y,
						}));
				};
		}

		void SetMinResize(const SizeF& size)
		{
			m_minResize = size;
		}

		void SetBorderSize(float size)
		{
			m_borderSize = size;
		}

		engine::event::Event<const SizeF&> OnContentAreaChanged;	

		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawResizeableFrame(*this, context);
		}

		void AddContent(std::unique_ptr<Widget> widget)
		{
			m_content->AddChild(std::move(widget));
		}
	};

	class ViewPort: public Widget
	{
	private:
		Widget* m_content;

	protected:

		void UpdateLayout()
		{
			// if content's position is > 0,0 then move it back to 0, 0
			PositionF position = m_content->GetPosition();
			SizeF size = m_content->GetSize();

			bool updatePos = false;

			if (position.x + size.width < GetSize().width)
			{
				position.x = GetSize().width - size.width;
				updatePos = true;
			}
			if (position.y + size.height < GetSize().height)
			{
				position.y = GetSize().height - size.height;
				updatePos = true;
			}

			if (position.x > 0.0f)
			{
				position.x = 0.0f;
				updatePos = true;
			}
			if (position.y > 0.0f)
			{
				position.y = 0.0f;
				updatePos = true;
			}
			if (updatePos)
			{
				m_content->SetPosition(position);
			}
		}

		void OnSizeChanged(const SizeF& oldSize, const SizeF& newSize) override
		{
			UpdateLayout();
		}

	public:
		ViewPort()
		{		
			m_moveBehavior = MoveBehavior::None;
			m_droppable = false;
			m_focusable = false;

			std::unique_ptr<Widget> content = std::make_unique<Content>();
			m_content = content.get();

			AddChild(std::move(content));

			m_content->OnDragMove += [&](const Widget::DragEventArgs& args)
				{
					UpdateLayout();
				};

			m_content->OnResize += [&](const SizeF& size)
				{
					UpdateLayout();
				};
		}

		void SetContentSize(const SizeF& size)
		{
			m_content->SetSize(size);
		}
			
		void Draw(const UIDrawContext& context) const override
		{
			if (context.skin) context.skin->DrawViewPort(*this, context);
		}
	};

#pragma region // UI theme/skin

	class DefaultUISkin : public UISkin
	{
	public:
		void DrawButton(const Button& button, const UIDrawContext& context) const override
		{
			PositionF pos = button.GetAbsolutePosition();
			SizeF size = button.GetSize();

			if (&button == context.capture)
			{
				context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
			}
			//else if (&button == context.focus)
			//{
			//	context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
			//	context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
			//	context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0,0,0,1 }, 0);
			//	context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
			//}
			else if (&button == context.hover)
			{
				context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.55f,0.55f,0.55f,1 }, 0);
			}
			else
			{
				context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
			}
		}

		void DrawDraggable(const Draggable& button, const UIDrawContext& context) const override
		{
			PositionF pos = button.GetAbsolutePosition();
			SizeF size = button.GetSize();

			if (&button == context.capture)
			{
				context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
			}
			else if (&button == context.hover)
			{
				context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.55f,0.55f,0.55f,1 }, 0);
			}
			else
			{
				context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 0, 0 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
			}
		}

		void DrawLayer(const class Layer& overlay, const UIDrawContext& context) const override
		{
			PositionF pos = overlay.GetAbsolutePosition();
			SizeF size = overlay.GetSize();

			context.renderer.Draw(pos + PositionF{ 2, 2 }, size, { 0,0,0,1 }, 0);

			context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

			ColorF color = (&overlay == context.focus)? ColorF{0.6f, 0.6f, 0.6f, 1} : ColorF{0.5f, 0.5f, 0.5f, 1};
			context.renderer.Draw(pos + PositionF{1, 1}, size - SizeF{2,2}, color, 0);

			if (overlay.IsMenu())
			{
				PositionF ownerPos = overlay.GetOwner()->GetAbsolutePosition();
				SizeF ownerSize = overlay.GetOwner()->GetSize();

				context.renderer.Draw(pos + PositionF{ 1, 0 }, SizeF{ ownerSize.width - 2, 1 }, color, 0);
			}
			else if (overlay.GetType() == Layer::SubMenu)
			{
				SizeF ownerSize = overlay.GetOwner()->GetSize();

				context.renderer.Draw(pos + PositionF{ 0, 1 }, SizeF{ 1, ownerSize.height - 2 }, color, 0);
			}
		}

		void DrawTooltip(const class Tooltip& tooltip, const UIDrawContext& context) const override
		{
			PositionF pos = tooltip.GetAbsolutePosition();
			SizeF size = tooltip.GetSize();

			context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
			context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 1,1,1,1 }, 0);
		}

		void DrawLabel(const class Label& label, const UIDrawContext& context) const override
		{
			IFontAtlas* font = context.system.GetFont(label.GetFontType());
			if (!font)
			{
				throw std::runtime_error("font does not exist");
			}

			PositionF pos = label.GetTextAbsolutePosition();
			if (label.GetParent() && label.GetParent() == context.capture) pos += PositionF{ 1, 1 };

			ColorF color = { 0.3f,0.3f,0.3f,1 };
			if(label.GetParent() && label.GetParent() == context.focus) color = {0, 0, 0, 1};

			context.renderer.Draw(*font, label.Get(), pos, color);
		}

		void DrawImage(const class Image& image, const UIDrawContext& ctx) const override
		{
			Sprite sprite = image.Get();

			PositionF pivot = sprite.GetPivot();

			PositionF pivotInPixels = image.IsStretched() ? PositionF{ pivot.x * image.GetWidth(), pivot.y * image.GetHeight() } : sprite.GetPivotInPixels();

			PositionF pos = (image.IsStretched() ? image.GetAbsolutePosition() : image.GetImageAbsolutePosition()) + pivotInPixels;

			SizeF size = image.IsStretched() ? image.GetSize() : sprite.GetSize();

			ctx.renderer.Draw(image.GetImageAbsolutePosition(), image.IsStretched() ? image.GetSize() :sprite.GetSize(), { 0,1,0,0.5f }, 0);
			ctx.renderer.Draw(image.GetAbsolutePosition(), image.GetSize(), { 0,0,0,0.5f }, 0);

			ctx.renderer.Draw(sprite, pos, size, {1,1,1,1}, 0);
		}

		void DrawFrame(const class Frame& frame, const UIDrawContext& ctx) const override
		{
			PositionF pos = frame.GetAbsolutePosition();
			SizeF size = frame.GetSize();

			ctx.renderer.Draw(pos + PositionF{ 2, 2 }, size, { 0,0,0,1 }, 0);

			ctx.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

			ColorF color = (&frame == ctx.focus) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };
			ctx.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
		}

		void DrawMenuButton(const MenuButton& menuButton, const UIDrawContext& context) const override
		{
			PositionF pos = menuButton.GetAbsolutePosition();
			SizeF size = menuButton.GetSize();
			bool isExpanded = context.system.IsLayerExpanded(&menuButton);

			if (&menuButton == context.capture)
			{
				ColorF color = isExpanded ? ColorF{ 0.5f, 0.5f, 0.5f, 1 } : ColorF{ 0.6f, 0.6f, 0.6f, 1 };
				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

				if (isExpanded)
				{
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,1 }, color, 0);
				}
				else
				{
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
				}
			}
			else if (&menuButton == context.hover)
			{
				ColorF color = isExpanded ? ColorF{ 0.5f, 0.5f, 0.5f, 1 } : ColorF{ 0.6f, 0.6f, 0.6f, 1 };
				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

				if (isExpanded)
				{
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,1 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
				else
				{
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
				}
			}
			else
			{
				if (isExpanded)
				{
					context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,1 }, { 0.5f,0.5f,0.5f,1 }, 0);
				}
			}
		}

		void DrawSubMenuButton(const SubMenuButton& subMenuButton, const UIDrawContext& context) const override
		{
			PositionF pos = subMenuButton.GetAbsolutePosition();
			SizeF size = subMenuButton.GetSize();
			bool isExpanded = context.system.IsLayerExpanded(&subMenuButton);
			ColorF color = isExpanded ? ColorF{ 0.5f, 0.5f, 0.5f, 1 } : ColorF{ 0.6f, 0.6f, 0.6f, 1 };


			if (&subMenuButton == context.capture || &subMenuButton == context.hover)
			{
				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - (isExpanded? SizeF{ 1,2 }: SizeF{ 2,2 }), color, 0);
			}
			else
			{
				if (isExpanded)
				{
					context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
					context.renderer.Draw(pos + PositionF{ 1, 1 }, size - (isExpanded ? SizeF{ 1,2 } : SizeF{ 2,2 }), color, 0);
				}
			}
		}

		void DrawMenuItem(const MenuItem& menuItem, const UIDrawContext& context) const override
		{
			PositionF pos = menuItem.GetAbsolutePosition();
			SizeF size = menuItem.GetSize();

			if (&menuItem == context.capture)
			{
				context.renderer.Draw(pos + PositionF{ 4, 4 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 4,4 }, { 0.6f,0.6f,0.6f,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 4,4 }, { 0.5f,0.5f,0.5f,1 }, 0);
			}
			else if (&menuItem == context.hover)
			{
				context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
				context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.6f,0.6f,0.6f,1 }, 0);
			}
			else
			{
			}
		}

		void DrawSlider(const Slider& slider, const UIDrawContext& context) const override
		{
			PositionF pos = slider.GetAbsolutePosition();
			SizeF size = slider.GetSize();

			context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
			context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.5f,0.5f,0.5f,1 }, 0);
		}

		void DrawThumb(const Thumb& thumb, const UIDrawContext& context) const override
		{
			PositionF pos = thumb.GetAbsolutePosition();
			SizeF size = thumb.GetSize();

			ColorF color = (&thumb == context.hover)? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };

			context.renderer.Draw(pos + PositionF{ 2, 2 }, size - SizeF{ 4,4 }, { 0,0,0,1 }, 0);
			context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 6,6 }, { 0.5f,0.5f,0.5f,1 }, 0);


		}

		void DrawCheckBox(const CheckBox& checkbox, const UIDrawContext& context) const override
		{

		}

		void DrawRadioButton(const RadioButton& radiobutton, const UIDrawContext& context) const override
		{
			PositionF pos = radiobutton.GetAbsolutePosition();
			SizeF size = radiobutton.GetSize();

			context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
			context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.5f,0.5f,0.5f,1 }, 0);

			if (radiobutton.IsOn())
			{
				context.renderer.Draw(pos + PositionF{ 3, 3 }, size - SizeF{ 6, 6 }, { 0,0,0,1 }, 0);
			}			
		}

		void DrawScrollBar(const ScrollBar& scrollbar, const UIDrawContext& context) const override
		{
			PositionF pos = scrollbar.GetAbsolutePosition();
			SizeF size = scrollbar.GetSize();

			context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);
			context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, { 0.5f,0.5f,0.5f,1 }, 0);
		}

		void DrawGrip(const Grip& grip, const UIDrawContext& context) const override
		{
			PositionF pos = grip.GetAbsolutePosition();
			SizeF size = grip.GetSize();

			ColorF color = (&grip == context.hover) ? ColorF{ 0.5f,0,0,0.4f } : ColorF{ 0.5f,0,0,0.2f };
			context.renderer.Draw(pos, size, color, 0);

		}

		void DrawResizeableFrame(const ResizeableFrame& frame, const UIDrawContext& context) const override
		{
			PositionF pos = frame.GetAbsolutePosition();
			SizeF size = frame.GetSize();

			context.renderer.Draw(pos + PositionF{ 2, 2 }, size, { 0,0,0,1 }, 0);

			context.renderer.Draw(pos, size, { 0,0,0,1 }, 0);

			ColorF color = (&frame == context.focus) ? ColorF{ 0.6f, 0.6f, 0.6f, 1 } : ColorF{ 0.5f, 0.5f, 0.5f, 1 };
			context.renderer.Draw(pos + PositionF{ 1, 1 }, size - SizeF{ 2,2 }, color, 0);
		}

		void DrawViewPort(const ViewPort& vp, const UIDrawContext& context) const override
		{
			PositionF pos = vp.GetAbsolutePosition();
			SizeF size = vp.GetSize();

			context.renderer.Draw(pos, size, { 0,0,0,0.3f }, 0);
		}
		void DrawContent(const Content& content, const UIDrawContext& context) const override
		{
			PositionF pos = content.GetAbsolutePosition();
			SizeF size = content.GetSize();

			context.renderer.Draw(pos, size, { 0,0,0,0.3f }, 0);
		}

	};

#pragma endregion

#pragma region // gui scene
	class GuiScene : public Scene
	{
		PositionF m_mousePos;
		Widget* m_overlayTrigger = nullptr;
		Widget* m_multiOverlayTrigger = nullptr;
		Widget* m_dialog = nullptr;
		Widget* m_multiModalTrigger = nullptr;
		Image* m_image = nullptr;
		Widget* m_menu = nullptr;
		Widget* m_menuBar = nullptr;
		Slider* m_slider = nullptr;
		ViewPort* m_viewport = nullptr;
		int m_imageState = 0;
		UISystem m_ux;
		Button* m_button = nullptr;
		bool m_fullscreen = false;

		bool m_showProp = false;
		bool m_showTerrain = false;
		bool m_showFineGrid = false;
		bool m_showTileGrid = false;

		Widget* m_controlDialog = nullptr;
		Widget* m_testDialog = nullptr;

		std::unique_ptr<Widget> CreateWidget(const PositionF& pos, const SizeF& size)
		{
			std::unique_ptr<Widget> widget = std::make_unique<Widget>();
			widget->SetPosition(pos);
			widget->SetSize(size);
			return widget;
		}

	public:
		// this method creates a overlay build description where a OverlayTrigger's Layer contains another OverlayTrigger
		// this cascades multiple overlays. the depth determines how many tiers of overlays can exist. This is used to test 
		// overlay behavior of the UI system
		Layer::BuildDescription CreateBuildDescWithCascadedOverlays(int tier, const SizeF& size, const PositionF& position)
		{
			Layer::BuildDescription cmd;

			// this is the position of the overlay relative to its owner OverlayTrigger local space
			cmd.position = position;

			// size of the Layer. this will also be the size of the OverlayTrigger that will be child of the Layer
			cmd.size = size;

			// we will cascade up to given number of tier
			if (tier > 0)
			{
				// this will be the build function. it will build a overlay trigger which will be child to this trigger's overlay
				cmd.builder = [this, tier, size, position](Widget* parent)
					{
						std::unique_ptr<OverlayTrigger> overlayTrigger = std::make_unique<OverlayTrigger>(CreateBuildDescWithCascadedOverlays(tier - 1, size, position));

						// this trigger's position is at top left corner of its parent overlay
						overlayTrigger->SetPosition({0,0});

						// this trigger's size is same as its parent overlay size so it will fill up the whole overlay
						overlayTrigger->SetSize(size);

						parent->AddChild(std::move(overlayTrigger));			
					};
			}

			return cmd;
		}

		void SpawnMessageBox(UISystem& ux, const PositionF& position)
		{
			Layer::BuildDescription cmd
			{
				position,
				SizeF{200, 150},
				nullptr,
				Layer::Modal
			};

			cmd.builder = [&](Widget* parent)
				{
					auto button = std::make_unique<Button>();
					button->SetPosition({ 50, 80 });
					button->SetSize({ 100, 50 });

					button->OnClick += [&ux]()
						{
							ux.Collapse();
						};

					parent->AddChild(std::move(button));
				};

			ux.AddLayer(cmd);
		}

		std::unique_ptr<Label> CreateLabel(
			const PositionF& position, 
			const SizeF& size, 
			const std::string& text, 
			Widget::HorizontalAlignment hAlign = Widget::HorizontalAlignment::Center, 
			Widget::VerticalAlignment vAlign = Widget::VerticalAlignment::Center
		)
		{
			std::unique_ptr<Label> label = std::make_unique<Label>(text);
			label->SetPosition(position);
			label->SetSize(size);
			label->SetAlignment(vAlign, hAlign);
			return std::move(label);
		}

		std::unique_ptr<Button> CreateButton(const PositionF& position, const SizeF& size, const std::string& label)
		{
			std::unique_ptr<Button> button = std::make_unique<Button>();
			button->SetPosition(position);
			button->SetSize(size);
			button->AddChild(CreateLabel({ 0,0 }, size, label));

			return std::move(button);
		}

		std::unique_ptr<MenuButton> CreateMenu(const PositionF& position, const SizeF& size, const std::string& label)
		{
			Layer::BuildDescription cmd
			{
				PositionF{0, 50},
				SizeF({200, 200}),
				nullptr,
				Layer::Type::Popup,
				false
			};

			std::unique_ptr<MenuButton> menuButton = std::make_unique<MenuButton>(cmd);
			menuButton->SetPosition(position);
			menuButton->SetSize(size);
			menuButton->AddChild(CreateLabel({ 0,0 }, size, label));
		}

		void OnEnter() override
		{
			m_ux.SetPosition({ 0,0 });
			m_ux.SetSize({ 0,0 });
			m_ux.Show();
			IFontAtlas& font = AssetManager().Get<IFontAtlas>("font");
			m_ux.SetFont(&font, UIResources::FontType::Default);

			std::unique_ptr<ScrollBar> scrollbar = std::make_unique<ScrollBar>(500.0f, 350.0f);
			scrollbar->SetPosition({ 300,300 });
			scrollbar->SetSize({ 300, 50 });
			m_ux.AddWidget(std::move(scrollbar));

			std::unique_ptr<ResizeableFrame> resizeableframe = std::make_unique<ResizeableFrame>(20.0f);
			resizeableframe->SetPosition({ 300,300 });
			resizeableframe->SetSize({ 600, 600 });

			std::unique_ptr<ViewPort> viewport = std::make_unique<ViewPort>();
			viewport->SetPosition({ 0, 0 });
			viewport->SetSize({ 0, 0 });
			viewport->SetContentSize({ 500, 500 });
			m_viewport = viewport.get();
			resizeableframe->OnContentAreaChanged += [&](const SizeF& size)
				{
					m_viewport->SetPosition({ 0,0 });
					m_viewport->SetSize(size);
				};
			resizeableframe->AddContent(std::move(viewport));

			m_ux.AddWidget(std::move(resizeableframe));

			// let's create menu system
			{
				std::unique_ptr<Frame> menuBar = std::make_unique<Frame>(false, false);
				menuBar->SetPosition({ 0,0 });
				menuBar->SetSize({ 0, 50 });
				m_menuBar = menuBar.get();

				// add map menu
				{
					Layer::BuildDescription cmd
					{
						PositionF{0, 40},
						SizeF({200, 140}),
						nullptr,
						Layer::Type::Menu,
						false
					};

					cmd.builder = [&](Widget* parent)
						{
							// add "select map to load" submenu
							{
								Layer::BuildDescription cmd
								{
									PositionF{190, 0},
									SizeF({200, 200}),
									nullptr,
									Layer::Type::Menu,
									false
								};

								std::unique_ptr<SubMenuButton> menuButton = std::make_unique<SubMenuButton>(cmd);
								menuButton->SetPosition({ 5,5 });
								menuButton->SetSize({ 190,40 });
								menuButton->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Load...", Widget::HorizontalAlignment::Left));
								parent->AddChild(std::move(menuButton));
							}

							// add "save" menuitem
							{
								std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
								menuItem->SetPosition({ 5, 50 });
								menuItem->SetSize({ 190,40 });
								menuItem->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Save", Widget::HorizontalAlignment::Left));
								parent->AddChild(std::move(menuItem));
							}

							// add "clear" menuitem
							{
								std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
								menuItem->SetPosition({ 5, 95 });
								menuItem->SetSize({ 190,40 });
								menuItem->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Clear", Widget::HorizontalAlignment::Left));
								parent->AddChild(std::move(menuItem));
							}
						};

					std::unique_ptr<MenuButton> menuButton = std::make_unique<MenuButton>(cmd);
					menuButton->SetPosition({ 5,5 });
					menuButton->SetSize({ 100,40 });
					menuButton->AddChild(CreateLabel({ 0,0 }, { 100,40 }, "Map"));
					m_menuBar->AddChild(std::move(menuButton));
				}

				// add view menu
				{
					Layer::BuildDescription cmd
					{
						PositionF{0, 40},
						SizeF({220, 185}),
						nullptr,
						Layer::Type::Menu,
						false
					};

					cmd.builder = [&](Widget* parent)
						{
							// add "show grids" submenu
							{
								Layer::BuildDescription cmd
								{
									PositionF{190, 0},
									SizeF({200, 95}),
									nullptr,
									Layer::Type::Menu,
									false
								};

								cmd.builder = [&](Widget* parent)
									{
										// add show tile grid menuitem
										{
											std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
											menuItem->SetPosition({ 5, 5 });
											menuItem->SetSize({ 190,40 });
											menuItem->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Tile", Widget::HorizontalAlignment::Left));

											std::unique_ptr<RadioButton> radioButton = std::make_unique<RadioButton>();
											radioButton->SetPosition({ 10, 10 });
											radioButton->SetSize({ 20, 20 });
											radioButton->Bind(m_showTileGrid);
											RadioButton* rb = radioButton.get();

											menuItem->OnClick += [&, rb]()
												{
													rb->Toggle();
												};

											menuItem->AddChild(std::move(radioButton));
											parent->AddChild(std::move(menuItem));
										}

										// add show fine grid menuitem
										{
											std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
											menuItem->SetPosition({ 5, 50 });
											menuItem->SetSize({ 190,40 });
											menuItem->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Fine", Widget::HorizontalAlignment::Left));

											std::unique_ptr<RadioButton> radioButton = std::make_unique<RadioButton>();
											radioButton->SetPosition({ 10, 10 });
											radioButton->SetSize({ 20, 20 });
											radioButton->Bind(m_showFineGrid);
											RadioButton* rb = radioButton.get();

											menuItem->OnClick += [&, rb]()
												{
													rb->Toggle();
												};

											menuItem->AddChild(std::move(radioButton));
											parent->AddChild(std::move(menuItem));
										}
									};
		

								std::unique_ptr<SubMenuButton> menuButton = std::make_unique<SubMenuButton>(cmd);
								menuButton->SetPosition({ 5,5 });
								menuButton->SetSize({ 190,40 });
								menuButton->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Grids...", Widget::HorizontalAlignment::Left));
								parent->AddChild(std::move(menuButton));
							}

							// add "show props" menuitem
							{
								std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
								menuItem->SetPosition({ 5, 50 });
								menuItem->SetSize({ 190,40 });
								menuItem->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Props", Widget::HorizontalAlignment::Left));

								std::unique_ptr<RadioButton> radioButton = std::make_unique<RadioButton>();
								radioButton->SetPosition({ 10, 10 });
								radioButton->SetSize({ 20, 20 });
								radioButton->Bind(m_showProp);
								RadioButton* rb = radioButton.get();

								menuItem->OnClick += [&, rb]()
									{
										rb->Toggle();
									};

								menuItem->AddChild(std::move(radioButton));
								parent->AddChild(std::move(menuItem));
							}

							// add "show terrain" menuitem
							{
								std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
								menuItem->SetPosition({ 5, 95 });
								menuItem->SetSize({ 190,40 });
								menuItem->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Terrain", Widget::HorizontalAlignment::Left));

								std::unique_ptr<RadioButton> radioButton = std::make_unique<RadioButton>();
								radioButton->SetPosition({ 10, 10 });
								radioButton->SetSize({ 20, 20 });
								radioButton->Bind(m_showTerrain);
								RadioButton* rb = radioButton.get();

								menuItem->OnClick += [&, rb]()
									{
										rb->Toggle();
									};

								menuItem->AddChild(std::move(radioButton));
								parent->AddChild(std::move(menuItem));
							}

							// add "overlays" submenu
							{
								Layer::BuildDescription cmd
								{
									PositionF{190, 0},
									SizeF({200, 200}),
									nullptr,
									Layer::Type::Menu,
									false
								};

								std::unique_ptr<SubMenuButton> menuButton = std::make_unique<SubMenuButton>(cmd);
								menuButton->SetPosition({ 5, 140 });
								menuButton->SetSize({ 190,40 });
								menuButton->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Overlays...", Widget::HorizontalAlignment::Left));
								parent->AddChild(std::move(menuButton));
							}
						};

					std::unique_ptr<MenuButton> menuButton = std::make_unique<MenuButton>(cmd);
					menuButton->SetPosition({ 110, 5 });
					menuButton->SetSize({ 100,40 });
					menuButton->AddChild(CreateLabel({ 0,0 }, { 100,40 }, "View"));
					m_menuBar->AddChild(std::move(menuButton));
				}

				// add settings view
				{
					Layer::BuildDescription cmd
					{
						PositionF{0, 40},
						SizeF({220, 185}),
						nullptr,
						Layer::Type::Menu,
						false
					};

					cmd.builder = [&](Widget* parent)
						{
							// add "Display" submenu
							{
								Layer::BuildDescription cmd
								{
									PositionF{190, 0},
									SizeF({250, 140}),
									nullptr,
									Layer::Type::Menu,
									false
								};

								cmd.builder = [&](Widget* parent)
									{
										AssetManager assets;
										ICanvas& canvas = assets.Get<ICanvas>("canvas");
										IWindow& window = assets.Get<IWindow>("window");

										// add show full screen menuitem
										{
											std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
											menuItem->SetPosition({ 5, 5 });
											menuItem->SetSize({ 225,40 });
											menuItem->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Full Screen", Widget::HorizontalAlignment::Left));

											std::unique_ptr<RadioButton> radioButton = std::make_unique<RadioButton>();
											radioButton->SetPosition({ 10, 10 });
											radioButton->SetSize({ 20, 20 });
											RadioButton* rb = radioButton.get();
											radioButton->Bind([&]()
												{
													// both window and canvas is full screen
													return canvas.IsFullScreen() && window.IsFullScreen();
												},
												[&](const bool& state)
												{
													// set both window and canvas as full screen
													window.SetFullscreen(state);
													canvas.SetFullscreen(state);
												});

											menuItem->OnClick += [&, rb]()
												{
													rb->TurnOn();
												};

											menuItem->AddChild(std::move(radioButton));
											parent->AddChild(std::move(menuItem));
										}

										// add show borderless window menuitem
										{
											std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
											menuItem->SetPosition({ 5, 50 });
											menuItem->SetSize({ 225,40 });
											menuItem->AddChild(CreateLabel({ 50,0 }, { 180,40 }, "Borderless Window", Widget::HorizontalAlignment::Left));

											std::unique_ptr<RadioButton> radioButton = std::make_unique<RadioButton>();
											radioButton->SetPosition({ 10, 10 });
											radioButton->SetSize({ 20, 20 });
											RadioButton* rb = radioButton.get();
											radioButton->Bind([&]()
												{
													// window is full screen but canvas is not
													return !canvas.IsFullScreen() && window.IsFullScreen();
												},
												[&](const bool& state)
												{
													// set both window and canvas as full screen
													window.SetFullscreen(state);
													canvas.SetFullscreen(!state);
												});

											menuItem->OnClick += [&, rb]()
												{
													rb->TurnOn();
												};

											menuItem->AddChild(std::move(radioButton));
											parent->AddChild(std::move(menuItem));
										}


										// add show fine grid menuitem
										{
											std::unique_ptr<MenuItem> menuItem = std::make_unique<MenuItem>();
											menuItem->SetPosition({ 5, 95 });
											menuItem->SetSize({ 225,40 });
											menuItem->AddChild(CreateLabel({ 50,0 }, { 180,40 }, "Windowed", Widget::HorizontalAlignment::Left));

											std::unique_ptr<RadioButton> radioButton = std::make_unique<RadioButton>();
											radioButton->SetPosition({ 10, 10 });
											radioButton->SetSize({ 20, 20 });
											RadioButton* rb = radioButton.get();
											radioButton->Bind([&]()
												{
													// both window and canvas not full screen
													return !canvas.IsFullScreen() && !window.IsFullScreen();
												},
												[&](const bool& state)
												{
													// set both window and canvas as full screen
													window.SetFullscreen(!state);
													canvas.SetFullscreen(!state);
												});

											menuItem->OnClick += [&, rb]()
												{
													rb->TurnOn();
												};

											menuItem->AddChild(std::move(radioButton));
											parent->AddChild(std::move(menuItem));
										}
									};


								std::unique_ptr<SubMenuButton> menuButton = std::make_unique<SubMenuButton>(cmd);
								menuButton->SetPosition({ 5,5 });
								menuButton->SetSize({ 190,40 });
								menuButton->AddChild(CreateLabel({ 50,0 }, { 130,40 }, "Display...", Widget::HorizontalAlignment::Left));
								parent->AddChild(std::move(menuButton));
							}
						};

					std::unique_ptr<MenuButton> menuButton = std::make_unique<MenuButton>(cmd);
					menuButton->SetPosition({ 220, 5 });
					menuButton->SetSize({ 100,40 });
					menuButton->AddChild(CreateLabel({ 0,0 }, { 100,40 }, "Settings"));
					m_menuBar->AddChild(std::move(menuButton));
				}

				m_ux.AddWidget(std::move(menuBar));
			}
		}

		void OnMouseMove(int x, int y) override
		{
			m_mousePos = PositionF((float)x, (float)y);


			m_ux.MouseMove(m_mousePos);
		}

		void OnMouseDown(int btn, int x, int y)
		{
			m_mousePos = PositionF((float)x, (float)y);

			// this button is for panning the camera
			if (btn == 1)
			{
				m_ux.MouseDown(m_mousePos);
			}
			// if this button is clicked, move our focus in this position
			if (btn == 2)
			{
				Layer::BuildDescription cmd
				{
					m_mousePos,
					SizeF({200, 400}),
					nullptr,
					Layer::Popup
				};

				m_ux.Collapse();
				m_ux.AddLayer(cmd);
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
			m_mousePos = PositionF((float)x, (float)y);

			m_ux.MouseUp(m_mousePos);
		}

		void OnKeyDown(int key) override
		{
			m_ux.KeyDown(key);

			switch (key)
			{
			case 9: // TAB
				break;
			case 27: // ESC
				m_ux.Collapse();
				break;
			case 32: // SPACE
			{
				AssetManager assets;
				ICanvas& canvas = assets.Get<ICanvas>("canvas");
				IWindow& window = assets.Get<IWindow>("window");

				bool _fullscreen = canvas.IsFullScreen();
				bool windowFullScreen = window.IsFullScreen();

				bool fullscreen = canvas.IsFullScreen() && window.IsFullScreen();

				window.SetFullscreen(!fullscreen);
				canvas.SetFullscreen(!fullscreen);

				break;
			}
			case 49: // 1
				if (!m_button)
				{
					// demo using button widget. create this button. when clicked, it spawns a modal "message box"
					// the modal "message box" is just an overlay with a button in it. when the button in the modal is clicked, it collapses the modal again
					std::unique_ptr<Button> button = std::make_unique<Button>();
					button->SetPosition({ 300, 100 });
					button->SetSize({ 200, 50 });
					m_button = button.get();

					// set default tooltip for debug purposes. can formalize this later
					button->SetTooltip([&](Widget& owner, Widget& tooltip)
						{
							tooltip.SetSize({ 80,30 });
							tooltip.SetPosition(owner.GetAbsolutePosition() + PositionF{ owner.GetSize().width + 5, 0 });
						});

					std::unique_ptr<Label> label = std::make_unique<Label>("Message Box");
					label->SetPosition({ 0, 0 });
					label->SetSize({ 200, 50 });
					button->AddChild(std::move(label));

					button->OnClick += [&]() 
						{
							Layer::BuildDescription cmd
							{
								PositionF{500, 250},
								SizeF({320, 320}),
								nullptr,
								Layer::Modal,
								true
							};

							cmd.builder = [&](Widget* parent)
								{
									auto& animSet = AssetManager().Get<AnimationSet<Sprite>>("birchtree_anim_set");
									std::unique_ptr<Animated> anim = std::make_unique<Animated>(animSet, "birch_tree_idle");
									std::unique_ptr<Image> image = std::make_unique<Image>(std::move(anim));
									image->SetPosition({ 100, 80 });
									image->SetSize({ 120, 120 });
									m_image = image.get();
									parent->AddChild(std::move(image));

									std::unique_ptr<Button> button = std::make_unique<Button>();
									button->SetPosition({ 200, 270 });
									button->SetSize({ 100, 32 });

									button->SetTooltip([&](Widget& owner, Widget& tooltip)
										{
											tooltip.SetSize({ 80,30 });
											tooltip.SetPosition(owner.GetAbsolutePosition() + PositionF{ owner.GetSize().width + 5, 0 });
										});

									std::unique_ptr<Label> label = std::make_unique<Label>("Close");
									label->SetPosition({ 0, 0 });
									label->SetSize({ 100, 32 });
									button->AddChild(std::move(label));

									button->OnClick += [&]()
										{
											m_ux.Collapse();
										};

									parent->AddChild(std::move(button));

									button = std::make_unique<Button>();
									button->SetPosition({ 20, 270 });
									button->SetSize({ 100, 32 });

									button->SetTooltip([&](Widget& owner, Widget& tooltip)
										{
											tooltip.SetSize({ 80,30 });
											tooltip.SetPosition(owner.GetAbsolutePosition() + PositionF{ owner.GetSize().width + 5, 0 });
										});

									label = std::make_unique<Label>("Toggle");
									label->SetPosition({ 0, 0 });
									label->SetSize({ 100, 32 });
									button->AddChild(std::move(label));

									button->OnClick += [&]()
										{
											switch (m_imageState)
											{
											case 0:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Top, Widget::HorizontalAlignment::Left);
												m_imageState = 1;
												break;
											case 1:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Top, Widget::HorizontalAlignment::Center);
												m_imageState = 2;
												break;
											case 2:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Top, Widget::HorizontalAlignment::Right);
												m_imageState = 3;
												break;
											case 3:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Center, Widget::HorizontalAlignment::Left);
												m_imageState = 4;
												break;
											case 4:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Center, Widget::HorizontalAlignment::Center);
												m_imageState = 5;
												break;
											case 5:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Center, Widget::HorizontalAlignment::Right);
												m_imageState = 6;
												break;
											case 6:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Bottom, Widget::HorizontalAlignment::Left);
												m_imageState = 7;
												break;
											case 7:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Bottom, Widget::HorizontalAlignment::Center);
												m_imageState = 8;
												break;
											case 8:
												m_image->EnableStretch(false);
												m_image->SetAlignment(Widget::VerticalAlignment::Bottom, Widget::HorizontalAlignment::Right);
												m_imageState = 9;
												break;
											case 9:
												m_image->EnableStretch(true);
												m_imageState = 0;
												break;
											}
										};

									parent->AddChild(std::move(button));
								};

							m_ux.AddLayer(cmd);
						};

					m_ux.AddWidget(std::move(button));
				}
				else
				{
					m_ux.RemoveWidget(m_button);
					m_button = nullptr;
				}
				break;
			case 50: // 2
				if (!m_controlDialog)
				{
					std::unique_ptr<Frame> dialog = std::make_unique<Frame>();
					dialog->SetPosition({ 700, 100 });
					dialog->SetSize({ 320, 480 });
					m_controlDialog = dialog.get();

					{
						std::unique_ptr<Slider> slider = std::make_unique<Slider>(0.0f, 100.0f, 50.0f);
						slider->SetPosition({ 10, 10 });
						slider->SetSize({ 200, 32 });
						slider->Min(-1.0f);
						slider->Max(200.0f);
						slider->Value(100.0f);
						slider->SetStepCount(1000);

						slider->OnChange += [&](float value)
							{
								m_slider->SetStepCount((int)value);
							};

						m_controlDialog->AddChild(std::move(slider));
					}

					{
						std::unique_ptr<Slider> slider = std::make_unique<Slider>(0.0f, 100.0f, 50.0f);
						slider->SetPosition({ 10, 45 });
						slider->SetSize({ 200, 32 });
						slider->Min(-100.0f);
						slider->Max(100.0f);
						slider->Value(0.0f);
						slider->SetStepCount(1000);

						slider->OnChange += [&](float value)
							{

							};

						m_controlDialog->AddChild(std::move(slider));
					}

					m_ux.AddWidget(std::move(dialog));
				}
				else
				{
					m_ux.RemoveWidget(m_controlDialog);
					m_controlDialog = nullptr;
				}

				if (!m_testDialog)
				{
					std::unique_ptr<Frame> dialog = std::make_unique<Frame>();
					dialog->SetPosition({ 50, 100 });
					dialog->SetSize({ 640, 480 });
					m_testDialog = dialog.get();

					std::unique_ptr<Slider> slider = std::make_unique<Slider>(0.0f, 100.0f, 50.0f);
					slider->SetPosition({ 25, 25 });
					slider->SetSize({ 300, 50 });
					slider->Min(-1.0f);
					slider->Max(1.0f);
					slider->Value(0.0f);
					slider->SetStepCount(3);
					m_slider = slider.get();
					m_testDialog->AddChild(std::move(slider));

					m_ux.AddWidget(std::move(dialog));
				}
				else
				{
					m_ux.RemoveWidget(m_testDialog);
					m_testDialog = nullptr;
				}
				break;
			case 51: // 3 
				m_slider->Horizontal(!m_slider->Horizontal());

				//m_slider->SetThumbLength(1000.0f);


				break;
			case 52: // 4
				if (!m_multiModalTrigger)
				{
					Layer::BuildDescription cmd
					{
						PositionF{0, 50},
						SizeF({200, 200}),
						nullptr,
						Layer::Popup
					};

					cmd.builder = [](Widget* parent)
						{
							Layer::BuildDescription cmd2
							{
								PositionF{200, 10},
								SizeF({200, 200}),
								nullptr,
								Layer::Modal
							};

							cmd2.builder = [](Widget* parent)
								{
									Layer::BuildDescription cmd3
									{
										PositionF{200, 10},
										SizeF({200, 200}),
										nullptr,
										Layer::Popup
									};

									std::unique_ptr<OverlayTrigger> widget3 = std::make_unique<OverlayTrigger>(cmd3);
									widget3->SetPosition({ 10, 10 });
									widget3->SetSize({ 180, 50 });

									parent->AddChild(std::move(widget3));
								};

							std::unique_ptr<OverlayTrigger> widget2 = std::make_unique<OverlayTrigger>(cmd2);
							widget2->SetPosition({ 10, 10 });
							widget2->SetSize({ 180, 50 });

							parent->AddChild(std::move(widget2));
						};

					std::unique_ptr<OverlayTrigger> widget = std::make_unique<OverlayTrigger>(cmd);
					widget->SetPosition({ 650, 100 });
					widget->SetSize({ 100, 50 });

					m_ux.AddWidget(std::move(widget));
				}
				else
				{
					m_ux.RemoveWidget(m_multiModalTrigger);
					m_multiModalTrigger = nullptr;
				}
				break;
			case 53: // 5
			{
				Layer::BuildDescription cmd
				{
					PositionF{300, 300},
					SizeF({300, 300}),
					nullptr,
					Layer::Modal
				};

				m_ux.Collapse();
				m_ux.AddLayer(cmd);

				break;
			}
			case 54: // 6
			{
				std::unique_ptr<Frame> frame = std::make_unique<Frame>();
				frame->SetPosition({ 100, 100 });
				frame->SetSize({ 300, 300 });

				//std::unique_ptr<Button> button = std::make_unique<Button>();
				//button->SetPosition({ 50, 50 });
				//button->SetSize({ 120, 120 });
				//frame->AddChild(std::move(button));

				std::unique_ptr<Draggable> draggable = std::make_unique<Draggable>();
				draggable->SetPosition({ 100, 100 });
				draggable->SetSize({ 128, 128 });

				{
					auto& animSet = AssetManager().Get<AnimationSet<Sprite>>("birchtree_anim_set");
					std::unique_ptr<Animated> anim = std::make_unique<Animated>(animSet, "birch_tree_idle");
					std::unique_ptr<Image> image = std::make_unique<Image>(std::move(anim));
					image->SetPosition({ 8, 8 });
					image->SetSize({ 112, 112 });
					image->EnableStretch(true);
					m_image = image.get();
					draggable->AddChild(std::move(image));
				}

				frame->AddChild(std::move(draggable));

				m_ux.AddWidget(std::move(frame));

				frame = std::make_unique<Frame>();
				frame->SetPosition({ 300, 250 });
				frame->SetSize({ 300, 300 });

				draggable = std::make_unique<Draggable>();
				draggable->SetPosition({ 50, 50 });
				draggable->SetSize({ 128, 128 });

				{
					auto& animSet = AssetManager().Get<AnimationSet<Sprite>>("pinetree_anim_set");
					std::unique_ptr<Animated> anim = std::make_unique<Animated>(animSet, "pine_tree_idle");
					std::unique_ptr<Image> image = std::make_unique<Image>(std::move(anim));
					image->SetPosition({ 8, 8 });
					image->SetSize({ 112, 112 });
					image->EnableStretch(true);
					m_image = image.get();
					draggable->AddChild(std::move(image));
				}

				frame->AddChild(std::move(draggable));

				m_ux.AddWidget(std::move(frame));

				break;
			}

			default:
				break;
			}
		}

		void OnUpdate(double dt) override
		{
			m_ux.Begin();

			// update input to trigger input events
			Input::Instance().Update();

			m_ux.End();
		}

		void OnRender() override
		{
			AssetManager assets;
			ICanvas& canvas = assets.Get<ICanvas>("canvas");
			IRenderer& renderer = assets.Get<IRenderer>("renderer");
			renderer.EnableClipping(false);
			renderer.SetClipRegion(canvas.GetViewPort());

			DefaultUISkin skin;
			UIDrawContext context{ renderer, m_ux, &skin };
			m_ux.Draw(context);

			IFontAtlas& font = AssetManager().Get<IFontAtlas>("font");
			if(m_slider) renderer.Draw(font, std::to_string(m_slider->Value()), { 350, 100 }, { 1,1,1,1 });		}

		void OnResize(size_t width, size_t height) override
		{
			m_ux.SetSize({ static_cast<float>(width), static_cast<float>(height) });
			m_menuBar->SetSize({ static_cast<float>(width), m_menuBar->GetSize().height });
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
			window.Create(L"Test Map Editor", 1400, 1080);

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

				// create sprite atlas for the water splash animation
				SpriteAtlasFactory::Create("splash_anim_sprites", L"../Assets/3072x192px_1x17tile_waterfoam.png", 1, 16);

				// create animation set where we will store animated tiles like splash animation
				Registry<AnimationSet<Sprite>>::Instance().Register("splash_anim_set", std::make_unique<AnimationSet<Sprite>>());

				// create animation for splash and store in animation set
				ISpriteAtlas& splashAnimSprites = assets.Get<ISpriteAtlas>("splash_anim_sprites");
				AnimationSet<Sprite>& splashAnimSet = assets.Get<AnimationSet<Sprite>>("splash_anim_set");
				splashAnimSet.Register("splash_anim", SpriteAnimationFactory::Create(splashAnimSprites, 100.0f, true, { .33f, .355f }));


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
				waterRocksAnimSet.Register("water_rocks_idle", SpriteAnimationFactory::Create(waterRockAtlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, 100.0f, true, PositionF{ 0.5f, 0.8f }));




				// create our tileset and load all sprites from sprite atlas
				{
					Dictionary<int, TileConstraint> grassTilesConstraint;
					grassTilesConstraint.Register(0, TileConstraint::NONE);
					grassTilesConstraint.Register(1, TileConstraint::NONE);
					grassTilesConstraint.Register(2, TileConstraint::NONE);
					grassTilesConstraint.Register(3, TileConstraint::NONE);
					grassTilesConstraint.Register(5, TileConstraint::NONE);
					grassTilesConstraint.Register(6, TileConstraint::NONE);
					grassTilesConstraint.Register(7, TileConstraint::NONE);
					grassTilesConstraint.Register(8, TileConstraint::NONE);
					grassTilesConstraint.Register(9, TileConstraint::NONE);
					grassTilesConstraint.Register(10, TileConstraint::NONE);
					grassTilesConstraint.Register(11, TileConstraint::NONE);
					grassTilesConstraint.Register(12, TileConstraint::NONE);
					grassTilesConstraint.Register(14, TileConstraint::NONE);
					grassTilesConstraint.Register(15, TileConstraint::NONE);
					grassTilesConstraint.Register(16, TileConstraint::NONE);
					grassTilesConstraint.Register(17, TileConstraint::NONE);
					grassTilesConstraint.Register(18, TileConstraint::NONE);
					grassTilesConstraint.Register(19, TileConstraint::NONE);
					grassTilesConstraint.Register(20, TileConstraint::NONE);
					grassTilesConstraint.Register(21, TileConstraint::NONE);
					grassTilesConstraint.Register(23, TileConstraint::NONE);
					grassTilesConstraint.Register(24, TileConstraint::NONE);
					grassTilesConstraint.Register(25, TileConstraint::NONE);
					grassTilesConstraint.Register(26, TileConstraint::NONE);
					grassTilesConstraint.Register(27, TileConstraint::NONE);
					grassTilesConstraint.Register(28, TileConstraint::NONE);
					grassTilesConstraint.Register(29, TileConstraint::NONE);
					grassTilesConstraint.Register(30, TileConstraint::NONE);
					grassTilesConstraint.Register(32, TileConstraint::NONE);
					grassTilesConstraint.Register(33, TileConstraint::NONE);
					grassTilesConstraint.Register(34, TileConstraint::NONE);
					grassTilesConstraint.Register(35, TileConstraint::NONE);

					TilesetLoader::LoadTerrainSet("grass_tileset", "grass_tile_sprites", grassTilesConstraint, TileConstraint::BLOCKED);

					Registry<TerrainSet>::Instance().Register("splash_tileset", std::make_unique<TerrainSet>("splash_tileset"));
					TerrainSet& splashTileset = assets.Get<TerrainSet>("splash_tileset");
					std::unique_ptr<TileDefinition> tiledef = std::make_unique<TileDefinition>();
					tiledef->renderable = std::make_unique<Animated>(splashAnimSet, "splash_anim");
					tiledef->constraint = TileConstraint::NONE;
					splashTileset.Register(0, std::move(tiledef));
				}
			}

			// initialize scenes
			{
				m_sceneManager.CreateScene<DebugScene>("Debug");
				m_sceneManager.CreateScene<CameraScene>("Camera");
				m_sceneManager.CreateScene<TerrainEditScene>("Terrain");
				m_sceneManager.CreateScene<EditWorldCameraScene>("Edit");
				m_sceneManager.CreateScene<LoadSaveWorldScene>("Save");
				m_sceneManager.CreateScene<GuiScene>("ux");
				m_sceneManager.SetActive("ux");
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
			}
		}

		void OnKeyDown(int key)
		{
			return;
		}

		void OnMouseDown(int btn, int x, int y)
		{
			return;
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
			ICanvas& canvas = AssetManager().Get<ICanvas>("canvas");
			canvas.Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			canvas.SetViewPort();

			m_sceneManager.OnResize(nWidth, nHeight);
		}

	};
}