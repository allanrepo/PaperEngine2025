#pragma once
#include <Timer/StopWatch.h>
#include <algorithm>
#include <functional>
#include <deque>
#include <Job/IJob.h>

namespace engine
{
	namespace job
	{
		class Job : public IJob
		{
		private:
			friend class JobQueue;

			// the actual work
			std::function<void()> m_task;

			// persistent flag
			bool m_persistent;

			// optional completion condition
			std::function<bool()> m_isDone;

			// optional event triggered when job is done
			std::function<void()> m_done;


		public:
			Job(
				std::function<void()> task,
				bool persistent = false,
				std::function<bool()> isDone = nullptr,
				std::function<void()> done = nullptr
			) :
				m_task(std::move(task)),
				m_persistent(persistent),
				m_isDone(std::move(isDone)),
				m_done(std::move(done))
			{
			}

			virtual ~Job() = default;

			void Execute()
			{
				if (m_task) m_task();
			}

			bool IsDone() const
			{
				return m_isDone ? m_isDone() : false;
			}

			void Done()
			{
				if (m_done) m_done();
			}

			bool IsPersistent() const
			{
				return m_persistent;
			}
		};

		class JobQueue
		{
		private:
			std::deque<std::unique_ptr<IJob>> m_jobs;
			std::deque<std::unique_ptr<IJob>> m_submitted;

		public:
			void Submit(std::unique_ptr<IJob> job)
			{
				m_submitted.emplace_back(std::move(job));
			}

			void Update(double maxElapsedTimeMS = 1.0)
			{
				// add the submitted jobs here. this is to make sure we can submit jobs within jobs and not mess up the job iteration
				// we are also adding submitted jobs here so they get executed within this update
				for (auto& job : m_submitted)
				{
					m_jobs.emplace_back(std::move(job));
				}
				m_submitted.clear();

				timer::StopWatch sw;
				sw.Start();

				for (auto it = m_jobs.begin(); it != m_jobs.end();)
				{
					// run the job
					(*it)->Execute();

					if (!(*it)->IsPersistent())
					{
						// notify that the job is done
						(*it)->Done();

						// one-shot job → remove immediately
						it = m_jobs.erase(it);
					}
					else
					{
						// persistent job → check condition
						if ((*it)->IsDone())
						{
							// notify that the job is done
							(*it)->Done();

							// finished → remove
							it = m_jobs.erase(it);
						}
						else
						{
							// keep it for next frame
							it++;
						}
					}

					if (sw.Peek<timer::milliseconds>() >= maxElapsedTimeMS)
					{
						break;
					}
				}
			}
		};
	}
}