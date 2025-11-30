#pragma once
#include <Utilities/Logger.h>
#include <Timer/StopWatch.h>
#include <Timer/Pulse.h>
#include <chrono>
#include <thread>

namespace timer
{
    static void SleepMS(unsigned int ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    namespace stopwatch
    {
        static void Test()
        {
            // test start, stop, peek
            {
                StopWatch sw;
                unsigned long sleep = 125;
                sw.Start();
                SleepMS(sleep);
                float total = static_cast<float>(sleep);
                std::cout << "time started... sleep for " << std::to_string(sleep) << " ms" << std::endl;

                float peek = sw.Peek<timer::milliseconds>();
                std::cout << "peeked...did we see " << std::to_string(total) << " ms? peek: " << std::to_string(peek) << " ms" << std::endl;

                sleep = 25;
                SleepMS(sleep);
                total += sleep;
                peek = sw.Peek<timer::milliseconds>();
                std::cout << "sleep for " << std::to_string(sleep) << " ms.." << std::endl;
                std::cout << "peeked again...did we see " << std::to_string(total) << " ms? peek: " << std::to_string(peek) << " ms" << std::endl;

                sleep = 50;
                SleepMS(sleep);
                total += sleep;
                float stop = sw.Stop<timer::milliseconds>();
                std::cout << "sleep for " << std::to_string(sleep) << " ms... then stop. " << std::endl;
                std::cout << "Expected is " << std::to_string(total) << " ms. stop: " << std::to_string(stop) << " ms" << std::endl;
            }

            // test lap
            {
                std::cout << std::endl;

                StopWatch sw;
                unsigned long sleep = 125;
                sw.Start();
                SleepMS(sleep);
                float totalExpectedElapsed = static_cast<float>(sleep);
                std::cout << "time started... sleep for " << std::to_string(sleep) << " ms" << std::endl;

                float lap = sw.Lap<timer::milliseconds>();
                std::cout << "lapped...did we see " << std::to_string(sleep) << " ms? peek: " << std::to_string(lap) << " ms" << std::endl;

                sleep = 75;
                SleepMS(sleep);
                totalExpectedElapsed += sleep;
                lap = sw.Lap<timer::milliseconds>();
                std::cout << "lapped...did we see " << std::to_string(sleep) << " ms? peek: " << std::to_string(lap) << " ms" << std::endl;

                sleep = 123;
                SleepMS(sleep);
                totalExpectedElapsed += sleep;
                lap = sw.Lap<timer::milliseconds>();
                std::cout << "lapped...did we see " << std::to_string(sleep) << " ms? peek: " << std::to_string(lap) << " ms" << std::endl;

                sleep = 249;
                SleepMS(sleep);
                totalExpectedElapsed += sleep;
                lap = sw.Lap<timer::milliseconds>();
                std::cout << "lapped...did we see " << std::to_string(sleep) << " ms? peek: " << std::to_string(lap) << " ms" << std::endl;

                float stop = sw.Stop<timer::milliseconds>();
                std::cout << "Stopped. Expected is " << std::to_string(totalExpectedElapsed) << " ms. stop: " << std::to_string(stop) << " ms" << std::endl;
            }

            {
                std::cout << std::endl;

                StopWatch sw;
                unsigned long sleep = 125;
                sw.Start();
                SleepMS(sleep);
                float totalExpectedElapsed = static_cast<float>(sleep);
                float totalLapElapsed = static_cast<float>(sleep);
                std::cout << "time started... sleep for " << std::to_string(sleep) << " ms" << std::endl;

                sw.Pause();
                sleep = 50;
                SleepMS(sleep);
                sw.Resume();
                std::cout << "paused and sleep for " << std::to_string(sleep) << " ms then resume" << std::endl;

                float lap = sw.Lap<timer::milliseconds>();
                std::cout << "lapped. expected: " << std::to_string(totalLapElapsed) << " ms. lap: " << std::to_string(lap) << " ms." << std::endl;
                totalLapElapsed = 0;

                sleep = 45;
                SleepMS(sleep);
                totalExpectedElapsed += sleep;
                totalLapElapsed += sleep;
                std::cout << "sleep for " << std::to_string(sleep) << " ms then paused" << std::endl;

                sw.Pause();
                sleep = 30;
                SleepMS(sleep);
                std::cout << "paused and sleep for " << std::to_string(sleep) << " ms." << std::endl;


                float stop = sw.Stop<timer::milliseconds>();
                std::cout << "Stopped. Expected is " << std::to_string(totalExpectedElapsed) << " ms. stop: " << std::to_string(stop) << " ms" << std::endl;
            }

            {
                std::cout << std::endl;

                StopWatch sw;
                unsigned long sleep;
                float totalLapElapsed = 0;
                float totalElapsed = 0;

                sw.Start();

                sleep = 125;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                sw.Pause();

                sleep = 60;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                float lap = sw.Lap<timer::milliseconds>();
                std::cout << "Lap. Expected: " << std::to_string(totalLapElapsed) << " ms. Actual: " << std::to_string(lap) << " ms" << std::endl;
                totalLapElapsed = 0;

                sleep = 75;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                lap = sw.Lap<timer::milliseconds>();
                std::cout << "Lap. Expected: " << std::to_string(totalLapElapsed) << " ms. Actual: " << std::to_string(lap) << " ms" << std::endl;
                totalLapElapsed = 0;

                sleep = 40;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                sw.Resume();

                sleep = 115;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                lap = sw.Lap<timer::milliseconds>();
                std::cout << "Lap. Expected: " << std::to_string(totalLapElapsed) << " ms. Actual: " << std::to_string(lap) << " ms" << std::endl;
                totalLapElapsed = 0;

                sleep = 70;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                sw.Pause();

                sleep = 25;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                sw.Resume();

                sleep = 110;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                sw.Pause();

                sleep = 50;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                sw.Resume();

                sleep = 20;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                lap = sw.Lap<timer::milliseconds>();
                std::cout << "Lap. Expected: " << std::to_string(totalLapElapsed) << " ms. Actual: " << std::to_string(lap) << " ms" << std::endl;
                totalLapElapsed = 0;

                sleep = 150;
                SleepMS(sleep);
                if (!sw.IsPaused()) totalLapElapsed += sleep;
                if (!sw.IsPaused()) totalElapsed += sleep;

                float stop = sw.Stop<timer::milliseconds>();
                std::cout << "Stopped. Expected: " << std::to_string(totalElapsed) << " ms. Actual: " << std::to_string(stop) << " ms" << std::endl;

                std::cout << std::endl;
            }
        }
    }

    namespace alarmclock
    {
        class TestClass
        {
        private:
            StopWatch stopwatch;
            IntervalTimer timerPersistent;
            IntervalTimer timerOneShot;

            // listener 
            void OnTimeOut()
            {
                LOG("[" << std::to_string(stopwatch.Peek<timer::milliseconds>()) << "] Timeout triggered. Delaying... 11 seconds");
                SleepMS(11000);
            }

            // listener to event when interval timer (persistent) reaches max interval in a single update.
            void OnMaxIntervalPerUpdateReached()
            {
                LOG("[" << std::to_string(stopwatch.Peek<timer::milliseconds>()) << "] alarm clock's maximum alarm triggered. Remaining elapsed time will be proceed on next update.");
            }

            // listener to our persistent interval timer trigger 
            void OnInterval()
            {
                LOG("[" << std::to_string(stopwatch.Peek<timer::milliseconds>()) << "] Alarm triggered.");
            }

            // listener to game loop's interval (triggered by stopwatch' Lap)
            // update interval timers here
            void OnLoop(float delta)
            {
                timerPersistent.Update(delta);
                timerOneShot.Update(delta);
            }

        public:
            // on constructor, build the two timers - one is persistent, another is one-shot
            TestClass():
                timerPersistent(1000, timer::IntervalTimer::Mode::Persistent, 5),
                timerOneShot(2500, timer::IntervalTimer::Mode::OneShot)
            {          
                // listen to stopwatch' Lap(). timers will be updated by this event listener
                stopwatch.OnLap += event::Handler(this, &TestClass::OnLoop);

                // listen to persistent timer
                timerPersistent.OnMaxIntervalPerUpdateReached += event::Handler(this, &TestClass::OnMaxIntervalPerUpdateReached);
                timerPersistent.OnInterval += event::Handler(this, &TestClass::OnInterval);

                // listen to one-shot timer
                timerOneShot.OnTimeOut += event::Handler(this, &TestClass::OnTimeOut);

                // start the stopwatch
                stopwatch.Start();

                // simulate game loop
                while (true)
                {
                    // measure elapsed time per loop. this will fire up OnLap event
                    float delta = stopwatch.Lap<timer::milliseconds>();
                }
            }

        };

        static void Test()
        {
            TestClass test;
        }
    }
}


