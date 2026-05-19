#pragma once

#include "Window.h"
#include "CanvasFactory.h"
#include "RendererFactory.h"
#include "TextureFactory.h"
#include "Logger.h"
#include "FontAtlas.h"
#include "DrawableSurface.h"
#include "Timer.h"
#include "SpriteFactory.h"

using namespace math::geometry;
using namespace graphics;


class StressTestRenderer
{
private:
	struct Quad
	{
		spatial::PosF pos;
		spatial::SizeF size;
		graphics::ColorF color;
	};
	std::vector<Quad> quads;

	std::unique_ptr<Win32::Window> window;
	std::unique_ptr<graphics::renderer::ICanvas> canvas;
	std::unique_ptr<graphics::renderer::IRenderer> RendererImmediate;
	std::unique_ptr<graphics::renderer::IRenderer> RendererBatch;
	std::shared_ptr<graphics::resource::IDrawableSurface> surface;
	timer::StopWatch stopwatch;
	float elapsedTime = 0;

public:
	StressTestRenderer()
	{
		Win32::Window::OnInitialize += event::Handler(this, &StressTestRenderer::OnInitialize);
		Win32::Window::OnExit += event::Handler(this, &StressTestRenderer::OnExit);
		Win32::Window::OnIdle += event::Handler(this, &StressTestRenderer::OnIdle);

		Win32::Window::Run();
	}

	// function that will be called just before we enter into message loop
	void OnInitialize()
	{
		// create our window here
		window = std::make_unique<Win32::Window>();
		window->OnClose += event::Handler(this, &StressTestRenderer::OnWindowClose);
		window->OnCreate += event::Handler(this, &StressTestRenderer::OnWindowCreate);
		window->OnSize += event::Handler(this, &StressTestRenderer::OnWindowSize);
		window->Create(L"Test Bedding", 1400, 900);
	}

	// when window is created
	void OnWindowCreate(void* hWnd)
	{
		// create dx11 canvas
		canvas = graphics::CanvasFactory::Create();
		canvas->Initialize(hWnd);

		// create sprite renderer immediate
		RendererImmediate = graphics::factory::RendererFactory::Create("Immediate");
		RendererImmediate->Initialize();

		// create sprite renderer batch
		RendererBatch = graphics::factory::RendererFactory::Create("Batch");
		RendererBatch->Initialize();

		// create an image surface
		surface = std::make_shared<graphics::resource::DrawableSurface>(graphics::factory::TextureFactory::Create());
		surface->Initialize(64, 64);
		surface->Begin();
		surface->Clear(1, 1, 1, 1);
		surface->End();

		// setup stopwatch to manage timing
		stopwatch.OnLap += event::Handler(this, &StressTestRenderer::OnLap);
		stopwatch.Start();		
	}

	void OnLap(float delta)
	{
		elapsedTime += delta;
	}

	// fun stuff. this is called on each loop of the message loop. this is where we draw!
	void OnIdle()
	{
		stopwatch.Lap<timer::milliseconds>();

		// start the canvas. we can draw from here
		canvas->Begin();
		{
			canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);
			canvas->SetViewPort();

			// render sprites using immediate renderer. renders a bunch of quads and text at the right side of screen
			RendererImmediate->Begin();
			{
			}
			RendererImmediate->End();

			// render sprites using batch renderer. 
			RendererBatch->Begin();
			{
				float j = 0;
				for (Quad i : quads)
				{
					//SpriteBatch::SetVisibleRegion(vp);
					//RendererBatch->Draw(i.pos.x, i.pos.y, i.size.width, i.size.height, i.color.red, i.color.green, i.color.blue, i.color.alpha, elapsedTime / j);
					RendererBatch->DrawRenderable(surface, i.pos, i.size, i.color, elapsedTime / j);
					j += 0.1f;
					if (j > 5) j = 0;
				}
			}
			RendererBatch->End();

		}
		// end the canvas. we don't draw anything past this.
		canvas->End();
	}

	void OnExit()
	{

	}

	void OnWindowClose()
	{

	}

	void OnWindowSize(size_t nWidth, size_t nHeight)
	{
		LOG("window resize width = " << nWidth << " height = " << nHeight);
		canvas->Resize(static_cast<uint32_t>(nWidth), static_cast<uint32_t>(nHeight));

		CalculateQuads(spatial::SizeF{ static_cast<float>(nWidth), static_cast<float>(nHeight) });
	}

	void CalculateQuads(spatial::SizeF size)
	{
#pragma region // calculate quads
		{
			unsigned int row = 10;
			unsigned int col = 10;
			float max = 200000;

			quads.clear();
			float width = (float)size.width / col;
			float height = (float)size.height / row;
			float r = 0;
			float g = 0;
			float b = 0;
			float frames = max / (row * col);
			for (int k = 0; k < frames; k++)
			{
				for (unsigned int i = 0; i < row; i++)
				{
					for (unsigned int j = 0; j < col; j++)
					{
						spatial::PosF pos{ width * j, height * i };
						Quad qi;
						qi.pos = spatial::PosF{ width * j, height * i };
						qi.size = spatial::SizeF{ width, height };
						qi.color = graphics::ColorF{ r, g, b, 0.5f };
						quads.push_back(qi);
						r += 0.01f;
						g += 0.05f;
						b += 0.09f;
						r = r > 1 ? r -= 1 : r;
						g = g > 1 ? g -= 1 : g;
						b = b > 1 ? b -= 1 : b;
					}
				}
			}
		}
#pragma endregion
	}



};

