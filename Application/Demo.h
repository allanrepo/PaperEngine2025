#include <Engine/Engine.h>
#include <state/State.h>
#include <state/StateMachine.h>
#include <Command/ICommand.h>
#include <Performance/FrameRateMonitor.h>

namespace demo
{
	class Demo
	{
	private:
		engine::Engine m_engine;
		state::StateMachine<Demo> m_stateMachine;

	public:
		Demo();
		virtual ~Demo();
		void OnStart();
		void OnRender();
		void OnUpdate(float delta);
		void OnExit();

		engine::Engine& Engine()
		{
			return m_engine;
		}
	};

	class LaunchState : public state::State<Demo>
	{
	private:
		std::unique_ptr<graphics::renderable::IFontAtlas> m_fontAtlas;
		performance::FrameRateMonitor m_frameRateMonitor;

	public:
		LaunchState();

		virtual void Enter(Demo& owner) override;
		virtual void Exit(Demo& owner) override;
		virtual void Update(Demo& owner, float delta) override;
		virtual bool IsFinished(Demo& owner) override;
	};


}

