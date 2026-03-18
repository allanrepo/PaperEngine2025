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
#include <Engine/Factory/AnimationFactory.h>
#include "Actor.h"

namespace TestActorNavigation
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

	class Test
	{
		using AnimationSet = engine::graphics::animation::AnimationSet<engine::graphics::Sprite>;
		using AnimationController = engine::graphics::animation::AnimationController<engine::graphics::Sprite, Actor>;
		using AnimationFactory = engine::graphics::factory::AnimationFactory;

	private:
		std::unique_ptr<Window> m_window;
		std::unique_ptr<ICanvas> m_canvas;
		std::unique_ptr<IRenderer> m_renderer;

		StopWatch m_stopwatch;
		double m_elapsed;

		Input m_input;

		Position<int> m_mousePos;

		PathFinder m_pathFinder;
		Coord m_startTile;
		Coord m_endTile;
		std::vector<Coord> m_path;
		PositionF lastTargetPos;
		bool m_drawDebugGraph;

	public:
		Test():
			//m_pathFinder(
			//	[](int row, int col) -> bool
			//	{
			//		return Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile").Get(row, col)->IsWalkable();
			//	},
			//	nullptr,
			//	true,
			//	false
			//),
			m_pathFinder(
				std::make_unique<engine::navigation::tile::BinaryNavigationResolver>(
					//[this](int row, int col) -> engine::navigation::tile::TileConstraint
					//{
					//	return m_regionTD->Get(row, col)->GetConstraint();
					//}),
					[this](int row, int col) -> bool
					{
						return Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile").Get(row, col)->IsWalkable();
					}), 
				true
			),

			m_startTile({ 0,0 }),
			m_endTile({ 0,0 }),
			m_drawDebugGraph(true)
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

			// setup tilemap
			{
				// create sprite atlas to be used by tilemap
				SpriteAtlasFactory::Create("1x8_256x32_tile", L"../Assets/1x8_256x32_tile.png", 1, 8);
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("1x8_256x32_tile");

				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("1x8_256x32_tile", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("1x8_256x32_tile");

				// load sprites from sprite atlas into our tileset
				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), true)); // walkable
				tileset.Register(1, std::make_unique<RenderableTile>(atlas.MakeSprite(1), true)); // obstacle
				tileset.Register(2, std::make_unique<RenderableTile>(atlas.MakeSprite(2), true)); // obstacle
				tileset.Register(3, std::make_unique<RenderableTile>(atlas.MakeSprite(3), true)); // obstacle
				tileset.Register(4, std::make_unique<RenderableTile>(atlas.MakeSprite(4), false)); // walkable
				tileset.Register(5, std::make_unique<RenderableTile>(atlas.MakeSprite(5), true)); // obstacle
				tileset.Register(6, std::make_unique<RenderableTile>(atlas.MakeSprite(6), true)); // obstacle
				tileset.Register(7, std::make_unique<RenderableTile>(atlas.MakeSprite(7), true)); // obstacle

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("1x8_256x32_tile", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");

				// load tile region by filling it with all '0' tile
				Table<string> map({ 20, 12 }, "0");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile>{ return tileset.MakeTile(cell); });

				// set tile region's parameters
				Registry<SizeF>::Instance().Register("1x8_256x32_tile", make_unique<SizeF>(64.0f, 64.0f));
				Registry<PositionF>::Instance().Register("1x8_256x32_tile", make_unique<PositionF>(50.0f, 50.0f));
			}

			// setup actor
			{
				// create sprite atlas hero actor
				SpriteAtlasFactory::Create("actor", L"../Assets/CharacterTest_2304x1536_12x8.png", 8, 12);
				ISpriteAtlas& atlas = cache::Registry<ISpriteAtlas>::Instance().Get("actor");

				// create animation set for actor and store in registry
				Registry<AnimationSet>::Instance().Register("dust", std::make_unique<AnimationSet>());
				AnimationSet& animset = Registry<AnimationSet>::Instance().Get("dust");

				// create actor animations and store in animation set
				animset.Register("idle right", AnimationFactory::Create(atlas, { 0, 1, 2, 3, 4, 5 }, 100, true, PositionF{ 0.5f, 0.65f }));
				animset.Register("idle left", AnimationFactory::Create(atlas, { 6, 7, 8, 9, 10, 11 }, 100, true, PositionF{ 0.5f, 0.65f }));
				animset.Register("walk right", AnimationFactory::Create(atlas, { 12, 13, 14, 15, 16, 17 }, 100, true, PositionF{ 0.5f, 0.65f }));
				animset.Register("walk left", AnimationFactory::Create(atlas, { 18, 19, 20, 21, 22, 23, }, 100, true, PositionF{ 0.5f, 0.65f }));

				// create actor. pass our animation manager. it will make a copy of it internally but will reference to same set of animations from animManager object
				Registry<Actor>::Instance().Register("actor", make_unique<Actor>(animset, "actor"));
				Actor& actor = Registry<Actor>::Instance().Get("actor");

				// get parameters of tilemap
				PositionF pos = Registry<PositionF>::Instance().Get("1x8_256x32_tile");
				SizeF tilesize = Registry<SizeF>::Instance().Get("1x8_256x32_tile");

				// calculate position at center of the tile where mouse position is
				PositionF target;
				target.x = pos.x + tilesize.width * m_startTile.col + tilesize.width / 2.0f;
				target.y = pos.y + tilesize.height * m_startTile.row + tilesize.height / 2.0f;

				// set actor default state
				actor.SetState<ActorIdleState>();
				actor.SetPosition(target);
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");
			Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("1x8_256x32_tile");

			switch (key)
			{
			case 27: // escape
				break;
			case 32: // space
				m_drawDebugGraph = !m_drawDebugGraph;
				break;
			case 49: // 1
			{
				// remove all obstacles
				for (int row = 0; row < region.GetHeight(); row++)
				{
					for (int col = 0; col < region.GetWidth(); col++)
					{
						if (!region.Get(row, col)->IsWalkable())
						{
							region.Set(row, col, tileset.MakeTile(0));
						}
					}
				}


				if (m_path.size())
				{
					PositionF target = GetCenterPositionFromTile(m_path[m_path.size() - 1]);
					PositionF curr = Registry<Actor>::Instance().Get("actor").GetPosition();
					UpdatePath(curr, target);
				}

				break;
			}
			case 50: // 2
			{
				// turn selected tile into blocked tile
				Coord coord;
				if (!GetTileFromPos(m_mousePos.x, m_mousePos.y, false, coord)) return;
				region.Set(coord.row, coord.col, tileset.MakeTile(4));

				if (m_path.size())
				{
					PositionF target = GetCenterPositionFromTile(m_path[m_path.size() - 1]);
					PositionF curr = Registry<Actor>::Instance().Get("actor").GetPosition();
					UpdatePath(curr, target);
				}

				break;
			}
			case 51: // 3
			{
				// turn selected tile into walkable tile
				Coord coord;
				if (!GetTileFromPos(m_mousePos.x, m_mousePos.y, false, coord)) return;
				region.Set(coord.row, coord.col, tileset.MakeTile(0));


				if (m_path.size())
				{
					PositionF target = GetCenterPositionFromTile(m_path[m_path.size() - 1]);
					PositionF curr = Registry<Actor>::Instance().Get("actor").GetPosition();
					UpdatePath(curr, target);
				}

				break;
			}

			default:
				break;
			}
		}

		PositionF GetCenterPositionFromTile(const Coord& coord)
		{
			// get parameters of tilemap
			PositionF pos = Registry<PositionF>::Instance().Get("1x8_256x32_tile");
			SizeF tilesize = Registry<SizeF>::Instance().Get("1x8_256x32_tile");

			// calculate position at center of the tile where mouse position is
			PositionF target;
			target.x = pos.x + tilesize.width * coord.col + tilesize.width / 2.0f;
			target.y = pos.y + tilesize.height * coord.row + tilesize.height / 2.0f;

			return target;
		}

		bool GetTileFromPos(int x, int y, bool checkWalkable, Coord& out)
		{
			// get parameters of tilemap
			PositionF pos = Registry<PositionF>::Instance().Get("1x8_256x32_tile");
			SizeF tilesize = Registry<SizeF>::Instance().Get("1x8_256x32_tile");

			// calculate the coordinate of tile that intersect wih mouse click position
			Coord coord;
			coord.col = (int)((x - pos.x) / tilesize.width);
			coord.row = (int)((y - pos.y) / tilesize.height);

			TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");

			// if row and col is out of bounds, bail out
			if (!region.IsInBounds(coord)) return false;
		
			// is target tile walkable?
			if (!region.Get(coord)->IsWalkable() && checkWalkable) return false;

			out.row = coord.row;
			out.col = coord.col;

			return true;
		}

		Coord PositionToMapCoord(const PositionF& pos)
		{
			// get parameters of tilemap
			PositionF mapPos = Registry<PositionF>::Instance().Get("1x8_256x32_tile");
			SizeF tilesize = Registry<SizeF>::Instance().Get("1x8_256x32_tile");

			// calculate the coordinate of tile that intersect wih mouse click position
			Coord coord;
			coord.col = (int)((pos.x - mapPos.x) / tilesize.width);
			coord.row = (int)((pos.y - mapPos.y) / tilesize.height);

			return coord;
		}

		bool GetTileFromPos(const PositionF& pos, bool checkWalkable, Coord& out)
		{
			// calculate the coordinate of tile that intersect wih mouse click position
			Coord coord = PositionToMapCoord(pos);

			// if row and col is out of bounds, bail out
			TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");
			if (!region.IsInBounds(coord)) return false;

			// is target tile walkable?
			if (!region.Get(coord)->IsWalkable() && checkWalkable) return false;

			out = coord;

			return true;
		}

		bool UpdatePath(const PositionF& curr, const PositionF& target)
		{
			// check first if target tile is walkable. bail out if not.
			Coord targetTile;
			if (!GetTileFromPos(target, false, targetTile)) return false;

			// get tile where current position is located
			Coord currTile;
			if (!GetTileFromPos(curr, false, currTile)) return false;

			// get region extents of our tilemap. we will feed it into path finder			
			TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");
			math::geometry::Rect<int> map = { 0, 0, (int)region.GetWidth(), (int)region.GetHeight() };

			// find path
			vector<Coord> path;
			if (m_pathFinder.FindPath(map, currTile, targetTile, path))
			{
				m_startTile = currTile;
				m_endTile = targetTile;
				m_path = path;

				// if we found a path, get smoothed way points
				vector<Coord> wp = SmoothWayPoints(path, [&region](int row, int col){ return region.IsInBounds(row, col) ? region.Get(row, col)->IsWalkable() : false; });

				// get parameters of tilemap
				PositionF pos = Registry<PositionF>::Instance().Get("1x8_256x32_tile");
				SizeF tilesize = Registry<SizeF>::Instance().Get("1x8_256x32_tile");

				Actor& actor = Registry<Actor>::Instance().Get("actor");

				// first waypoint is always the tile where the actor currently is. so we just move from its position
				if (wp.size())
				{
					actor.FlushStates();
					actor.SetState<engine::state::ActorWalkToState>(actor.GetPosition(), 0.2f);

					if (wp.size() == 1)
					{

					}
				}
				//
				for (int i = 1; i < wp.size(); i++)
				{
					// calculate position at center of the tile where mouse position is
					PositionF target;
					target.x = pos.x + tilesize.width * wp[i].col + tilesize.width / 2.0f;
					target.y = pos.y + tilesize.height * wp[i].row + tilesize.height / 2.0f;

					actor.QueueState<engine::state::ActorWalkToState>(target, 0.2f);

					if (i == wp.size() - 1)
					{

					}
				}

				return true;
			}
			return false;
		}

		void OnMouseDown(int btn, int x, int y)
		{
			if(btn == 1)
			{
				Actor& actor = Registry<Actor>::Instance().Get("actor");
				UpdatePath(actor.GetPosition(), { (float)x, (float)y });
				return;
			}

		}

		void OnMouseMove(int x, int y)
		{
			m_mousePos = Position<int>(x, y);
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			Registry<Actor>::Instance().Get("actor").Update(time);
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

				// render tilemap
				{
					// get our tile region and make tilemap out of it
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("1x8_256x32_tile");
					SizeF tilesize = Registry<SizeF>::Instance().Get("1x8_256x32_tile");

					// draw tilemap 
					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
				}

				{
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x8_256x32_tile");
					PositionF pos = Registry<PositionF>::Instance().Get("1x8_256x32_tile");
					SizeF tilesize = Registry<SizeF>::Instance().Get("1x8_256x32_tile");

					if (m_drawDebugGraph)
					{
						vector<Coord> wp = SmoothWayPoints(m_path, [&region](int row, int col) { return region.IsInBounds(row, col) ? region.Get(row, col)->IsWalkable() : false; });
						DrawWaypoints(*m_renderer, wp, tilesize, pos, { 1,0,0,1 }, 6.0f);

						wp = GetWayPoints(m_path);
						DrawWaypoints(*m_renderer, wp, tilesize, pos, { 1,1,1,1 }, 2.0f);
					}
				}
				 
				// render actor
				{
					Actor& actor = Registry<Actor>::Instance().Get("actor");
					m_renderer->Draw(actor.GetSprite(), actor.GetPosition(), actor.GetSprite().GetSize(), { 1,1,1,1 }, 0);
					//m_renderer->Draw(actor.GetSprite(), actor.GetPosition(), {500, 500 }, {1,1,1,1}, 0);

					if(m_drawDebugGraph) m_renderer->Draw(actor.GetPosition() - PositionF{ 4, 4 }, { 8, 8 }, { 1,0,0,1 }, 0);
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