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
#include <Graphics/Renderable/Sprite.h>
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

namespace debug
{
	// Internal interface for all tile definitions
	struct ITileDefinition
	{
		virtual ~ITileDefinition() = default;
		virtual int Index() const = 0;
		virtual bool IsWalkable() const = 0;
		virtual const engine::graphics::renderable::Sprite& GetSprite() const = 0;
	};

	class Tile : public engine::tile::Tile<ITileDefinition>
	{
	public:
		Tile(ITileDefinition* def) :
			engine::tile::Tile<ITileDefinition>(def)
		{
		}
		// TODO: still expose ITileDefinition methods here for mor natural API feel.
		// but for now just use -> operator to access ITileDefinition methods
	};

	class Tileset : public engine::component::tile::Tileset<ITileDefinition>
	{
	public:

	};
}

namespace TestSpecializedTileDefinition
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

	template<typename T>
	using Registry = engine::cache::Registry<T>;

	template<typename T>
	using Animation = engine::graphics::animation::Animation<T>;

	class RenderableTile
	{
	private:
		Sprite m_sprite;
		bool m_walkable;

	public:
		RenderableTile(const Sprite& sprite, bool walkable) :
			m_sprite(sprite),
			m_walkable(walkable)
		{
			debug::Tileset ts;
			engine::component::tile::Tile<debug::ITileDefinition> tile = ts.MakeTile(0);



		
		}

		const Sprite& GetSprite() const
		{
			return m_sprite;
		}

		bool IsWalkable() const
		{
			return m_walkable;
		}
	};

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

	public:
		Test()
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
			m_window->Create(L"TestActorNavigation", 1400, 900);
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

			// set map parameters
			{
				Registry<SizeF>::Instance().Register("tile_size", make_unique<SizeF>(64.0f, 64.0f));
				Registry<PositionF>::Instance().Register("map_position", make_unique<PositionF>(50.0f, 50.0f));
				Registry<Size<size_t>>::Instance().Register("map_size", make_unique<Size<size_t>>(20, 12));
			}

			// create sprite atlas to be used by tilemap
			{
				SpriteAtlasFactory::Create("1x1_64x64_water_background", L"../Assets/1x1_64x64_water_background.png", 1, 1);
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			switch (key)
			{
			case 27: // escape
			{
				break;
			}
			case 32: // space
				break;
			case 49: // 1
			{
				break;
			}
			case 50: // 2
			{
				break;
			}
			case 51: // 3
			{
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

		Coord PositionToMapCoord(const PositionF& pos)
		{
			// get parameters of tilemap
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

			// calculate the coordinate of tile that intersect wih mouse click position
			Coord coord;
			coord.col = (int)((pos.x - mapPos.x) / tilesize.width);
			coord.row = (int)((pos.y - mapPos.y) / tilesize.height);

			return coord;
		}


	};

}