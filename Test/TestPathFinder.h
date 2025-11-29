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

namespace testPathFinder
{
	class TestPathFinder
	{
	private:
		engine::Engine m_engine;
		component::tile::TileLayer m_tileLayer;
		component::tile::Tileset m_tileset;
		spatial::Camera m_camera;
		std::shared_ptr<graphics::resource::IFontAtlas> m_fontSmall;
		std::shared_ptr<graphics::resource::IFontAtlas> m_fontLarge;
		component::tile::TileCoord m_startTile;
		component::tile::TileCoord m_goalTile;
		navigation::tile::PathFinder m_pathFinderVector;
		navigation::tile::PathFinderUsingPriorityQueue m_pathFinderPriorityQueue;
		std::vector<component::tile::TileCoord> m_path;
		spatial::SizeF m_tileSize;
		bool m_drawText = true;
		bool m_drawPathFindingTiles = true;
		bool m_drawWaypoint = true;
		spatial::PosF m_lastMousePos;
		int m_step = 0;
		navigation::tile::PathFinder* m_pathFinder;

	public:
		TestPathFinder() :
			m_engine("DirectX11", "Batch"),
			m_camera({ 50, 50, 1024, 768 }),
			m_pathFinderVector(
				[this](int currRow, int currCol, int row, int col) -> bool
				{
					return component::tile::IsWalkable(m_tileLayer, m_tileset, row, col);
				},
				0,
				true,
				false
			),
			m_pathFinderPriorityQueue(
				[this](int currRow, int currCol, int row, int col) -> bool
				{
					return component::tile::IsWalkable(m_tileLayer, m_tileset, row, col);
				},
				0,
				true,
				false
			),
			m_pathFinder(&m_pathFinderVector),
			m_tileSize{ 48, 48 },
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
				// store the tile where the goal position is placed
				component::tile::TileCoord tc = GetTileCoordFromMapPosition(m_tileSize, m_camera.ScreenToWorld({ (float)x, (float)y }));
				if (m_tileLayer.IsValidTile(tc)) m_startTile = tc;
			}
			else if (btn == 2) // right button
			{
				// store the tile where the goal position is placed
				component::tile::TileCoord tc = GetTileCoordFromMapPosition(m_tileSize, m_camera.ScreenToWorld({ (float)x, (float)y }));
				if (m_tileLayer.IsValidTile(tc)) m_goalTile = tc;
			}

			// reset step when start or goal tile changes
			m_pathFinder->SetMaxSteps(m_step = 0);
		}

		void OnMouseUp(int btn, int x, int y)
		{
		}

		void OnKeyDown(int key)
		{
			switch (key)
			{
			case 27: // escape
				m_pathFinder->SetMaxSteps(m_step = 0);
				break;
			case 32: // space
				m_pathFinder->SetMaxSteps(m_step++);
				break;
			case 49: // 1
			{
				// toggle selected tile 
				spatial::PosF worldPos = m_camera.ScreenToWorld(m_lastMousePos);
				component::tile::TileCoord tileCoord
				{
					static_cast<int>(worldPos.y / m_tileSize.height),
					static_cast<int>(worldPos.x / m_tileSize.width)
				};
				if (tileCoord.row >= 0 && tileCoord.row < m_tileLayer.GetHeight() && tileCoord.col >= 0 && tileCoord.col < m_tileLayer.GetWidth())
				{
					m_tileLayer.SetTileInstance(tileCoord.row, tileCoord.col, component::tile::TileInstance{ component::tile::IsWalkable(m_tileLayer, m_tileset, tileCoord.row, tileCoord.col) ? 1 : 0 });
				}
				m_pathFinder->SetMaxSteps(m_step = 0);
				break;
			}
			case 50: // 2
				// remove all obstacles
				for (int row = 0; row < m_tileLayer.GetHeight(); row++)
				{
					for (int col = 0; col < m_tileLayer.GetWidth(); col++)
					{
						if (!m_tileset.GetTile(m_tileLayer.GetTileInstance(row, col).index).IsWalkable())
						{
							component::tile::TileInstance tileInst;
							tileInst.index = 0;
							m_tileLayer.SetTileInstance(row, col, tileInst);
						}
					}
				}
				m_pathFinder->SetMaxSteps(m_step = 0);
				break;
			case 51: // 3
				m_drawText = !m_drawText;
				break;
			case 52: // 4
				m_drawPathFindingTiles = !m_drawPathFindingTiles;
				break;
			case 53: // 5
				m_drawWaypoint = !m_drawWaypoint;
				break;
			case 54: // 6
				m_pathFinder->EnableDiagonal(!m_pathFinder->IsDiagonalEnabled());
				break;
			case 55: // 7
				m_pathFinder->EnableCutCorners(!m_pathFinder->IsCutCornersEnabled());
				break;
			case 56: // 8
				m_pathFinder = (m_pathFinder == &m_pathFinderVector) ? &m_pathFinderPriorityQueue : &m_pathFinderVector;
				//m_pathFinder->SetMaxSteps(m_step = 0);
				break;
			case 81: // q
				SetTileLayer(m_tileLayer, 24, 16, component::tile::TileInstance{ 0 });
				m_pathFinder->SetMaxSteps(m_step = 0);
				break;
			case 87: // w
				m_tileLayer = engine::io::TileLayerLoader<int>::LoadFromCSV("PathFindingMap_24x16.csv", ',');
				m_pathFinder->SetMaxSteps(m_step = 0);
				break;
			default:
				break;
			}
		}
		

		void OnStart()
		{
			m_tileset.Register(0, std::make_unique<component::tile::WalkableTile>());   // ID 0 → Walkable
			m_tileset.Register(1, std::make_unique<component::tile::ObstacleTile>());   // ID 1 → Obstacle

			SetTileLayer(m_tileLayer, 24, 16, component::tile::TileInstance{ 0 });


			m_camera.SetViewport(
				{
					50,
					50,
					50 + m_tileLayer.GetWidth() * m_tileSize.width,
					50 + m_tileLayer.GetHeight() * m_tileSize.height
				}
			);

			m_camera.SetWorldSize(
				m_tileLayer.GetWidth() * m_tileSize.width,
				m_tileLayer.GetHeight() * m_tileSize.height
			);

			m_startTile = component::tile::TileCoord{ m_tileLayer.GetHeight() - 1, 2 };
			m_goalTile = component::tile::TileCoord{ 0, m_tileLayer.GetWidth() - 3 };

			m_engine.GetRenderer().SetClipRegion(m_camera.GetViewport());
			m_engine.GetRenderer().EnableClipping(true);

			// create utility font atlas
			m_fontSmall = std::make_shared<graphics::resource::FontAtlas>(graphics::factory::TextureFactory::Create());
			m_fontSmall->Initialize("Arial", 12);

			m_fontLarge = std::make_shared<graphics::resource::FontAtlas>(graphics::factory::TextureFactory::Create());
			m_fontLarge->Initialize("Arial", 16);

		}

		void OnUpdate(float delta)
		{
			m_pathFinder->FindPath(
				math::geometry::Rect<int>{ 0, 0, m_tileLayer.GetWidth(), m_tileLayer.GetHeight() },
				m_startTile,
				m_goalTile,
				m_path
			);


		}

		void OnRender()
		{
			m_engine.GetRenderer().EnableClipping(false);

			// render tilelayer
			RenderTileLayer(m_tileLayer);

			// render open tiles
			if (m_drawPathFindingTiles)
			{
				RenderTileCoords(m_pathFinder->GetOpenTiles(), { 0, 0.5f, 0, 0.5f });
				RenderTileCoords(m_pathFinder->GetClosedTiles(), { 0.5f, 0, 0, 0.5f });
				RenderTileCoords(m_path, { 0.5f, 0.5f, 0, 1 });
			}

			// render start and goal tile
			RenderTileCoord(m_startTile, { 0,1,0,1 });
			RenderTileCoord(m_goalTile, { 0,0,1,1 });

			// draw costs
			if (m_drawText) DrawCosts();

			// render waypoints
			if (m_drawWaypoint){ RenderWaypoints(); }
		}

		void RenderWaypoints()
		{
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

		void RenderTileCoords(const std::vector<component::tile::TileCoord>& tileCoords, graphics::ColorF color)
		{
			for (const component::tile::TileCoord& tile : tileCoords)
			{
				RenderTileCoord(tile, color);
			}
		}

		void RenderTileCoord(const component::tile::TileCoord tileCoord, graphics::ColorF color)
		{
			float thickness = 0.5f;
			spatial::SizeF size{ m_tileSize.width - thickness * 2, m_tileSize.height - thickness * 2 };
			spatial::PosF pos
			{
					m_tileSize.width* tileCoord.col + thickness,
					m_tileSize.height* tileCoord.row + thickness
			};

			m_engine.GetRenderer().Draw(m_camera.WorldToScreen(pos), size, color, 0);
		}

		void RenderTileLayer(const component::tile::TileLayer& tilemap)
		{
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

					// draw stuff
					m_engine.GetRenderer().Draw(m_camera.WorldToScreen(pos), m_tileSize, { 0,0,0,1 }, 0);

					RenderTileCoord(component::tile::TileCoord{row, col}, color);
				}
			}
		}

		void DrawCosts()
		{
			std::string str;
			str.reserve(128);

			// draw costs for open tiles
			for (const component::tile::TileCoord& tile : m_pathFinder->GetOpenTiles())
			{
				navigation::tile::Node node = m_pathFinder->GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.g));
				str.append(",");
				str.append(std::to_string(node.h));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + 0,
					m_tileSize.height * tile.row + 0// + 16
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
			for (const component::tile::TileCoord& tile : m_pathFinder->GetClosedTiles())
			{
				navigation::tile::Node node = m_pathFinder->GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.g));
				str.append(",");
				str.append(std::to_string(node.h));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + 0,
					m_tileSize.height * tile.row + 0// + 16
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
			for (const component::tile::TileCoord& tile : m_pathFinder->GetOpenTiles())
			{
				navigation::tile::Node node = m_pathFinder->GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.f()));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + 0,
					m_tileSize.height * tile.row + 0 + 16// + 32
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
			for (const component::tile::TileCoord& tile : m_pathFinder->GetClosedTiles())
			{
				navigation::tile::Node node = m_pathFinder->GetNodes()[tile.row][tile.col];

				str.clear();
				str.append(std::to_string(node.f()));

				spatial::PosF pos
				{
					m_tileSize.width * tile.col + 0,
					m_tileSize.height * tile.row + 0 + 16// + 32
				};

				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					m_camera.WorldToScreen(pos).x,
					m_camera.WorldToScreen(pos).y,
					1, 1, 1, 1
				);
			}

			// draw step count
			{
				str.clear();
				str.append("Step: ");
				str.append(std::to_string(m_step));

				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					1210,
					50,
					1, 1, 1, 1
				);
			}

			{
				str.clear();
				str.append("Path: ");
				str.append((m_pathFinder == & m_pathFinderPriorityQueue)? "Priority Queue": "Vector");

				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					1210,
					100,
					1, 1, 1, 1
				);
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

		component::tile::TileCoord GetTileCoordFromMapPosition(const spatial::SizeF& tileSize, const spatial::PosF pos)
		{
			return component::tile::TileCoord
			{
				static_cast<int>(std::floor(pos.y / tileSize.height)),
				static_cast<int>(std::floor(pos.x / tileSize.width))
			};
		}
	};
}

