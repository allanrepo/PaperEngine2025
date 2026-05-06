#pragma once
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Core/Sprite.h>
#include <Command/ICommand.h>
#include <Command/CommandQueue.h>
#include <algorithm>

namespace engine
{
	namespace command
	{
		namespace graphics
		{
			namespace renderer
			{
				// base class for all render commands
				class DrawCommandBase : public ICommand
				{
				protected:
					engine::graphics::renderer::IRenderer& m_renderer;

				public:
					DrawCommandBase(::engine::graphics::renderer::IRenderer& renderer) :
						m_renderer(renderer)
					{
					}
					virtual ~DrawCommandBase() = default;
					Type GetType() const override
					{
						return Type::Render;
					}
				};

				class DrawQuadCommand : public DrawCommandBase
				{
				private:
					engine::spatial::PositionF m_pos;
					spatial::SizeF m_size;
					::engine::graphics::ColorF m_color;
					float m_rotation;

				public:
					DrawQuadCommand(
						::engine::graphics::renderer::IRenderer& renderer,
						engine::spatial::PositionF pos,
						spatial::SizeF size,
						::engine::graphics::ColorF color,
						float rotation
					) :
						DrawCommandBase(renderer),
						m_pos(pos),
						m_size(size),
						m_color(color),
						m_rotation(rotation)
					{
					}

					void Execute() override
					{
						m_renderer.Draw(m_pos, m_size, m_color, m_rotation);
					}
				};

				class DrawTextCommand : public DrawCommandBase
				{
				private:
					const engine::graphics::resource::IFontAtlas& m_font;
					std::string m_text;
					engine::spatial::PositionF m_pos;
					engine::graphics::ColorF m_color;
				public:
					DrawTextCommand(
						engine::graphics::renderer::IRenderer& renderer,
						const engine::graphics::resource::IFontAtlas& font,
						const std::string& text,
						engine::spatial::PositionF pos,
						engine::graphics::ColorF color
					) :
						DrawCommandBase(renderer),
						m_font(font),
						m_text(text),
						m_pos(pos),
						m_color(color)
					{
					}

					void Execute() override
					{
						m_renderer.Draw(m_font, m_text, m_pos, m_color);
					}
				};

				class DrawCommand : public DrawCommandBase
				{
				private:
					const ::engine::graphics::Sprite& m_sprite;
					engine::spatial::PositionF m_pos;
					spatial::SizeF m_size;
					::engine::graphics::ColorF m_color;
					float m_rotation;
				public:
					DrawCommand(
						engine::graphics::renderer::IRenderer& renderer,
						const engine::graphics::Sprite& sprite,
						engine::spatial::PositionF pos,
						spatial::SizeF size,
						::engine::graphics::ColorF color,
						float rotation
					) :
						DrawCommandBase(renderer),
						m_sprite(sprite),
						m_pos(pos),
						m_size(size),
						m_color(color),
						m_rotation(rotation)
					{
					}
					void Execute() override
					{
						m_renderer.Draw(m_sprite, m_pos, m_size, m_color, m_rotation);
					}
				};

				class DrawSpriteCommand : public DrawCommandBase
				{
				private:
					const ::engine::graphics::Sprite m_sprite;
					engine::spatial::PositionF m_pos;
					spatial::SizeF m_size;
					::engine::graphics::ColorF m_color;
					float m_rotation;
				public:
					DrawSpriteCommand(
						::engine::graphics::renderer::IRenderer& renderer,
						const ::engine::graphics::Sprite& sprite,
						engine::spatial::PositionF pos,
						spatial::SizeF size,
						::engine::graphics::ColorF color,
						float rotation
					) :
						DrawCommandBase(renderer),
						m_sprite(sprite),
						m_pos(pos),
						m_size(size),
						m_color(color),
						m_rotation(rotation)
					{
					}
					void Execute() override
					{
						m_renderer.Draw(m_sprite, m_pos, m_size, m_color, m_rotation);
					}
				};

				class SetClipRegionCommand : public DrawCommandBase
				{
				private:
					const engine::math::geometry::RectF m_region;
					bool m_enable;

				public:
					SetClipRegionCommand(
						::engine::graphics::renderer::IRenderer& renderer,
						const engine::math::geometry::RectF& region,
						bool enable
					) :
						DrawCommandBase(renderer),
						m_region(region),
						m_enable(enable)
					{
					}

					void Execute() override
					{
						if (m_enable)
						{
							m_renderer.SetClipRegion(m_region);
						}
						m_renderer.EnableClipping(m_enable);
					}
				};

				class DrawSortedSpritesCommand : public DrawCommandBase
				{
				public:
					struct Item
					{
						engine::graphics::Sprite sprite;	// what to draw
						engine::spatial::PositionF pos;		// world position
						engine::spatial::SizeF size;		// size on screen
						engine::graphics::ColorF tint;		// color modulation
						float rotation;						// rotation angle
						float depth;						// depth value	
					};

				private:
					std::vector<Item> m_items;

				public:
					DrawSortedSpritesCommand(
						engine::graphics::renderer::IRenderer& renderer,
						size_t capacity
					) :
						DrawCommandBase(renderer)
					{
						m_items.reserve(capacity);
					}

					void Add(const Item& item)
					{
						m_items.push_back(item);
					}

					void Sort()
					{
						std::sort(m_items.begin(), m_items.end(),
							[](const Item& a, const Item& b)
							{
								// if depth is not same, e.g. lower and higher tile, higher tile (b) has higher depth than lower tile (a). draw lower tile first
								if (a.depth != b.depth) return a.depth < b.depth;

								// if same depth, whichever is farthest from screen(a) gets drawn first. nearest from screen (b) is drawn last
								return a.pos.y < b.pos.y; // depth by Y
							});
					}

					void Execute() override
					{
						for (auto& item : m_items)
						{
							m_renderer.Draw(item.sprite, item.pos, item.size, item.tint, item.rotation);
						}
					}

					void Clear()
					{
						m_items.clear();
					}
				};
			};
		}
	}
}
