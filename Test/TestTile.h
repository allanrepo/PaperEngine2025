#pragma once
#include "Engine.h"
#include "Input.h"
#include "Logger.h"
#include "Tile.h"
#include <algorithm>
#include "Camera.h"
#include <iostream>
#include <sstream>
#include "ActorState.h"
#include "IActor.h"
#include "ActorFactory.h"
#include "Color.h"

class TestTile
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
	spatial::PosF m_lastMousePos;
	bool m_isPanning = false;
	std::unordered_map<std::string, std::unique_ptr<component::IActor>> m_actors;
	spatial::PosF m_currMousePos;

public:
	TestTile() :
		m_engine("DirectX11", "Batch"),
		m_camera({ 50, 50, 1024, 768 })
	{
		m_engine.OnStart += event::Handler(this, &TestTile::OnStart);
		m_engine.OnUpdate += event::Handler(this, &TestTile::OnUpdate);
		m_engine.OnRender += event::Handler(this, &TestTile::OnRender);
		m_engine.OnResize += event::Handler(this, &TestTile::OnResize);

		input::Input::Instance().OnKeyDown += event::Handler(this, &TestTile::OnKeyDown);
		input::Input::Instance().OnMouseDown += event::Handler(this, &TestTile::OnMouseDown);
		input::Input::Instance().OnMouseMove += event::Handler(this, &TestTile::OnMouseMove);
		input::Input::Instance().OnMouseUp += event::Handler(this, &TestTile::OnMouseUp);

		m_engine.Run();
	}

	void RenderActor(const component::IActor& actor)
	{
		m_engine.GetRenderer().DrawRenderable(
			actor,																	// actor
			m_camera.WorldToScreen(													// convert world to screen pos
				actor.GetTransform().GetPosition() +								// actor world pos 
				actor.GetAnimator().GetCurrentFrame()->element.GetRenderOffset()	// actor render offset
			),
			actor.GetSize(),														// actor size									
			{ 1, 1, 1, 1 },															// tint	
			0																		// rotation
		);
	}
	void RenderTileMap(component::tile::TileLayer& tilemap)
	{
		spatial::SizeF size{ 64.0f, 64.0f };

		int index = 0;
		for (int row = 0; row < tilemap.GetHeight(); row++)
		{
			for (int col = 0; col < tilemap.GetWidth(); col++)
			{
				component::tile::TileInstance tile = tilemap.GetTileInstance(row, col);

				graphics::ColorF color;
				switch (tile.index)
				{
				case 0:
					color = { 0.5f, 0.5f, 0.5f, 1 };
					break;
				case 1:
					color = { 0, 0, 1, 1 };
					break;
				default:
					color = { 0, 0, 0, 1 };
					break;
				}

				spatial::PosF pos
				{
					size.width * col,
					size.height * row
				};

				// draw stuff
				m_engine.GetRenderer().Draw(m_camera.WorldToScreen(pos), size, color, 0);

				index++;
			}
		}
	}

	// how renderer rotates a rectangle around its center:
	// 1. translate the rectangle so that its center is at the origin (0,0)
	// 2. apply the rotation transformation
	// 3. translate the rectangle back to its original position
	// when rotation is applied to rectangle, the rectangle is rotated around its center point
	// we want it to be rotated around the start point of the line segment
	// to achieve this, we want to find the center point of the line segment, 
	// then we find the start point of the rectangle where the center point of the line segment is also the center point of the rectangle
	// to do that, we need to do the following:
	//
	void DrawLineSegment(const spatial::PosF& start, const spatial::PosF& end, const graphics::ColorF& color = { 1, 1, 1, 1 }, float thickness = 1.0f)
	{
		// the points given are in screen space. let's make start point as the origin (0,0) to make the calculation easier
		math::VecF lineSegment = end - start;

		// let's find the center point of the line segment
		spatial::PosF lineSegmentCenterPoint = lineSegment / 2.0f;

		// now if we draw a line segment from (0,0) to the center point, this line segment is half of the original line segment

		// we are basing our calculation on horizontal axis, so we will shift the center point to the left by half of the length of the line segment
		// here's where the magic happens. By shifting the center point to the left by half of the length of the line segment, 
		// we end up with the position of the rectangle's top-left corner where the rectangle's center is at the center of the line segment
		float length = lineSegment.Magnitude();
		lineSegmentCenterPoint.x -= length / 2.0f;

		// now we need to translate this position back in screen space where this position is the start point of the line segment
		spatial::PosF position = lineSegmentCenterPoint + start;

		// finally, we need to adjust the position to account for the thickness of the line. we want it to be centered on the line segment
		position -= math::VecF(0, thickness / 2.0f);

		// define the line segment's rectangle size. this is the size of the line segment before rotation is applied
		spatial::SizeF size{ length, thickness };

		// calculate the angle of the line segment in radians
		float angle = std::atan2(end.y - start.y, end.x - start.x);

		// draw it!
		m_engine.GetRenderer().Draw(position, size, color, -angle);
	}

	void DrawCircleOutline(
		const spatial::PosF& center,
		float radius,
		const graphics::ColorF& color = { 1, 1, 1, 1 },
		float thickness = 1.0f,
		int segments = 32)
	{
		using namespace math::geometry;

		const float angleStep = 2.0f * 3.14159265f / segments;

		for (int i = 0; i < segments; ++i)
		{
			float angle0 = i * angleStep;
			float angle1 = (i + 1) * angleStep;

			spatial::PosF p0 = {
				center.x + radius * std::cos(angle0),
				center.y + radius * std::sin(angle0)
			};

			spatial::PosF p1 = {
				center.x + radius * std::cos(angle1),
				center.y + radius * std::sin(angle1)
			};

			DrawLineSegment(p0, p1, color, thickness);
		}
	}

	void OnResize(size_t width, size_t height)
	{
	}

	void OnMouseMove(int x, int y)
	{
		m_currMousePos = { (float)x, (float)y };

		if (m_isPanning)
		{
			math::VecF delta = math::VecF((float)x, (float)y) - m_lastMousePos;

			m_camera.MoveBy(delta);

			m_lastMousePos = { (float)x, (float)y };
		}
	}

	void OnMouseDown(int btn, int x, int y)
	{
		// this button is for panning the camera
		if (btn == 1)
		{
			m_isPanning = true;
			m_lastMousePos = { (float)x, (float)y };
		}
		// this button is for commanding the hero actor to walk to a target position
		else if (btn == 2)
		{
			// convert screen pos to world pos
			spatial::PosF worldPos = m_camera.ScreenToWorld(spatial::PosF{ static_cast<float>(x), static_cast<float>(y) });

			// set the hero actor to walk to the target position, then queue idle state after that
			m_actors["hero"]->SetState(std::make_unique<state::ActorWalkToState>(worldPos, 0.2f));
			m_actors["hero"]->QueueState(std::make_unique<state::ActorIdleState>());
		}
	}

	void OnMouseUp(int btn, int x, int y)
	{
		m_isPanning = false;
	}

	void OnKeyDown(int key)
	{
		LOG("OnKeyDown Key: " << key);
	}


	void OnStart()
	{
		m_engine.GetRenderer().SetClipRegion(m_camera.GetViewport());
		m_engine.GetRenderer().EnableClipping(true);


		// create a roaming enemy actor. set default state and position (world)
		m_actors["RoamingEnemy"] = component::factory::ActorFactory::Create("CharacterTestStates.csv");
		m_actors["RoamingEnemy"]->GetTransform().SetPosition(spatial::PosF{ 250, 250 });
		m_actors["RoamingEnemy"]->SetState(std::make_unique<state::ActorPatrolIdleState>(m_actors["RoamingEnemy"]->GetTransform().GetPosition()));

		// create another roaming enemy actor. set default state and position (world)
		m_actors["AnotherRoamingEnemy"] = component::factory::ActorFactory::Create("CharacterTestStates.csv");
		m_actors["AnotherRoamingEnemy"]->GetTransform().SetPosition(spatial::PosF{ 450, 350 });
		m_actors["AnotherRoamingEnemy"]->SetState(std::make_unique<state::ActorPatrolIdleState>(m_actors["AnotherRoamingEnemy"]->GetTransform().GetPosition()));

		// create another roaming enemy actor. set default state and position (world)
		m_actors["MoarRoamingEnemy"] = component::factory::ActorFactory::Create("CharacterTestStates.csv");
		m_actors["MoarRoamingEnemy"]->GetTransform().SetPosition(spatial::PosF{ 550, 550 });
		m_actors["MoarRoamingEnemy"]->SetState(std::make_unique<state::ActorPatrolIdleState>(m_actors["MoarRoamingEnemy"]->GetTransform().GetPosition()));

		// create a hero actor. set default state and position (world)
		m_actors["hero"] = component::factory::ActorFactory::Create("CharacterTestStates.csv");
		m_actors["hero"]->GetTransform().SetPosition(spatial::PosF{ 400, 300 });
		m_actors["hero"]->SetState(std::make_unique<state::ActorIdleState>());

		m_tilemap = engine::io::TileLayerLoader<int>::LoadFromCSV("tilemap.csv", ',');

		m_camera.SetWorldSize(
			m_tilemap.GetWidth() * 64.0f,
			m_tilemap.GetHeight() * 64.0f
		);

		m_tileset.Register(0, std::make_unique<component::tile::WalkableTile>());   // ID 0 ? Walkable
		m_tileset.Register(1, std::make_unique<component::tile::ObstacleTile>());   // ID 1 ? Obstacle
	}

	void OnUpdate(float delta)
	{
		// update all actors
		for (const auto& kv : m_actors)
		{
			kv.second->Update(delta);
		}

		m_camera.CenterOn(m_actors["hero"]->GetTransform().GetPosition());
	}

	void PrimitiveRenderTest(spatial::PosF pos, float thickness)
	{
		m_engine.GetRenderer().EnableClipping(false);

		spatial::PosF start{ 400, 300 };
		spatial::SizeF size{ 20, 20 };
		math::VecF offset{ size.width / 2.0f, size.height / 2.0f };

		DrawLineSegment(start, pos, { 1, 0, 0, 1 }, thickness);
		m_engine.GetRenderer().Draw(start - offset, size, { 1, 1, 1, 0.5f }, 0);
		m_engine.GetRenderer().Draw(m_currMousePos - offset, size, { 1, 1, 1, 0.5f }, 0);

		DrawCircleOutline(m_currMousePos, 100.0f, { 0, 1, 0, 1 }, thickness, 64);
	}

	void OnRender()
	{
		m_engine.GetRenderer().EnableClipping(true);

		RenderTileMap(m_tilemap);

		//m_engine.GetRenderer().Draw(m_camera.GetViewport().GetTopLeft(), m_camera.GetViewport().GetSize(), { 1, 1, 1, 0.3f }, 0);

		// draw the actors
		for (const auto& kv : m_actors)
		{
			RenderActor(*kv.second);
		}

		//PrimitiveRenderTest(m_currMousePos, 5.0f);


		////m_engine.GetRenderer().Draw({ 100, 100 }, { 400, 300 }, { 1, 0, 0, 1.0f }, 0);


		std::stringstream ss;
		ss << "Mouse Pos: " << std::to_string(m_currMousePos.x) << ", " << std::to_string(m_currMousePos.y);
		m_engine.PrintText(ss.str(), { 60, 60 }, { 1, 1, 1, 1 });
		//ss.clear();
		//ss << "Camera Screen Pos: " << std::to_string(m_camera.WorldToScreen(m_camera.GetPosition()).x) << ", " << std::to_string(m_camera.WorldToScreen(m_camera.GetPosition()).y);
		//m_engine.PrintText(ss.str(), { 10, 40 }, { 1, 1, 0, 1 });

	}
};

