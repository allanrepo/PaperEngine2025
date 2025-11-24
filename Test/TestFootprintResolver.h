#pragma once
#include "Camera.h"
#include "Engine.h"
#include "Input.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include "ActorState.h"
#include "IActor.h"
#include "ActorFactory.h"
#include "DrawableSurface.h"
#include "ImageSurface.h"
#include "DX11ImageFileHelper.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include "Pos.h"
#include "PathFinder.h"
#include "FootprintResolver.h"

namespace test
{
	class TestFootprintResolver
	{
	private:
		struct Tile
		{
			int index;
		};
		engine::Engine m_engine;
		component::tile::TileLayer m_tilemap;
		component::tile::Tileset m_tileset;
		spatial::Camera m_camera;
		std::shared_ptr<graphics::resource::IFontAtlas> m_fontSmall;
		std::shared_ptr<graphics::resource::IFontAtlas> m_fontLarge;

		bool m_searchDone = false;

		spatial::SizeF m_tileSize;

		spatial::PosF m_lastMousePos;
		spatial::SizeF m_footPrintSize;

	public:
		TestFootprintResolver() :
			m_engine("DirectX11", "Batch"),
			m_camera({ 50, 50, 1024, 768 }),
			m_tileSize{ 50, 50 },
			m_footPrintSize{ 480, 480 }
		{
			m_engine.OnStart += event::Handler(this, &TestFootprintResolver::OnStart);
			m_engine.OnUpdate += event::Handler(this, &TestFootprintResolver::OnUpdate);
			m_engine.OnRender += event::Handler(this, &TestFootprintResolver::OnRender);
			m_engine.OnResize += event::Handler(this, &TestFootprintResolver::OnResize);

			input::Input::Instance().OnKeyDown += event::Handler(this, &TestFootprintResolver::OnKeyDown);
			input::Input::Instance().OnMouseDown += event::Handler(this, &TestFootprintResolver::OnMouseDown);
			input::Input::Instance().OnMouseMove += event::Handler(this, &TestFootprintResolver::OnMouseMove);
			input::Input::Instance().OnMouseUp += event::Handler(this, &TestFootprintResolver::OnMouseUp);

			m_engine.Run();
		}

		void OnResize(size_t width, size_t height)
		{
		}

		void OnMouseMove(int x, int y)
		{
			m_lastMousePos = { static_cast<float>(x), static_cast<float>(y) };
		}

		void OnMouseDown(int btn, int x, int y)
		{
			if (btn == 1) // left button
			{
			}
			else if (btn == 2) // right button
			{
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
		}

		void OnKeyDown(int key)
		{
			if (key == 49) // 1
			{
				// set selected tile as obstacle
				spatial::PosF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
				component::tile::TileCoord tileCoord
				{
					static_cast<int>(worldPos.y / m_tileSize.height),
					static_cast<int>(worldPos.x / m_tileSize.width)
				};
				if (tileCoord.row >= 0 && tileCoord.row < m_tilemap.GetHeight() && tileCoord.col >= 0 && tileCoord.col < m_tilemap.GetWidth())
				{
					component::tile::TileInstance tileInst;
					tileInst.index = 1;
					m_tilemap.SetTileInstance(tileCoord.row, tileCoord.col, tileInst);
				}
			}
			if (key == 50) // 2
			{
				// set selected tile as obstacle
				spatial::PosF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
				component::tile::TileCoord tileCoord
				{
					static_cast<int>(worldPos.y / m_tileSize.height),
					static_cast<int>(worldPos.x / m_tileSize.width)
				};
				if (tileCoord.row >= 0 && tileCoord.row < m_tilemap.GetHeight() && tileCoord.col >= 0 && tileCoord.col < m_tilemap.GetWidth())
				{
					component::tile::TileInstance tileInst;
					tileInst.index = 0;
					m_tilemap.SetTileInstance(tileCoord.row, tileCoord.col, tileInst);
				}
			}
			if (key == 51) // 3
			{
				// remove all obstacles
				for (int row = 0; row < m_tilemap.GetHeight(); row++)
				{
					for (int col = 0; col < m_tilemap.GetWidth(); col++)
					{
						if (!m_tileset.GetTile(m_tilemap.GetTileInstance(row, col).index).IsWalkable())
						{
							component::tile::TileInstance tileInst;
							tileInst.index = 0;
							m_tilemap.SetTileInstance(row, col, tileInst);
						}
					}
				}
			}

			if (key == 32) // space key
			{
			}
			if (key == 53)
			{
				m_footPrintSize.width += 10;
			}
			if (key == 54)
			{
				m_footPrintSize.height += 10;
			}
			if (key == 55)
			{
				m_footPrintSize.width -= 10;
			}
			if (key == 56)
			{
				m_footPrintSize.height -= 10;
			}
		}

		void OnStart()
		{
			m_tileset.Register(0, std::make_unique<component::tile::WalkableTile>());   // ID 0 → Walkable
			m_tileset.Register(1, std::make_unique<component::tile::ObstacleTile>());   // ID 1 → Obstacle
			SetTileLayer(m_tilemap, 16, 16, component::tile::TileInstance{ 0 });

			m_camera.SetViewport(
				{
					50,
					50,
					50 + m_tilemap.GetWidth() * m_tileSize.width,
					50 + m_tilemap.GetHeight() * m_tileSize.height
				}
			);

			m_camera.SetWorldSize(
				m_tilemap.GetWidth() * m_tileSize.width,
				m_tilemap.GetHeight() * m_tileSize.height
			);

			m_engine.GetRenderer().SetClipRegion(m_camera.GetViewport());
			m_engine.GetRenderer().EnableClipping(true);

			// create utility font atlas
			m_fontSmall = std::make_shared<graphics::resource::FontAtlas>(graphics::factory::TextureFactory::Create());
			m_fontSmall->Initialize("Arial", 12);

			m_fontLarge = std::make_shared<graphics::resource::FontAtlas>(graphics::factory::TextureFactory::Create());
			m_fontLarge->Initialize("Arial", 24);

		}

		void OnUpdate(float delta)
		{
		}

		void OnRender()
		{
			m_engine.GetRenderer().EnableClipping(true);

			RenderTileMap(m_tilemap);

			navigation::tile::Footprint footprint;
			footprint.position = m_camera.ScreenToWorld(m_lastMousePos);
			footprint.size = m_footPrintSize;

			navigation::tile::FootprintResolver footprintResolver(
				[this](int row, int col) -> bool
				{
					return component::tile::IsWalkable(m_tilemap, m_tileset, row, col);
				},
				0.1f, footprint.size.width / 2.0f, footprint.size.height / 2.0f, true
			);

			bool IsWalkable = footprintResolver.IsValid(
				m_tilemap,
				m_tileSize,
				footprint
			);

			bool isNudged = false;
			navigation::tile::Footprint nudgedFootprint = footprint;
			if (!IsWalkable)
			{
				isNudged = footprintResolver.TryResolve(
					m_tilemap,
					m_tileSize,
					footprint,
					nudgedFootprint
				);
			}

			spatial::PosF nudgedPosition;
			if (isNudged)
			{
				nudgedPosition = nudgedFootprint.position;
			}

			spatial::PosF footPrintTopLeftPos
			{
				m_lastMousePos.x - m_footPrintSize.width / 2,
				m_lastMousePos.y - m_footPrintSize.height / 2
			};

			spatial::PosF footPrintTopLeftNudgedPos
			{
				nudgedPosition.x - m_footPrintSize.width / 2,
				nudgedPosition.y - m_footPrintSize.height / 2
			};

			// if walkable
			if (IsWalkable)
			{
				m_engine.GetRenderer().Draw(footPrintTopLeftPos, m_footPrintSize, graphics::ColorF{ 0,1,0,0.5f }, 0);
			}

			// if not walkable, but nudged
			if (!IsWalkable && isNudged)
			{
				m_engine.GetRenderer().Draw(footPrintTopLeftPos, m_footPrintSize, graphics::ColorF{ 1,1,0.5f,0.1f }, 0);
				m_engine.GetRenderer().Draw(m_camera.WorldToScreen(footPrintTopLeftNudgedPos), m_footPrintSize, graphics::ColorF{ 0,1,0,0.5f }, 0);
			}

			// if not walkable, and not nudged
			if (!IsWalkable && !isNudged)
			{
				m_engine.GetRenderer().Draw(footPrintTopLeftPos, m_footPrintSize, graphics::ColorF{ 1,0,0,0.5f }, 0);
			}
		}

		void RenderTileMap(component::tile::TileLayer& tilemap)
		{
			float thickness = 1.0f;
			spatial::SizeF inner{ m_tileSize.width - thickness * 2, m_tileSize.height - thickness * 2 };

			// draw the tile map
			for (int row = 0; row < tilemap.GetHeight(); row++)
			{
				for (int col = 0; col < tilemap.GetWidth(); col++)
				{
					component::tile::TileInstance tileInst = tilemap.GetTileInstance(row, col);

					graphics::ColorF color;
					switch (tileInst.index)
					{
					case 0:
						color = { 0.5f, 0.5f, 0.5f, 1 };
						break;
					case 1:
						color = { 0, 0, 0, 1 };
						break;
					default:
						color = { 0, 0, 0, 1 };
						break;
					}
					spatial::PosF pos
					{
						m_tileSize.width * col,
						m_tileSize.height * row
					};

					spatial::PosF posInner
					{
						m_tileSize.width * col + thickness,
						m_tileSize.height * row + thickness
					};

					// draw stuff
					m_engine.GetRenderer().Draw(m_camera.WorldToScreen(pos), m_tileSize, { 0,0,0,1 }, 0);
					m_engine.GetRenderer().Draw(m_camera.WorldToScreen(posInner), inner, color, 0);
				}
			}
		}
		void SetTileLayer(component::tile::TileLayer& tileLayer, int width, int height, component::tile::TileInstance tileInst)
		{
			tileLayer.SetSize({ width, height });
			for (int row = 0; row < height; row++)
			{
				for (int col = 0; col < width; col++)
				{
					tileLayer.SetTileInstance(row, col, tileInst);
				}
			}
		}

	};
}

