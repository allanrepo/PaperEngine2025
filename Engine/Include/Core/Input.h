#pragma once
#include <vector>
#include <array>
#include <cassert>
#include "Event.h"
#include <Windows.h>
#include "Singleton.h"

namespace input
{
	// Represent every kind of input your game cares about
	enum class InputType
	{
		KeyDown,
		KeyUp,
		KeyHeld,
		MouseDown,
		MouseUp,
		MouseMove,
		MouseHeld
	};

	// A universal event struct with minimal data
	struct InputEvent
	{
		InputType		type;
		unsigned int	code;        // VK_ code for keys, button index for mouse
		int				x, y;        // mouse position (valid for move/down/up)
	};

	class Input: public core::Singleton<Input>
	{
	protected:
		std::vector<InputEvent>      events;
		std::array<bool, 256>        keyState = {};
		std::array<bool, 5>          mouseState = {};  // 1=left, 2=right, ...

	public:
		Input() = default;
		virtual ~Input() = default;

		event::Event<int> OnKeyDown;
		event::Event<int> OnKeyUp;
		event::Event<int, int, int> OnMouseDown;
		event::Event<int, int, int> OnMouseUp;
		event::Event<int, int> OnMouseMove;


		virtual void HandleMouseMove(int x, int y) noexcept
		{
			InputEvent evt{};
			evt.type = InputType::MouseMove;
			evt.x = x;
			evt.y = y;
			events.push_back(evt);
		}

		virtual void HandleKeyState(unsigned int key, bool pressed) noexcept
		{
			InputEvent evt{};
			evt.type = pressed ? InputType::KeyDown : InputType::KeyUp;
			evt.code = key;
			events.push_back(evt);

			assert(key < keyState.size());
			keyState[evt.code] = true;
		}

		virtual void HandleMouseClick(unsigned int btn, bool pressed, int x, int y) noexcept
		{
			InputEvent evt{};
			evt.type = pressed ? InputType::MouseDown : InputType::MouseUp;
			evt.code = btn;
			evt.x = x;
			evt.y = y;
			events.push_back(evt);

			assert(btn < mouseState.size());
			mouseState[evt.code] = evt.type == InputType::MouseDown? true : false;
		}

		virtual void ProcessWin32Message(UINT msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg)
			{
			case WM_KEYDOWN:
				HandleKeyState(static_cast<int>(wParam), true);
				break;
			case WM_KEYUP:
				HandleKeyState(static_cast<int>(wParam), false);
				break;
			case WM_MOUSEMOVE:
				HandleMouseMove(LOWORD(lParam), HIWORD(lParam));// x, y
				break;
			case WM_LBUTTONDOWN:
				HandleMouseClick(1, true, LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_RBUTTONDOWN:
				HandleMouseClick(2, true, LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_MBUTTONDOWN:
				HandleMouseClick(3, true, LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_LBUTTONUP:
				HandleMouseClick(1, false, LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_RBUTTONUP:
				HandleMouseClick(2, false, LOWORD(lParam), HIWORD(lParam));
				break;
			case WM_MBUTTONUP:
				HandleMouseClick(3, false, LOWORD(lParam), HIWORD(lParam));
				break;
			default:
				break;
			}
		}

		virtual void Update()
		{
			// dispatch inputs on queue
			for (InputEvent& evt : events)
			{
				switch (evt.type)
				{
				case InputType::KeyDown:
					OnKeyDown(evt.code);
					break;
				case InputType::KeyUp:
					OnKeyUp(evt.code);
					break;
				case InputType::MouseDown:
					OnMouseDown(evt.code, evt.x, evt.y);
					break;
				case InputType::MouseUp:
					OnMouseUp(evt.code, evt.x, evt.y);
					break;
				case InputType::MouseMove:
					OnMouseMove(evt.x, evt.y);
					break;
				default:
					break;
				}
			}

			// dispatch key held??
			// TODO: add later if needed. provide option to enable/disable

			// dispatch mouse held??
			// TODO: add later if needed. provide option to enable/disable

			// clear input event queue
			events.clear();
		}
	};
}