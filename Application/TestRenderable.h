#pragma once

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
#include <Spatial/Coord.h>
#include <Engine/Factory/FontFactory.h>
#include <Algorithm/Resolvers.h>

using namespace engine;
using namespace engine::graphics;

namespace TestRenderable
{
	class IRenderable;
	class IRenderItem;
	class RenderItem;
	class AnimatedItem;

	using FontFactory = engine::graphics::factory::FontFactory;
	using IFontAtlas = engine::graphics::resource::IFontAtlas;
	using ISpriteAtlas = engine::graphics::resource::ISpriteAtlas;
	using Sprite = engine::graphics::Sprite;
	using SpriteAtlasFactory = engine::graphics::factory::SpriteAtlasFactory;
	using AnimationFactory = engine::graphics::factory::AnimationFactory;
	using TileConstraint = engine::navigation::tile::TileConstraint;
	using Animator = engine::graphics::animation::Animator<Sprite>;
	using AnimationSet = engine::graphics::animation::AnimationSet<Sprite>;
	using AnimationSystemCache = engine::graphics::animation::AnimationSystemCache<Sprite>;
	using Coord = engine::spatial::Coord;


	//template<typename T, typename Owner>
	//using AnimationSystem = engine::graphics::animation::AnimationSystem<T, Owner>;

	//template<typename T, typename Owner>
	//using Animated = engine::graphics::animation::Animated<T, Owner>;

	template<typename Owner>
	using AnimationController = engine::graphics::animation::AnimationController<Sprite, Owner>;

	//template<typename T>
	//using Registry = engine::cache::Registry<T>;

	//template<typename T, typename U>
	//using Dictionary = engine::container::Dictionary<T, U>;


	class IRenderable
	{
	public:
		virtual const Sprite GetSprite() const = 0;
	};

	template<typename T = std::string>
	class IAnimated
	{
	public:
		virtual bool Play(const T& name) = 0;
		virtual void Update(double time) = 0;
	};

	class Renderable : public IRenderable
	{
	private:
		Sprite m_sprite;

	public:
		Renderable(const Sprite& sprite) :
			m_sprite(sprite)
		{
		}

		const Sprite GetSprite() const override final
		{
			return m_sprite;
		}
	};

	class Animated : public IRenderable, public IAnimated<std::string>
	{
	private:
		AnimationController<Animated> m_animationController;

	public:
		Animated(AnimationSet& set, const std::string& name):
			m_animationController(set, this)
		{
			Play(name);
		}

		const Sprite GetSprite() const override final
		{
			return m_animationController.GetCurrent();
		}

		bool Play(const std::string& name) override final
		{
			return m_animationController.Play(name);
		}

		void Update(double time) override final
		{
			m_animationController.Update(time);
		}
	};

	//// define data structure that contains renderable and relative position (normalized) of this renderable to the tile
	//// e.g. if center -> {0.5, 0.5}
	//// we need a resolver that maps a constraint with a relative position
	//class Tile
	//{
	//private:
	//	Dictionary<TileConstraint, std::unique_ptr<IRenderable>> m_renderables;

	//public:
	//	Tile()
	//	{
	//	}

	//	void Set(TileConstraint constraint, std::unique_ptr<IRenderable> renderable)
	//	{
	//		if (m_renderables.Has(constraint))
	//		{
	//			m_renderables.Unregister(constraint);
	//		}
	//		m_renderables.Register(constraint, std::move(renderable));
	//	}

	//	void Remove(TileConstraint constraint)
	//	{
	//		m_renderables.Unregister(constraint);
	//	}

	//	bool Has(TileConstraint constraint) const
	//	{
	//		return m_renderables.Has(constraint);
	//	}

	//	const IRenderable& Get(engine::navigation::tile::TileConstraint constraint) const
	//	{
	//		// unsafe. caller should check Has() before calling this. 
	//		return *m_renderables[constraint];
	//	}

	//	// clears all props from this tile
	//	void Clear()
	//	{
	//		m_renderables.Clear();
	//	}
	//};


		// tile instance holds a reference to tile data from tileset
	// lightweight view into tile data 
	template<typename T>
	class Tile : public core::View<T>
	{
		template<typename T, typename K>
		friend class Tileset;

	protected:
		Tile(T* data = nullptr) :
			core::View<T>(data)
		{
		}

	public:

		~Tile() = default;

		Tile(const Tile&) = default;
		Tile& operator=(const Tile&) = default;
		Tile(Tile&&) = default;
		Tile& operator=(Tile&&) = default;
	};

	// manages registration and retrieval of tile data by ID
	template<typename T, typename K>
	class Tileset
	{
	protected:
		container::Dictionary<K, std::unique_ptr<T>> m_registry;

	public:
		Tileset() = default;
		~Tileset() = default;

		// non copyable, non movable
		Tileset(const Tileset&) = delete;
		Tileset& operator=(const Tileset&) = delete;
		Tileset(Tileset&&) = delete;
		Tileset& operator=(Tileset&&) = delete;

		bool Register(K id, std::unique_ptr<T> data)
		{
			return m_registry.Register(id, std::move(data));
		}
		bool IsValid(K id) const
		{
			return m_registry.Has(id);
		}

		const T* Get(K id) const
		{
			return m_registry.Has(id) ? *m_registry.Get(id) : nullptr;
		}

		// creates a tile instance for the given id. returns invalid tile if id not found
		Tile<T> MakeTile(K id) const
		{
			return m_registry.Has(id) ? Tile<T>(m_registry.Get(id).get()) : Tile<T>();
		}

		// define iterator for our container
		using iterator = typename container::Dictionary<K, std::unique_ptr<T>>::iterator;
		using const_iterator = typename container::Dictionary<K, std::unique_ptr<T>>::const_iterator;

		// iterator access
		iterator begin() { return m_registry.begin(); }
		iterator end() { return m_registry.end(); }
		const_iterator begin() const { return m_registry.begin(); }
		const_iterator end() const { return m_registry.end(); }
		const_iterator cbegin() const { return m_registry.cbegin(); }
		const_iterator cend() const { return m_registry.cend(); }
	};

	// tile layer represents a 2d grid of tile instances
	template<typename T>
	class TileGrid
	{
	private:
		engine::container::Grid<Tile<T>> m_map;

	public:
#pragma region // constructor/destructor
		TileGrid() :
			m_map(0)
		{
		}
#pragma endregion

#pragma region // non copyable, non movable
		TileGrid(const TileGrid&) = delete;
		TileGrid& operator=(const TileGrid&) = delete;
		TileGrid(TileGrid&&) = delete;
		TileGrid& operator=(TileGrid&&) = delete;
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

		// check for bounds via index
		bool IsInBounds(const size_t index) const
		{
			return m_map.IsInBounds(index);
		}
#pragma endregion

#pragma region // accessors
		const Tile<T>& Get(size_t index) const
		{
			return m_map.Get(index);
		}

		Tile<T>& Get(size_t index)
		{
			return m_map.Get(index);
		}

		Tile<T>& Back()
		{
			return m_map.Back();
		}

		const Tile<T>& Back() const
		{
			return m_map.Back();
		}

		Tile<T>& Get(int row, int col)
		{
			return m_map.Get(row, col);
		}

		const Tile<T>& Get(int row, int col) const
		{
			return m_map.Get(row, col);
		}

		// retrieves the data at Coord
		Tile<T>& Get(const engine::spatial::Coord& coord)
		{
			return m_map.Get(coord.row, coord.col);
		}

		// retrieves the data at Coord
		const Tile<T>& Get(const engine::spatial::Coord& coord) const
		{
			return m_map.Get(coord.row, coord.col);
		}
#pragma endregion

#pragma region // replace value
		void Set(int row, int col, const Tile<T>& data)
		{
			m_map.Set(row, col, data);
		}

		void Set(int row, int col, Tile<T>&& data)
		{
			m_map.Set(row, col, std::move(data));
		}

		void Set(const engine::spatial::Coord& coord, const Tile<T>& data)
		{
			m_map.Set(coord, data);
		}

		void Set(const engine::spatial::Coord& coord, Tile<T>&& data)
		{
			m_map.Set(coord, std::move(data));
		}
#pragma endregion

#pragma region // iterator support
		typename std::vector<T>::iterator begin() { return m_map.begin(); }
		typename std::vector<T>::iterator end() { return m_map.end(); }
		typename std::vector<T>::const_iterator begin() const { return m_map.begin(); }
		typename std::vector<T>::const_iterator end() const { return m_map.end(); }
		typename std::vector<T>::const_iterator cbegin() const { return m_map.cbegin(); }
		typename std::vector<T>::const_iterator cend() const { return m_map.cend(); }
		typename std::vector<T>::reverse_iterator rbegin() { return m_map.rbegin(); }
		typename std::vector<T>::reverse_iterator rend() { return m_map.rend(); }
		typename std::vector<T>::const_reverse_iterator rbegin() const { return m_map.rbegin(); }
		typename std::vector<T>::const_reverse_iterator rend() const { return m_map.rend(); }
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

		// sets grid width only
		void SetWidth(const size_t width)
		{
			m_map.SetWidth(width);
		}

		void Add(const Tile<T>& data)
		{
			m_map.Add(data);
		}

		void Add(Tile<T>&& data)
		{
			m_map.Add(std::move(data));
		}

		void AddRange(const std::vector<Tile<T>>& data)
		{
			m_map.AddRange(data);
		};

		void AddRange(std::vector<Tile<T>>&& data)
		{
			m_map.AddRange(std::move(data));
		}

		void Pop()
		{
			m_map.Pop();
		}

		//void Fill(const Tile<T>& data)
		//{
		//	for (int row = 0; row < m_map.GetHeight(); row++)
		//	{
		//		for (int col = 0; col < m_map.GetWidth(); col++)
		//		{
		//			m_map.Set(row, col, data);
		//		}
		//	}
		//}
#pragma endregion

#pragma region // make maps
		//const TileMap<T> MakeTileMap() const
		//{
		//	return TileMap<T>(this);
		//}

		//TileMap<T> MakeTileMap()
		//{
		//	return TileMap<T>(this);
		//}
#pragma endregion
	};


	class Test
	{
	private:
		std::unique_ptr<win32::Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<renderer::IRenderer> m_renderer;
		timer::StopWatch m_stopwatch;

	public:

		Test()	
		{
			win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
			win32::Window::OnExit += event::Handler(this, &Test::OnExit);
			win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);
			win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<win32::Window>();
			m_window->OnClose += event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += event::Handler(this, &Test::OnWindowSize);
			m_window->Create(L"Test Renderable", 1400, 900);
			m_window->OnWindowMessage += event::Handler(&input::Input::Instance(), &input::Input::ProcessWin32Message);

			input::Input::Instance().KeyDownEvent += event::Handler(this, &Test::OnKeyDown);
			input::Input::Instance().MouseDownEvent += event::Handler(this, &Test::OnMouseDown);
			input::Input::Instance().MouseMoveEvent += event::Handler(this, &Test::OnMouseMove);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<Canvas>(std::make_unique<dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = std::make_unique<renderer::Renderer>(std::make_unique<dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer Batch (DX11) created...");

			// create font and store in cache
			FontFactory::Create("font", "Terminal", 12);
			LOG("Font atlas (terminal, 12) created...");

			// create sprite atlases and store in cache
			{
				SpriteAtlasFactory::Create("tile", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9); // tile
				SpriteAtlasFactory::Create("tree", L"../Assets/tree_1x8_1536x192.png", 1, 8); // tree
				SpriteAtlasFactory::Create("pine_tree", L"../Assets/tree_1x8_1536x256.png", 1, 8); // pine tree
			}

			// setup resources for tree item
			{
				// create animation set and store in cache
				Registry<AnimationSet>::Instance().Register("tree", make_unique<AnimationSet>()); 
				AnimationSet& animset = Registry<AnimationSet>::Instance().Get("tree");

				// get tree atlas
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tree");
				
				// create animation for trees and store in animation set
				animset.Register("storm", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 25.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("idle", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.85f }));
				animset.Register("frozen", AnimationFactory::Create(atlas, std::vector<int>{ 0 }, 1000.0f, true, PositionF{ 0.5f, 0.85f }));

				//Registry<Tile>::Instance().Register("tree", std::make_unique<Tile>());
				//Tile& tile = Registry<Tile>::Instance().Get("tree");

				//tile.Set(TileConstraint::CENTER, std::make_unique<Animated>(animset, "storm"));
				//tile.Set(TileConstraint::SW, std::make_unique<Animated>(animset, "idle"));
				//tile.Set(TileConstraint::NE, std::make_unique<Renderable>(atlas.MakeSprite(0, PositionF{ 0.5f, 0.85f })));

				//Registry<Dictionary<TileConstraint, PositionF>>::Instance().Register("tree", std::make_unique<Dictionary<TileConstraint, PositionF>>());
				//Dictionary<TileConstraint, PositionF>& positionLookup = Registry<Dictionary<TileConstraint, PositionF>>::Instance().Get("tree");
				//positionLookup.Register(TileConstraint::CENTER, PositionF{ 0.5f, 0.5f });
				//positionLookup.Register(TileConstraint::NW, PositionF{ 0.1f, 0.1f });
				//positionLookup.Register(TileConstraint::NE, PositionF{ 0.9f, 0.1f });
				//positionLookup.Register(TileConstraint::SW, PositionF{ 0.1f, 0.9f });
				//positionLookup.Register(TileConstraint::SE, PositionF{ 0.9f, 0.9f });


			}
			
			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			switch (key)
			{
			case 32:
				break;
			case 49: // 1
				break;
			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			return;
		}

		void OnMouseMove(int x, int y)
		{

		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			AnimationSystemCache::Instance().Update(time);
		}

		PositionF GetPos(TileConstraint constraint, SizeF size, PositionF origin)
		{
			Dictionary<TileConstraint, PositionF>& positionLookup = Registry<Dictionary<TileConstraint, PositionF>>::Instance().Get("tree");
			PositionF factor = positionLookup.Get(constraint);

			factor.x *= size.width;
			factor.y *= size.height;

			factor += origin;

			return factor;
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			engine::input::Input::Instance().Update();

			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_renderer->Begin();

				//IRenderable& animtree = Registry<AnimatedItem>::Instance().Get("tree");
				//m_renderer->Draw(animtree.GetSprite(), { 300, 300 }, animtree.GetSprite().GetSize(), { 1,1,1,1 }, 0);

				//Tile& tile = Registry<Tile>::Instance().Get("tree");

				//SizeF tilesize(128, 128);
				//m_renderer->Draw(
				//	tile.Get(TileConstraint::CENTER).GetSprite(), 
				//	GetPos(TileConstraint::CENTER, tilesize, {400, 400}), 
				//	tile.Get(TileConstraint::CENTER).GetSprite().GetSize(), 
				//	{ 1,1,1,1 }, 0);
				//m_renderer->Draw(
				//	tile.Get(TileConstraint::SW).GetSprite(),
				//	GetPos(TileConstraint::SW, tilesize, { 400, 400 }),
				//	tile.Get(TileConstraint::SW).GetSprite().GetSize(),
				//	{ 1,1,1,1 }, 0);
				//m_renderer->Draw(
				//	tile.Get(TileConstraint::NE).GetSprite(),
				//	GetPos(TileConstraint::NE, tilesize, { 400, 400 }),
				//	tile.Get(TileConstraint::NE).GetSprite().GetSize(),
				//	{ 1,1,1,1 }, 0);

			//	m_renderer->Draw(tile.Get(TileConstraint::SW).GetSprite(), { 300, 400 }, tile.Get(TileConstraint::SW).GetSprite().GetSize(), { 1,1,1,1 }, 0);
		    //	m_renderer->Draw(tile.Get(TileConstraint::NE).GetSprite(), { 400, 400 }, tile.Get(TileConstraint::NE).GetSprite().GetSize(), { 1,1,1,1 }, 0);



				std::string msg = "Animators (Cache): " + std::to_string(AnimationSystemCache::Instance().Size());
				m_renderer->Draw(Registry<IFontAtlas>::Instance().Get("font"), msg, { 600, 5 }, { 1,1,1,1 });

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
			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}



	};
}