//#pragma once
//
//#include <Win32/Window.h>
//#include <Core/Event.h>
//#include <Utilities/Logger.h>
//#include <Graphics/Core/ICanvas.h>
//#include <Graphics/Core/Canvas.h>
//#include <Graphics/Core/DX11CanvasImpl.h>
//#include <Graphics/Renderer/IRenderer.h>
//#include <Graphics/Renderer/DX11RendererBatchImpl.h>
//#include <Graphics/Renderer/DX11RendererImmediateImpl.h>
//#include <Graphics/Renderer/Renderer.h>
//#include <Graphics/Resource/ISpriteAtlas.h>
//#include <Engine/Factory/SpriteAtlasFactory.h>
//#include <Graphics/Core/Sprite.h>
//#include <Core/Input.h>
//#include <Graphics/Resource/IFontAtlas.h>
//#include <Graphics/Resource/FontAtlas.h>
//#include <Graphics/Resource/SpriteAtlas.h>
//#include <Cache/Registry.h>
//#include <Graphics/Animation/Animation.h>
//#include <Timer/StopWatch.h>
//#include <Algorithm/Pathfinding.h>
//#include <Engine/Loader/AsyncLoader.h>
//#include <Graphics/Core/Primitives.h>
//#include <Engine/Graphics/Draw.h>
//#include <Spatial/Coord.h>
//#include <Engine/Factory/FontFactory.h>
//
//using namespace engine;
//using namespace engine::graphics;
//
//namespace TestRenderable
//{
//	class IRenderable;
//	class IRenderItem;
//	class RenderItem;
//	class AnimatedItem;
//
//	using FontFactory = engine::graphics::factory::FontFactory;
//	using IFontAtlas = engine::graphics::resource::IFontAtlas;
//	using ISpriteAtlas = engine::graphics::resource::ISpriteAtlas;
//	using Sprite = engine::graphics::Sprite;
//	using SpriteAtlasFactory = engine::graphics::factory::SpriteAtlasFactory;
//	using AnimationFactory = engine::graphics::factory::AnimationFactory;
//	using TileConstraint = engine::navigation::tile::TileConstraint;
//	using Animator = engine::graphics::animation::Animator<Sprite>;
//	using AnimationSet = engine::graphics::animation::AnimationSet<Sprite>;
//
//
//	//template<typename T, typename Owner>
//	//using AnimationSystem = engine::graphics::animation::AnimationSystem<T, Owner>;
//
//	//template<typename T, typename Owner>
//	//using Animated = engine::graphics::animation::Animated<T, Owner>;
//
//	//template<typename T, typename Owner>
//	//using AnimationController = engine::graphics::animation::AnimationController<T, Owner>;
//
//	//template<typename T>
//	//using Registry = engine::cache::Registry<T>;
//
//	//template<typename T, typename U>
//	//using Dictionary = engine::container::Dictionary<T, U>;
//
//
//	class IRenderable
//	{
//	public:
//		virtual const Sprite GetSprite() const = 0;
//	};
//
//	template<typename T = std::string>
//	class IAnimated
//	{
//	public:
//		virtual void Play(const T& name) = 0;
//		virtual void Update(double time) = 0;
//	};
//
//
//
//	class Renderable : public IRenderable
//	{
//	private:
//		Sprite m_sprite;
//
//	public:
//		Renderable(const Sprite& sprite) :
//			m_sprite(sprite)
//		{
//		}
//
//		const Sprite GetSprite() const override final
//		{
//			return m_sprite;
//		}
//	};
//
//	class Animated : public IRenderable, public IAnimated<std::string>
//	{
//	private:
//		core::View<AnimationSet> m_set;
//		Animator m_animator;
//
//	public:
//		Animated(AnimationSet& set, const std::string& name):
//			m_set(&set)
//		{
//			Play(name);
//		}
//
//		const Sprite GetSprite() const override final
//		{
//			return m_animator.GetCurrent();
//		}
//
//		void Play(const std::string& name) override final
//		{
//			if (m_set.IsValid() && m_set->Has(name))
//			{
//				m_animator.Play(m_set->Get(name));
//			}
//		}
//
//		void Update(double time) override final
//		{
//			m_animator.Update(time);
//		}
//	};
//
//	//class AnimatedItem : public IRenderable, public IAnimated<std::string>
//	//{
//	//private:
//	//	Animated<Sprite, IRenderable> m_animated;
//
//	//public:
//	//	AnimatedItem(Animated<Sprite, IRenderable> animated):
//	//		m_animated(animated)
//	//	{
//
//	//	}
//
//	//	AnimatedItem(AnimationSystem<Sprite, IRenderable>* animsys):
//	//		m_animated(animsys->MakeAnimated(this))
//	//	{
//	//	}
//
//	//	AnimatedItem(AnimationSystem<Sprite, IRenderable>* animsys, const std::string& name) :
//	//		m_animated(animsys->MakeAnimated(this))
//	//	{
//	//		m_animated.Play(name);
//	//	}
//
//	//	AnimatedItem(AnimationSystem<Sprite, IRenderable>* animsys, const std::string& name, int loopCount) :
//	//		m_animated(animsys->MakeAnimated(this))
//	//	{
//	//		m_animated.Play(name);
//	//		m_animated.SetPolicy(PlaybackPolicy::FiniteLoop, loopCount);
//	//	}
//
//	//	virtual ~AnimatedItem()
//	//	{
//	//		m_animated.Destroy();
//	//	}
//
//	//	const Sprite GetSprite() const override final
//	//	{
//	//		return m_animated.GetCurrent();
//	//	}
//
//	//	void Play(const std::string& name) override final
//	//	{
//	//		m_animated.Play(name);
//	//	}
//
//	//	void Update(double time) override final
//	//	{
//	//		m_animated.Update(time);
//	//	}
//	//};
//
//	//class InteractiveAnimatedItem : public IRenderable, public IAnimated<std::string>
//	//{
//	//private:
//	//	// we don't manage this object. someone else updates it. someone else will manage its life cycle
//	//	Animated<Sprite, IRenderable> m_idle;
//
//	//	// this is the handle to our own animation controller
//	//	Animated<Sprite, IRenderable> m_interactive;
//
//	//	// this is our animation controller for interactive animation
//	//	AnimationController<Sprite, IRenderable> m_interactiveController;
//
//	//	// points to current state
//	//	Animated<Sprite, IRenderable>* m_curr;
//
//	//public:
//	//	InteractiveAnimatedItem(Animated<Sprite, IRenderable> idle, AnimationSet set) :
//	//		m_idle(idle),
//	//		m_interactiveController(&set),
//	//		m_interactive(&m_interactiveController),
//	//		m_curr(&m_idle)
//	//	{
//	//	}
//
//	//	virtual ~InteractiveAnimatedItem()
//	//	{
//	//	}
//
//	//	const Sprite GetSprite() const override final
//	//	{
//	//		return m_curr->GetCurrent();
//	//	}
//
//	//	void Play(const std::string& name) override final
//	//	{
//	//		m_curr->Play(name);
//	//	}
//
//	//	void Update(double time) override final
//	//	{
//	//		// we only update our own
//	//		if(m_curr == &m_interactive) m_curr->Update(time);
//	//	}
//
//	//	void Interact(const std::string& name, int loopCount)
//	//	{
//	//		m_curr = &m_interactive;
//
//	//		m_interactive.Play(name);
//
//	//		m_interactive.SetPolicy(PlaybackPolicy::FiniteLoop, loopCount);
//	//		m_interactive.GetEndEvent() += engine::event::Handler(this, &InteractiveAnimatedItem::OnInteractiveEnd);
//
//	//	}
//
//	//	void OnInteractiveEnd(IRenderable&)
//	//	{
//	//		m_curr = &m_idle;
//	//	}
//
//
//
//	//};
//
//	class Tile
//	{
//	private:
//		Dictionary<TileConstraint, std::unique_ptr<IRenderable>> m_renderables;
//
//	public:
//		Tile()
//		{
//		}
//
//		void Set(TileConstraint constraint, std::unique_ptr<IRenderable> renderable)
//		{
//			if (m_renderables.Has(constraint))
//			{
//				m_renderables.Unregister(constraint);
//			}
//			m_renderables.Register(constraint, std::move(renderable));
//		}
//
//		void Remove(TileConstraint constraint)
//		{
//			m_renderables.Unregister(constraint);
//		}
//
//		bool Has(TileConstraint constraint) const
//		{
//			return m_renderables.Has(constraint);
//		}
//
//		const IRenderable& Get(engine::navigation::tile::TileConstraint constraint) const
//		{
//			// unsafe. caller should check Has() before calling this. 
//			return *m_renderables[constraint];
//		}
//
//		// clears all props from this tile
//		void Clear()
//		{
//			m_renderables.Clear();
//		}
//	};
//
//
//	class Test
//	{
//
//
//	private:
//		std::unique_ptr<win32::Window> m_window;
//		std::unique_ptr<ICanvas> m_canvas;
//		std::unique_ptr<renderer::IRenderer> m_renderer;
//		timer::StopWatch m_stopwatch;
//
//	public:
//
//		Test()	
//		{
//			win32::Window::OnInitialize += event::Handler(this, &Test::OnInitialize);
//			win32::Window::OnExit += event::Handler(this, &Test::OnExit);
//			win32::Window::OnIdle += event::Handler(this, &Test::OnIdle);
//			win32::Window::Run();
//		}
//
//		// function that will be called just before we enter into message loop
//		void OnInitialize()
//		{
//			// create our window here
//			m_window = std::make_unique<win32::Window>();
//			m_window->OnClose += event::Handler(this, &Test::OnWindowClose);
//			m_window->OnCreate += event::Handler(this, &Test::OnWindowCreate);
//			m_window->OnSize += event::Handler(this, &Test::OnWindowSize);
//			m_window->Create(L"Test Renderable", 1400, 900);
//			m_window->OnWindowMessage += event::Handler(&input::Input::Instance(), &input::Input::ProcessWin32Message);
//
//			input::Input::Instance().KeyDownEvent += event::Handler(this, &Test::OnKeyDown);
//			input::Input::Instance().MouseDownEvent += event::Handler(this, &Test::OnMouseDown);
//			input::Input::Instance().MouseMoveEvent += event::Handler(this, &Test::OnMouseMove);
//		}
//
//		// when window is created. we can now safely create resources dependent on window
//		void OnWindowCreate(void* hWnd)
//		{
//			LOG("Window created...");
//
//			// create dx11 canvas
//			m_canvas = std::make_unique<Canvas>(std::make_unique<dx11::DX11CanvasImpl>());
//			m_canvas->Initialize(hWnd);
//			m_canvas->SetViewPort();
//			LOG("Canvas (DX11) created...");
//
//			// create dx11 renderer batched
//			m_renderer = std::make_unique<renderer::Renderer>(std::make_unique<dx11::renderer::DX11RendererBatchImpl>());
//			m_renderer->Initialize();
//			LOG("Renderer Batch (DX11) created...");
//
//			// create font and store in cache
//			FontFactory::Create("font", "Terminal", 12);
//			LOG("Font atlas (terminal, 12) created...");
//
//			// create sprite atlases and store in cache
//			{
//				SpriteAtlasFactory::Create("tile", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9); // tile
//				SpriteAtlasFactory::Create("tree", L"../Assets/tree_1x8_1536x192.png", 1, 8); // tree
//				SpriteAtlasFactory::Create("pine_tree", L"../Assets/tree_1x8_1536x256.png", 1, 8); // pine tree
//			}
//
//			// setup resources for tree item
//			{
//				// create animation set and store in cache
//				Registry<AnimationSet>::Instance().Register("tree", make_unique<AnimationSet>()); 
//				AnimationSet& animset = Registry<AnimationSet>::Instance().Get("tree");
//
//				// get tree atlas
//				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("tree");
//				
//				// create animation for trees and store in animation set
//				animset.Register("storm", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 25.0f, true, PositionF{ 0.5f, 0.85f }));
//				animset.Register("idle", AnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 200.0f, true, PositionF{ 0.5f, 0.85f }));
//				animset.Register("frozen", AnimationFactory::Create(atlas, std::vector<int>{ 0 }, 1000.0f, true, PositionF{ 0.5f, 0.85f }));
//
//				// create animation system for trees
//				Registry<AnimationSystem<Sprite, IRenderable>>::Instance().Register("tree", make_unique<AnimationSystem<Sprite, IRenderable>>(&animset));
//				AnimationSystem<Sprite, IRenderable>& animsys = Registry<AnimationSystem<Sprite, IRenderable>>::Instance().Get("tree");
//
//				//Registry<AnimatedItem>::Instance().Register("tree", std::make_unique<AnimatedItem>(&animsys));
//				//AnimatedItem& animtree = Registry<AnimatedItem>::Instance().Get("tree");
//				//animtree.Play("idle");
//
//				Registry<Tile>::Instance().Register("tree", std::make_unique<Tile>());
//				Tile& tile = Registry<Tile>::Instance().Get("tree");
//
//				//tile.Set(TileConstraint::CENTER, std::make_unique<AnimatedItem>(&animsys, "idle"));
//				tile.Set(TileConstraint::SW, std::make_unique<Animated>(animset, "idle"));
//				tile.Set(TileConstraint::NE, std::make_unique<Renderable>(atlas.MakeSprite(0, PositionF{ 0.5f, 0.85f })));
//
//
//			}
//			
//			// setup stopwatch to manage timing and start it
//			m_stopwatch.OnLap += event::Handler(this, &Test::OnLap);
//			m_stopwatch.Start();
//		}
//
//		void OnKeyDown(int key)
//		{
//			switch (key)
//			{
//			case 32:
//				break;
//			case 49: // 1
//				break;
//			default:
//				break;
//			}
//		}
//
//		void OnMouseDown(int btn, int x, int y)
//		{
//			return;
//		}
//
//		void OnMouseMove(int x, int y)
//		{
//
//		}
//
//		// this method is fired up whenever the OnLap event is triggered from stopwatch
//		void OnLap(double time)
//		{
//			//AnimationSystem<Sprite, AnimatedItem>& animsys = Registry<AnimationSystem<Sprite, AnimatedItem>>::Instance().Get("tree");
//			//animsys.Update(time);
//			//animsys.Flush();
//		}
//
//		// fun stuff. this is called on each loop of the message loop. this is where we draw!
//		void OnIdle()
//		{
//			// call lap to get elapsed time and trigger OnLap event
//			m_stopwatch.Lap<engine::timer::milliseconds>();
//
//			engine::input::Input::Instance().Update();
//
//			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });
//
//			// start the canvas. we can draw from here
//			m_canvas->Begin();
//			{
//				m_renderer->Begin();
//
//				//IRenderable& animtree = Registry<AnimatedItem>::Instance().Get("tree");
//				//m_renderer->Draw(animtree.GetSprite(), { 300, 300 }, animtree.GetSprite().GetSize(), { 1,1,1,1 }, 0);
//
//				Tile& tile = Registry<Tile>::Instance().Get("tree");
//
//				//m_renderer->Draw(tile.Get(TileConstraint::CENTER).GetSprite(), { 200, 400 }, tile.Get(TileConstraint::CENTER).GetSprite().GetSize(), { 1,1,1,1 }, 0);
//				m_renderer->Draw(tile.Get(TileConstraint::SW).GetSprite(), { 300, 400 }, tile.Get(TileConstraint::SW).GetSprite().GetSize(), { 1,1,1,1 }, 0);
//				m_renderer->Draw(tile.Get(TileConstraint::NE).GetSprite(), { 400, 400 }, tile.Get(TileConstraint::NE).GetSprite().GetSize(), { 1,1,1,1 }, 0);
//
//
//				m_renderer->Draw(Registry<IFontAtlas>::Instance().Get("font"), "Hello world", { 600, 5 }, { 1,1,1,1 });
//
//				m_renderer->End();
//			}
//			m_canvas->End();
//		}
//
//		void OnExit()
//		{
//
//		}
//
//		void OnWindowClose()
//		{
//		}
//
//		void OnWindowSize(size_t nWidth, size_t nHeight)
//		{
//			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
//			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
//			m_canvas->SetViewPort();
//		}
//
//
//
//	};
//}