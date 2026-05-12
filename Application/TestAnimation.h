// this test demonstrates how to create a simple animation system using the Animator and Animation classes.
// notes:
// - SpriteAtlas' constructors are protected, so we create MockSpriteAtlas classes to instantiate 
//	 them directly for testing purposes without using factories.
// - a function is defined to calculate UV rectangles for SpriteAtlas with the assumption that the layout is a grid. 
//	 the sprites in SpriteAtlas are also assumed to be sequenced row by row.
// - Animation is also created manually without using any factory or loader and frames were added directly
// - we needed to provide elapsed time to run the animation, so a stopwatch is used to measure time between frames
//   StopWatch's Lap method is called to measure and provide elapsed time per frame to the animator
// - since we use Sprite here to demonstrate animation, we actually test Sprite as well

#pragma once

#include <Win32/Window.h>
#include <Core/Event.h>
#include <Utilities/Logger.h>
#include <Graphics/Core/ICanvas.h>
#include <Graphics/Core/Canvas.h>
#include <Graphics/Core/DX11CanvasImpl.h>
#include <Graphics/Renderer/IRenderer.h>
#include <Graphics/Renderer/DX11RendererBatchImpl.h>
#include <Graphics/Renderer/Renderer.h>
#include <Graphics/Resource/ISpriteAtlas.h>
#include <Graphics/Resource/SpriteAtlas.h>
#include <Engine/Factory/SpriteAtlasFactory.h>
#include <Graphics/Core/Sprite.h>
#include <Core/Input.h>
#include <Graphics/Animation/Animation.h>
#include <Graphics/Resource/DX11TextureImpl.h>
#include <Timer/StopWatch.h>
#include <Engine/Factory/SpriteAnimationFactory.h>
#include <Core/View.h>
#include <memory>
#include <algorithm>	
#include <Utilities/Utilities.h>
#include <Graphics/Resource/FontAtlas.h>
#include <Timer/Pulse.h>

namespace test
{
	struct Item;
	using SpriteAtlasFactory = engine::graphics::factory::SpriteAtlasFactory;
	using ISpriteAtlas = engine::graphics::resource::ISpriteAtlas;
	using SpriteAtlas = engine::graphics::resource::SpriteAtlas;
	using SpriteAnimationFactory = engine::graphics::factory::SpriteAnimationFactory;
	using AnimationSet = engine::graphics::animation::AnimationSet<engine::graphics::Sprite>;
	using IFontAtlas = engine::graphics::resource::IFontAtlas;
	using FontAtlas = engine::graphics::resource::FontAtlas;
	using DX11TextureImpl = engine::graphics::dx11::resource::DX11TextureImpl;
	using Sprite = engine::graphics::Sprite;
	using AnimationSystemCache = engine::graphics::animation::AnimationSystemCache< engine::graphics::Sprite>;

	template <typename Owner>
	using AnimationController = engine::graphics::animation::AnimationController<engine::graphics::Sprite, Owner>;
	using AnimationSystem = engine::graphics::animation::AnimationSystem<engine::graphics::Sprite>;


	struct Item
	{
		AnimationController<Item> animated;
		PositionF pos;

		Item(const AnimationSet& set, PositionF p, const std::string& name, int loopCount, AnimationSystem* system = nullptr):
			animated(set, this, system),
			pos(p)
		{
			animated.Play(name, loopCount);
		}
	};

	class TestAnimation
	{
	private:
		std::unique_ptr<engine::win32::Window> m_window;
		std::unique_ptr<engine::graphics::ICanvas> m_canvas;
		std::unique_ptr<engine::graphics::renderer::IRenderer> m_renderer;
		engine::timer::StopWatch m_stopwatch;

		engine::input::Input m_input;

		std::vector<std::unique_ptr<Item>> m_items;

		std::unique_ptr<engine::graphics::resource::IFontAtlas> m_FontAtlas;

		//std::vector<std::vector<std::unique_ptr<Item>>::iterator> m_pendingDestroyItems;
		std::vector<Item*> m_pendingDestroyItems;

		engine::timer::Pulse m_pulse;
		int m_count = 0;

	public:
		TestAnimation():
			m_pulse(0.2f)
		{
			engine::win32::Window::OnInitialize += engine::event::Handler(this, &TestAnimation::OnInitialize);
			engine::win32::Window::OnExit += engine::event::Handler(this, &TestAnimation::OnExit);
			engine::win32::Window::OnIdle += engine::event::Handler(this, &TestAnimation::OnIdle);

			engine::win32::Window::Run();
		}

		// function that will be called just before we enter into message loop
		void OnInitialize()
		{
			// create our window here
			m_window = std::make_unique<engine::win32::Window>();
			m_window->OnClose += engine::event::Handler(this, &TestAnimation::OnWindowClose);
			m_window->OnCreate += engine::event::Handler(this, &TestAnimation::OnWindowCreate);
			m_window->OnSize += engine::event::Handler(this, &TestAnimation::OnWindowSize);
			m_window->Create(L"TestAnimation", 1400, 900);

			m_window->OnWindowMessage += engine::event::Handler(&m_input, &engine::input::Input::ProcessWin32Message);

			m_input.KeyDownEvent += engine::event::Handler(this, &TestAnimation::OnKeyDown);
			m_input.MouseDownEvent += engine::event::Handler(this, &TestAnimation::OnMouseDown);
			m_input.MouseMoveEvent += engine::event::Handler(this, &TestAnimation::OnMouseMove);
		}

		void CreateAnimation(const std::string& name, const std::wstring& filepath, const size_t row, const size_t col, const std::vector<int>& animationFrameIndice, float duration)
		{
			// create sprite atlas using factory. it will be stored in cache and will auto generate UV's based on given row and col
			engine::graphics::factory::SpriteAtlasFactory::Create(name, filepath, row, col);
			engine::graphics::resource::ISpriteAtlas& atlas = engine::cache::Registry<engine::graphics::resource::ISpriteAtlas>::Instance().Get(name);

			// create animation using factory. it will be stored in cache as well. we set loop to true for this animation
			engine::graphics::factory::SpriteAnimationFactory::Create(name, atlas, animationFrameIndice, duration, true);

			// create animator and load the animation. it will also be stored in cache. we can have multiple animators using the same animation
			engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::Sprite>>::Instance().Register(name, std::make_unique<engine::graphics::animation::Animator<engine::graphics::Sprite>>());

			// get animator from cache and play the animation. since we set loop to true, it will keep looping through the frames in the animation
			engine::graphics::animation::Animator<engine::graphics::Sprite>& animator = engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::Sprite>>::Instance().Get(name);
			animator.Play(engine::cache::Registry<engine::graphics::animation::Animation<engine::graphics::Sprite>>::Instance().Get(name));
		}

		void UpdateAnimation(const std::string& name, double delta)
		{
			engine::graphics::animation::Animator<engine::graphics::Sprite>& animator = engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::Sprite>>::Instance().Get(name);
			animator.Update(delta);
		}

		void DrawAnimation(const std::string& name, const engine::spatial::PositionF& pos, engine::graphics::ColorF color = { 1.0f, 1.0f, 1.0f, 1.0f }, float rotation = 0.0f)
		{
			engine::graphics::animation::Animator<engine::graphics::Sprite>& animator = engine::cache::Registry<engine::graphics::animation::Animator<engine::graphics::Sprite>>::Instance().Get(name);

			m_renderer->Draw(
				pos,			// position
				animator.GetCurrent().GetSize(),	// get the sprite size from animator's current frame
				{1,1,1,0.5f},			// color
				rotation		// rotation
			);

			m_renderer->Draw(
				animator.GetCurrent(),				// get the sprite from animator's current frame
				pos,				// position
				animator.GetCurrent().GetSize(),	// get the sprite size from animator's current frame
				color,			// color
				rotation
			);
		}

		// when window is created. we can now safely create resources dependent on window
		void OnWindowCreate(void* hWnd)
		{
			LOG("Window created...");

			// create dx11 canvas
			m_canvas = std::make_unique<engine::graphics::Canvas>(std::make_unique<engine::graphics::dx11::DX11CanvasImpl>());
			m_canvas->Initialize(hWnd);
			m_canvas->SetViewPort();
			LOG("Canvas (DX11) created...");

			// create dx11 renderer batched
			m_renderer = std::make_unique<engine::graphics::renderer::Renderer>(std::make_unique<engine::graphics::dx11::renderer::DX11RendererBatchImpl>());
			m_renderer->Initialize();
			LOG("Renderer (DX11) created...");

			// create font
			m_FontAtlas = std::make_unique<FontAtlas>(std::make_unique<SpriteAtlas>(std::make_unique<DX11TextureImpl>()));
			m_FontAtlas->Initialize("Terminal", 12);

			CreateAnimation("CharacterTest_Animation", L"../Assets/CharacterTest_2304x1536_12x8.png", 8, 12, { 12,13,14,15,16,17 }, 100.0f);
			CreateAnimation("Tree1", L"../Assets/tree_1x8_1536x256.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);
			CreateAnimation("Tree2", L"../Assets/tree_1x8_1536x192.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);
			CreateAnimation("Tree3", L"../Assets/tree1_1x8_1536x256.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);
			CreateAnimation("Tree4", L"../Assets/tree1_1x8_1536x192.png", 1, 8, { 0,1,2,3,4,5,6,7 }, 100.0f);


			{
				SpriteAtlasFactory::Create("dust", L"../Assets/Dust_02.png", 1, 10);
				ISpriteAtlas& atlas = engine::cache::Registry<ISpriteAtlas>::Instance().Get("dust");

				engine::cache::Registry<AnimationSet>::Instance().Register("dust", std::make_unique<AnimationSet>());
				AnimationSet& animset = engine::cache::Registry<AnimationSet>::Instance().Get("dust");
				animset.Register("dust", SpriteAnimationFactory::Create(atlas, std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 150.0f, true, PositionF{ 0.5f, 0.5f }));

				engine::cache::Registry<AnimationSystem>::Instance().Register("dust", std::make_unique<AnimationSystem>());
				AnimationSystem& animSystem = engine::cache::Registry<AnimationSystem>::Instance().Get("dust");
			}

			//{
			//	ISpriteAtlas& atlas = engine::cache::Registry<ISpriteAtlas>::Instance().Get("dust");
			//	AnimationSet& animset = engine::cache::Registry<AnimationSet>::Instance().Get("dust");

			//	engine::cache::Registry<AnimationSystem>::Instance().Register("dust", std::make_unique<AnimationSystem>());
			//	AnimationSystem& animsys = engine::cache::Registry<AnimationSystem>::Instance().Get("dust");

			//}

			// setup stopwatch to manage timing and start it
			m_stopwatch.OnLap += engine::event::Handler(this, &TestAnimation::OnLap);
			m_stopwatch.Start();

			m_pulse.IntervalEvent += engine::event::Handler(this, &TestAnimation::OnPulse);
			m_pulse.Pause();
		}

		void OnPulse(double)
		{
			AnimationSet& set = engine::cache::Registry<AnimationSet>::Instance().Get("dust");
			AnimationSystem& sys = engine::cache::Registry<AnimationSystem>::Instance().Get("dust");
			m_items.push_back(std::make_unique<Item>(set, PositionF{400, 400}, "dust", 2, &sys));
			m_items.back()->animated.EndEvent += engine::event::Handler(this, &TestAnimation::OnEndItem);
		}

		void OnKeyDown(int key)
		{
			switch (key)
			{
			case 27: // escape
			{


				break;
			}
			case 32: // space
			{
				//engine::graphics::animation::AnimationSystem<engine::graphics::Sprite>& animSystem = engine::cache::Registry<engine::graphics::animation::AnimationSystem<engine::graphics::Sprite>>::Instance().Get("dust");
				//engine::graphics::animation::Animated<engine::graphics::Sprite> animated = animSystem.GetAnimated("one");
				//animated.Destroy();

				break;
			}
			case 49: // 1
			{
				m_pulse.Resume();


				break;
			}
			case 50: // 2
			{
				m_pulse.Pause();

				break;
			}
			case 51: // 3
			{

				break;
			}
			case 52: // 4
			{
				break;
			}
			case 53: // 5
			{
				
				break;
			}
			case 54: // 6
			{
				break;
			}
			case 55: // 7
			{
				break;
			}
			case 56: // 8
			{
				break;
			}

			default:
				break;
			}
		}


		void OnEndItem(Item& item)
		{
			m_pendingDestroyItems.push_back(&item);
			return;

			{
				// if we reach this point, it means the item passed was not erased. it must be invalid. why???
				AnimationSystem& animSystem = engine::cache::Registry<AnimationSystem>::Instance().Get("dust");
				size_t n = animSystem.Size();
				LOG("Animators: " << std::to_string(n));
			}

		}

		void OnMouseDown(int btn, int x, int y)
		{
			if(btn == 1)
			{
				AnimationSet& set = engine::cache::Registry<AnimationSet>::Instance().Get("dust");
				AnimationSystem& sys = engine::cache::Registry<AnimationSystem>::Instance().Get("dust");
				m_items.push_back(std::make_unique<Item>(set, PositionF((float)x, (float)y), "dust", 2, &sys));
				m_items.back()->animated.EndEvent += engine::event::Handler(this, &TestAnimation::OnEndItem);
			}
			else if (btn == 2)
			{
				AnimationSet& set = engine::cache::Registry<AnimationSet>::Instance().Get("dust");
				m_items.push_back(std::make_unique<Item>(set, PositionF((float)x, (float)y), "dust", 2));
				m_items.back()->animated.EndEvent += engine::event::Handler(this, &TestAnimation::OnEndItem);
			}
		}

		void OnMouseMove(int x, int y)
		{
		}

		// this method is fired up whenever the OnLap event is triggered from stopwatch
		void OnLap(double time)
		{
			UpdateAnimation("CharacterTest_Animation", time);
			UpdateAnimation("Tree1", time);
			UpdateAnimation("Tree2", time);
			UpdateAnimation("Tree3", time);
			UpdateAnimation("Tree4", time);

			// animation system update to forward all registered animators
			engine::cache::Registry<AnimationSystem>::Instance().Get("dust").Update(time);

			// in case we are using the system cache, update it
			AnimationSystemCache::Instance().Update(time);

			// proper deleting of to be destroyed items
			for (Item* item : m_pendingDestroyItems) 
			{
				// find with condition such that the item is same as the item to be destroyed
				auto it = std::find_if(m_items.begin(), m_items.end(),
					// condition predicate
					[&](const std::unique_ptr<Item>& ptr) 
					{ 
						return ptr.get() == item; 
					});

				// we found it? delete it!
				if (it != m_items.end()) 
				{
					m_items.erase(it);
				}
			}
			m_pendingDestroyItems.clear();

			m_pulse.Update(time);
		}

		// fun stuff. this is called on each loop of the message loop. this is where we draw!
		void OnIdle()
		{
			// call lap to get elapsed time and trigger OnLap event
			m_stopwatch.Lap<engine::timer::milliseconds>();

			m_input.Update();

			// start the canvas. we can draw from here
			m_canvas->Begin();
			{
				m_canvas->Clear({ 0.2f, 0.2f, 1.0f, 1.0f });

				m_renderer->Begin();
				{
					DrawAnimation("CharacterTest_Animation", { 100.0f, 100.0f });
					DrawAnimation("Tree1", { 400.0f, 100.0f });
					DrawAnimation("Tree2", { 600.0f, 200.0f });
					DrawAnimation("Tree3", { 800.0f, 200.0f });
					DrawAnimation("Tree4", { 1000.0f, 200.0f });
	
					for (std::vector<std::unique_ptr<Item>>::iterator it = m_items.begin(); it != m_items.end(); it++)
					{
						Sprite sprite = (*it)->animated.GetCurrent();
						PositionF pos = (*it)->pos;
						m_renderer->Draw(sprite, pos, sprite.GetSize(), { 1,0.2f,1,1 }, 0.0f);
					}

					{
						std::string msg = "Items: " + std::to_string(m_items.size());
						m_renderer->Draw(*m_FontAtlas, msg, { 600, 5 }, { 1,1,1,1 });

						AnimationSystem& animSystem = engine::cache::Registry<AnimationSystem>::Instance().Get("dust");
						msg.clear();
						msg = "Animators: " + std::to_string(animSystem.Size());
						m_renderer->Draw(*m_FontAtlas, msg, { 600, 30 }, { 1,1,1,1 });

						msg.clear();
						msg = "Animators (Cache): " + std::to_string(AnimationSystemCache::Instance().Size());
						m_renderer->Draw(*m_FontAtlas, msg, { 600, 55 }, { 1,1,1,1 });
					}
				}
				m_renderer->End();
			}
			m_canvas->End();
		}

		void OnExit()
		{
		}

		void OnWindowClose()
		{
		}

		void OnWindowSize(size_t nWidth, size_t nHeight)
		{
			LOG("Window resized to: " + std::to_string(nWidth) + ", " + std::to_string(nHeight));
			m_canvas->Resize({ static_cast<unsigned int>(nWidth), static_cast<unsigned int>(nHeight) });
			m_canvas->SetViewPort();
		}
	};
}