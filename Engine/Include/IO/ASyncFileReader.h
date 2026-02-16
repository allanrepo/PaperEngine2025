#pragma once
#include <fstream>
#include <Core/Event.h>
#include <Timer/StopWatch.h>
#include <Core/Loader.h>

namespace engine
{
	namespace io
	{
		class AsyncFileReader : public engine::loader::IAsyncLoader
		{
		private:
			mutable std::ifstream m_filestream;
			std::string m_filename;
			size_t m_maxBytesPerRead = 0xFF;

		public:
			AsyncFileReader(size_t maxBytesPerRead = 0xFF) :
				m_maxBytesPerRead(maxBytesPerRead)
			{
			}

			virtual ~AsyncFileReader() = default;

			const std::string& GetFileName() const
			{
				return m_filename;
			}

			bool Open(const std::string& filename)
			{
				// ensure clean state
				Close();

				// open the file 
				m_filename = filename;
				m_filestream.open(m_filename.c_str(), std::ios::binary);
				if (!m_filestream.is_open())
				{
					return false;
				}

				return true;
			}

			void Close()
			{
				if (m_filestream.is_open())
				{
					m_filestream.close();
					m_filename.clear();
				}
			}

			engine::event::Event<const char*, size_t> ProcessChunkEvent;
			engine::event::Event<> EndOfFileFoundEvent;

			// this method is to read the file in one shot
			bool SyncReadAll(size_t maxBytesPerRead = 0xFF, double maxTimeToReadMS = 1.0)
			{
				// calling Update will return false if file is closed. so if we loop it until it returns true (done)
				// we will loop forever if the file is closed. so we check if file is open here explicitly
				if (!m_filestream.is_open())
				{
					return false; // no file open
				}

				// Keep looping until Update() reports EOF
				while (!IsDone())
				{
					// keep calling Update until we reach EOF
					Update(maxTimeToReadMS);
				}

				// if file is already EOF, we will never get inside while loop so to return result, we do it here.
				return IsDone();
			}

			bool IsOpen() const
			{
				return m_filestream.is_open();
			}

			void Update(double maxTimeToReadMS = 1) override
			{
				// is file open?
				if (!m_filestream.is_open())
				{
					return;
				}

				// did we finished reading the file already?
				if (IsDone())
				{
					return;
				}

				engine::timer::StopWatch sw;
				sw.Start();
				while (!m_filestream.eof())
				{
					// read chunk size data
					std::vector<char> buffer(m_maxBytesPerRead);
					m_filestream.read(buffer.data(), buffer.size());

					std::streamsize n = m_filestream.gcount();

					if (n > 0)
					{
						ProcessChunkEvent(buffer.data(), static_cast<size_t>(n));
					}

					if (m_filestream.eof())
					{
						EndOfFileFoundEvent();
					}

					// we do check if elapsed time pass max time here so we give it a chance to do once at least
					if (sw.Peek<timer::milliseconds>() >= maxTimeToReadMS)
					{
						break;
					}
				}
				sw.Stop();
			}

			size_t GetCurrent() const override
			{
				// is file open?
				if (!m_filestream.is_open())
				{
					return 0;
				}

				// get current position. if tellg fails, it returns 0
				std::streampos currentPos = m_filestream.tellg();
				if (currentPos == std::streampos(-1))
				{
					return 0; // tellg failed
				}

				// convert to long long for safe comparison
				long long size64 = static_cast<long long>(currentPos);
				if (size64 < 0)
				{
					return 0;
				}

				// Note: call the member function through a pointer-to-function style to stop macro expansion to fix conflix with max macro on Windows
				if (static_cast<unsigned long long>(size64) > static_cast<unsigned long long>((std::numeric_limits<long>::max)()))
				{
					return 0; // would overflow long 
				}

				// return size. warning: this cast may lose data if file is larger than what long can hold
				return static_cast<long>(size64);
			}

			// if any error occurs, this method returns 0
			size_t GetTotal() const override
			{
				// if file is not open, return 0. unopen file has size 0
				if (!m_filestream.is_open())
				{
					return 0;
				}

				// store current position. if tellg fails, it returns 0. 
				// if cannot save position, we cannot restore it later so we cannot measure size.
				std::streampos currentPos = m_filestream.tellg();
				if (currentPos == std::streampos(-1))
				{
					return 0; // tellg failed
				}

				// clear any eof flags before seeking
				m_filestream.clear();

				// seek to end.
				m_filestream.seekg(0, std::ios::end);

				// if seekg failed, the stream state will be bad. restore to original position and return 0
				if (!m_filestream.good())
				{
					// attempt to clear bad state
					m_filestream.clear();

					// restore original position
					m_filestream.seekg(currentPos);

					return 0;
				}

				// get end position. if tellg fails, it returns 0
				std::streampos endPos = m_filestream.tellg();
				if (endPos == std::streampos(-1))
				{
					// attempt to clear bad state
					m_filestream.clear();

					// restore original position
					m_filestream.seekg(currentPos);

					return 0;
				}

				// seek back to original position
				m_filestream.clear();
				m_filestream.seekg(currentPos);
				if (!m_filestream.good())
				{
					// attempt to clear bad state
					m_filestream.clear();

					return 0;
				}


				// convert to long long for safe comparison
				long long size64 = static_cast<long long>(endPos);
				if (size64 < 0)
				{
					return 0;
				}

				// Note: call the member function through a pointer-to-function style to stop macro expansion to fix conflict with max macro on Windows
				if (static_cast<unsigned long long>(size64) > static_cast<unsigned long long>((std::numeric_limits<long>::max)()))
				{
					return 0; // would overflow long 
				}

				// return size. warning: this cast may lose data if file is larger than what long can hold
				return static_cast<long>(size64);
			}

			double GetProgress() const override
			{
				size_t total = GetTotal();
				return total > 0 ? static_cast<double>(GetCurrent()) / static_cast<double>(total) : 0.0;
			}

			std::string GetLabel() const override
			{
				return "Reading " + m_filename;
			}

			bool IsDone() const override
			{
				return m_filestream.eof();
			}
		};

	}
};