#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <cassert>
#include <Core/Input.h>
#include <Containers/Dictionary.h>

namespace engine
{
	namespace scene
	{
#pragma region // Scene
		class Scene
		{
		public:
			virtual ~Scene() = default;

			// lifecycle
			virtual void OnEnter() {}
			virtual void OnExit() {}

			// main loop
			virtual void OnUpdate(double delta) {}
			virtual void OnRender() {}

			// input (event-based)
			virtual void OnKeyDown(int key) {}
			virtual void OnKeyUp(int key) {}

			virtual void OnMouseDown(int btn, int x, int y) {}
			virtual void OnMouseUp(int btn, int x, int y) {}
			virtual void OnMouseMove(int x, int y) {}

			virtual void OnInputEvent(const engine::input::InputEvent& inputEvent) {}
		};
#pragma endregion

#pragma region // SceneManager

		class SceneManager
		{
		private:
			engine::container::Dictionary<std::string, std::unique_ptr<Scene>> m_scenes;
			Scene* m_activeScene = nullptr;

		public:
			// Create and register scene
			template<typename T, typename... Args>
			void CreateScene(const std::string& name, Args&&... args)
			{
				static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");

				m_scenes[name] = std::make_unique<T>(std::forward<Args>(args)...);
			}

			// Switch active scene
			void SetActive(const std::string& name)
			{
				if (!m_scenes.Has(name))
				{
					return;
				}

				if (m_activeScene)
				{
					m_activeScene->OnExit();
				}

				m_activeScene = m_scenes[name].get();

				if (m_activeScene)
				{
					m_activeScene->OnEnter();
				}
			}

			Scene* GetActive()
			{
				return m_activeScene;
			}

			// Main loop forwarding
			void OnUpdate(double delta)
			{
				if (m_activeScene)
				{
					m_activeScene->OnUpdate(delta);
				}
			}

			void OnRender()
			{
				if (m_activeScene)
				{
					m_activeScene->OnRender();
				}
			}

			// Input forwarding
			void OnKeyDown(int key)
			{
				if (m_activeScene)
				{
					m_activeScene->OnKeyDown(key);
				}
			}

			void OnKeyUp(int key)
			{
				if (m_activeScene)
				{
					m_activeScene->OnKeyUp(key);
				}
			}

			void OnMouseDown(int btn, int x, int y)
			{
				if (m_activeScene)
				{
					m_activeScene->OnMouseDown(btn, x, y);
				}
			}

			void OnMouseUp(int btn, int x, int y)
			{
				if (m_activeScene)
				{
					m_activeScene->OnMouseUp(btn, x, y);
				}
			}

			void OnMouseMove(int x, int y)
			{
				if (m_activeScene)
				{
					m_activeScene->OnMouseMove(x, y);
				}
			}

			void OnInputEvent(const engine::input::InputEvent& inputEvent)
			{
				if (m_activeScene)
				{
					m_activeScene->OnInputEvent(inputEvent);
				}
			}
		};
#pragma endregion
	}
}