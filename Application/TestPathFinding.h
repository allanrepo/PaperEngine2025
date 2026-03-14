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

using namespace engine;
using namespace engine::graphics;

namespace TestPathFinding
{
	class RenderableTile
	{
	private:
		engine::graphics::Sprite m_sprite;
		bool m_walkable;

	public:
		RenderableTile(const engine::graphics::Sprite& sprite, bool walkable) :
			m_sprite(sprite),
			m_walkable(walkable)
		{
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


	// this is a tile definition class, not exactly tile class. tile class is Tile<T> and this is what is assigned to T
	class TileDefinition
	{
	public:
	private:
		engine::graphics::animation::Animator<engine::graphics::Sprite> m_animator;
		std::unordered_map<std::string, engine::graphics::animation::Animation<engine::graphics::Sprite>> m_animations;
		engine::navigation::tile::TileConstraint m_mask;

	public:
		TileDefinition(const std::string& name, const engine::graphics::animation::Animation<engine::graphics::Sprite>& anim)
		{
			// copy the animation into our container
			m_animations[name] = anim;

			// assign the animation from our container into animator (don't assign the passed animation. that is reference to animation outside which is not safe
			m_animator.Play(m_animations[name]);
		}

		TileDefinition(const std::string& name, const engine::graphics::Sprite& sprite, engine::navigation::tile::TileConstraint mask):
			m_mask(mask)
		{
			engine::graphics::animation::Animation<engine::graphics::Sprite> anim;

			// we're hardcoding a fixed duration because we don't intend to animate this. only the passed sprite will be rendered all the time. it is static.
			anim.frames.push_back({ sprite, 1000.0f });

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

		bool IsWalkable() const
		{
			return m_mask == engine::navigation::tile::TileConstraint::NONE;
		}

		bool IsBlocked() const
		{
			return ((m_mask & engine::navigation::tile::TileConstraint::CENTER) != engine::navigation::tile::TileConstraint::NONE);
		}

		engine::navigation::tile::TileConstraint GetConstraint() const
		{
			return m_mask;
		}
	};

	class Test
	{
	private:
		std::unique_ptr<win32::Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<renderer::IRenderer> m_rendererBatch;

		input::Input m_input;
		std::unique_ptr<resource::IFontAtlas> m_FontAtlas;

		timer::StopWatch m_stopwatch;
		double m_elapsed;

		engine::spatial::PositionF m_pos;
		engine::spatial::SizeF m_tilesize;

		
		std::unique_ptr<engine::component::tile::TileRegion<TileDefinition>> m_regionTD;
		engine::navigation::tile::PathFinder m_pathFinder;
		engine::spatial::Coord m_startTile;
		engine::spatial::Coord m_goalTile;

		std::vector<engine::spatial::Coord> m_path;

		engine::navigation::tile::PathFinder m_pathFinder1;

		bool m_useTileNavigationResolver;

		std::unique_ptr<engine::component::tile::TileRegion<RenderableTile>> m_regionRT;


	public:

#if 0
		std::function<bool(const engine::spatial::Coord&, const engine::spatial::Coord&)> CanMoveDiagonally =
			[this](const engine::spatial::Coord& curr, const engine::spatial::Coord& next) -> bool
			{
				enum Direction
				{
					NE,
					SE,
					NW,
					SW,
					NONE
				};

				// get direction 
				engine::math::Vec<int> dir;
				dir.y = next.row - curr.row;
				dir.x = next.col - curr.col;

				Direction direction;
				direction =
					dir.y > 0 && dir.x > 0 ? SE :
					dir.y < 0 && dir.x < 0 ? NW :
					dir.y < 0 && dir.x > 0 ? NE :
					dir.y > 0 && dir.x < 0 ? SW :
					NONE;

				// if not diagonal, just do a regular compare if block or not
				if (dir.y == 0 || dir.x == 0)
				{
					return m_region->Get(next)->IsWalkable();
				}

				// get adjacent tiles bitmask
				TileDesc CRNC = m_region->Get(curr.row, next.col)->GetMask();
				TileDesc NRCC = m_region->Get(next.row, curr.col)->GetMask();

				//// tile descriptor
				//enum class TileDesc: int
				//{						// NWSE
				//	BLOCKED	= 0b0000,	// 0000 -> clear
				//	CLEAR	= 0b1111,	// 1111 -> blocked
				//	NW		= 0b1100,	// 1100 -> NW blocked half diagonal
				//	SE		= 0b0011,	// 0011 -> SE blocked half diagonal
				//	NE		= 0b1001,	// 1001 -> NE blocked half diagonal
				//	SW		= 0b0110	// 0110 -> SW blocked half diagonal
				//};

				if (CRNC ==TileDefinition::NONE && NRCC ==TileDefinition::NONE)
				{
					return true;
				}
				if (direction == Direction::SE && CRNC == TileDesc::NE && NRCC == TileDesc::SW)
				{
					return true;
				}
				else if (direction == Direction::NW && CRNC == TileDesc::SW && NRCC == TileDesc::NE)
				{
					return true;
				}
				else if (direction == Direction::NE && CRNC == TileDesc::SE && NRCC == TileDesc::NW)
				{
					return true;
				}
				else if (direction == Direction::SW && CRNC == TileDesc::NW && NRCC == TileDesc::SE)
				{
					return true;
				}
				else
				{
					return false;
				}
			};
#endif

		bool matchesPattern(unsigned int mask, unsigned int pattern) 
		{
			// BLOCKED = 0b111111111 (all 9 bits set)
			//return (mask & (~pattern & TileDefinition::BLOCKED)) == 0;
			return false;
		}

		Test() :
			m_pos(50, 50),
			m_tilesize(32, 32),
			m_startTile({ 0,0 }),
			m_goalTile({ 0,0 }),
			m_pathFinder1(
				std::make_unique<engine::navigation::tile::TileNavigationResolver>(
					[this](int row, int col) -> engine::navigation::tile::TileConstraint
					{
						return m_regionTD->Get(row, col)->GetConstraint();
					}),
				true
			),
			m_pathFinder(
				std::make_unique<engine::navigation::tile::BinaryNavigationResolver>(
					[this](int row, int col) -> bool
					{
						return m_regionRT->Get(row, col)->IsWalkable();
					}), 
				true
			),
			m_useTileNavigationResolver(false)
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
			m_window->Create(L"Test Font", 1400, 900);
			m_window->OnWindowMessage += event::Handler(&m_input, &input::Input::ProcessWin32Message);

			m_input.KeyDownEvent += event::Handler(this, &Test::OnKeyDown);
			m_input.MouseDownEvent += event::Handler(this, &Test::OnMouseDown);
			m_input.MouseMoveEvent += event::Handler(this, &Test::OnMouseMove);
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
			m_rendererBatch = std::make_unique<renderer::Renderer>(std::make_unique<dx11::renderer::DX11RendererBatchImpl>());
			m_rendererBatch->Initialize();
			LOG("Renderer Batch (DX11) created...");

			// create font
			m_FontAtlas = std::make_unique<resource::FontAtlas>(std::make_unique<resource::SpriteAtlas>(std::make_unique<dx11::resource::DX11TextureImpl>()));
			m_FontAtlas->Initialize("Terminal", 12);

			// create sprite atlas
			{
				// create sprite atlas to be used by tilemap
				engine::graphics::factory::SpriteAtlasFactory::Create("12x2_384x64_tile", L"../Assets/12x2_384x64_tile.png", 2, 12);
			}

			// create tilemap with TileDefinition 
			{
				// create tileset and store in cache
				engine::cache::Registry<engine::component::tile::Tileset<TileDefinition>>::Instance().Register("TileDefinition", std::make_unique<engine::component::tile::Tileset<TileDefinition>>());

				// define tiles
				engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("12x2_384x64_tile");
				engine::component::tile::Tileset<TileDefinition>& tileset = engine::cache::Registry<engine::component::tile::Tileset<TileDefinition>>::Instance().Get("TileDefinition");

				tileset.Register(0, std::make_unique<TileDefinition>("td", atlas.MakeSprite(0), engine::navigation::tile::TileConstraint::NONE)); // walkable
				tileset.Register(1, std::make_unique<TileDefinition>("td", atlas.MakeSprite(1), engine::navigation::tile::TileConstraint::NONE)); // obstacle
				tileset.Register(2, std::make_unique<TileDefinition>("td", atlas.MakeSprite(2), engine::navigation::tile::TileConstraint::NONE)); // obstacle
				tileset.Register(3, std::make_unique<TileDefinition>("td", atlas.MakeSprite(3), engine::navigation::tile::TileConstraint::NONE)); // obstacle
				tileset.Register(4, std::make_unique<TileDefinition>("td", atlas.MakeSprite(4), engine::navigation::tile::TileConstraint::BLOCKED)); // walkable
				tileset.Register(5, std::make_unique<TileDefinition>("td", atlas.MakeSprite(5), engine::navigation::tile::TileConstraint::NONE)); // obstacle
				tileset.Register(6, std::make_unique<TileDefinition>("td", atlas.MakeSprite(6), engine::navigation::tile::TileConstraint::NONE)); // obstacle
				tileset.Register(7, std::make_unique<TileDefinition>("td", atlas.MakeSprite(7), engine::navigation::tile::TileConstraint::NONE)); // obstacle
				tileset.Register(8, std::make_unique<TileDefinition>("td", atlas.MakeSprite(8), engine::navigation::tile::TileConstraint::SE_HALFTRI)); // obstacle
				tileset.Register(9, std::make_unique<TileDefinition>("td", atlas.MakeSprite(9), engine::navigation::tile::TileConstraint::SW_HALFTRI)); // obstacle
				tileset.Register(10, std::make_unique<TileDefinition>("td", atlas.MakeSprite(10), engine::navigation::tile::TileConstraint::NE_HALFTRI)); // obstacle
				tileset.Register(11, std::make_unique<TileDefinition>("td", atlas.MakeSprite(11), engine::navigation::tile::TileConstraint::NW_HALFTRI)); // obstacle
				tileset.Register(12, std::make_unique<TileDefinition>("td", atlas.MakeSprite(12), engine::navigation::tile::TileConstraint::W_WALL)); // obstacle
				tileset.Register(13, std::make_unique<TileDefinition>("td", atlas.MakeSprite(13), engine::navigation::tile::TileConstraint::S_WALL)); // obstacle
				tileset.Register(14, std::make_unique<TileDefinition>("td", atlas.MakeSprite(14), engine::navigation::tile::TileConstraint::E_WALL)); // obstacle
				tileset.Register(15, std::make_unique<TileDefinition>("td", atlas.MakeSprite(15), engine::navigation::tile::TileConstraint::N_WALL)); // obstacle
				tileset.Register(16, std::make_unique<TileDefinition>("td", atlas.MakeSprite(16), engine::navigation::tile::TileConstraint::NW)); // obstacle
				tileset.Register(17, std::make_unique<TileDefinition>("td", atlas.MakeSprite(17), engine::navigation::tile::TileConstraint::SW)); // obstacle
				tileset.Register(18, std::make_unique<TileDefinition>("td", atlas.MakeSprite(18), engine::navigation::tile::TileConstraint::SE)); // obstacle
				tileset.Register(19, std::make_unique<TileDefinition>("td", atlas.MakeSprite(19), engine::navigation::tile::TileConstraint::NE)); // obstacle
				tileset.Register(20, std::make_unique<TileDefinition>("td", atlas.MakeSprite(20), engine::navigation::tile::TileConstraint::CENTER)); // obstacle

				// create table filled with 0's
				engine::container::Table<std::string> map({ 24, 16 }, "0");

				// create tile region
				m_regionTD = std::make_unique<engine::component::tile::TileRegion<TileDefinition>>();

				// load tile region with our table data
				engine::loader::tile::AsyncTileRegionLoader<TileDefinition, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(*m_regionTD, map,
					[this, &tileset](const int& cell) -> engine::component::tile::Tile<TileDefinition>
					{
						return tileset.MakeTile(cell);
					});	

				m_goalTile.row = (int)m_regionTD->GetHeight() - 1;
				m_goalTile.col = (int)m_regionTD->GetWidth() - 1;

				//m_regionTD->Set(m_startTile.row, m_startTile.col, tileset.MakeTile(1));
				//m_regionTD->Set(m_goalTile.row, m_goalTile.col, tileset.MakeTile(6));

				m_regionTD->Set(8, 4, tileset.MakeTile(8));
				m_regionTD->Set(7, 3, tileset.MakeTile(11));
				m_regionTD->Set(5, 15, tileset.MakeTile(11));

				m_regionTD->Set(11, 12, tileset.MakeTile(9));
				m_regionTD->Set(10, 13, tileset.MakeTile(10));
				m_regionTD->Set(10, 11, tileset.MakeTile(9));
				m_regionTD->Set( 9, 12, tileset.MakeTile(10));
				m_regionTD->Set( 9, 10, tileset.MakeTile(9));
				m_regionTD->Set( 8, 11, tileset.MakeTile(10));

				m_regionTD->Set(13, 2, tileset.MakeTile(12));
				m_regionTD->Set(13, 5, tileset.MakeTile(13));
				m_regionTD->Set(13, 8, tileset.MakeTile(14));
				m_regionTD->Set(13, 11, tileset.MakeTile(15));


				//m_regionTD->Set(15, 21, tileset.MakeTile(4));
				//m_regionTD->Set(14, 21, tileset.MakeTile(4));
				//m_regionTD->Set(13, 21, tileset.MakeTile(4));
				//m_regionTD->Set(12, 21, tileset.MakeTile(4));
				//m_regionTD->Set(11, 21, tileset.MakeTile(4));
				//m_regionTD->Set(10, 21, tileset.MakeTile(4));
				//m_regionTD->Set(9, 21, tileset.MakeTile(4));
				//m_regionTD->Set(8, 21, tileset.MakeTile(4));
				m_regionTD->Set(3, 21, tileset.MakeTile(20));
				m_regionTD->Set(6, 21, tileset.MakeTile(19));
				m_regionTD->Set(9, 21, tileset.MakeTile(18));
				m_regionTD->Set(11, 21, tileset.MakeTile(17));
				m_regionTD->Set(14, 21, tileset.MakeTile(16));
			}

			// create tilemap with RenderableTile 
			{
				// create tileset
				engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Register("RenderableTile", std::make_unique<engine::component::tile::Tileset<RenderableTile>>());
				engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("RenderableTile");

				// get our sprite atlas
				engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get("12x2_384x64_tile");

				// load tiles
				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), true)); // walkable
				tileset.Register(1, std::make_unique<RenderableTile>(atlas.MakeSprite(1), true)); // obstacle
				tileset.Register(2, std::make_unique<RenderableTile>(atlas.MakeSprite(2), true)); // obstacle
				tileset.Register(3, std::make_unique<RenderableTile>(atlas.MakeSprite(3), true)); // obstacle
				tileset.Register(4, std::make_unique<RenderableTile>(atlas.MakeSprite(4), false)); // walkable
				tileset.Register(5, std::make_unique<RenderableTile>(atlas.MakeSprite(5), true)); // obstacle
				tileset.Register(6, std::make_unique<RenderableTile>(atlas.MakeSprite(6), true)); // obstacle
				tileset.Register(7, std::make_unique<RenderableTile>(atlas.MakeSprite(7), true)); // obstacle
				tileset.Register(8, std::make_unique<RenderableTile>(atlas.MakeSprite(8), false)); // obstacle
				tileset.Register(9, std::make_unique<RenderableTile>(atlas.MakeSprite(9), false)); // obstacle
				tileset.Register(10, std::make_unique<RenderableTile>(atlas.MakeSprite(10), false)); // obstacle
				tileset.Register(11, std::make_unique<RenderableTile>(atlas.MakeSprite(11), false)); // obstacle

				// create table filled with 0's
				engine::container::Table<std::string> map({ 24, 16 }, "7");

				// create tile region
				m_regionRT = std::make_unique<engine::component::tile::TileRegion<RenderableTile>>();

				// load tile region with our table data
				engine::loader::tile::AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(*m_regionRT, map,
					[this, &tileset](const int& cell) -> engine::component::tile::Tile<RenderableTile>
					{
						return tileset.MakeTile(cell);
					});

				//m_regionRT->Set(m_startTile.row, m_startTile.col, tileset.MakeTile(1));
				//m_regionRT->Set(m_goalTile.row, m_goalTile.col, tileset.MakeTile(6));

				m_regionRT->Set(15, 21, tileset.MakeTile(4));
				m_regionRT->Set(14, 21, tileset.MakeTile(4));
				m_regionRT->Set(13, 21, tileset.MakeTile(4));
				m_regionRT->Set(12, 21, tileset.MakeTile(4));
				m_regionRT->Set(11, 21, tileset.MakeTile(4));
				m_regionRT->Set(10, 21, tileset.MakeTile(4));
				m_regionRT->Set(9, 21, tileset.MakeTile(4));
				m_regionRT->Set(8, 21, tileset.MakeTile(4));
				m_regionRT->Set(7, 21, tileset.MakeTile(4));
				m_regionRT->Set(6, 21, tileset.MakeTile(4));
				m_regionRT->Set(5, 21, tileset.MakeTile(4));
				m_regionRT->Set(4, 21, tileset.MakeTile(4));
				m_regionRT->Set(3, 21, tileset.MakeTile(4));
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
				m_useTileNavigationResolver = !m_useTileNavigationResolver;
				//m_pathFinder.SetMaxSteps(m_pathFinder.GetMaxSteps() + 1);
				break;
			case 49: // 1
				if (m_useTileNavigationResolver)
				{
					engine::component::tile::Tileset<TileDefinition>& tileset = engine::cache::Registry<engine::component::tile::Tileset<TileDefinition>>::Instance().Get("TileDefinition");

					// remove all obstacles
					for (int row = 0; row < m_regionTD->GetHeight(); row++)
					{
						for (int col = 0; col < m_regionTD->GetWidth(); col++)
						{
							if (!m_regionTD->Get(row, col)->IsWalkable())
							{
								m_regionTD->Set(row, col, tileset.MakeTile(0));
							}
						}
					}
				}
				else
				{
					engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("RenderableTile");
					// remove all obstacles
					for (int row = 0; row < m_regionRT->GetHeight(); row++)
					{
						for (int col = 0; col < m_regionRT->GetWidth(); col++)
						{
							if (!m_regionRT->Get(row, col)->IsWalkable())
							{
								m_regionRT->Set(row, col, tileset.MakeTile(7));
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			// get selected tile row and col based on mouse position
			int col = (int)((x - m_pos.x) / m_tilesize.width);
			int row = (int)((y - m_pos.y) / m_tilesize.height);

			if (m_useTileNavigationResolver)
			{
				// get map size
				engine::spatial::Size<size_t> size = m_regionTD->GetSize();

				// if row and col is out of bounds, bail out
				if (col >= size.width || row >= size.height) return;

				// skip if this tile is block tile
				if (m_regionTD->Get(row, col)->IsBlocked()) return;

				// reset the map to all walkable
				engine::component::tile::Tileset<TileDefinition>& tileset = engine::cache::Registry<engine::component::tile::Tileset<TileDefinition>>::Instance().Get("TileDefinition");

				if (btn == 1)
				{
					// reset old start tile
					//m_regionTD->Set(m_startTile.row, m_startTile.col, tileset.MakeTile(0));

					m_startTile.row = row;
					m_startTile.col = col;

					// replace start tile
					//m_regionTD->Set(row, col, tileset.MakeTile(1));
				}
				else if (btn == 2)
				{
					// reset old goal tile
					//m_regionTD->Set(m_goalTile.row, m_goalTile.col, tileset.MakeTile(0));

					m_goalTile.row = row;
					m_goalTile.col = col;

					// replace goal tile 
					//m_regionTD->Set(row, col, tileset.MakeTile(6));

				}
				else if (btn == 3)
				{
					// set tile to blocked
					m_regionTD->Set(row, col, tileset.MakeTile(4));

				}

			}
			else
			{
				// get map size
				engine::spatial::Size<size_t> size = m_regionRT->GetSize();

				// if row and col is out of bounds, bail out
				if (col >= size.width || row >= size.height) return;

				// skip if this tile is block tile
				if (!m_regionRT->Get(row, col)->IsWalkable()) return;

				// reset the map to all walkable
				engine::component::tile::Tileset<RenderableTile>& tileset = engine::cache::Registry<engine::component::tile::Tileset<RenderableTile>>::Instance().Get("RenderableTile");

				if (btn == 1)
				{
					// reset old start tile
					//m_regionRT->Set(m_startTile.row, m_startTile.col, tileset.MakeTile(7));

					m_startTile.row = row;
					m_startTile.col = col;

					// replace start tile
					//m_regionRT->Set(row, col, tileset.MakeTile(1));
				}
				else if (btn == 2)
				{
					// reset old goal tile
					//m_regionRT->Set(m_goalTile.row, m_goalTile.col, tileset.MakeTile(7));

					m_goalTile.row = row;
					m_goalTile.col = col;

				}
				else if (btn == 3)
				{
					// set tile to blocked
					m_regionRT->Set(row, col, tileset.MakeTile(4));

				}
			}

			return;
		}

		void OnMouseMove(int x, int y)
		{

		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			// save current path
			std::vector<engine::spatial::Coord> m_prevPath = m_path;

			if (m_useTileNavigationResolver)
			{
				// find path
				math::geometry::Rect<int> map = { 0, 0, (int)m_regionTD->GetWidth(), (int)m_regionTD->GetHeight() };
				m_pathFinder1.FindPath(
					map,
					m_startTile,
					m_goalTile,
					m_path
				);
			}
			else
			{
				// find path
				math::geometry::Rect<int> map = { 0, 0, (int)m_regionRT->GetWidth(), (int)m_regionRT->GetHeight() };
				m_pathFinder.FindPath(
					map,
					m_startTile,
					m_goalTile,
					m_path
				);
			}


		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			m_input.Update();

			m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_rendererBatch->Begin();

				if (m_useTileNavigationResolver)
				{
					// get the tile region
					engine::component::tile::TileMap<TileDefinition> tilemap = m_regionTD->MakeTileMap();

					// draw the tile region
					engine::graphics::tile::DrawTileMap(*m_rendererBatch, tilemap, m_tilesize, m_pos, { 1,1,1,1 });

					DrawNavigation(*m_rendererBatch, tilemap, m_tilesize, m_pos, { 0,1,1,1 }, m_startTile, { 0.7f, 0.7f });
					DrawNavigation(*m_rendererBatch, tilemap, m_tilesize, m_pos, { 1,0,0,1 }, m_goalTile, { 0.6f, 0.6f });

					std::vector<engine::spatial::Coord> wp = engine::navigation::tile::GetWayPoints(m_path);
					engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 1,1,1,1 }, 6.0f);

					wp = engine::navigation::tile::SmoothWayPoints(wp, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
					engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 0,1,0,1 }, 4.0f);

					wp = engine::navigation::tile::SmoothWayPoints(m_path, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
					engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 1,0,1,1 }, 2.0f);


					std::string str = "Closed Tiles: " + std::to_string(m_pathFinder1.GetClosedTiles().size());
					m_rendererBatch->Draw(*m_FontAtlas, str, { 1000, 10 }, { 1,1,1,1 });
					str.clear();
					str = "Open Tiles: " + std::to_string(m_pathFinder1.GetOpenTiles().size());
					m_rendererBatch->Draw(*m_FontAtlas, str, { 1000, 30 }, { 1,1,1,1 });

				}
				// draw RenderableTile region
				else
				{
					engine::component::tile::TileMap<RenderableTile> tilemap = m_regionRT->MakeTileMap();

					engine::graphics::tile::DrawTileMap(*m_rendererBatch, tilemap, m_tilesize, m_pos, { 1,1,1,1 });

					DrawNavigation(*m_rendererBatch, tilemap, m_tilesize, m_pos, { 0,1,1,1 }, m_startTile, { 0.7f, 0.7f });
					DrawNavigation(*m_rendererBatch, tilemap, m_tilesize, m_pos, { 1,0,0,1 }, m_goalTile, { 0.6f, 0.6f });

					std::vector<engine::spatial::Coord> wp = engine::navigation::tile::GetWayPoints(m_path);
					engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 1,1,1,1 }, 6.0f);

					wp = engine::navigation::tile::SmoothWayPoints(wp, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
					engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 0,1,0,1 }, 4.0f);

					wp = engine::navigation::tile::SmoothWayPoints(m_path, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
					engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 1,0,1,1 }, 2.0f);

					std::string str = "Closed Tiles: " + std::to_string(m_pathFinder.GetClosedTiles().size());
					m_rendererBatch->Draw(*m_FontAtlas, str, { 1000, 10 }, { 1,1,1,1 });
					str.clear();
					str = "Open Tiles: " + std::to_string(m_pathFinder.GetOpenTiles().size());
					m_rendererBatch->Draw(*m_FontAtlas, str, { 1000, 30 }, { 1,1,1,1 });
				}


								
				m_rendererBatch->End();
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


		template<typename T>
		void DrawNavigation(
			engine::graphics::renderer::IRenderer& renderer,
			const engine::component::tile::TileMap<T>& tilemap,
			const engine::spatial::SizeF& tilesize,
			const engine::spatial::PositionF& pos,
			const engine::graphics::ColorF& color,
			const engine::spatial::Coord& coord,
			const math::VecF scale
		)
		{
			if (!tilemap.IsInBounds(coord.row, coord.col))
			{
				return;
			}

			engine::spatial::PositionF origin =
			{
				coord.col * tilesize.width,
				coord.row * tilesize.height
			};

			engine::spatial::SizeF size = tilesize;
			size.width *= scale.x;
			size.height *= scale.y;

			math::VecF shift;

			shift.x = (tilesize.width - size.width) / 2.0f;
			shift.y = (tilesize.height - size.height) / 2.0f;

			renderer.Draw(pos + origin + shift, size, color, 0.0f);
		}
	};
}