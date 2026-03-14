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
	namespace debug
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
			std::vector<DrawInfo> m_batch;

		public:
			DrawTileMapCommand(
				engine::graphics::renderer::IRenderer& renderer,
				const engine::spatial::PositionF& pos,
				const spatial::SizeF& tilesize
			) :
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


		class ILogic
		{
		public:
			virtual const engine::graphics::Sprite GetSprite() const = 0;
			virtual bool IsValid() const = 0;
			virtual engine::navigation::tile::TileConstraint GetConstraint() const = 0;
			virtual void Append(engine::navigation::tile::TileConstraint constraint) = 0;
			virtual void Set(engine::navigation::tile::TileConstraint constraint) = 0;
		};

		class Logic : public ILogic
		{
		private:
			engine::graphics::Sprite m_sprite;
			engine::navigation::tile::TileConstraint m_constraint;

		public:
			Logic(const engine::graphics::Sprite& sprite, const engine::navigation::tile::TileConstraint constraint) :
				m_sprite(sprite),
				m_constraint(constraint)
			{
			}

			engine::navigation::tile::TileConstraint GetConstraint() const
			{
				return m_constraint;
			}

			const engine::graphics::Sprite GetSprite() const override
			{
				return m_sprite;
			}

			bool IsValid() const override
			{
				return m_sprite.IsValid();
			}

			virtual void Append(engine::navigation::tile::TileConstraint constraint) override
			{
				m_constraint |= constraint;
			}

			virtual void Set(engine::navigation::tile::TileConstraint constraint) override
			{
				m_constraint = constraint;
			}


		};

		class LogicHandle : public ILogic
		{
		private:
			core::Handle<ILogic> m_handle;

		public:
			// use this constructor if you have the sprite atlas and the source rect
			LogicHandle(ILogic* logic) :
				m_handle(logic)
			{
			}

			LogicHandle() :
				m_handle(nullptr)
			{
			}

		public:
			~LogicHandle() = default;

			bool IsValid() const override final
			{
				return m_handle.IsValid();
			}

			const engine::graphics::Sprite GetSprite() const  override final
			{
				return m_handle->GetSprite();
			}

			engine::navigation::tile::TileConstraint GetConstraint() const
			{
				return m_handle->GetConstraint();
			}

			virtual void Append(engine::navigation::tile::TileConstraint constraint) 
			{
				m_handle->Append(constraint);
			}
			virtual void Set(engine::navigation::tile::TileConstraint constraint)
			{
				m_handle->Set(constraint);
			}
		};


		class LogicSet: public engine::component::tile::Tileset<LogicHandle, engine::navigation::tile::TileConstraint>
		{
		protected:
		public:
			LogicSet() = default;
			~LogicSet() = default;
		};


		class ConstraintMap
		{
		private:
			engine::container::Grid<engine::navigation::tile::TileConstraint> m_grid;
			engine::navigation::tile::PathFinder m_pathFinder;

		public:
			ConstraintMap(size_t width = 0) :
				m_grid(width),
				m_pathFinder(
					std::make_unique<engine::navigation::tile::TileNavigationResolver>(
						[this](int row, int col) -> engine::navigation::tile::TileConstraint
						{
							return m_grid.Get(row, col);
						}),
					true
				)
			{
			}

			bool FindPath(const engine::spatial::Coord& start, const engine::spatial::Coord& end, std::vector<engine::spatial::Coord>& path)
			{
				math::geometry::Rect<int> map = { 0, 0, (int)m_grid.GetWidth(), (int)m_grid.GetHeight() };

				return m_pathFinder.FindPath(
					map,
					start,
					end,
					path
				);
			}

			void Initialize(size_t width, size_t height, engine::navigation::tile::TileConstraint constraint)
			{
				m_grid.Clear();
				m_grid.SetWidth(width);
				m_grid.Reserve({ width, height });

				for (size_t i = 0; i < width * height; ++i)
				{
					m_grid.Add(constraint);
				}
			}

			void Initialize(engine::spatial::Size<size_t> size, engine::navigation::tile::TileConstraint constraint)
			{
				Initialize(size.width, size.height, constraint);
			}

			void Set(engine::navigation::tile::TileConstraint constraint)
			{
				for (int row = 0; row < m_grid.GetHeight(); row++)
				{
					for (int col = 0; col < m_grid.GetWidth(); col++)
					{
						m_grid.Set(row, col, constraint);
					}
				}
			}

			engine::navigation::tile::TileConstraint Get(int row, int col)
			{
				return m_grid.Get(row, col);
			}

			void Set(int row, int col, engine::navigation::tile::TileConstraint constraint)
			{
				m_grid.Set(row, col, constraint);
			}

			void Set(const engine::spatial::Coord& coord, engine::navigation::tile::TileConstraint constraint)
			{
				m_grid.Set(coord, constraint);

			}
			bool IsInBounds(int row, int col) const
			{
				return m_grid.IsInBounds(row, col);
			}

			bool IsInBounds(const engine::spatial::Coord& coord) const
			{
				return m_grid.IsInBounds(coord);
			}

			void Append(int row, int col, engine::navigation::tile::TileConstraint constraint)
			{
				m_grid.Set(row, col, m_grid.Get(row, col) | constraint);
			}

			void Append(const engine::spatial::Coord& coord, engine::navigation::tile::TileConstraint constraint)
			{
				m_grid.Set(coord, m_grid.Get(coord) | constraint);
			}

			bool HasFlag(int row, int col, engine::navigation::tile::TileConstraint constraint)
			{
				return (m_grid.Get(row, col) | constraint) != engine::navigation::tile::TileConstraint::NONE;
			}

			engine::spatial::Size<size_t> GetSize() const
			{
				return m_grid.GetSize();
			}
		};

		void Queue(
			DrawQueue& queue,
			const engine::spatial::Coord& coord,
			const engine::spatial::SizeF& tilesize,
			const engine::spatial::PositionF& pos,
			const engine::graphics::Sprite sprite,
			const engine::graphics::ColorF& tint = { 1,1,1,1 }
		)
		{
			engine::spatial::PositionF origin =
			{
				coord.col * tilesize.width,
				coord.row * tilesize.height
			};
			queue.Add({ sprite, pos + origin, tilesize, tint, 0.0f,	69 });
		}

		void Queue(
			ConstraintMap& map,
			DrawQueue& queue,
			const engine::graphics::resource::ISpriteAtlas& atlas,
			const engine::spatial::SizeF& tilesize,
			const engine::spatial::PositionF& pos,
			float depth,
			const engine::graphics::ColorF& tint = { 1,1,1,1 }
		)
		{
			engine::spatial::Size<size_t> size = map.GetSize();

			for (int row = 0; row < size.height; row++)
			{
				for (int col = 0; col < size.width; col++)
				{
					//if (!map.IsInBounds(row, col)) continue;

					engine::navigation::tile::TileConstraint constraint = map.Get(row, col);


					engine::spatial::PositionF origin =
					{
						col * tilesize.width,
						row * tilesize.height
					};

					if (constraint == engine::navigation::tile::TileConstraint::NONE)
					{
						continue;
					}

					// if constraint is block, set block tile and bail. 
					if ((constraint & engine::navigation::tile::TileConstraint::BLOCKED) == engine::navigation::tile::TileConstraint::BLOCKED)
					{
						//queue.Add({atlas.MakeSprite(4), pos + origin, tilesize, tint, 0.0f,	depth});
						continue;
					}

					engine::navigation::tile::TileConstraint queued = engine::navigation::tile::TileConstraint::NONE;

					bool drawCenterIfNeeded = true;
					if ((constraint & engine::navigation::tile::TileConstraint::SE_HALFTRI) == engine::navigation::tile::TileConstraint::SE_HALFTRI)
					{
						queue.Add({ atlas.MakeSprite(8), pos + origin, tilesize, tint, 0.0f,	depth });
						queued |= engine::navigation::tile::TileConstraint::SE_HALFTRI;
						drawCenterIfNeeded = false;
					}

					if ((constraint & engine::navigation::tile::TileConstraint::SW_HALFTRI) == engine::navigation::tile::TileConstraint::SW_HALFTRI)
					{
						queue.Add({ atlas.MakeSprite(9), pos + origin, tilesize, tint, 0.0f,	depth });
						queued |= engine::navigation::tile::TileConstraint::SW_HALFTRI;
						drawCenterIfNeeded = false;
					}

					if ((constraint & engine::navigation::tile::TileConstraint::NE_HALFTRI) == engine::navigation::tile::TileConstraint::NE_HALFTRI)
					{
						queue.Add({ atlas.MakeSprite(10), pos + origin, tilesize, tint, 0.0f,	depth });
						queued |= engine::navigation::tile::TileConstraint::NE_HALFTRI;
						drawCenterIfNeeded = false;
					}

					if ((constraint & engine::navigation::tile::TileConstraint::NW_HALFTRI) == engine::navigation::tile::TileConstraint::NW_HALFTRI)
					{
						queue.Add({ atlas.MakeSprite(11), pos + origin, tilesize, tint, 0.0f,	depth });
						queued |= engine::navigation::tile::TileConstraint::NW_HALFTRI;
						drawCenterIfNeeded = false;
					}

					// if west wall is blocked
					if ((constraint & engine::navigation::tile::TileConstraint::W_WALL) == engine::navigation::tile::TileConstraint::W_WALL)
					{
						if ((queued & engine::navigation::tile::TileConstraint::W_WALL) != engine::navigation::tile::TileConstraint::W_WALL)
						{
							queue.Add({ atlas.MakeSprite(12), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::W_WALL;
						}
					}
					// if south wall is blocked
					if ((constraint & engine::navigation::tile::TileConstraint::S_WALL) == engine::navigation::tile::TileConstraint::S_WALL)
					{
						if ((queued & engine::navigation::tile::TileConstraint::S_WALL) != engine::navigation::tile::TileConstraint::S_WALL)
						{
							queue.Add({ atlas.MakeSprite(13), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::S_WALL;
						}
					}
					// if east wall is blocked
					if ((constraint & engine::navigation::tile::TileConstraint::E_WALL) == engine::navigation::tile::TileConstraint::E_WALL)
					{
						if ((queued & engine::navigation::tile::TileConstraint::E_WALL) != engine::navigation::tile::TileConstraint::E_WALL)
						{
							queue.Add({ atlas.MakeSprite(14), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::E_WALL;
						}
					}
					// if north wall is blocked
					if ((constraint & engine::navigation::tile::TileConstraint::N_WALL) == engine::navigation::tile::TileConstraint::N_WALL)
					{
						if ((queued & engine::navigation::tile::TileConstraint::N_WALL) != engine::navigation::tile::TileConstraint::N_WALL)
						{
							queue.Add({ atlas.MakeSprite(15), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::N_WALL;
						}
					}

					if ((constraint & engine::navigation::tile::TileConstraint::NW) == engine::navigation::tile::TileConstraint::NW)
					{
						if ((queued & engine::navigation::tile::TileConstraint::NW) != engine::navigation::tile::TileConstraint::NW)
						{
							queue.Add({ atlas.MakeSprite(16), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::NW;
						}
					}

					if ((constraint & engine::navigation::tile::TileConstraint::SW) == engine::navigation::tile::TileConstraint::SW)
					{
						if ((queued & engine::navigation::tile::TileConstraint::SW) != engine::navigation::tile::TileConstraint::SW)
						{
							queue.Add({ atlas.MakeSprite(17), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::SW;
						}
					}

					if ((constraint & engine::navigation::tile::TileConstraint::SE) == engine::navigation::tile::TileConstraint::SE)
					{
						if ((queued & engine::navigation::tile::TileConstraint::SE) != engine::navigation::tile::TileConstraint::SE)
						{
							queue.Add({ atlas.MakeSprite(18), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::SE;
						}
					}

					if ((constraint & engine::navigation::tile::TileConstraint::NE) == engine::navigation::tile::TileConstraint::NE)
					{
						if ((queued & engine::navigation::tile::TileConstraint::NE) != engine::navigation::tile::TileConstraint::NE)
						{
							queue.Add({ atlas.MakeSprite(19), pos + origin, tilesize, tint, 0.0f,	depth });
							queued |= engine::navigation::tile::TileConstraint::NE;
						}
					}

					if ((constraint & engine::navigation::tile::TileConstraint::CENTER) == engine::navigation::tile::TileConstraint::CENTER && drawCenterIfNeeded)
					{
						queue.Add({ atlas.MakeSprite(20), pos + origin, tilesize, tint, 0.0f,	depth });
						queued |= engine::navigation::tile::TileConstraint::CENTER;
					}

				}
			}

		}


	}
}

namespace TestTree
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
	using namespace engine::debug;

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


		engine::spatial::Coord m_startTile;
		engine::spatial::Coord m_endTile;
		std::vector<engine::spatial::Coord> m_path;

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

			// set map parameters
			{
				Registry<SizeF>::Instance().Register("tile_size", make_unique<SizeF>(64.0f, 64.0f));
				Registry<PositionF>::Instance().Register("map_position", make_unique<PositionF>(50.0f, 50.0f));
				Registry<Size<size_t>>::Instance().Register("map_size", make_unique<Size<size_t>>(20, 12));
				Registry<PositionF>::Instance().Register("depth", make_unique<PositionF>(0.0f, 64.0f));
			}

			// create resources
			{
				// sprite atlases
				SpriteAtlasFactory::Create("logic_tile", L"../Assets/12x2_384x64_tile1.png", 2, 12); // logic tile
			}

			{
				// create tile region	
				Registry<ConstraintMap>::Instance().Register("constraintmap", make_unique<ConstraintMap>());
				ConstraintMap& map = Registry<ConstraintMap>::Instance().Get("constraintmap");
				map.Initialize(Registry<Size<size_t>>::Instance().Get("map_size"), engine::navigation::tile::TileConstraint::NONE);
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		void OnKeyDown(int key)
		{
			// all maps have the same size and position. so they share the same position to coord conversion. calculate coord based on mouse position and map position and tile size
			PositionF mapPos = Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
			engine::spatial::Coord coord = engine::spatial::PositionToCoord(m_mousePos - mapPos, tilesize);
			ConstraintMap& constraintmap = Registry<ConstraintMap>::Instance().Get("constraintmap");


			// sanity check. if coord is out of bounds, bail out.
			if (!constraintmap.IsInBounds(coord))
			{
				return;
			}

			switch (key)
			{
			case 27: // escape
			{
				constraintmap.Set(engine::navigation::tile::TileConstraint::NONE);
				break;
			}
			case 32: // space
			{
				m_toggle = !m_toggle;

				break;
			}
			case 49: // 1
			{
				m_startTile = coord;
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				break;
			}
			case 50: // 2
			{
				m_endTile = coord;
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				break;
			}
			case 51: // 3
			{
				constraintmap.Set(coord, engine::navigation::tile::TileConstraint::BLOCKED);

				break;
			}
			case 52: // 4
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::SE);

				break;
			}
			case 53: // 5
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::N_WALL);

				break;
			}
			case 54: // 6
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::NW);

				break;
			}
			case 55: // 7
			{
				break;
			}
			case 56: // 8
			{
				break;
			}

			default:
				break;
			}
		}

		void OnMouseDown(int btn, int x, int y)
		{
			ConstraintMap& constraintmap = Registry<ConstraintMap>::Instance().Get("constraintmap");
			PositionF pos = PositionF((float)x, (float)y) - Registry<PositionF>::Instance().Get("map_position");
			SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

			engine::spatial::Coord coord = engine::spatial::PositionToCoord(pos, tilesize);


			if (!constraintmap.IsInBounds(coord))
			{
				return;
			}

			if (btn == 2)
			{
				constraintmap.Set(coord, engine::navigation::tile::TileConstraint::NONE);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}


			pos.x -= tilesize.width * coord.col;
			pos.y -= tilesize.height * coord.row;

			PositionF nw(0, 0);
			float dist = (pos - nw).Magnitude();

			if (dist / tilesize.width < 0.3f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::NW);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			PositionF ne(tilesize.width, 0);
			dist = (pos - ne).Magnitude();

			if (dist / tilesize.width < 0.3f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::NE);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			PositionF sw(0, tilesize.height);
			dist = (pos - sw).Magnitude();

			if (dist / tilesize.width < 0.3f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::SW);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			PositionF se(tilesize.height, tilesize.width);
			dist = (pos - se).Magnitude();

			if (dist / tilesize.width < 0.3f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::SE);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			dist = pos.y;
			if (dist / tilesize.width < 0.2f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::N_WALL);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			dist = tilesize.height - pos.y;
			if (dist / tilesize.width < 0.2f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::S_WALL);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			dist = pos.x;
			if (dist / tilesize.width < 0.2f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::W_WALL);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}

			dist = tilesize.height - pos.x;
			if (dist / tilesize.width < 0.2f)
			{
				constraintmap.Append(coord, engine::navigation::tile::TileConstraint::E_WALL);
				constraintmap.FindPath(m_startTile, m_endTile, m_path);
				return;
			}
			
			constraintmap.Append(coord, engine::navigation::tile::TileConstraint::CENTER);
			constraintmap.FindPath(m_startTile, m_endTile, m_path);

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
				{
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
					Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
					PositionF depth = Registry<PositionF>::Instance().Get("depth");

					ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("logic_tile");
					ConstraintMap& constraintmap = Registry<ConstraintMap>::Instance().Get("constraintmap");


					if (m_toggle)
					{
						for (int row = 0; row < (int)mapsize.height; row++)
						{
							m_drawQueue.Clear();

							for (int col = 0; col < (int)mapsize.width; col++)
							{
								Queue(constraintmap, m_drawQueue, atlas, tilesize, pos, 1, { 1,1,1,1 });
							}

							m_drawQueue.Sort();
							m_drawQueue.Execute(*m_renderer);
						}
					}

					m_drawQueue.Clear();
					Queue(m_drawQueue, m_startTile, tilesize, pos, atlas.MakeSprite(0));
					Queue(m_drawQueue, m_endTile, tilesize, pos, atlas.MakeSprite(6));
					m_drawQueue.Sort();
					m_drawQueue.Execute(*m_renderer);


					std::vector<engine::spatial::Coord> wp = engine::navigation::tile::GetWayPoints(m_path);
					engine::graphics::navigation::DrawWaypoints(*m_renderer, wp, tilesize, pos, { 1,1,1,1 }, 6.0f);

					//wp = engine::navigation::tile::SmoothWayPoints(wp, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
					//engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 0,1,0,1 }, 4.0f);

					//wp = engine::navigation::tile::SmoothWayPoints(m_path, [&tilemap](int row, int col) { return tilemap.IsInBounds(row, col) ? tilemap.Get(row, col)->IsWalkable() : false; });
					//engine::graphics::navigation::DrawWaypoints(*m_rendererBatch, wp, m_tilesize, m_pos, { 1,0,1,1 }, 2.0f);
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