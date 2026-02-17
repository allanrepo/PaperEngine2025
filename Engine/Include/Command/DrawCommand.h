#pragma once
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderable/Sprite.h>
#include <Command/ICommand.h>
#include <Command/CommandQueue.h>

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
						m_renderer.DrawText(m_font, m_text, m_pos, m_color);
					}
				};

				class DrawRenderableCommand : public DrawCommandBase
				{
				private:
					const ::engine::graphics::renderable::IRenderable& m_renderable;
					engine::spatial::PositionF m_pos;
					spatial::SizeF m_size;
					::engine::graphics::ColorF m_color;
					float m_rotation;
				public:
					DrawRenderableCommand(
						::engine::graphics::renderer::IRenderer& renderer,
						const ::engine::graphics::renderable::IRenderable& renderable,
						engine::spatial::PositionF pos,
						spatial::SizeF size,
						::engine::graphics::ColorF color,
						float rotation
					) :
						DrawCommandBase(renderer),
						m_renderable(renderable),
						m_pos(pos),
						m_size(size),
						m_color(color),
						m_rotation(rotation)
					{
					}
					void Execute() override
					{
						m_renderer.DrawRenderable(m_renderable, m_pos, m_size, m_color, m_rotation);
					}
				};

				class DrawSpriteCommand : public DrawCommandBase
				{
				private:
					const ::engine::graphics::renderable::Sprite m_sprite;
					engine::spatial::PositionF m_pos;
					spatial::SizeF m_size;
					::engine::graphics::ColorF m_color;
					float m_rotation;
				public:
					DrawSpriteCommand(
						::engine::graphics::renderer::IRenderer& renderer,
						const ::engine::graphics::renderable::Sprite& sprite,
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
						m_renderer.DrawRenderable(m_sprite, m_pos, m_size, m_color, m_rotation);
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
			};
		}
	}
}
