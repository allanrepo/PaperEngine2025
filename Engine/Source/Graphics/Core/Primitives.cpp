#include <Graphics/Core/Primitives.h>




// how renderer rotates a rectangle around its center:
// 1. translate the rectangle so that its center is at the origin (0,0)
// 2. apply the rotation transformation
// 3. translate the rectangle back to its original position
// when rotation is applied to rectangle, the rectangle is rotated around its center point
// we want it to be rotated around the start point of the line segment
// to achieve this, we want to find the center point of the line segment, 
// then we find the start point of the rectangle where the center point of the line segment is also the center point of the rectangle
// to do that, we need to do the following:
void engine::graphics::primitives::DrawLineSegment(
	engine::graphics::renderer::IRenderer& renderer, 
	const engine::spatial::PositionF& start, 
	const engine::spatial::PositionF& end, 
	const engine::graphics::ColorF& color, 
	float thickness)
{
	// the points given are in screen space. let's make start point as the origin (0,0) to make the calculation easier
	engine::math::VecF lineSegment = end - start;

	// let's find the center point of the line segment
	engine::spatial::PositionF lineSegmentCenterPoint = lineSegment / 2.0f;

	// now if we draw a line segment from (0,0) to the center point, this line segment is half of the original line segment

	// we are basing our calculation on horizontal axis, so we will shift the center point to the left by half of the length of the line segment
	// here's where the magic happens. By shifting the center point to the left by half of the length of the line segment, 
	// we end up with the position of the rectangle's top-left corner where the rectangle's center is at the center of the line segment
	float length = lineSegment.Magnitude();
	lineSegmentCenterPoint.x -= length / 2.0f;

	// now we need to translate this position back in screen space where this position is the start point of the line segment
	engine::spatial::PositionF position = lineSegmentCenterPoint + start;

	// finally, we need to adjust the position to account for the thickness of the line. we want it to be centered on the line segment
	position -= engine::math::VecF(0.0f, thickness / 2.0f);

	// define the line segment's rectangle size. this is the size of the line segment before rotation is applied
	engine::math::SizeF size{ length, thickness };

	// calculate the angle of the line segment in radians
	float angle = std::atan2(end.y - start.y, end.x - start.x);

	// draw it!
	renderer.Draw(position, size, color, -angle);
}

void engine::graphics::primitives::DrawCircleOutline(
	engine::graphics::renderer::IRenderer& renderer, 
	const engine::spatial::PositionF& center, 
	float radius, 
	const engine::graphics::ColorF& color, 
	float thickness, int segments)
{
	using namespace math;

	const float angleStep = 2.0f * 3.14159265f / segments;

	for (int i = 0; i < segments; ++i)
	{
		float angle0 = i * angleStep;
		float angle1 = (i + 1) * angleStep;

		engine::spatial::PositionF p0 = {
			center.x + radius * std::cos(angle0),
			center.y + radius * std::sin(angle0)
		};

		engine::spatial::PositionF p1 = {
			center.x + radius * std::cos(angle1),
			center.y + radius * std::sin(angle1)
		};

		DrawLineSegment(renderer, p0, p1, color, thickness);
	}
}
