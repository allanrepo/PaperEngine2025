#pragma once

#include "Window.h"
#include "CanvasFactory.h"
#include "RendererFactory.h"
#include "TextureFactory.h"
#include "Logger.h"
#include "FontAtlas.h"
#include "DrawableSurface.h"

using namespace math::geometry;
using namespace graphics;

/***************************************************************************************************************
this class tests the sprite renderer class implementations.
there are 2 sprite renderer class implementation as of now:
1.	DX11RendererBatchImpl
2.	DX11RendererImmediateImpl

both uses directx 11. in future, may support vulkan and directx 12 as well. Although that is low priority
the tests draws 3 identical sets of sprites, rendered differently:
1.	first is a texture surface and using DX11RendererImmediateImpl (can also use DX11RendererBatchImpl),
	it draws into the texture surface. the texture surface is then rendered using DX11RendererBatchImpl as
	a texture
2.	second is drawn directly into the canvas. the quads and texts rendered are exactly the same as the ones drawn
	in texture surface, including transparency.
3.	third is same as second, except it uses DX11RendererImmediateImpl

Since all are drawn in similar way, they are all expected to look the same. you need to observe visually if there
are any difference in the 3 sets of sprites drawn.
***************************************************************************************************************/
class TestRendererVisualComparison
{
private:
	std::unique_ptr<Win32::Window> window;
	std::unique_ptr<graphics::renderer::ICanvas> canvas;
	std::unique_ptr<graphics::renderer::IRenderer> RendererImmediate;
	std::unique_ptr<graphics::renderer::IRenderer> RendererBatch;
	std::shared_ptr<graphics::resource::DrawableSurface> surface;
	std::shared_ptr<graphics::resource::IFontAtlas> fontAtlasComicSansMS;
	std::shared_ptr<graphics::resource::IFontAtlas> fontAtlasTerminal;
	std::shared_ptr<graphics::resource::IFontAtlas> fontAtlasRoman;

public:
	TestRendererVisualComparison()
	{
		Win32::Window::OnInitialize += event::Handler(this, &TestRendererVisualComparison::OnInitialize);
		Win32::Window::OnExit += event::Handler(this, &TestRendererVisualComparison::OnExit);
		Win32::Window::OnIdle += event::Handler(this, &TestRendererVisualComparison::OnIdle);

		Win32::Window::Run();
	}

	// function that will be called just before we enter into message loop
	void OnInitialize()
	{
		// create our window here
		window = std::make_unique<Win32::Window>();
		window->OnClose += event::Handler(this, &TestRendererVisualComparison::OnWindowClose);
		window->OnCreate += event::Handler(this, &TestRendererVisualComparison::OnWindowCreate);
		window->OnSize += event::Handler(this, &TestRendererVisualComparison::OnWindowSize);
		window->Create(L"Test Bedding", 1400, 900);

		
			//// let's draw something in our drawable texture. rendering like this in main loop is bad. but we're testing...
			//surface->Begin();
			//surface->Clear(1, 1, 1, 1);
			//{
			//	std::unique_ptr<graphics::renderer::IRenderer> localRenderer = graphics::factory::RendererFactory::Create("Immediate");
			//	localRenderer->Initialize();

			//	// let's begin drawing stuff in the texture
			//	localRenderer->Begin();
			//	{
			//		localRenderer->Draw(10.0f, 10.0f, 80.0f, 80.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0); // Draw a rectangle to the texture
			//		localRenderer->Draw(50.0f, 40.0f, 120.0f, 80.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0); // Draw a rectangle to the texture
			//		localRenderer->Draw(70.0f, 50.0f, 80.0f, 100.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0); // Draw a rectangle to the texture

			//		localRenderer->Draw(5.0f, 155.0f, 40.0f, 40.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0); // Draw a rectangle to the texture
			//		localRenderer->DrawText(fontAtlasRoman, "Hello", 0.0f, 150.0f, 1, 0, 0, 1);
			//		localRenderer->Draw(20.0f, 160.0f, 80.0f, 30.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0); // Draw a rectangle to the texture
			//	}
			//	localRenderer->End();
			//}
			//surface->End();
	}

	// fun stuff. this is called on each loop of the message loop. this is where we draw!
	void OnIdle()
	{
		// start the canvas. we can draw from here
		canvas->Begin();
		{
			canvas->Clear(0.2f, 0.2f, 1.0f, 1.0f);
			canvas->SetViewPort();

			// render sprites using immediate renderer. renders a bunch of quads and text at the right side of screen
			RendererImmediate->Begin();
			{
				RendererImmediate->Draw(645.0f, 5.0f, 300.0f, 200.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
				RendererImmediate->Draw(655.0f, 15.0f, 80.0f, 80.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0);
				RendererImmediate->Draw(695.0f, 45.0f, 120.0f, 80.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0);
				RendererImmediate->Draw(715.0f, 55.0f, 80.0f, 100.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0);

				RendererImmediate->Draw(650.0f, 155.0f, 40.0f, 40.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0);
				RendererImmediate->DrawText(fontAtlasRoman, "Hello", 645.0f, 150.0f, 1, 0, 0, 1);
				RendererImmediate->Draw(665.0f, 160.0f, 80.0f, 30.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0);

			}
			RendererImmediate->End();

			// let's draw something in our drawable texture. rendering like this in main loop is bad. but we're testing...
			surface->Begin();
			surface->Clear(1, 1, 1, 1);
			{
				std::unique_ptr<graphics::renderer::IRenderer> localRenderer = graphics::factory::RendererFactory::Create("Immediate");
				localRenderer->Initialize();

				// let's begin drawing stuff in the texture
				localRenderer->Begin();
				{
					localRenderer->Draw(10.0f, 10.0f, 80.0f, 80.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0); // Draw a rectangle to the texture
					localRenderer->Draw(50.0f, 40.0f, 120.0f, 80.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0); // Draw a rectangle to the texture
					localRenderer->Draw(70.0f, 50.0f, 80.0f, 100.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0); // Draw a rectangle to the texture

					localRenderer->Draw(5.0f, 155.0f, 40.0f, 40.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0); // Draw a rectangle to the texture
					localRenderer->DrawText(fontAtlasRoman, "Hello", 0.0f, 150.0f, 1, 0, 0, 1);
					localRenderer->Draw(20.0f, 160.0f, 80.0f, 30.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0); // Draw a rectangle to the texture
				}
				localRenderer->End();
			}
			surface->End();

			// render sprites using batch renderer. 
			RendererBatch->Begin();
			{
				RendererBatch->DrawText(fontAtlasComicSansMS, "Texture Surface", 5.0f, 220.0f, 1, 1, 0, 1);
				RendererBatch->DrawText(fontAtlasTerminal, "Batch DX11", 350.0f, 220.0f, 1, 1, 0, 1);
				RendererBatch->DrawText(fontAtlasRoman, "Immediate DX11", 650.0f, 220.0f, 1, 1, 0, 1);

				// renders our drawable surface into left.it must look like the other ones we render from center and right of screen
				RendererBatch->DrawRenderable(surface, spatial::PosF{ 5.0f, 5.0f }, spatial::SizeF{ surface->GetWidth(), surface->GetHeight() }, ColorF{ 1,1,1,1 }, 0.0f);

				// renders the same thing in center side of screen so it must look like the same one...
				RendererBatch->Draw(325.0f, 5.0f, 300.0f, 200.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
				RendererBatch->Draw(335.0f, 15.0f, 80.0f, 80.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0);
				RendererBatch->Draw(375.0f, 45.0f, 120.0f, 80.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0);
				RendererBatch->Draw(395.0f, 55.0f, 80.0f, 100.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0);

				RendererBatch->Draw(330.0f, 155.0f, 40.0f, 40.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0);
				RendererBatch->DrawText(fontAtlasRoman, "Hello", 325.0f, 150.0f, 1, 0, 0, 1);
				RendererBatch->Draw(345.0f, 160.0f, 80.0f, 30.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0);
			}
			RendererBatch->End();

		}
		// end the canvas. we don't draw anything past this.
		canvas->End();
	}

	void OnExit()
	{

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

		// create font atlas 
		fontAtlasRoman = std::make_shared<graphics::resource::FontAtlas>(graphics::factory::TextureFactory::Create());
		fontAtlasRoman->Initialize("Roman", 32);

		fontAtlasTerminal = std::make_shared<graphics::resource::FontAtlas>(graphics::factory::TextureFactory::Create());
		fontAtlasTerminal->Initialize("Terminal", 32);

		fontAtlasComicSansMS = std::make_shared<graphics::resource::FontAtlas>(graphics::factory::TextureFactory::Create());
		fontAtlasComicSansMS->Initialize("Comic Sans MS", 32);

		// create a drawable texture
		surface = std::make_shared<graphics::resource::DrawableSurface>(graphics::factory::TextureFactory::Create());
		surface->Initialize(300, 200);
	}

	void OnWindowClose()
	{

	}

	void OnWindowSize(size_t nWidth, size_t nHeight)
	{
		LOG("window resize width = " << nWidth << " height = " << nHeight);
		canvas->Resize(static_cast<uint32_t>(nWidth), static_cast<uint32_t>(nHeight));
	}

};

