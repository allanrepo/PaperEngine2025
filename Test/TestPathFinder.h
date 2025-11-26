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

#define FOOTPRINT_AWARE_BUT_SNAP_TILE_CENTER 1

namespace testPathFinder
{
	class TestPathFinder
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

		std::vector<component::tile::TileCoord> m_path;

		int m_step = 0;
		bool m_searchDone = false;

		spatial::SizeF m_tileSize;
		navigation::tile::PathFinder m_pathFinder;
		component::tile::TileCoord m_startTile;
		component::tile::TileCoord m_goalTile;

		spatial::PosF m_lastMousePos;

		spatial::SizeF m_footPrintSize;

	public:
		TestPathFinder() :
			m_engine("DirectX11", "Batch"),
			m_camera({ 50, 50, 1024, 768 }),
			m_pathFinder(
#if FOOTPRINT_AWARE_BUT_SNAP_TILE_CENTER == 1
				[this](int currRow, int currCol, int row, int col) -> bool
				{
					// quick check if the tile itself is walkable
					if (!component::tile::IsWalkable(m_tilemap, m_tileset, row, col)) return false;

					// define corners of the footprint as positions. we want it as positions so we can iterate through it easier
					std::vector<spatial::PosF> corners;
					corners.push_back({ 0, 0 }); // top-left
					corners.push_back({ 0, m_footPrintSize.height }); // bottom-left
					corners.push_back({ m_footPrintSize.width, 0 }); // top-right
					corners.push_back({ m_footPrintSize.width, m_footPrintSize.height }); // bottom-right


					for (spatial::PosF pos : corners)
					{
						// translate corners so that it is center at footprint's origin (0,0)
						pos -= {m_footPrintSize.width / 2, m_footPrintSize.height / 2};

						// translate corners to world space based on tile coordinate
						pos += {
							col* m_tileSize.width,
								row* m_tileSize.height
						};

						// translate corners so that it's center is at center of the tile
						pos += {
							m_tileSize.width / 2,
								m_tileSize.height / 2
						};

						// get the tile coord that the corner intersects with
						component::tile::TileCoord tc =
						{
							(int)std::floor(pos.y / m_tileSize.height),
							(int)std::floor(pos.x / m_tileSize.width)
						};

						// is the tile coordinate valid? if not, then out of bounds is unwalkable
						if (m_tilemap.IsValidTile(tc.row, tc.col) == false)
						{
							return false; // out of bounds is unwalkable
						}

						// check if this tile coordinate is unwalkable
						if (!component::tile::IsWalkable(m_tilemap, m_tileset, tc.row, tc.col)) return false;
					}

					// if we reach this point
					return true;
				},
#else
				[this](int row, int col) -> bool
				{
					Tile tile = m_tilemap.GetTile(row, col);
					return tile.index != 1; // walkable if index is not 1
				},
#endif				
				1000,
				true,
				false
			),
			m_tileSize{ 64.0f, 64.0f },
			m_footPrintSize{ 48.0f, 48.0f },
			m_startTile{ -1, -1 },
			m_goalTile{ -1, -1 }
		{
			m_engine.OnStart += event::Handler(this, &TestPathFinder::OnStart);
			m_engine.OnUpdate += event::Handler(this, &TestPathFinder::OnUpdate);
			m_engine.OnRender += event::Handler(this, &TestPathFinder::OnRender);
			m_engine.OnResize += event::Handler(this, &TestPathFinder::OnResize);

			input::Input::Instance().OnKeyDown += event::Handler(this, &TestPathFinder::OnKeyDown);
			input::Input::Instance().OnMouseDown += event::Handler(this, &TestPathFinder::OnMouseDown);
			input::Input::Instance().OnMouseMove += event::Handler(this, &TestPathFinder::OnMouseMove);
			input::Input::Instance().OnMouseUp += event::Handler(this, &TestPathFinder::OnMouseUp);

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
				// set start tile

				// this is screen coordinate. convert to world coordinate (tilemap coordinate)
				spatial::PosF worldPos = m_camera.ScreenToWorld({ static_cast<float>(x), static_cast<float>(y) });

				component::tile::TileCoord tileCoord
				{
					static_cast<int>(worldPos.y / m_tileSize.height),
					static_cast<int>(worldPos.x / m_tileSize.width)
				};
				if (tileCoord.row >= 0 && tileCoord.row < m_tilemap.GetHeight() && tileCoord.col >= 0 && tileCoord.col < m_tilemap.GetWidth())
				{
					m_startTile = tileCoord;
				}

				m_step = 0; // reset step
			}
			else if (btn == 2) // right button
			{
				// set goal tile

				// this is screen coordinate. convert to world coordinate (tilemap coordinate)
				spatial::PosF worldPos = m_camera.ScreenToWorld({ static_cast<float>(x), static_cast<float>(y) });

				component::tile::TileCoord tileCoord
				{
					static_cast<int>(worldPos.y / m_tileSize.height),
					static_cast<int>(worldPos.x / m_tileSize.width)
				};
				if (tileCoord.row >= 0 && tileCoord.row < m_tilemap.GetHeight() && tileCoord.col >= 0 && tileCoord.col < m_tilemap.GetWidth())
				{
					m_goalTile = tileCoord;
				}

				m_step = 0; // reset step
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
		}

		void OnKeyDown(int key)
		{

			if (key == 32) // space key
			{
				m_step++;
			}
			if (key == 49) // 1
			{
				m_step = 0;
			}
			if (key == 50) // 1
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
			if (key == 51) // 2
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
			if (key == 52) // 3
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
			LOG("step: " << m_step);
		}


		void OnStart()
		{
			m_tileset.Register(0, std::make_unique<component::tile::WalkableTile>());   // ID 0 → Walkable
			m_tileset.Register(1, std::make_unique<component::tile::ObstacleTile>());   // ID 1 → Obstacle

			m_tilemap = engine::io::TileLayerLoader<int>::LoadFromCSV("PathfindingTileMap.csv", ',');

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

			m_startTile = component::tile::TileCoord{ m_tilemap.GetHeight() - 1, 2 };
			m_goalTile = component::tile::TileCoord{ 0, m_tilemap.GetWidth() - 3 };

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

			//if (!m_searchDone)
			{
				m_searchDone = m_pathFinder.FindPath(
					math::geometry::Rect<int>{ 0, 0, m_tilemap.GetWidth(), m_tilemap.GetHeight() },
					m_startTile,
					m_goalTile,
					m_path,
					m_step
				);
			}

			RenderTileMap(m_tilemap);

			std::vector<spatial::PosF> wp = navigation::tile::GetWayPoints(m_path);

			for (size_t i = 1; i < wp.size(); i++)
			{
				spatial::PosF start
				{
					wp[i - 1].x * m_tileSize.width + m_tileSize.width / 2,
					wp[i - 1].y * m_tileSize.height + m_tileSize.height / 2
				};
				spatial::PosF end
				{
					wp[i].x * m_tileSize.width + m_tileSize.width / 2,
					wp[i].y * m_tileSize.height + m_tileSize.height / 2
				};
				m_engine.DrawLineSegment(
					m_camera.WorldToScreen(start),
					m_camera.WorldToScreen(end),
					{ 1, 0, 1, 1 },
					4.0f
				);
			}
		}

		void RenderTileMap(component::tile::TileLayer& tilemap)
		{
			float thickness = 2.0f;
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

					for (const component::tile::TileCoord& tile : m_pathFinder.GetOpenTiles())
					{
						if (tile.row == row && tile.col == col)
						{
							color = { 0, 0.5f, 0, 1 };
						}
					}

					for (const component::tile::TileCoord& tile : m_pathFinder.GetClosedTiles())
					{
						if (tile.row == row && tile.col == col)
						{
							color = { 0.5f, 0, 0, 1 };
						}
					}

					if (m_startTile.row == row && m_startTile.col == col)
					{
						color = { 0.3f, 0.3f, 0, 1 };
					}

					if (m_goalTile.row == row && m_goalTile.col == col)
					{
						color = { 0, 0.3f, 0.3f, 1 };
					}

					for (const component::tile::TileCoord& tile : m_path)
					{
						if (tile.row == row && tile.col == col)
						{
							color = { 0, 0, 0.5f, 1 };
						}
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

			for (const component::tile::TileCoord& tile : m_path)
			{
				spatial::PosF pos
				{
					m_tileSize.width * tile.col + (m_tileSize.width - m_footPrintSize.width) / 2.0f,
					m_tileSize.height * tile.row + (m_tileSize.height - m_footPrintSize.height) / 2.0f
				};

				m_engine.GetRenderer().Draw(m_camera.WorldToScreen(pos), m_footPrintSize, { 1, 1, 0, 0.3f }, 0);
			}

			std::string str;
			str.reserve(128);
			// draw tile coordinates
			for (int row = 0; row < tilemap.GetHeight(); row++)
			{
				for (int col = 0; col < tilemap.GetWidth(); col++)
				{
					spatial::PosF pos
					{
						m_tileSize.width * col + thickness,
						m_tileSize.height * row + thickness
					};

					str.clear();
					str.append(std::to_string(row));
					str.append(",");
					str.append(std::to_string(col));

					m_engine.GetRenderer().DrawText(
						m_fontSmall,
						str,
						m_camera.WorldToScreen(pos).x,
						m_camera.WorldToScreen(pos).y,
						1, 1, 1, 1
					);
				}
			}

			// draw costs for open tiles
			for (const component::tile::TileCoord& tile : m_pathFinder.GetOpenTiles())
			{
				navigation::tile::Node node = m_pathFinder.GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.g));
				str.append(",");
				str.append(std::to_string(node.h));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + thickness,
					m_tileSize.height * tile.row + thickness + 16
				};

				m_engine.GetRenderer().DrawText(
					m_fontSmall,
					str,
					m_camera.WorldToScreen(pos).x,
					m_camera.WorldToScreen(pos).y,
					1, 1, 1, 1
				);
			}

			// draw costs for close tiles
			for (const component::tile::TileCoord& tile : m_pathFinder.GetClosedTiles())
			{
				navigation::tile::Node node = m_pathFinder.GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.g));
				str.append(",");
				str.append(std::to_string(node.h));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + thickness,
					m_tileSize.height * tile.row + thickness + 16
				};

				m_engine.GetRenderer().DrawText(
					m_fontSmall,
					str,
					m_camera.WorldToScreen(pos).x,
					m_camera.WorldToScreen(pos).y,
					1, 1, 1, 1
				);
			}

			// draw total cost for open tiles
			for (const component::tile::TileCoord& tile : m_pathFinder.GetOpenTiles())
			{
				navigation::tile::Node node = m_pathFinder.GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.f()));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + thickness,
					m_tileSize.height * tile.row + thickness + 32
				};

				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					m_camera.WorldToScreen(pos).x,
					m_camera.WorldToScreen(pos).y,
					1, 1, 1, 1
				);
			}

			// draw total cost for close tiles
			for (const component::tile::TileCoord& tile : m_pathFinder.GetClosedTiles())
			{
				navigation::tile::Node node = m_pathFinder.GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.f()));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + thickness,
					m_tileSize.height * tile.row + thickness + 32
				};

				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					m_camera.WorldToScreen(pos).x,
					m_camera.WorldToScreen(pos).y,
					1, 1, 1, 1
				);
			}
		}

	};
}

