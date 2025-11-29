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
	class TestGridScaling
	{
	private:
		engine::Engine m_engine;
		component::tile::Tileset m_tileset;
		spatial::Camera m_camera;
		std::shared_ptr<graphics::resource::IFontAtlas> m_fontSmall;
		std::shared_ptr<graphics::resource::IFontAtlas> m_fontLarge;

		component::tile::TileLayer m_tileLayer;
		component::tile::TileLayer m_tileLayerPruned;
		bool m_drawPruned = false;

		spatial::SizeF m_tileSize;

		spatial::PosF m_lastMousePos;
		spatial::SizeF m_footPrintSize;

		navigation::tile::PathFinder m_pathFinder;
		std::vector<component::tile::TileCoord> m_path;
		std::vector<spatial::PosF> m_wayPoints;
		navigation::tile::FootprintResolver m_footprintResolver;

		// cache start and goal positions 
		navigation::tile::Footprint m_startFP;
		component::tile::TileCoord m_startTC;
		navigation::tile::Footprint m_goalFP;
		component::tile::TileCoord m_goalTC;

	public:
		TestGridScaling() :
			m_engine("DirectX11", "Batch"),
			m_camera({ 50, 50, 1024, 768 }),
			m_tileSize{ 50, 50 },
			m_footPrintSize{ 480, 480 },
			m_footprintResolver
			(
				[this](int row, int col) -> bool
				{
					return component::tile::IsWalkable(m_tileLayer, m_tileset, row, col);
				},
				0.1f, m_tileSize.width / 0.1f, m_tileSize.height / 0.1f, true
			),
			m_pathFinder(
				nullptr,
				1000,
				false,
				false
			)
		{
			m_engine.OnStart += event::Handler(this, &TestGridScaling::OnStart);
			m_engine.OnUpdate += event::Handler(this, &TestGridScaling::OnUpdate);
			m_engine.OnRender += event::Handler(this, &TestGridScaling::OnRender);
			m_engine.OnResize += event::Handler(this, &TestGridScaling::OnResize);

			input::Input::Instance().OnKeyDown += event::Handler(this, &TestGridScaling::OnKeyDown);
			input::Input::Instance().OnMouseDown += event::Handler(this, &TestGridScaling::OnMouseDown);
			input::Input::Instance().OnMouseMove += event::Handler(this, &TestGridScaling::OnMouseMove);
			input::Input::Instance().OnMouseUp += event::Handler(this, &TestGridScaling::OnMouseUp);

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
				m_startFP.position = m_camera.ScreenToWorld({ (float)x, (float)y });

				// resolve positions of start and goal footprints in case they collide with obstacles
				m_footprintResolver.TryResolve(m_tileLayer, m_tileSize, m_startFP, m_startFP);

				// store the tile where the start position is placed
				m_startTC = GetTileCoordFromMapPosition(m_tileSize, m_startFP.position);
			}
			else if (btn == 2) // right button
			{
				m_goalFP.position = m_camera.ScreenToWorld({ (float)x, (float)y });

				// resolve positions of start and goal footprints in case they collide with obstacles
				m_footprintResolver.TryResolve(m_tileLayer, m_tileSize, m_goalFP, m_goalFP);

				// store the tile where the goal position is placed
				m_goalTC = GetTileCoordFromMapPosition(m_tileSize, m_goalFP.position);
			}
		}

		void OnMouseUp(int btn, int x, int y)
		{
		}

		void OnKeyDown(int key)
		{

			if (key == 32) // space key
			{
				m_drawPruned = !m_drawPruned;
			}
			if (key == 49) // 1
			{
				// toggle tile walkable/obstacle
				component::tile::TileCoord tc = GetTileCoordFromMapPosition(m_tileSize, m_camera.ScreenToWorld(m_lastMousePos));
				component::tile::TileInstance ti = { component::tile::IsWalkable(m_tileLayer, m_tileset, tc.row, tc.col) ? 1 : 0};
				m_tileLayer.SetTileInstance(tc.row, tc.col, ti);

				m_tileLayerPruned = GeneratePrunedLayer(m_tileLayer, m_tileset, m_tileSize, m_startFP);
			}
			if (key == 50) // 2
			{
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

				m_tileLayerPruned = GeneratePrunedLayer(m_tileLayer, m_tileset, m_tileSize, m_startFP);
			}
			if (key == 51) // 3
			{
			}
			if (key == 52) // 4
			{

			}
			if (key == 53)
			{
				m_startFP.size.width += 5;
				m_goalFP.size.width += 5;
			}
			if (key == 54)
			{
				m_startFP.size.width -= 5;
				m_goalFP.size.width -= 5;
			}
			if (key == 55)
			{
				m_startFP.size.height += 5;
				m_goalFP.size.height += 5;
			}

			if (key == 56)
			{
				m_startFP.size.height -= 5;
				m_goalFP.size.height -= 5;
			}
		}

		void OnStart()
		{
			// create tileset
			m_tileset.Register(0, std::make_unique<component::tile::WalkableTile>());   // ID 0 → Walkable
			m_tileset.Register(1, std::make_unique<component::tile::ObstacleTile>());   // ID 1 → Obstacle

			// create tilelayer
			SetTileLayer(m_tileLayer, 16, 16, component::tile::TileInstance{ 0 });
			//m_tileLayer = engine::io::TileLayerLoader<int>::LoadFromCSV("PathfindingTileMap.csv", ',');

			// set viewport
			m_camera.SetViewport(
				{
					50,
					50,
					50 + m_tileLayer.GetWidth() * m_tileSize.width,
					50 + m_tileLayer.GetHeight() * m_tileSize.height
				}
			);

			// set world size
			m_camera.SetWorldSize(
				m_tileLayer.GetWidth() * m_tileSize.width,
				m_tileLayer.GetHeight() * m_tileSize.height
			);

			// set clipping region
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
			// get the path
			{
				if (m_tileLayer.IsValidTile(m_startTC) && m_tileLayer.IsValidTile(m_goalTC))
				{
					m_pathFinder.FindPath(
						math::geometry::Rect<int>{ 0, 0, m_tileLayer.GetWidth(), m_tileLayer.GetHeight() },
						m_startTC,
						m_goalTC,
						m_path
					);
				}
			}

			// calculate waypoints
			{
				navigation::tile::Footprint fp;
				fp.anchor = navigation::tile::Anchor::Center;
				fp.size = m_startFP.size;

				m_wayPoints.clear();
				for (int i = 0; i < m_path.size(); i++)
				{
					// preferred position in tile is at center so we calculate center (at world/tile space)
					fp.position.x = m_path[i].col * m_tileSize.width + m_tileSize.width / 2;
					fp.position.y = m_path[i].row * m_tileSize.height + m_tileSize.height / 2;

					if (!m_footprintResolver.IsValid(m_tileLayer, m_tileSize, fp))
					{
						// DEBUG:: we just set random value here. fix this so that we set appropriate value
						m_footprintResolver.TryResolve(m_tileLayer, m_tileSize, fp, fp);
					}
					m_wayPoints.push_back(fp.position);
				}
			}
		}

		void OnRender()
		{
			m_engine.GetRenderer().EnableClipping(false);

			// draw the map
			RenderTileMap(m_tileLayer);

			// draw the footprint at the start position
			//m_engine.GetRenderer().Draw(m_camera.WorldToScreen(m_startFP.GetRect().GetTopLeft()), m_startFP.size, graphics::ColorF{ 0,1,0,0.5f }, 0);

			// draw the footprint at the goal position
			//m_engine.GetRenderer().Draw(m_camera.WorldToScreen(m_goalFP.GetRect().GetTopLeft()), m_goalFP.size, graphics::ColorF{ 1,0,0,0.5f }, 0);

#if 0
			// render path
			{
				for (const component::tile::TileCoord& tile : m_path)
				{
					spatial::PosF pos
					{
						m_tileSize.width * tile.col,
						m_tileSize.height * tile.row

					};

					m_engine.GetRenderer().Draw(m_camera.WorldToScreen(pos), m_tileSize, graphics::ColorF{ 0,0,0.5,0.5f }, 0);
				}
			}
#endif

#if 0
			// render waypoints
			{
				math::VecF translate = { m_startFP.size.width / 2, m_startFP.size.height / 2 };
				for (size_t i = 0; i < m_wayPoints.size(); i++)
				{
					// ignore start (first) and goal (last) tile. it is drawn along with tilemap
					if (i > 0 && i < m_wayPoints.size())
					{
						m_engine.GetRenderer().Draw(m_camera.WorldToScreen(m_wayPoints[i] - translate), m_startFP.size, graphics::ColorF{ 1,1,0,0.2f }, 0);
					}

					if (i > 0)
					{
						m_engine.DrawLineSegment(
							m_camera.WorldToScreen(m_wayPoints[i - 1]),
							m_camera.WorldToScreen(m_wayPoints[i]),
							{ 1, 0, 1, 1 },
							4.0f
						);
					}
				}
			}

			// render text
			{
				m_engine.GetRenderer().EnableClipping(false);

				std::string str;

				str.clear();
				str.append("tile size: ");
				str.append(std::to_string((int)std::round(m_tileSize.width)));
				str.append(",");
				str.append(std::to_string((int)std::round(m_tileSize.height)));
				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					900,
					50,
					1, 1, 1, 1
				);

				str.clear();
				str.append("foot size: ");
				str.append(std::to_string((int)std::round(m_startFP.size.width)));
				str.append(",");
				str.append(std::to_string((int)std::round(m_startFP.size.height)));
				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					900,
					100,
					1, 1, 1, 1
				);

				component::tile::TileCoord tc = this->GetTileCoordFromMapPosition(m_tileSize, m_camera.ScreenToWorld(m_lastMousePos));

				str.clear();
				str.append("tile coord [row,col]: ");
				str.append(std::to_string((int)std::round(tc.row)));
				str.append(",");
				str.append(std::to_string((int)std::round(tc.col)));
				m_engine.GetRenderer().DrawText(
					m_fontLarge,
					str,
					900,
					150,
					1, 1, 1, 1
				);
			}
#endif
			return;

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

		component::tile::TileCoord GetTileCoordFromMapPosition(const spatial::SizeF& tileSize, const spatial::PosF pos)
		{
			return component::tile::TileCoord
			{
				static_cast<int>(std::floor(pos.y / tileSize.height)),
				static_cast<int>(std::floor(pos.x / tileSize.width))
			};
		}

		// Generate a pruned map for a given footprint.
		// tileSize: size of one tile in world units (e.g. 10x10)
		// footprint: actor footprint in world units (can be rectangular, fractional)
		component::tile::TileLayer GeneratePrunedLayer(const component::tile::TileLayer& base,
			const component::tile::Tileset& tileset,
			const spatial::SizeF& tileSize,
			const navigation::tile::Footprint& footprint)
		{
			// how many tiles does the footprint span horizontally/vertically?
			int spanCols = static_cast<int>(std::ceil(footprint.size.width / tileSize.width));
			int spanRows = static_cast<int>(std::ceil(footprint.size.height / tileSize.height));

			// if footprint fits entirely inside one tile, no pruning needed
			if (spanCols <= 1 && spanRows <= 1)
			{
				return base; // return copy of base
			}

			component::tile::TileLayer pruned;
			pruned.SetSize(base.GetSize());

			for (int row = 0; row < base.GetHeight(); ++row)
			{
				for (int col = 0; col < base.GetWidth(); ++col)
				{
					component::tile::TileInstance inst = base.GetTileInstance(row, col);

					// if base tile is already blocked, keep it blocked
					if (!IsWalkable(base, tileset, row, col))
					{
						pruned.SetTileInstance(row, col, inst);
						continue;
					}

					bool blocked = false;

					// check the footprint area around (row,col)
					for (int dy = 0; dy < spanRows; ++dy)
					{
						for (int dx = 0; dx < spanCols; ++dx)
						{
							int checkRow = row + dy;
							int checkCol = col + dx;

							if (!base.IsValidTile(checkRow, checkCol) ||
								!IsWalkable(base, tileset, checkRow, checkCol))
							{
								blocked = true;
								break;
							}
						}
						if (blocked) break;
					}

					if (blocked)
					{
						component::tile::TileInstance blockedInst;
						blockedInst.index = -1; // or your obstacle id
						pruned.SetTileInstance(row, col, blockedInst);
					}
					else
					{
						pruned.SetTileInstance(row, col, inst);
					}
				}
			}

			return pruned;
		}
	};

}

