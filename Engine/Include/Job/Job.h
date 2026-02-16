#pragma once
#include <Timer/StopWatch.h>
#include <algorithm>
#include <functional>
#include <deque>
#include <Job/IJob.h>
#include <Core/Event.h>

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

			std::function<void()> m_start;

			bool m_started;


		public:
			Job(
				std::function<void()> start,
				std::function<void()> task,
				bool persistent = false,
				std::function<bool()> isDone = nullptr,
				std::function<void()> done = nullptr
			) :
				m_start(std::move(start)),
				m_task(std::move(task)),
				m_persistent(persistent),
				m_isDone(std::move(isDone)),
				m_done(std::move(done)),
				m_started(false)
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
				DoneEvent(*this);
			}

			bool IsPersistent() const
			{
				return m_persistent;
			}

			void Start() override
			{
				if (m_start) m_start();
				m_started = true;
			}

			bool IsStarted() const override
			{
				return m_started;
			}

			engine::event::Event<const Job&> DoneEvent;

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

				engine::timer::StopWatch sw;
				sw.Start();

				for (auto it = m_jobs.begin(); it != m_jobs.end();)
				{
					if((*it)->IsStarted() == false)
					{
						(*it)->Start();
					}
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
	
		class JobChain : public engine::job::IJob 
		{
			JobQueue& m_queue;
			std::deque<std::unique_ptr<Job>> m_jobs;
			bool m_started = false;
			bool m_done = false;

		public:
			JobChain(JobQueue& queue) : 
				m_queue(queue) 
			{
			}

			// queue a job to the chain
			void AddJob(std::unique_ptr<engine::job::Job> job) 
			{
				m_jobs.emplace_back(std::move(job));
			}

			void Execute() override 
			{
				if (IsDone()) return;

				// Submit the first job only once
				if (!m_started && !m_jobs.empty()) 
				{
					SubmitNext();
					m_started = true;
				}
			}

			bool IsDone() const override 
			{ 
				return m_done; 
			}

			bool IsPersistent() const override 
			{ 
				return true; 
			}

			void Done() override 
			{
			}

			void Start() override
			{
			}

			bool IsStarted() const override
			{
				return true;
			}

		private:
			void SubmitNext() 
			{
				// if no more jobs, we are done
				if (m_jobs.empty()) 
				{
					m_done = true;
					return;
				}

				// get next job in the chain
				std::unique_ptr<Job> job = std::move(m_jobs.front());
				m_jobs.pop_front();

				// attach event handler for when job is done
				job->DoneEvent += engine::event::Handler(this, &JobChain::OnJobDone);

				// submit job to the queue
				m_queue.Submit(std::move(job));
			}

			// event handler when job is done. we submit the next job in the chain here
			void OnJobDone(const Job& job) 
			{
				SubmitNext(); 
			}
		};

	}
}

