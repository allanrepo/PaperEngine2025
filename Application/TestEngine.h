#pragma once
#include <Engine/Engine.h>

namespace test
{
	class TestEngine
	{
	private:
		engine::Engine m_engine;

	public:
		TestEngine() :
			m_engine("DirectX11", "Batch")
		{
			m_engine.OnStart += event::Handler(this, &TestEngine::OnStart);
			m_engine.OnUpdate += event::Handler(this, &TestEngine::OnUpdate);
			m_engine.OnRender += event::Handler(this, &TestEngine::OnRender);
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
		}

		void OnRender()
		{
		}
	};

}

