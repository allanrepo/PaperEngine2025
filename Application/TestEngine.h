#pragma once
#include <Engine/Engine.h>
#include <Graphics/Renderable/DrawableSurface.h>
#include <Engine/Factory/TextureFactory.h>

namespace test
{
	class TestEngine
	{
	private:
		engine::Engine m_engine;
		std::unique_ptr<graphics::renderable::IDrawableSurface> m_surface;

	public:
		TestEngine() :
			m_engine("Engine Testing", "DirectX11", "Batch")
		{
			m_engine.OnStart += event::Handler(this, &TestEngine::OnStart);
			//m_engine.OnUpdate += event::Handler(this, &TestEngine::OnUpdate);
			//m_engine.OnRender += event::Handler(this, &TestEngine::OnRender);
			m_engine.OnResize += event::Handler(this, &TestEngine::OnResize);

			m_engine.Run();
		}

		void OnResize(size_t width, size_t height)
		{
		}

		void OnUpdate(float delta)
		{
		}

		void OnStart()
		{
			// we can trust these lookup tables are already registered in cache by the engine

			// register our sprite name to corresponding sprite atlas key

			// register our image file to atlas UVs lookup table into cache. our atlas UV's are stored in csv file

			// create a sprite using the factory

			// create an image surface for alternate rendering



			// create dx11 texture and use on drawable surface
			m_surface = std::make_unique<graphics::renderable::DrawableSurface>(graphics::TextureFactory::Create());

			// draw stuff on the drawable surface
			m_surface->Initialize(128, 128);
			m_surface->Begin();
			{
				m_surface->Clear(0, 0.5f, 0, 1);
				m_engine.Renderer().Begin();
				{
					m_engine.Renderer().Draw(spatial::PositionF{ 32, 32 }, spatial::SizeF{ 64, 64 }, graphics::ColorF{ 0.5f,0,0,1 }, 0);
					m_engine.Renderer().Draw(spatial::PositionF{ 48, 56 }, spatial::SizeF{ 64, 48 }, graphics::ColorF{ 0,0,0.5f,1 }, 0);
				}
				m_engine.Renderer().End();
			}
			m_surface->End();
		}

		void OnRender()
		{
			// draw a rectangle fill
			m_engine.Renderer().Draw(spatial::PositionF{50, 50}, spatial::SizeF{100, 100}, graphics::ColorF{1,1,0,1}, 0);

			// draw the drawable surface
			m_engine.Renderer().DrawRenderable(*m_surface, spatial::PositionF{ 250, 250 }, m_surface->GetSize(), graphics::ColorF{ 1,1,1,1 }, 0);
		}
	};

}

