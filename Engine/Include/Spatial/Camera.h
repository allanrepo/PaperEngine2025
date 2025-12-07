#pragma once
#include <Spatial/Position.h>
#include <Math/Rect.h>
#include <algorithm>

namespace spatial
{
	template<typename T>
	class Camera
	{
	private:
		spatial::Position<T> m_position;
		math::geometry::Rect<T> m_viewport;
		spatial::Size<T> m_worldSize;

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

		void MoveBy(math::Vec<T> delta)
		{
			// why it's negative? because we are moving the camera (in world space), not the world
			m_position -= delta;
			ClampToBounds();
		}

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

		spatial::PosF GetPosition() const
		{
			return m_position;
		}

		math::geometry::Rect<T> GetViewport() const
		{
			return m_viewport;
		}

		// Converts a world position to screen-space
		spatial::PosF WorldToScreen(spatial::Position<T> worldPos) const
		{
			// translate the world position by viewport's top left position so the world position is now in viewport space
			// then offset it by camera's position to scroll the world to the correct position in the viewport
			return worldPos + spatial::Position<T>{ m_viewport.left, m_viewport.top } - m_position;
		}

		spatial::PosF ScreenToWorld(spatial::Position<T> screenPos) const
		{
			// translate the screen position by viewport's top left position so the screen position is now in viewport space
			// then offset it by camera's position to scroll the world to the correct position in the viewport
			return screenPos - spatial::Position<T>{ m_viewport.left, m_viewport.top } + m_position;
		}

		using CameraF = Camera<T>;
	};

}
