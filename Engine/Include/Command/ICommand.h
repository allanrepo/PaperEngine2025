#pragma once
#include <Graphics/Renderer/IRenderer.h>
#include <unordered_map>

namespace engine
{
	namespace command
	{
		enum class Type 
		{ 
			Logic, 
			Render, 
			Audio, 
			Input 
		};

		class ICommand
		{
		public:
			virtual ~ICommand() = default;
			virtual void Execute() = 0;
			virtual Type GetType() const = 0;
		};

		class CommandQueue
		{
		private:
			std::unordered_map<engine::command::Type, std::vector<std::unique_ptr<ICommand>>> queues;

		public:
			virtual ~CommandQueue() = default;
			
			void Enqueue(std::unique_ptr<ICommand> command)
			{
				queues[command->GetType()].emplace_back(std::move(command));
			}

			void Dispatch(engine::command::Type type, bool clear = true)
			{
				auto& queue = queues[type];
				for (auto& cmd : queue)
				{
					cmd->Execute();
				}

				if (clear)
				{
					queue.clear();
				}
			}

			void Clear(engine::command::Type type)
			{
				auto& queue = queues[type];
				queue.clear();
			}

			virtual bool IsEmpty() const
			{
				for (const auto& pair : queues)
				{
					if (!pair.second.empty())
					{
						return false;
					}
				}
				return true;
			}

			void Clear() 
			{
				for (auto& [type, q] : queues)
				{
					q.clear();
				}
			}
		};

		namespace graphics
		{
			namespace renderer
			{
				// base class for all render commands
				class DrawCommandBase : public ICommand
				{
				protected:
					::graphics::renderer::IRenderer& m_renderer;

				public:
					DrawCommandBase(::graphics::renderer::IRenderer& renderer) :
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
					spatial::PositionF m_pos;
					spatial::SizeF m_size;
					::graphics::ColorF m_color;
					float m_rotation;

				public:
					DrawQuadCommand(
						::graphics::renderer::IRenderer& renderer,
						spatial::PositionF pos,
						spatial::SizeF size,
						::graphics::ColorF color,
						float rotation
					): 
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
					const ::graphics::renderable::IFontAtlas& m_font;
					std::string m_text;
					spatial::PositionF m_pos;
					::graphics::ColorF m_color;
				public:
					DrawTextCommand(
						::graphics::renderer::IRenderer& renderer,
						const ::graphics::renderable::IFontAtlas& font,
						const std::string& text,
						spatial::PositionF pos,
						::graphics::ColorF color
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
					const ::graphics::renderable::IRenderable& m_renderable;
					spatial::PositionF m_pos;
					spatial::SizeF m_size;
					::graphics::ColorF m_color;
					float m_rotation;
				public:
					DrawRenderableCommand(
						::graphics::renderer::IRenderer& renderer,
						const ::graphics::renderable::IRenderable& renderable,
						spatial::PositionF pos,
						spatial::SizeF size,
						::graphics::ColorF color,
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
			};
		}
	}
}
