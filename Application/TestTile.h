#pragma once

// this test demonstrates how to create a simple animation system using the Animator and Animation classes.
// notes:
// - SpriteAtlas' constructors are protected, so we create MockSpriteAtlas classes to instantiate 
//	 them directly for testing purposes without using factories.
// - a function is defined to calculate UV rectangles for SpriteAtlas with the assumption that the layout is a grid. 
//	 the sprites in SpriteAtlas are also assumed to be sequenced row by row.
// - Animation is also created manually without using any factory or loader and frames were added directly
// - we needed to provide elapsed time to run the animation, so a stopwatch is used to measure time between frames
//   StopWatch's Lap method is called to measure and provide elapsed time per frame to the animator
// - since we use Sprite here to demonstrate animation, we actually test Sprite as well

#pragma once

#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/Canvas.h>
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/Renderer.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Core/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Animation/Animation.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Timer/StopWatch.h>
#include <Components/Tile.h>
#include <Engine/Loader/AsyncLoader.h>

namespace TestTile
{	
	// we are mocking the sprite atlas class here for demo purpose so we can create sprite directly without using factory
	class MockSpriteAtlas : public engine::graphics::resource::SpriteAtlas
	{
	public:
		MockSpriteAtlas(std::unique_ptr<engine::graphics::resource::ITexture> tex) :
			SpriteAtlas(std::move(tex))
		{
		}
	};

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
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		std::unique_ptr<MockSpriteAtlas> m_spriteAtlas;
		engine::timer::StopWatch m_stopwatch;
		engine::component::tile1::Tileset<RenderableTile> m_tileset;
		engine::component::tile1::TileGrid<RenderableTile> m_tilegrid;
		engine::spatial::SizeF m_tileSize{ 32.0f, 32.0f };

	public:
		Test()
		{
			engine::win32::Window::OnInitialize += engine::event::Handler(this, &Test::OnInitialize);
			engine::win32::Window::OnExit += engine::event::Handler(this, &Test::OnExit);
			engine::win32::Window::OnIdle += engine::event::Handler(this, &Test::OnIdle);

			engine::win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<engine::win32::Window>();
			m_window->OnClose += engine::event::Handler(this, &Test::OnWindowClose);
			m_window->OnCreate += engine::event::Handler(this, &Test::OnWindowCreate);
			m_window->OnSize += engine::event::Handler(this, &Test::OnWindowSize);
			m_window->Create(L"Test Sprite", 1400, 900);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<engine::graphics::Canvas>(std::make_unique<engine::graphics::dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer (DX11) created...");

			// create sprite atlas manually for demo purpose
			m_spriteAtlas = std::make_unique<MockSpriteAtlas>(std::make_unique<engine::graphics::dx11::resource::DX11TextureImpl>());

			// load sprite atlas from file manually for demo purpose
			m_spriteAtlas->Initialize(L"../Assets/4x1_128x32_tile.png");

			// load sprite atlas UVs from csv manually for demo purpose. we calculate UVs here by assuming a grid of 8 rows and 12 columns
			// in real scenario, you would use SpriteAtlasLoader to load from csv file 
			std::vector<engine::math::geometry::RectF> uvs = CalcUV(1, 4, (int)m_spriteAtlas->GetWidth(), (int)m_spriteAtlas->GetHeight());
			for (engine::math::geometry::RectF& rect : uvs)
			{
				m_spriteAtlas->AddUVRect(rect);
			}

			// register tiles
			m_tileset.Register(0, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(0), true)); // walkable
			m_tileset.Register(1, std::make_unique<RenderableTile>(m_spriteAtlas->MakeSprite(1), false)); // obstacle

			// load map into tile layer
			{
				engine::io::AsyncFileReader	fileReader(0xFF);
				fileReader.Open("../Assets/PathFindingMap_24x16.csv");

				engine::utilities::parser::CSVParser csvParser;
				fileReader.ProcessChunkEvent += engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
				fileReader.EndOfFileFoundEvent += engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
				engine::container::Table<std::string> table;

				csvParser.ParseRowEvent += engine::event::Handler(&table, &engine::container::Table<std::string>::AddRow);
				csvParser.ParseRemainingEvent += engine::event::Handler(&table, &engine::container::Table<std::string>::AddRange);

				fileReader.SyncReadAll(0xFF, 5.0);

				engine::loader::tile::AsyncTileGridLoader<RenderableTile, int> tileLoader;
				tileLoader.LoadImmediate(
					m_tilegrid,
					table,
					[this](const int& cell) -> engine::component::tile1::Tile<RenderableTile>
					{
						// this is safe. tileset will return "empty" tile if id is invalid. "empty" means does not have reference to tile data. tile is invalid
						return m_tileset.MakeTile(cell);
					},
					0xFFFF,
					5.0
				);

				fileReader.ProcessChunkEvent -= engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
				fileReader.EndOfFileFoundEvent -= engine::event::Handler(&csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
				csvParser.ParseRowEvent -= engine::event::Handler(&table, &engine::container::Table<std::string>::AddRow);
				csvParser.ParseRemainingEvent -= engine::event::Handler(&table, &engine::container::Table<std::string>::AddRange);
			}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += engine::event::Handler(this, &Test::OnLap);
			m_stopwatch.Start();
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					RenderTiles(m_tilegrid);
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
			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}

		std::vector<engine::math::geometry::RectF> CalcUV(int row, int col, int fileWidth, int fileHeight)
		{
			std::vector<engine::math::geometry::RectF> uvs;
			float width = static_cast<float>(fileWidth / col);
			float height = static_cast<float>(fileHeight / row);
			float left = 0;
			float top = 0;
			float right = left + width;
			float bottom = top + height;

			for (int r = 0; r < row; r++)
			{
				for (int c = 0; c < col; c++)
				{
					left = width * c;
					top = height * r;
					right = left + width;
					bottom = top + height;

					left /= fileWidth;
					top /= fileHeight;
					right /= fileWidth;
					bottom /= fileHeight;

					uvs.push_back(engine::math::geometry::RectF{ left, top, right, bottom });

					//LOG(std::to_string(left) << ", " << std::to_string(top) << ", " << std::to_string(right) << ", " << std::to_string(bottom));
				}
			}
			return uvs;
		}

		void RenderTiles(engine::component::tile1::TileGrid<RenderableTile>& TileGrid)
		{
			for (int row = 0; row < TileGrid.GetHeight(); ++row)
			{
				for (int col = 0; col < TileGrid.GetWidth(); ++col)
				{
					const engine::component::tile1::Tile<RenderableTile>& tile = TileGrid.Get(row, col);
					if (tile.IsValid())
					{
						m_renderer->Draw(
							tile->GetSprite(),
							engine::spatial::PositionF{ 50.0f + col * m_tileSize.width, 50.0f + row * m_tileSize.height },
							m_tileSize,
							engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, 1.0f },
							0.0f
						);
					}
				}
			}
		}

		
	};
}