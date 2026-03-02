#pragma once
#include <Components/Tile.h>
#include <Spatial/Coord.h>
#include <Graphics/Core/Color.h>
#include <Graphics/Core/Primitives.h>
#include <Graphics/Renderer/IRenderer.h>

namespace engine
{
	namespace graphics
	{
		namespace tile
		{
			template<typename T>
			void DrawTileMap(
				engine::graphics::renderer::IRenderer& renderer,
				const engine::component::tile::TileMap<T>& tilemap,
				const engine::spatial::SizeF& tilesize,
				const engine::spatial::PositionF& pos,
				const engine::graphics::ColorF& color
			)
			{
				for (int row = 0; row <= tilemap.GetHeight(); ++row)
				{
					for (int col = 0; col <= tilemap.GetWidth(); ++col)
					{
						if (!tilemap.IsInBounds(row, col))
						{
							continue;
						}

						const engine::component::tile::Tile<T>& tile = tilemap.Get(row, col);
						if (tile.isValid())
						{
							engine::spatial::PositionF origin =
							{
								col * tilesize.width,
								row * tilesize.height
							};

							renderer.DrawRenderable(tile->GetSprite(), pos + origin, tilesize, color, 0.0f);
						}
					}
				}
			}


			template<typename T>
			void DrawTileMap(
				engine::graphics::renderer::IRenderer& renderer,
				const engine::component::tile::TileMap<T>& tilemap,
				const engine::spatial::SizeF& tilesize,
				const engine::spatial::PositionF& pos,
				const engine::graphics::ColorF& color,
				const engine::spatial::PositionF& offset,
				const engine::math::VecF& scale,
				float alpha	
			) 
			{
				for (int row = 0; row <= tilemap.GetHeight(); ++row)
				{
					for (int col = 0; col <= tilemap.GetWidth(); ++col)
					{
						if (!tilemap.IsInBounds(row, col))
						{
							continue;
						}

						// get the tile
						const engine::component::tile::Tile<T>& tile = tilemap.Get(row, col);

						// defensive. we're never sure if the tile has valid sprite, so do check
						if (tile.isValid())
						{
							// this will be the top-left position of this tile in map coordinate.
							engine::spatial::PositionF origin =
							{
								col * tilesize.width,
								row * tilesize.height
							};

							origin += offset;

							engine::spatial::SizeF ts =
							{
								tilesize.width * scale.x,
								tilesize.height * scale.y
							};

							renderer.DrawRenderable(
								tile->GetSprite(),
								pos + origin,
								ts,
								engine::graphics::ColorF{ 1.0f, 1.0f, 1.0f, alpha },
								0.0f
							);
						}
					}
				}
			}
		}

		namespace navigation
		{
			void DrawWaypoints(
				engine::graphics::renderer::IRenderer& renderer,
				const std::vector<engine::spatial::Coord>& wp,
				const engine::spatial::SizeF& tilesize,
				const engine::spatial::PositionF& pos,
				const engine::graphics::ColorF& color,
				float thickness)
			{
				for (size_t i = 1; i < wp.size(); i++)
				{
					engine::spatial::PositionF start
					{
						wp[i - 1].col * tilesize.width + tilesize.width / 2,
						wp[i - 1].row * tilesize.height + tilesize.height / 2
					};
					engine::spatial::PositionF end
					{
						wp[i].col * tilesize.width + tilesize.width / 2,
						wp[i].row * tilesize.height + tilesize.height / 2
					};

					start += pos;
					end += pos;

					engine::graphics::primitives::DrawLineSegment(renderer, start, end, color, thickness);
				}
			}
		}
	}
}
