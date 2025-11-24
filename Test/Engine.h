#pragma once
#include <memory>
#include "Timer.h"
#include "Input.h"
#include "Tile.h"
#include "Color.h"

// forward declare
namespace graphics
{
	namespace renderer
	{
		class ICanvas;
		class IRenderer;
	}

	namespace resource
	{
		class IFontAtlas;
	}
}

// forward declare
namespace Win32
{
	class Window;
}

namespace engine
{
	class Engine
	{
	private:
		std::unique_ptr<Win32::Window> m_window;
		std::unique_ptr<graphics::renderer::ICanvas> m_canvas;
		std::unique_ptr<graphics::renderer::IRenderer> m_Renderer;
		timer::StopWatch m_stopwatch;

		// default font atlas for rendering text utility
		std::shared_ptr<graphics::resource::IFontAtlas> m_fontAtlas;


		void OnInitialize();
		void OnIdle();
		void OnExit();

		void OnWindowClose();
		void OnWindowSize(size_t nWidth, size_t nHeight);
		void OnWindowCreate(void* hWnd);
		void ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam);

		void OnLap(float delta);

	public:
		Engine(
			std::string API = "DirectX11",
			std::string RenderMode = "Batch"
		);
		~Engine();

		event::Event<> OnStart;
		event::Event<> OnRender;
		event::Event<float> OnUpdate;
		event::Event<size_t, size_t> OnResize;
		event::Event<UINT, WPARAM, LPARAM> OnProcessWin32Message;

		graphics::renderer::IRenderer& GetRenderer() 
		{
			return *m_Renderer;
		}

		timer::StopWatch& GetTimer()
		{
			return m_stopwatch;
		}

		void PrintText(const std::string& text, spatial::PosF position = {0, 0}, graphics::ColorF color = { 1, 1, 1, 1});

		void Run();

		void DrawLineSegment(
			const spatial::PosF& start,
			const spatial::PosF& end,
			const graphics::ColorF& color = { 1, 1, 1, 1 },
			float thickness = 1.0f
		);

		void DrawCircleOutline(
			const spatial::PosF& center,
			float radius,
			const graphics::ColorF& color = { 1, 1, 1, 1 },
			float thickness = 1.0f,
			int segments = 32
		);
	};

	namespace io
	{
		template<typename U>
		class TileLayerLoader {
		public:
			static component::tile::TileLayer LoadFromCSV(
				const std::string& filename,
				char delimiter = ',',
				std::function<component::tile::TileInstance(int, int, const U&)> tileLoader =
				[](int row, int col, const U& cell) -> component::tile::TileInstance
				{
					// assumes T has a constructor like T{int}
					return component::tile::TileInstance{ cell };
				}
			)
			{
				utilities::fileio::CSVFile csvFile(filename, delimiter);
				if (!csvFile.read())
				{
					throw std::runtime_error("Failed to read tile layer CSV file.");
				}

				int height = static_cast<int>(csvFile.GetRowCount());
				int width = static_cast<int>(csvFile.GetColCount(0)); // assume uniform width

				component::tile::TileLayer layer;
				layer.SetSize({ width, height });

				for (int row = 0; row < height; ++row)
				{
					for (int col = 0; col < width; ++col)
					{
						// skip rows with inconsistent column count
						if (static_cast<int>(csvFile.GetColCount(row)) != width)
						{
							continue;
						}

						U cell = csvFile.GetValue<U>(row, col);

						component::tile::TileInstance tile = tileLoader(row, col, cell);

						layer.SetTileInstance(row, col, tile);
					}
				}

				return layer;
			}
		};
	}
}

