#pragma once
#include <Spatial/Position.h>
#include <Math/Rect.h>
#include <algorithm>

namespace spatial
{
	// a 2d camera abstraction for world-to-screen transformations.
	//
	// the Camera defines a viewport rectangle in screen space and a position in world space.
	// it ensures the viewport (anchored at m_position as its top-left corner in world space)
	// stays fully inside the world bounds. It provides methods to move, center, and clamp
	// the camera, as well as to convert coordinates between world space and screen space.
	//
	// key members:
	// - m_position: The camera’s top-left corner in world space. Defines which part of the world
	//   is currently visible through the viewport.
	// - m_viewport: The rectangle in screen space where the world is drawn. Defines visible area size
	//   and offset (left/top).
	// - m_worldSize: The total size of the world/map in world units. Used to clamp the camera so
	//   the viewport never scrolls outside the world.
	//	
	// coordinate systems:
	// - world space (0,0) is at the top-left corner of the map.
	// - screen space is relative to the viewport rectangle.
	template<typename T>
	class Camera
	{
	private:
		// the camera’s top‑left corner in world space
		// think of it as “where the camera is looking from.”
		// in world space, this is the top-left position of the viewport
		spatial::Position<T> m_position;

		// the rectangle on the screen where the world is drawn.
		// defines the visible area size (width/height) and offset (left/top).
		math::geometry::Rect<T> m_viewport;

		// the total size of the world/map
		spatial::Size<T> m_worldSize;

		// clamps camera position so that the viewport rectangle, anchored at m_position 
		// as its top-left position in world space, stays fully inside the world bounds.
		// camera position is clamped between 0 and world.size - viewport.size
		void ClampToBounds()
		{
			// camera position is in world space, so we need to clamp it to the map size minus the viewport size
			// also make sure we don't clamp to negative values if the map is smaller than the viewport
			// world space (0,0) is at top-left corner
			m_position.x = std::clamp(m_position.x, 0.0f, std::max<float>(0.0f, m_worldSize.width - m_viewport.GetWidth()));
			m_position.y = std::clamp(m_position.y, 0.0f, std::max<float>(0.0f, m_worldSize.height - m_viewport.GetHeight()));
		}

	public:
		Camera(math::geometry::RectF viewport) :
			m_viewport(viewport),
			m_position({ 0, 0 }),
			m_worldSize({ 0, 0 })
		{
		}

		void SetWorldSize(float width, float height)
		{
			m_worldSize = { width, height };
			ClampToBounds();
		}

		void SetViewport(math::geometry::Rect<T> viewport)
		{
			m_viewport = viewport;
			ClampToBounds();
		}

		// move the camera in the world by given delta. this pans the world in the viewport
		void MoveBy(math::Vec<T> delta)
		{
			// why it's negative? because we are moving the camera (in world space), not the world
			m_position -= delta;
			ClampToBounds();
		}

		// set the camera in the world by given position in world space.
		void SetPosition(spatial::Position<T> position)
		{
			m_position = position;
			ClampToBounds();
		}

		// set camera position so that the specified world position is at the center of the viewport
		void CenterOn(spatial::Position<T> worldPos)
		{
			m_position = worldPos - math::Vec<T>(m_viewport.GetWidth() / 2, m_viewport.GetHeight() / 2);
			ClampToBounds();
		}

		spatial::Position<T> GetPosition() const
		{
			return m_position;
		}

		math::geometry::Rect<T> GetViewport() const
		{
			return m_viewport;
		}

		// Converts a world position to screen-space
		spatial::Position<T> WorldToScreen(spatial::Position<T> worldPos) const
		{
			// translate the world position by viewport's top left position so the world position is now in viewport space
			// then offset it by camera's position to scroll the world to the correct position in the viewport
			return worldPos + spatial::Position<T>{ m_viewport.left, m_viewport.top } - m_position;
		}

		spatial::Position<T> ScreenToWorld(spatial::Position<T> screenPos) const
		{
			// translate the screen position by viewport's top left position so the screen position is now in viewport space
			// then offset it by camera's position to scroll the world to the correct position in the viewport
			return screenPos - spatial::Position<T>{ m_viewport.left, m_viewport.top } + m_position;
		}
	};


	using CameraF = Camera<float>;

}
