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
#include <Graphics/Renderable/IRenderable.h>

//
// Prop has AnimationManager
// AnimationManager has Animator and Animations
// 
//

namespace engine
{
	namespace graphics
	{
		namespace renderable
		{
			class IProp 
			{
			public:
				virtual void Update(double delta) = 0;
				virtual void PlayAnimation(const std::string& key) = 0;
				virtual bool IsValid() const = 0;
				virtual ~IProp() = default;
			};

			class Prop : public IProp 
			{
			private:
				Animator<Sprite> m_animator;
				AnimationManager<Sprite> m_animationManager;

			public:
				// we're copying the reference of animation manager because we want to share the same animation manager across multiple props. 
				// if we assign the passed animation manager directly, it will be reference to animation manager outside which is not safe. 
				// we want to have our own copy of animation manager that shares the same animations but has its own animator
				Prop(AnimationManager<Sprite> manager): 
					m_animator(), 
					m_animationManager(manager) 
				{
					m_animationManager.Set(m_animator);
				}

				void Update(double delta) override 
				{
					m_animationManager.Update(delta);
				}

				void PlayAnimation(const std::string& key) override 
				{
					m_animationManager.Play(key);
				}

				// Instead of inheriting IRenderable, just expose the renderable
				const Sprite& GetSprite() const 
				{
					return m_animator.GetCurrent();
				}

				bool IsValid() const override 
				{
					return m_animator.IsRunning();
				}
			};

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

	template<typename T>
	using Registry = engine::cache::Registry<T>;

	template<typename T>
	using Animation = engine::graphics::animation::Animation<T>;


	// this is a tile definition class, not exactly tile class. tile class is Tile<T> and this is what is assigned to T
	class AnimatedTile
	{
	private:
		engine::graphics::animation::Animator<engine::graphics::renderable::Sprite> m_animator;
		std::unordered_map<std::string, engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>> m_animations;
		bool m_walkable;
		int m_index;

	public:
		AnimatedTile(bool walkable, const std::string& name, const engine::graphics::animation::Animation<engine::graphics::renderable::Sprite>& anim, int index) :
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

		const engine::graphics::renderable::Sprite& GetSprite() const
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
		Sprite m_sprite;
		bool m_walkable;
		int m_index;

	public:
		RenderableTile(const Sprite& sprite, bool walkable, int index) :
			m_sprite(sprite),
			m_walkable(walkable),
			m_index(index)
		{
		}

		int GetIndex() const
		{
			return m_index;
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


	template<typename T>
	class TileMapRenderer
	{
	public:
		TileMapRenderer()
		{

		}
		engine::event::Event<const engine::spatial::Coord&, const engine::spatial::PositionF&, const engine::spatial::SizeF&> TileRenderedEvent;

		void DrawTileMap(
			engine::graphics::renderer::IRenderer& renderer,
			const engine::component::tile::TileMap<T>& tilemap,
			const engine::spatial::SizeF& tilesize,
			const engine::spatial::PositionF& pos,
			const engine::graphics::ColorF& color,
			const engine::spatial::PositionF& offset,
			const engine::math::VecF& scale,
			float alpha
		)
		{
			for (int row = 0; row <= tilemap.GetHeight(); ++row)
			{
				for (int col = 0; col <= tilemap.GetWidth(); ++col)
				{
					if (!tilemap.IsInBounds(row, col))
					{
						continue;
					}

					// get the tile
					const engine::component::tile::Tile<T>& tile = tilemap.Get(row, col);

					// defensive. we're never sure if the tile has valid sprite, so do check
					if (tile.IsValid())
					{
						// this will be the top-left position of this tile in map coordinate.
						engine::spatial::PositionF origin =
						{
							col * tilesize.width,
							row * tilesize.height
						};

						origin += offset;

						engine::spatial::SizeF ts =
						{
							tilesize.width * scale.x,
							tilesize.height * scale.y
						};

						renderer.DrawRenderable(
							tile->GetSprite(),
							pos + origin,
							ts,
							engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, alpha },
							0.0f
						);

						TileRenderedEvent({ row, col }, pos + origin, ts);

					}
				}
			}
		}

		void DrawTileMap(
			engine::graphics::renderer::IRenderer& renderer,
			const engine::component::tile::TileMap<T>& tilemap,
			const engine::spatial::SizeF& tilesize,
			const engine::spatial::PositionF& pos,
			const engine::graphics::ColorF& color
		)
		{
			for (int row = 0; row <= tilemap.GetHeight(); ++row)
			{
				for (int col = 0; col <= tilemap.GetWidth(); ++col)
				{
					if (!tilemap.IsInBounds(row, col))
					{
						continue;
					}

					const engine::component::tile::Tile<T>& tile = tilemap.Get(row, col);
					if (tile.IsValid())
					{
						engine::spatial::PositionF origin =
						{
							col * tilesize.width,
							row * tilesize.height
						};

						renderer.DrawRenderable(tile->GetSprite(), pos + origin, tilesize, color, 0.0f);

						TileRenderedEvent({ row, col }, pos + origin, tilesize);

					}
				}
			}
		}
	};

	//class LogicTile
	//{
	//private:
	//	Sprite m_sprite;
	//	engine::navigation::tile::TileConstraint m_mask;
	//public:
	//	LogicTile(const Sprite& sprite, engine::navigation::tile::TileConstraint mask) :
	//		m_sprite(sprite),
	//		m_mask(mask)
	//	{
	//	}
	//	const Sprite& GetSprite() const
	//	{
	//		return m_sprite;
	//	}
	//	engine::navigation::tile::TileConstraint GetMask() const
	//	{
	//		return m_mask;
	//	}
	//}

	//class LogicMap: public engine::container::Grid<LogicTile>
	//{
	//public:
	//	LogicMap(size_t width = 0) :
	//		Grid<LogicalTile>(width)
	//	{
	//	}

	//	void Draw(engine::graphics::renderer::IRenderer& renderer, const engine::spatial::SizeF& tilesize, const engine::spatial::PositionF& pos)
	//	{
	//		for (int row = 0; row <= GetHeight(); ++row)
	//		{
	//			for (int col = 0; col <= GetWidth(); ++col)
	//			{
	//				if (!IsInBounds(row, col))
	//				{
	//					continue;
	//				}
	//				engine::spatial::PositionF origin =
	//				{
	//					col * tilesize.width,
	//					row * tilesize.height
	//				};
	//				Get(row, col).Draw({ row, col }, pos + origin, tilesize);
	//			}
	//		}
	//	}
	//};



	struct DrawCommand
	{
		Sprite sprite;              // what to draw
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

		void Execute(IRenderer& renderer)
		{
			for (auto& cmd : m_commands)
			{
				renderer.DrawRenderable(cmd.sprite, cmd.pos, cmd.size, cmd.tint, cmd.rotation);
			}
		}

		void Clear()
		{
			m_commands.clear();
		}
	};



	class IProp 
	{
	public:
		virtual ~IProp() = default;

		virtual const Sprite& GetSprite() const = 0;

		virtual void Update(double delta) = 0;
	};

	class Prop : public IProp
	{
	private:
		std::unique_ptr<engine::graphics::resource::ISpriteAtlas> m_atlas;
		engine::graphics::animation::Animation<engine::graphics::renderable::Sprite> m_anim;
		std::unique_ptr<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>> m_animator;


	public:
		Prop(const std::wstring& filepath, const size_t row, const size_t col, const std::vector<int>& animationFrameIndice, float duration, const PositionF& anchor)
		{
			m_atlas = engine::graphics::factory::SpriteAtlasFactory::Create(filepath, row, col);
			m_anim = engine::graphics::factory::AnimationFactory::Create(*m_atlas, animationFrameIndice, duration, true, anchor);
			m_animator = std::make_unique<engine::graphics::animation::Animator<engine::graphics::renderable::Sprite>>();
			m_animator->Play(m_anim);
		}



		void Update(double delta)
		{
			m_animator->Update(delta);
		}

		const Sprite& GetSprite() const override
		{
			return m_animator->GetCurrent();
		}
	};


	class PropTile
	{
	private:
		friend class PropMap;
		engine::container::Dictionary<engine::navigation::tile::TileConstraint, IProp*> m_props;

	public:
		PropTile()
		{
		}

		void Set(engine::navigation::tile::TileConstraint constraint, IProp& prop)
		{
			m_props[constraint] = &prop;
		}

		bool Has(engine::navigation::tile::TileConstraint constraint) const
		{
			return m_props.Has(constraint);
		}

		IProp& Get(engine::navigation::tile::TileConstraint constraint) const
		{
			// unsafe. caller should check Has() before calling this. 
			return *m_props[constraint];
		}

		// clears all props from this tile
		void Clear()
		{
			//m_prop = nullptr;
			m_props.Clear();	
		}
	};

	class PropHandle : public core::Handle<PropTile>
	{
	public:
		PropHandle(PropTile* data = nullptr) :
			core::Handle<PropTile>(data)
		{
		}

		~PropHandle()
		{
		}

		PropHandle(const PropHandle&) = default;
		PropHandle& operator=(const PropHandle&) = default;
		PropHandle(PropHandle&&) = default;
		PropHandle& operator=(PropHandle&&) = default;
	};

	class PropMap 
	{
	private:
		engine::container::Grid<PropTile> m_grid;

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
				m_grid.Add(PropTile{});
			}
		}

		void Initialize(engine::spatial::Size<size_t> size) 
		{
			Initialize(size.width, size.height);
		}

		void Clear()
		{
			for(int i = 0; i < m_grid.GetElementCount(); i++)
			{
				PropTile& tile = m_grid.Get(i);
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

		PropHandle Get(int row, int col) 
		{
			return PropHandle(&m_grid.Get(row, col));
		}

		PropHandle Get(const Coord& coord)
		{
			return PropHandle(&m_grid.Get(coord));
		}

		void Queue(DrawQueue& queue, const engine::spatial::SizeF& tilesize, const engine::spatial::PositionF& pos, const math::VecF& offset = { 0,0 }, const engine::graphics::ColorF& tint = { 1,1,1,1 })
		{
			// prop map decides where in the tile the props should be drawn based on the constraints assigned to the props. 
			// for example, if a prop has CENTER constraint, it will be drawn at the center of the tile. 
			// if it has NW constraint, it will be drawn at the north-west corner of the tile, and so on. 
			// if a prop has no constraint, it will be drawn at the top-left corner of the tile by default.
			for (int row = 0; row < m_grid.GetHeight(); ++row)
			{
				for (int col = 0; col < m_grid.GetWidth(); ++col)
				{
					if (!m_grid.IsInBounds(row, col)) continue;

					PropHandle handle = Get(row, col);

					if (handle->Has(engine::navigation::tile::TileConstraint::CENTER))
					{
						const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::CENTER).GetSprite();

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
							1
							});
					}

					if (handle->Has(engine::navigation::tile::TileConstraint::NW))
					{
						const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::NW).GetSprite();

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
							1
							});
					}

					if (handle->Has(engine::navigation::tile::TileConstraint::NE))
					{
						const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::NE).GetSprite();

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
							1
							});
					}

					if (handle->Has(engine::navigation::tile::TileConstraint::SW))
					{
						const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::SW).GetSprite();

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
							1
							});
					}

					if (handle->Has(engine::navigation::tile::TileConstraint::SE))
					{
						const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::SE).GetSprite();
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
							1
							});
					}
				}
			}
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

			PropHandle handle = Get(row, col);

			if (handle->Has(engine::navigation::tile::TileConstraint::CENTER))
			{

				const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::CENTER).GetSprite();

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

			if (handle->Has(engine::navigation::tile::TileConstraint::NW))
			{
				const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::NW).GetSprite();

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

			if (handle->Has(engine::navigation::tile::TileConstraint::NE))
			{
				const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::NE).GetSprite();

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

			if (handle->Has(engine::navigation::tile::TileConstraint::SW))
			{
				const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::SW).GetSprite();

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

			if (handle->Has(engine::navigation::tile::TileConstraint::SE))
			{
				const Sprite& sprite = handle->Get(engine::navigation::tile::TileConstraint::SE).GetSprite();
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

	template<typename T>
	void Queue(
		TileMap<T>& map,
		DrawQueue& queue, 
		const engine::spatial::SizeF& tilesize, 
		const engine::spatial::PositionF& pos, 
		const engine::graphics::ColorF& tint = { 1,1,1,1 }
	)
	{
		for (int row = 0; row <= map.GetHeight(); ++row)
		{
			for (int col = 0; col <= map.GetWidth(); ++col)
			{
				if (!map.IsInBounds(row, col))
				{
					continue;
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
						1
						});
				}
			}
		}
	}

	template<typename T>
	void Queue(
		TileMap<T>& map,
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

	class LookupPropResolver
	{
	private:
		engine::container::Dictionary<engine::tile::TileVariant, int> m_variantToIndex;
		engine::container::Dictionary<int, engine::navigation::tile::TileConstraint> m_indexToConstraint;
		PropMap& m_map;
		Dictionary<int, std::unique_ptr<IProp>>& m_props;

	public:
		LookupPropResolver(Dictionary<int, std::unique_ptr<IProp>>& props, PropMap& map) :
			m_props(props),
			m_map(map)
		{
		}

		void Register(engine::tile::TileVariant variant, int index)
		{
			m_variantToIndex[variant] = index;
		}

		void Register(int index, engine::navigation::tile::TileConstraint constraint)
		{
			m_indexToConstraint[index] = constraint;
		}

		void Set(const engine::spatial::Coord& coord, engine::tile::TileVariant variant)
		{
			if (!m_map.IsInBounds(coord)) return;

			m_map.Get(coord)->Clear();

			if (!m_variantToIndex.Has(variant)) return;

			int index = m_variantToIndex[variant];
			if (!m_indexToConstraint.Has(index)) return;

			engine::navigation::tile::TileConstraint constraint = m_indexToConstraint[index];

			m_map.Get(coord)->Set(constraint, *(m_props.Get(index)));
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

		DrawQueue m_drawQueue;

		bool m_toggle;

	public:
		Test():
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
				Registry<PositionF>::Instance().Register("layer_height", make_unique<PositionF>(0.0f, 64.0f));
			}

			// setup water tilemap
			{
				// create sprite atlas to be used by tilemap
				SpriteAtlasFactory::Create("1x1_64x64_water_background", L"../Assets/1x1_64x64_water_background.png", 1, 1);
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("1x1_64x64_water_background");

				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("1x1_64x64_water_background", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("1x1_64x64_water_background");

				tileset.Register(0, std::make_unique<RenderableTile>(atlas.MakeSprite(0), false, 0)); // water so not walkable. doesn't matter. this is background map

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("1x1_64x64_water_background", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");

				// load tile region by filling it with all '0' tile
				Table<string> map({ 20, 12 }, "0");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });
			}

			// setup land map
			{
				// create sprite atlas to be used by tilemap
				SpriteAtlasFactory::Create("land_map", L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9);
				ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("land_map");

				// create our tileset
				Registry<Tileset<RenderableTile>>::Instance().Register("land_map", std::make_unique<Tileset<RenderableTile>>());
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("land_map");

				// each sprite from atlas is a static tile (single frame), so we create tile from each sprite
				for (int i = 0; i < atlas.GetUVRectCount(); i++)
				{
					tileset.Register(i, std::make_unique<RenderableTile>(atlas.MakeSprite(i), true, i)); // make it all walkable for now
				}

				// create tile region
				Registry<TileRegion<RenderableTile>>::Instance().Register("land_map", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("land_map");

				// load tile region by filling it with all '4' tile
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Table<string> map(mapsize, "4");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for land map. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Register("land_map", make_unique<engine::tile::AutoTileResolver<RenderableTile>>(region, tileset));
				engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");

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

			// create hill map
			{
				// we're using same tileset as land map since hill map is just variant of land map. it will use same sprites but different auto-tile mapping configuration
				Tileset<RenderableTile>& tileset = Registry<Tileset<RenderableTile>>::Instance().Get("land_map");

				// create tile region	
				Registry<TileRegion<RenderableTile>>::Instance().Register("hill_map", make_unique<TileRegion<RenderableTile>>());
				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("hill_map");

				// load tile region by filling it with all '4' tile
				Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
				Table<string> map(mapsize, "4");
				AsyncTileRegionLoader<RenderableTile, int> tileRegionLoader;
				tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<RenderableTile> { return tileset.MakeTile(cell); });

				// create lookup tile resolver for land map. this will be used to determine tile variant based on surrounding tiles
				Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Register("hill_map", make_unique<engine::tile::AutoTileResolver<RenderableTile>>(region, tileset));
				engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");

				// configure hill map auto-tile mapping
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


			//// setup logic map
			//{
			//	// create sprite atlas to be used by tilemap
			//	SpriteAtlasFactory::Create("logic_map", L"../Assets/12x2_384x64_tile.png", 2, 12);
			//	ISpriteAtlas& atlas = Registry<ISpriteAtlas>::Instance().Get("logic_map");

			//	// create our tileset
			//	Registry<Tileset<LogicalTile>>::Instance().Register("logic_map", std::make_unique<Tileset<LogicalTile>>());
			//	Tileset<LogicalTile>& tileset = Registry<Tileset<LogicalTile>>::Instance().Get("logic_map");

			//	tileset.Register(0, std::make_unique<LogicalTile>(*m_renderer, atlas.MakeSprite(0), engine::navigation::tile::TileConstraint::NONE));
			//	tileset.Register(1, std::make_unique<LogicalTile>(*m_renderer, atlas.MakeSprite(20), engine::navigation::tile::TileConstraint::CENTER));
			//	tileset.Register(2, std::make_unique<LogicalTile>(*m_renderer, atlas.MakeSprite(4), engine::navigation::tile::TileConstraint::BLOCKED));

			//	// create tile region
			//	Registry<TileRegion<LogicalTile>>::Instance().Register("logic_map", make_unique<TileRegion<LogicalTile>>());
			//	TileRegion<LogicalTile>& region = Registry<TileRegion<LogicalTile>>::Instance().Get("logic_map");

			//	// load tile region by filling it with all '2' tile
			//	Table<string> map({ 20, 12 }, "2");
			//	AsyncTileRegionLoader<LogicalTile, int> tileRegionLoader;
			//	tileRegionLoader.LoadImmediate(region, map, [&tileset](const int& cell) -> Tile<LogicalTile> { return tileset.MakeTile(cell); });

			//	// create lookup tile resolver
			//	Registry<engine::tile::LookupTileResolver<LogicalTile>>::Instance().Register("logic_map", make_unique<engine::tile::LookupTileResolver<LogicalTile>>(region, tileset));
			//	engine::tile::LookupTileResolver<LogicalTile>& resolver = Registry<engine::tile::LookupTileResolver<LogicalTile>>::Instance().Get("logic_map");

			//	// all tiles are walkable except empty tile.
			//	resolver.Register(engine::tile::TileVariant::Empty, 2);
			//	resolver.Register(engine::tile::TileVariant::Island, 0);
			//	resolver.Register(engine::tile::TileVariant::Full, 0);
			//	resolver.Register(engine::tile::TileVariant::NorthEdge, 0);
			//	resolver.Register(engine::tile::TileVariant::SouthEdge, 0);
			//	resolver.Register(engine::tile::TileVariant::EastEdge, 0);
			//	resolver.Register(engine::tile::TileVariant::WestEdge, 0);
			//	resolver.Register(engine::tile::TileVariant::NECorner, 0);
			//	resolver.Register(engine::tile::TileVariant::NWCorner, 0);
			//	resolver.Register(engine::tile::TileVariant::SECorner, 0);
			//	resolver.Register(engine::tile::TileVariant::SWCorner, 0);
			//	resolver.Register(engine::tile::TileVariant::Vertical, 0);
			//	resolver.Register(engine::tile::TileVariant::Horizontal, 0);		
			//	resolver.Register(engine::tile::TileVariant::TNorth, 0);
			//	resolver.Register(engine::tile::TileVariant::TSouth, 0);
			//	resolver.Register(engine::tile::TileVariant::TEast, 0);
			//	resolver.Register(engine::tile::TileVariant::TWest, 0);


			//	engine::tile::AutoTileResolver<RenderableTile>& landTileResolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
			//	landTileResolver.TileVariantChangedEvent += Handler(&resolver, &engine::tile::LookupTileResolver<LogicalTile>::Set);
			//}

			// reset land map to update tile variants based on logic map's initial state
			{
				engine::tile::AutoTileResolver<RenderableTile>& landTileResolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");

				TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("land_map");
				for (int row = 0; row < region.GetSize().height; row++)
				{
					for (int col = 0; col < region.GetSize().width; col++)
					{
						landTileResolver.Set({ row, col });
					}
				}
			}

			{
				Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Register("props", make_unique<Dictionary<int, std::unique_ptr<IProp>>>());
				Dictionary<int, std::unique_ptr<IProp>>& props = Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Get("props");

				props.Register(0, make_unique<Prop>(L"../Assets/tree_1x8_1536x256.png", 1, 8, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 100.0f, PositionF{ 0.5f, 0.9f })); // tree with taller trunk
				props.Register(1, make_unique<Prop>(L"../Assets/tree_1x8_1536x192.png", 1, 8, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7 }, 100.0f, PositionF{ 0.5f, 0.85f })); // tree with shorter trunk
				props.Register(41, make_unique<Prop>(L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9, std::vector<int>{ 41 }, 100.0f, PositionF{ 0.0f, 1.0f })); // wall tile from land map atlas.
				props.Register(42, make_unique<Prop>(L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9, std::vector<int>{ 42 }, 100.0f, PositionF{ 0.0f, 1.0f })); // wall tile from land map atlas.
				props.Register(43, make_unique<Prop>(L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9, std::vector<int>{ 43 }, 100.0f, PositionF{ 0.0f, 1.0f })); // wall tile from land map atlas.
				props.Register(44, make_unique<Prop>(L"../Assets/576x384px_6x9tile_TileMap.png", 6, 9, std::vector<int>{ 44 }, 100.0f, PositionF{ 0.0f, 1.0f })); // wall tile from land map atlas.
				props.Register(9, make_unique<Prop>(L"../Assets/CharacterTest_2304x1536_12x8.png", 8, 12, std::vector<int>{ 0, 1, 2, 3, 4, 5 }, 100.0f, PositionF{ 0.5f, 0.65f })); // knight character with walking animation
			}

			{
				Registry<PropMap>::Instance().Register("prop_map", make_unique<PropMap>());
				PropMap& map = Registry<PropMap>::Instance().Get("prop_map");

				map.Initialize(Registry<Size<size_t>>::Instance().Get("map_size"));
			}

			{
				Registry<LookupPropResolver>::Instance().Register("wall_prop_resolver", make_unique<LookupPropResolver>(
					Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Get("props"), 
					Registry<PropMap>::Instance().Get("prop_map")
				));
				LookupPropResolver& resolver = Registry<LookupPropResolver>::Instance().Get("wall_prop_resolver");

				//// create lookup tile resolver for wall map. this will be used to determine tile variant based on surrounding tiles
				//Registry<engine::tile::LookupTileResolver<RenderableTile>>::Instance().Register("wall_map", make_unique<engine::tile::LookupTileResolver<RenderableTile>>(region, tileset));
				//engine::tile::LookupTileResolver<RenderableTile>& resolver = Registry<engine::tile::LookupTileResolver<RenderableTile>>::Instance().Get("wall_map");

				// map tile index to tile variant for wall map. this will be used by auto-tile resolver to determine tile variant based on surrounding tiles
				resolver.Register(engine::tile::TileVariant::Empty, 4);
				resolver.Register(engine::tile::TileVariant::Island, 44);
				resolver.Register(engine::tile::TileVariant::Full, 4);
				resolver.Register(engine::tile::TileVariant::NorthEdge, 44);
				resolver.Register(engine::tile::TileVariant::SouthEdge, 4);
				resolver.Register(engine::tile::TileVariant::EastEdge, 43);
				resolver.Register(engine::tile::TileVariant::WestEdge, 41);
				resolver.Register(engine::tile::TileVariant::NECorner, 4);
				resolver.Register(engine::tile::TileVariant::NWCorner, 4);
				resolver.Register(engine::tile::TileVariant::SECorner, 41);
				resolver.Register(engine::tile::TileVariant::SWCorner, 43);
				resolver.Register(engine::tile::TileVariant::Vertical, 4);
				resolver.Register(engine::tile::TileVariant::Horizontal, 42);
				resolver.Register(engine::tile::TileVariant::TNorth, 4);
				resolver.Register(engine::tile::TileVariant::TSouth, 42);
				resolver.Register(engine::tile::TileVariant::TEast, 4);
				resolver.Register(engine::tile::TileVariant::TWest, 4);

				resolver.Register(41, engine::navigation::tile::TileConstraint::SW);
				resolver.Register(42, engine::navigation::tile::TileConstraint::SW);
				resolver.Register(43, engine::navigation::tile::TileConstraint::SW);
				resolver.Register(44, engine::navigation::tile::TileConstraint::SW);

				//// let wall tile resolver subscribe to hill auto-tile map so that it can update wall tile variants when hill tiles change
				engine::tile::AutoTileResolver<RenderableTile>& hillTileResolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");
				hillTileResolver.TileVariantChangedEvent += Handler(&resolver, &LookupPropResolver::Set);
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

			switch (key)
			{
			case 27: // escape
			{
				// clear map of land
				{
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("land_map");
					engine::tile::AutoTileResolver<RenderableTile>& landTileResolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					for (int row = 0; row < region.GetSize().height; row++)
					{
						for (int col = 0; col < region.GetSize().width; col++)
						{
							landTileResolver.Set({ row, col });
						}
					}
				}

				// clear map of hill
				{
					TileRegion<RenderableTile>& region = Registry<TileRegion<RenderableTile>>::Instance().Get("hill_map");
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");
					for (int row = 0; row < region.GetSize().height; row++)
					{
						for (int col = 0; col < region.GetSize().width; col++)
						{
							resolver.Remove({ row, col });
						}
					}
				}

				// clear props
				{
					PropMap& map = Registry<PropMap>::Instance().Get("prop_map");
					map.Clear();
				}
				break;
			}
			case 32: // space
				break;
			case 49: // 1
			{
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Set(coord);				
				}

				break;
			}
			case 50: // 2
			{
				// remove land tile at coord and update surrounding tiles
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Remove(coord);
				}
				// remove hill tile at coord and update surrounding tiles
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");
					resolver.Remove(coord);
				}
				// remove prop at coord if any
				{
					PropMap& propMap = Registry<PropMap>::Instance().Get("prop_map");
					PropHandle handle = propMap.Get(coord);
					handle->Clear(); 
				}
				break;
			}
			case 51: // 3
			{
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Set(coord);
				}
				{
					// set prop at coord to tree if not already set.
					PropMap& propMap = Registry<PropMap>::Instance().Get("prop_map");
					PropHandle handle = propMap.Get(coord);

					Dictionary<int, std::unique_ptr<IProp>>& props = Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Get("props");
					handle->Set(engine::navigation::tile::TileConstraint::CENTER, *props.Get(0));
				}
				break;
			}
			case 52: // 4
			{
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Set(coord);
				}
				{
					// set prop at coord to tree if not already set.
					PropMap& propMap = Registry<PropMap>::Instance().Get("prop_map");
					PropHandle handle = propMap.Get(coord);

					Dictionary<int, std::unique_ptr<IProp>>& props = Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Get("props");
					handle->Set(engine::navigation::tile::TileConstraint::CENTER, *props.Get(1));
				}
				break;
			}
			case 53: // 5
			{
				// set land tile at coord to hill and update surrounding tiles
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("land_map");
					resolver.Set(coord);
				}
				// set hill tile at coord and update surrounding tiles
				{
					engine::tile::AutoTileResolver<RenderableTile>& resolver = Registry<engine::tile::AutoTileResolver<RenderableTile>>::Instance().Get("hill_map");
					resolver.Set(coord);
				}
				//{
				//	// set prop at coord to wall if not already set.
				//	PropMap& propMap = Registry<PropMap>::Instance().Get("prop_map");
				//	PropHandle handle = propMap.Get(coord);

				//	Dictionary<int, std::unique_ptr<IProp>>& props = Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Get("props");
				//	handle->Set(engine::navigation::tile::TileConstraint::SW, *props.Get(2));
				//}
				break;
			}
			case 54:
			{
				m_toggle = !m_toggle;
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
			Dictionary<int, std::unique_ptr<IProp>>& props = Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Get("props");
			props.Get(0)->Update(time);
			props.Get(1)->Update(time);
			//props.Get(44)->Update(time);
			props.Get(9)->Update(time);
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

				m_drawQueue.Clear();

				// draw water background map
				{
					TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("1x1_64x64_water_background");
					TileMap<RenderableTile> tilemap = region.MakeTileMap();

					// get tilemap parameters
					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");


					DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
				}

				//// draw land map
				//{
				//	TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("land_map");
				//	TileMap<RenderableTile> tilemap = region.MakeTileMap();

				//	// get tilemap parameters
				//	PositionF pos = Registry<PositionF>::Instance().Get("map_position");
				//	SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

				//	Queue(tilemap, m_drawQueue, tilesize, pos, { 1,1,1,1 });

				//	//DrawTileMap(*m_renderer, tilemap, tilesize, pos, { 1,1,1,1 });
				//}

				//{
				//	PropMap& propMap = Registry<PropMap>::Instance().Get("prop_map");
				//	// get tilemap parameters
				//	PositionF pos = Registry<PositionF>::Instance().Get("map_position");
				//	SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
				//	//propMap.Draw(*m_renderer, tilesize, pos);

				//	//propMap.Queue(m_drawQueue, tilesize, pos);
				//}

				//// queue the knight character prop to be drawn at mouse position
				//{
				//	IProp& prop = *Registry<Dictionary<int, std::unique_ptr<IProp>>>::Instance().Get("props").Get(9);
				//	DrawCommand cmd
				//	{
				//		prop.GetSprite(),
				//		m_mousePos,
				//		prop.GetSprite().GetSize(),
				//		{ 1,1,1,1 },
				//		0.0f,
				//		1
				//	};
				//	//m_drawQueue.Add(cmd);	
				//}




				//// draw hill map
				//{
				//	TileRegion<RenderableTile> region = Registry<TileRegion<RenderableTile>>::Instance().Get("hill_map");
				//	TileMap<RenderableTile> tilemap = region.MakeTileMap();

				//	// get tilemap parameters
				//	PositionF pos = Registry<PositionF>::Instance().Get("map_position");
				//	SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");
				//	PositionF layerheight = Registry<PositionF>::Instance().Get("layer_height");

				//	Queue(tilemap, m_drawQueue, tilesize, pos - layerheight, { 1,1,1,1 });

				//	//if(m_toggle) DrawTileMap(*m_renderer, tilemap, tilesize, pos - layerheight, { 1,1,1,1 });

				//}

				// draw the props
				{
					m_drawQueue.Sort();
					m_drawQueue.Execute(*m_renderer);
				}

				{
					Size<size_t> mapsize = Registry<Size<size_t>>::Instance().Get("map_size");
					TileMap<RenderableTile> landmap = Registry<TileRegion<RenderableTile>>::Instance().Get("land_map").MakeTileMap();
					TileMap<RenderableTile> hillmap = Registry<TileRegion<RenderableTile>>::Instance().Get("hill_map").MakeTileMap();
					PositionF layerheight = Registry<PositionF>::Instance().Get("layer_height");

					PropMap& propMap = Registry<PropMap>::Instance().Get("prop_map");

					PositionF pos = Registry<PositionF>::Instance().Get("map_position");
					SizeF tilesize = Registry<SizeF>::Instance().Get("tile_size");

					for (int row = 0; row < (int)mapsize.height; row++)
					{
						m_drawQueue.Clear();

						for (int col = 0; col < (int)mapsize.width; col++)
						{
							Queue(landmap, m_drawQueue, row, col, tilesize, pos, 1.0f, { 1,1,1,1 });
							propMap.Queue(m_drawQueue, row, col, tilesize, pos, 1.0f);
							Queue(hillmap, m_drawQueue, row, col, tilesize, pos - layerheight, 2.0f, { 1,1,1,1 });
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