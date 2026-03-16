#pragma once
#include <Algorithm/AutoTileResolver.h>
#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/Canvas.h>
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/DX11RendererImmediateImpl.h>
#include <Graphics/Renderer/Renderer.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Core/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Resource/IFontAtlas.h>
#include <Graphics/Resource/FontAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Cache/Registry.h>
#include <Graphics/Animation/Animation.h>
#include <Timer/StopWatch.h>
#include <Algorithm/Pathfinding.h>
#include <Engine/Loader/AsyncLoader.h>
#include <Graphics/Core/Primitives.h>
#include <Engine/Graphics/Draw.h>
#include <Engine/Factory/AnimationFactory.h>
#include "Actor.h"
#include <Core/View.h>
#include <Containers/Dictionary.h>
#include <Algorithm/Resolvers.h>
#include <Components/Prop.h>
#include <Command/DrawCommand.h>



namespace engine
{
	namespace component
	{
		class DrawTileMapCommand : public engine::command::graphics::renderer::DrawCommandBase
		{
		public:
			struct DrawInfo
			{
				engine::graphics::Sprite sprite;						// what to draw
				engine::spatial::PositionF pos;		// world position
				engine::spatial::SizeF size;		// size on screen
				engine::graphics::ColorF tint;		// color modulation
				float depth;						// depth
				float rotation;						// rotation angle
			};

		private:
			engine::spatial::PositionF m_pos;
			spatial::SizeF m_tilesize;
			engine::graphics::ColorF m_color;
			std::vector<DrawInfo> m_batch;

		public:
			DrawTileMapCommand(
				engine::graphics::renderer::IRenderer& renderer,
				const engine::spatial::PositionF& pos,
				const spatial::SizeF& tilesize
			):
				DrawCommandBase(renderer),
				m_pos(pos),
				m_tilesize(tilesize)
			{

			}

			void Execute() override
			{
				Sort();

				for (auto& cmd : m_batch)
				{
					m_renderer.Draw(cmd.sprite, cmd.pos, cmd.size, cmd.tint, cmd.rotation);
				}
			}

			void Clear()
			{
				m_batch.clear();
			}

			void Reserve(size_t capacity)
			{
				m_batch.reserve(capacity);
			}

			void Queue(const DrawInfo& queue)
			{
				m_batch.push_back(queue);
			}

			void Sort()
			{
				std::sort(m_batch.begin(), m_batch.end(),
					[](const DrawInfo& a, const DrawInfo& b)
					{
						// if depth is not same, e.g. lower and higher tile, higher tile (b) has higher depth than lower tile (a). draw lower tile first
						if (a.depth != b.depth) return a.depth < b.depth;

						// if same depth, whichever is farthest from screen(a) gets drawn first. nearest from screen (b) is drawn last
						return a.pos.y < b.pos.y; // depth by Y
					});
			}
		};

		struct DrawCommand
		{
			engine::graphics::Sprite sprite;              // what to draw
			engine::spatial::PositionF pos;    // world position
			engine::spatial::SizeF size;       // size on screen
			engine::graphics::ColorF tint;     // color modulation
			float rotation;                    // rotation angle
			float depth;
		};

		class DrawQueue
		{
		private:
			std::vector<DrawCommand> m_commands;

		public:
			void Reserve(size_t capacity)
			{
				m_commands.reserve(capacity);
			}

			void Add(const DrawCommand& cmd)
			{
				m_commands.push_back(cmd);
			}

			void Sort()
			{
				std::sort(m_commands.begin(), m_commands.end(),
					[](const DrawCommand& a, const DrawCommand& b)
					{
						// if depth is not same, e.g. lower and higher tile, higher tile (b) has higher depth than lower tile (a). draw lower tile first
						if (a.depth != b.depth) return a.depth < b.depth;

						// if same depth, whichever is farthest from screen(a) gets drawn first. nearest from screen (b) is drawn last
						return a.pos.y < b.pos.y; // depth by Y
					});
			}

			void Execute(engine::graphics::renderer::IRenderer& renderer)
			{
				for (auto& cmd : m_commands)
				{
					renderer.Draw(cmd.sprite, cmd.pos, cmd.size, cmd.tint, cmd.rotation);
				}
			}

			void Clear()
			{
				m_commands.clear();
			}
		};

		template<typename T>
		void QueueDrawCommand(
			engine::component::tile::TileMap<T>& map,
			DrawQueue& queue,
			int row, int col,
			const engine::spatial::SizeF& tilesize,
			const engine::spatial::PositionF& pos,
			float depth,
			const engine::graphics::ColorF& tint = { 1,1,1,1 }
		)
		{
			if (!map.IsInBounds(row, col))
			{
				return;
			}

			const engine::component::tile::Tile<T>& tile = map.Get(row, col);
			if (tile.IsValid())
			{
				engine::spatial::PositionF origin =
				{
					col * tilesize.width,
					row * tilesize.height
				};

				queue.Add({
					tile->GetSprite(),
					pos + origin,   // world (tilemap)
					tilesize,
					tint,
					0.0f,
					depth
					});
			}
		}

		class PropMap
		{
		private:
			engine::container::Grid<engine::component::graphics::PropTile> m_grid;

		public:
			PropMap(size_t width = 0) :
				m_grid(width)
			{
			}

			void Initialize(size_t width, size_t height)
			{
				m_grid.Clear();
				m_grid.SetWidth(width);
				m_grid.Reserve({ width, height });

				for (size_t i = 0; i < width * height; ++i)
				{
					m_grid.Add(engine::component::graphics::PropTile{});
				}
			}

			void Initialize(engine::spatial::Size<size_t> size)
			{
				Initialize(size.width, size.height);
			}

			void Clear()
			{
				for (int i = 0; i < m_grid.GetElementCount(); i++)
				{
					engine::component::graphics::PropTile& tile = m_grid.Get(i);
					tile.Clear();
				}
			}

			bool IsInBounds(int row, int col) const
			{
				return m_grid.IsInBounds(row, col);
			}

			bool IsInBounds(const engine::spatial::Coord& coord) const
			{
				return m_grid.IsInBounds(coord);
			}

			void Set(int row, int col, engine::navigation::tile::TileConstraint constraint, const engine::component::graphics::PropHandle& prop)
			{
				m_grid.Get(row, col).Set(constraint, prop);
			}

			void Remove(int row, int col, engine::navigation::tile::TileConstraint constraint)
			{
				m_grid.Get(row, col).Remove(constraint);
			}

			bool Has(int row, int col, engine::navigation::tile::TileConstraint constraint) const
			{
				return m_grid.Get(row, col).Has(constraint);

			}

			void Clear(int row, int col)
			{
				m_grid.Get(row, col).Clear();
			}

			void Clear(const engine::spatial::Coord& coord)
			{
				m_grid.Get(coord).Clear();
			}

			void Queue(
				DrawQueue& queue,
				int row, int col,
				const engine::spatial::SizeF& tilesize,
				const engine::spatial::PositionF& pos,
				float depth,
				const math::VecF& offset = { 0,0 },
				const engine::graphics::ColorF& tint = { 1,1,1,1 }
			)
			{
				// prop map decides where in the tile the props should be drawn based on the constraints assigned to the props. 
				// for example, if a prop has CENTER constraint, it will be drawn at the center of the tile. 
				// if it has NW constraint, it will be drawn at the north-west corner of the tile, and so on. 
				// if a prop has no constraint, it will be drawn at the top-left corner of the tile by default.
				if (!m_grid.IsInBounds(row, col)) return;

				engine::component::graphics::PropTile proptile = m_grid.Get(row, col);

				if (proptile.Has(engine::navigation::tile::TileConstraint::CENTER))
				{
					const engine::graphics::Sprite& sprite = proptile.Get(engine::navigation::tile::TileConstraint::CENTER).GetSprite();

					// translate position so that the prop's anchor is at the center of the tile
					engine::spatial::PositionF translated = pos;
					translated.x += tilesize.width / 2.0f;
					translated.y += tilesize.height / 2.0f;

					// get the top-left position of this tile in world (tilemap) coordinate.
					engine::spatial::PositionF origin
					{
						col * tilesize.width,
						row * tilesize.height
					};

					queue.Add({
						sprite,
						translated + origin,   // world (tilemap)
						sprite.GetSize(),
						tint,
						0.0f,
						depth
						});
				}

				if (proptile.Has(engine::navigation::tile::TileConstraint::NW))
				{
					const engine::graphics::Sprite& sprite = proptile.Get(engine::navigation::tile::TileConstraint::NW).GetSprite();

					// no need to translate position since the prop's anchor is already at north-west corner of the tile

					// get the top-left position of this tile in world (tilemap) coordinate.
					engine::spatial::PositionF origin
					{
						col * tilesize.width,
						row * tilesize.height
					};

					queue.Add({
						sprite,
						pos + origin,   // world position
						sprite.GetSize(),
						tint,
						0.0f,
						depth
						});
				}

				if (proptile.Has(engine::navigation::tile::TileConstraint::NE))
				{
					const engine::graphics::Sprite& sprite = proptile.Get(engine::navigation::tile::TileConstraint::NE).GetSprite();

					// translate position so that prop's anchor is at north-east corner of the tile
					engine::spatial::PositionF translated = pos;
					translated.x += tilesize.width;

					// get the top-left position of this tile in world (tilemap) coordinate.
					engine::spatial::PositionF origin
					{
						col * tilesize.width,
						row * tilesize.height
					};
					queue.Add({
						sprite,
						translated + origin,   // world (tilemap)
						sprite.GetSize(),
						tint,
						0.0f,
						depth
						});
				}

				if (proptile.Has(engine::navigation::tile::TileConstraint::SW))
				{
					const engine::graphics::Sprite& sprite = proptile.Get(engine::navigation::tile::TileConstraint::SW).GetSprite();

					// translate position so that prop's anchor is at south-west corner of the tile
					engine::spatial::PositionF translated = pos;
					translated.y += tilesize.height;

					// get the top-left position of this tile in world (tilemap) coordinate.
					engine::spatial::PositionF origin
					{
						col * tilesize.width,
						row * tilesize.height
					};
					queue.Add({
						sprite,
						translated + origin,   // world (tilemap)
						sprite.GetSize(),
						tint,
						0.0f,
						depth
						});
				}

				if (proptile.Has(engine::navigation::tile::TileConstraint::SE))
				{
					const engine::graphics::Sprite& sprite = proptile.Get(engine::navigation::tile::TileConstraint::SE).GetSprite();
					// translate position so that prop's anchor is at south-east corner of the tile
					engine::spatial::PositionF translated = pos;
					translated.x += tilesize.width;
					translated.y += tilesize.height;

					// get the top-left position of this tile in world (tilemap) coordinate.
					engine::spatial::PositionF origin
					{
						col * tilesize.width,
						row * tilesize.height
					};
					queue.Add({
						sprite,
						translated + origin,   // world (tilemap)
						sprite.GetSize(),
						tint,
						0.0f,
						depth
						});
				}
			}
		};

	}
}

namespace TestProp
{
	using namespace std;
	using namespace engine;
	using namespace engine::graphics;
	using namespace engine::graphics::renderable;
	using namespace engine::graphics::animation;
	using namespace engine::win32;
	using namespace engine::graphics::renderer;
	using namespace engine::timer;
	using namespace engine::input;
	using namespace engine::event;
	using namespace engine::graphics::dx11::renderer;
	using namespace engine::graphics::dx11;
	using namespace engine::component::tile;
	using namespace engine::graphics::factory;
	using namespace engine::graphics::resource;
	using namespace engine::container;
	using namespace engine::loader::tile;
	using namespace engine::spatial;
	using namespace engine::math;
	using namespace engine::math::geometry;
	using namespace engine::component;
	using namespace engine::graphics::tile;
	using namespace engine::navigation::tile;
	using namespace engine::graphics::navigation;
	
	using LookupWallResolver = engine::algorithm::LookupResolver<const engine::spatial::Coord&, engine::tile::TileVariant, int>;


	template<typename T>
	using Registry = engine::cache::Registry<T>;

	template<typename T>
	using Animation = engine::graphics::animation::Animation<T>;

	// this is a tile definition class, not exactly tile class. tile class is Tile<T> and this is what is assigned to T
	class AnimatedTile
	{
	private:
		engine::graphics::animation::Animator<engine::graphics::Sprite> m_animator;
		std::unordered_map<std::string, engine::graphics::animation::Animation<engine::graphics::Sprite>> m_animations;
		bool m_walkable;
		int m_index;

	public:
		AnimatedTile(bool walkable, const std::string& name, const engine::graphics::animation::Animation<engine::graphics::Sprite>& anim, int index) :
			m_walkable(walkable),
			m_index(index)

		{
			// copy the animation into our container
			m_animations[name] = anim;

			// assign the animation from our container into animator (don't assign the passed animation. that is reference to animation outside which is not safe
			m_animator.Play(m_animations[name]);
		}

		bool IsRunning() const
		{
			return m_animator.IsRunning();
		}

		const engine::graphics::Sprite& GetSprite() const
		{
			return m_animator.GetCurrent();
		}

		void Update(double delta)
		{
			m_animator.Update(delta);
		}

		int GetIndex() const
		{
			return m_index;
		}
	};

	class RenderableTile
	{
	private:
		engine::graphics::Sprite m_sprite;
		bool m_walkable;
		int m_index;

	public:
		RenderableTile(const engine::graphics::Sprite& sprite, bool walkable, int index) :
			m_sprite(sprite),
			m_walkable(walkable),
			m_index(index)
		{
		}

		int GetIndex() const
		{
			return m_index;
		}

		const engine::graphics::Sprite& GetSprite() const
		{
			return m_sprite;
		}

		bool IsWalkable() const
		{
			return m_walkable;
		}
	};

	//template<typename T>
	//class Container//: public IContainerr<T>
	//{
	//	std::vector<T> m_data;
	//	size_t m_width;

	//public:

	//	void Add(T&& data) 
	//	{
	//		m_data.push_back(std::move(data));
	//	}
	//	//template<typename U = T, typename = typename std::enable_if<std::is_copy_constructible<U>::value>::type>
	//	void Add(const T& data) 
	//	{
	//		m_data.push_back(data);
	//	}

	//	void Set(int row, int col, const T& data) 
	//	{
	//		if (!IsInBounds(row, col))
	//		{
	//			throw std::out_of_range("index out of bounds");
	//		}
	//		m_data[row * m_width + col] = data;
	//	}

	//	void Set(int row, int col, T&& data)
	//	{
	//		if (!IsInBounds(row, col))
	//		{
	//			throw std::out_of_range("index out of bounds");
	//		}
	//		m_data[row * m_width + col] = std::move(data);
	//	}


	//	bool IsInBounds(int row, int col) const
	//	{
	//		return
	//			row >= 0 && col >= 0 &&					// make sure rows and columns are not negatives.
	//			col < m_width &&						// make sure column is within the grid's width
	//			row * m_width + col < m_data.size();	// make sure if you map the row and column, it is within the grid array's range
	//	}

	//	void Fill(const T& data) 
	//	{
	//		for (size_t i = 0; i < m_data.size(); i++) 
	//		{
	//			m_data[i] = data; // copy assignment
	//		}
	//	}


	//	//void Add(T& data) override
	//	//{
	//	//	m_data.push_back(std::move(data));
	//	//}

	//	//void Take(T&& data) override
	//	//{
	//	//	m_data.push_back(std::move(data));
	//	//}

	//	//void Take(T& data) override
	//	//{
	//	//	m_data.push_back(std::move(data));
	//	//}

	//	//virtual void AddRange(const std::vector<T>& data) = 0;

	//	//virtual void TakeRange(std::vector<T>&& data) = 0;

	//	//virtual void Pop() = 0;

	//	//virtual const T& Get(size_t index) const = 0;

	//	//virtual T& Get(size_t index) = 0;

	//	//virtual void Reserve(const spatial::Size<size_t>& size) = 0;

	//	//virtual size_t GetElementCount() const = 0;

	//	//virtual bool IsEmpty() const = 0;

	//	//virtual void Clear() = 0;

	//	//virtual bool IsInBounds(const size_t index) const = 0;

	//	//virtual T& Back() = 0;

	//	//virtual const T& Back() const = 0;
	//};

	class Test
	{
	public:


	private:
		std::unique_ptr<Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<IRenderer> m_renderer;

		StopWatch m_stopwatch;
		double m_elapsed;

		Input m_input;

		PositionF m_mousePos;

		bool m_toggle;

		std::vector<engine::graphics::animation::Animator<engine::graphics::Sprite>> m_animators;

		DrawQueue m_drawQueue;

	public:
		Test() :
			m_toggle(false)
		{
			Window::OnInitialize += Handler(this, &Test::OnInitialize);
			Window::OnExit += Handler(this, &Test::OnExit);
			Window::OnIdle += Handler(this, &Test::OnIdle);
			Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = make_unique<Window>();
			m_window->OnClose += Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += Handler(this, &Test::OnWindowSize);
			m_window->Create(L"TestEditMultiLayerMap", 1400, 900);
			m_window->OnWindowMessage += Handler(&m_input, &Input::ProcessWin32Message);

			m_input.KeyDownEvent += Handler(this, &Test::OnKeyDown);
			m_input.MouseDownEvent += Handler(this, &Test::OnMouseDown);
			m_input.MouseMoveEvent += Handler(this, &Test::OnMouseMove);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = make_unique<Canvas>(make_unique<DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = make_unique<Renderer>(make_unique<DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer Batch (DX11) created...");

			//Container<std::unique_ptr<std::string>> grid1;
			//grid1.Add(std::make_unique<std::string>("hello"));
			//grid1.Set(0, 0, std::make_unique<std::string>("world"));
			//Container<int> grid2;
			//grid2.Add(1);
			//grid2.Set(0, 0, 1);
			//int a = 69;
			//grid2.Add(a);
			//grid2.Set(0, 0, a);
			//Container< std::unique_ptr<engine::graphics::resource::ISpriteAtlas>> grid3;
			//grid3.Add(std::make_unique<engine::graphics::resource::SpriteAtlas>(nullptr));


			// set map parameters
			{
				Registry<SizeF>::Instance().Register("tile_size", make_unique<SizeF>(64.0f, 64.0f));
				Registry<PositionF>::Instance().Register("map_position", make_unique<PositionF>(50.0f, 50.0f));
				Registry<Size<size_t>>::Instance().Register("map_size", make_unique<Size<size_t>>(20, 12));
				Registry<PositionF>::Instance().Register("depth", make_unique<PositionF>(0.0f, 64.0f));
			}

			// create storages
			{
				
				Registry<engine::component::graphics::PropSet>::Instance().Register("props", make_unique<engine::component::graphics::PropSet>()); // prop storage				
				Registry<engine::graphics::animation::AnimationSet<engine::graphics::Sprite>>::Instance().Register("props", make_unique<engine::graphics::animation::AnimationSet<engine::graphics::Sprite>>()); // animation storage
			}

			// create sprite atlases
			{
				SpriteAtlasFactory::Create("tile", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9); // tile
				SpriteAtlasFactory::Create("tree", L"../Assets/tree_1x8_1536x192.png", 1, 8); // tree
				SpriteAtlasFactory::Create("pine_tree", L"../Assets/tree_1x8_1536x256.png", 1, 8); // pine tree
			}

			// setup resources for tree prop
			{
				// create animation objects and store in animation set
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tree");
				engine::graphics::animation::AnimationSet<engine::graphics::Sprite>& animset = Registry<engine::graphics::animation::AnimationSet<engine::graphics::Sprite>>::Instance().Get("props");
				animset.Register("storm", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 25.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("idle", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("frozen", AnimationFactory::Create(atlas, std::vector<int>{ 0 }, 1000.0f, true, PositionF{ 0.5f, 0.85f }));
			}

			// setup tree prop
			{
				// get our storage for easy access
				AnimationSet<engine::graphics::Sprite>& animset = Registry<AnimationSet<engine::graphics::Sprite>>::Instance().Get("props");
				engine::component::graphics::PropSet& props = Registry<engine::component::graphics::PropSet>::Instance().Get("props");
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tree");

				// create prop object and assign animation set for trees. build 3 of them as animated, and 1 as simple
				props.Register(0, std::make_unique<engine::component::graphics::AnimatedProp>(&animset));
				props.Register(1, std::make_unique<engine::component::graphics::AnimatedProp>(&animset));
				props.Register(2, std::make_unique<engine::component::graphics::AnimatedProp>(&animset));
				props.Register(3, std::make_unique<engine::component::graphics::SimpleProp>(atlas.MakeSprite(0, PositionF{ 0.5f, 0.85f })));

				// for the 3 animated props, play different animations
				props.MakePropHandle(0).Play("idle");
				props.MakePropHandle(1).Play("storm");
				props.MakePropHandle(2).Play("frozen");
			}

			// setup wall prop
			{
				engine::component::graphics::PropSet& props = Registry<engine::component::graphics::PropSet>::Instance().Get("props");
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tile");
				props.Register(10, std::make_unique<engine::component::graphics::SimpleProp>(atlas.MakeSprite(41, PositionF{ 0.0f, 1.0f }))); // left corner wall
				props.Register(11, std::make_unique<engine::component::graphics::SimpleProp>(atlas.MakeSprite(42, PositionF{ 0.0f, 1.0f }))); // center wall
				props.Register(12, std::make_unique<engine::component::graphics::SimpleProp>(atlas.MakeSprite(43, PositionF{ 0.0f, 1.0f }))); // right corner wall
				props.Register(13, std::make_unique<engine::component::graphics::SimpleProp>(atlas.MakeSprite(44, PositionF{ 0.0f, 1.0f }))); // island wall
			}

			// create propmap
			{
				Registry<PropMap>::Instance().Register("prop_map", make_unique<PropMap>());
				PropMap& map = Registry<PropMap>::Instance().Get("prop_map");
				map.Initialize(Registry<Size<size_t>>::Instance().Get("map_size"));
			}

			// setup tile region for floor 
			{
				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("tile", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("tile");

				// each sprite from atlas is a static tile (single frame), so we create tile from each sprite
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tile");
				for (int i = 0; i < atlas.GetUVRectCount(); i++) tileset.Register(i, std::make_unique<RenderableTile>(atlas.MakeSprite(i), true, i)); 

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("floor", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("floor");

				// load tile region by filling it with all '4' tile (empty)
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Table<string> map(mapsize, "4");
				AsyncTileRegionLoader<RenderableTile, int> loader;
				loader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for land map. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Register("floor", make_unique<engine::tile::AutoTileResolver<RenderableTile>>(region, tileset));
				engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("floor");

				// configure land map auto-tile mapping
				resolver.Register(4, engine::tile::TileVariant::Empty);
				resolver.Register(30, engine::tile::TileVariant::Island);
				resolver.Register(10, engine::tile::TileVariant::Full);

				resolver.Register(21, engine::tile::TileVariant::NorthEdge);
				resolver.Register(3, engine::tile::TileVariant::SouthEdge);
				resolver.Register(29, engine::tile::TileVariant::EastEdge);
				resolver.Register(27, engine::tile::TileVariant::WestEdge);

				resolver.Register(0, engine::tile::TileVariant::NECorner);
				resolver.Register(2, engine::tile::TileVariant::NWCorner);
				resolver.Register(18, engine::tile::TileVariant::SECorner);
				resolver.Register(20, engine::tile::TileVariant::SWCorner);

				resolver.Register(12, engine::tile::TileVariant::Vertical);
				resolver.Register(28, engine::tile::TileVariant::Horizontal);

				resolver.Register(1, engine::tile::TileVariant::TNorth);
				resolver.Register(19, engine::tile::TileVariant::TSouth);
				resolver.Register(9, engine::tile::TileVariant::TEast);
				resolver.Register(11, engine::tile::TileVariant::TWest);
			}

			// setup tile region for ceiling map
			{
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("tile");

				// create tile region	
				Registry<TileRegion<RenderableTile>>::Instance().Register("ceiling", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("ceiling");

				// load tile region by filling it with all '4' tile (empty)
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Table<string> map(mapsize, "4");
				AsyncTileRegionLoader<RenderableTile, int> loader;
				loader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for ceiling. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Register("ceiling", make_unique<engine::tile::AutoTileResolver<RenderableTile>>(region, tileset));
				engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("ceiling");

				// configure ceiling map auto-tile mapping
				resolver.Register(4, engine::tile::TileVariant::Empty);
				resolver.Register(35, engine::tile::TileVariant::Island);
				resolver.Register(15, engine::tile::TileVariant::Full);

				resolver.Register(26, engine::tile::TileVariant::NorthEdge);
				resolver.Register(8, engine::tile::TileVariant::SouthEdge);
				resolver.Register(34, engine::tile::TileVariant::EastEdge);
				resolver.Register(32, engine::tile::TileVariant::WestEdge);

				resolver.Register(5, engine::tile::TileVariant::NECorner);
				resolver.Register(7, engine::tile::TileVariant::NWCorner);
				resolver.Register(23, engine::tile::TileVariant::SECorner);
				resolver.Register(25, engine::tile::TileVariant::SWCorner);

				resolver.Register(17, engine::tile::TileVariant::Vertical);
				resolver.Register(33, engine::tile::TileVariant::Horizontal);

				resolver.Register(6, engine::tile::TileVariant::TNorth);
				resolver.Register(24, engine::tile::TileVariant::TSouth);
				resolver.Register(14, engine::tile::TileVariant::TEast);
				resolver.Register(16, engine::tile::TileVariant::TWest);
			}

			// setup water tilemap
			{
				// create sprite atlas to be used by tilemap
				SpriteAtlasFactory::Create("water", L"../Assets/1x1_64x64_water_background.png", 1, 1);
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("water");

				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("water", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("water");

				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), false, 0)); // water so not walkable. doesn't matter. this is background map

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("water", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("water");

				// load tile region by filling it with all '0' tile
				Table<string> map({ 20, 12 }, "0");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });
			}

			{
				Registry<LookupWallResolver>::Instance().Register("tile_to_wall", std::make_unique<LookupWallResolver>());
				LookupWallResolver& tile2wallresolver = Registry<LookupWallResolver>::Instance().Get("tile_to_wall");

				tile2wallresolver.Register(engine::tile::TileVariant::Empty, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::Island, 13);
				tile2wallresolver.Register(engine::tile::TileVariant::Full, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::NorthEdge, 13);
				tile2wallresolver.Register(engine::tile::TileVariant::SouthEdge, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::EastEdge, 12);
				tile2wallresolver.Register(engine::tile::TileVariant::WestEdge, 10);
				tile2wallresolver.Register(engine::tile::TileVariant::NECorner, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::NWCorner, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::SECorner, 10);
				tile2wallresolver.Register(engine::tile::TileVariant::SWCorner, 12);
				tile2wallresolver.Register(engine::tile::TileVariant::Vertical, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::Horizontal, 11);
				tile2wallresolver.Register(engine::tile::TileVariant::TNorth, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::TSouth, 11);
				tile2wallresolver.Register(engine::tile::TileVariant::TEast, -1);
				tile2wallresolver.Register(engine::tile::TileVariant::TWest, -1);

				engine::tile::AutoTileResolver<RenderableTile>& ceilingresolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("ceiling");
				ceilingresolver.TileVariantChangedEvent += engine::event::Handler(&tile2wallresolver, &LookupWallResolver::Set);

				tile2wallresolver.LookupEvent += engine::event::Handler(this, &Test::OnCeilingTilePlaced);
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnCeilingTilePlaced(const engine::spatial::Coord& coord, int index)
		{
			PropMap& propmap = Registry<PropMap>::Instance().Get("prop_map");
			engine::component::graphics::PropSet& propset = Registry<engine::component::graphics::PropSet>::Instance().Get("props");

			// if ceiling tile is placed, there should be no prop on this tile other than wall. so we clear it first
			propmap.Clear(coord.row, coord.col);

			// if index is not valid, means we don't have to put wall. it could be a center tile...
			if (propset.IsValid(index))
			{
				propmap.Set(coord.row, coord.col, engine::navigation::tile::TileConstraint::SW, propset.MakePropHandle(index));
			}
			else
			{
				propmap.Remove(coord.row, coord.col, engine::navigation::tile::TileConstraint::SW);
			}
		}

		void OnKeyDown(int key)
		{
			// all maps have the same size and position. so they share the same position to coord conversion. calculate coord based on mouse position and map position and tile size
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - mapPos, tilesize);
			PropMap& propmap = Registry<PropMap>::Instance().Get("prop_map");
			engine::component::graphics::PropSet& propset = Registry<engine::component::graphics::PropSet>::Instance().Get("props");
			TileRegion<RenderableTile>& floormap = Registry<TileRegion<RenderableTile>>::Instance().Get("floor");
			TileRegion<RenderableTile>& ceilingmap = Registry<TileRegion<RenderableTile>>::Instance().Get("ceiling");
			engine::tile::AutoTileResolver<RenderableTile>& floorresolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("floor");
			engine::tile::AutoTileResolver<RenderableTile>& ceilingresolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("ceiling");

			// sanity check. if coord is out of bounds, bail out.
			if (!floormap.IsInBounds(coord))
			{
				return;
			}

			switch (key)
			{
			case 27: // escape
			{
				{
					floorresolver.Remove();
					ceilingresolver.Remove();
				}

				// clear props
				{
					propmap.Clear();
				}

				break;
			}
			case 32: // space
			{
				break;
			}
			case 49: // 1
			{
				propmap.Clear(coord);
				floorresolver.Set(coord);
				ceilingresolver.Remove(coord);

				break;
			}
			case 50: // 2
			{
				propmap.Clear(coord);
				floorresolver.Remove(coord);
				ceilingresolver.Remove(coord);
				break;
			}
			case 51: // 3
			{
				floorresolver.Set(coord);
				ceilingresolver.Set(coord);
				break;
			}
			case 52: // 4
			{
				break;
			}
			case 53: // 5
			{
				ceilingresolver.Remove(coord);
				floorresolver.Set(coord);
				propmap.Set(coord.row, coord.col, engine::navigation::tile::TileConstraint::CENTER, propset.MakePropHandle(1));
				break;
			}
			case 54: // 6
			{
				propmap.Set(coord.row, coord.col, engine::navigation::tile::TileConstraint::CENTER, propset.MakePropHandle(1));
				break;
			}
			case 55: // 7
			{
				propmap.Set(coord.row, coord.col, engine::navigation::tile::TileConstraint::CENTER, propset.MakePropHandle(2));
				break;
			}
			case 56: // 8
			{
				propmap.Set(coord.row, coord.col, engine::navigation::tile::TileConstraint::CENTER, propset.MakePropHandle(3));
				break;
			}

			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
		}

		void OnMouseMove(int x, int y)
		{
			m_mousePos = PositionF((float)x, (float)y);
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			Registry<engine::component::graphics::PropSet>::Instance().Get("props").Update(time);
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<milliseconds>();

			m_input.Update();

			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_renderer->Begin();
				{
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
					Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
					PositionF depth = Registry<PositionF>::Instance().Get("depth");
					TileMap<RenderableTile> floormap = Registry<TileRegion<RenderableTile>>::Instance().Get("floor").MakeTileMap();
					TileMap<RenderableTile> ceilingmap = Registry<TileRegion<RenderableTile>>::Instance().Get("ceiling").MakeTileMap();
					TileMap<RenderableTile> watermap = Registry<TileRegion<RenderableTile>>::Instance().Get("water").MakeTileMap();

					PropMap& propMap = Registry<PropMap>::Instance().Get("prop_map");

					for (int row = 0; row < (int)mapsize.height; row++)
					{
						m_drawQueue.Clear();

						for (int col = 0; col < (int)mapsize.width; col++)
						{
							QueueDrawCommand(floormap, m_drawQueue, row, col, tilesize, pos, 1.0f, { 1,1,1,1 });
							QueueDrawCommand(watermap, m_drawQueue, row, col, tilesize, pos, 0.0f, { 1,1,1,1 });
							propMap.Queue(m_drawQueue, row, col, tilesize, pos, 1.0f);
							QueueDrawCommand(ceilingmap, m_drawQueue, row, col, tilesize, pos - depth, 2.0f, { 1,1,1,1 });
						}

						m_drawQueue.Sort();
						m_drawQueue.Execute(*m_renderer);
					}
				}
				
				m_renderer->End();
			}
			m_canvas->End();
		}

		void OnExit()
		{

		}

		void OnWindowClose()
		{
		}

		void OnWindowSize(size_t nWidth, size_t nHeight)
		{
			LOG("Window resized to: " + to_string(nWidth) + ", " + to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}
	};

}