#pragma once
#include <fstream>
#include <Core/Event.h>
#include <Timer/StopWatch.h>

namespace engine
{
	namespace io
	{
		class AsyncFileReader
		{
		private:
			std::ifstream m_filestream;
			std::string m_filename;

		public:
			AsyncFileReader()
			{
			}

			virtual ~AsyncFileReader()
			{
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
				}
			}

			event::Event<const char*, size_t> ProcessChunkEvent;
			event::Event<> EndOfFileFoundEvent;

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
				while (!m_filestream.eof())
				{
					// at this point, we guarantee the file is open, so if Update returns false, we know it is because 
					// it is not finished reading the whole file yet.
					if (Update(maxBytesPerRead, maxTimeToReadMS))
					{
						// finished reading
						return true;
					}
				}

				// if file is already EOF, we will never get inside while loop so to return result, we do it here.
				return m_filestream.eof();
			}

			bool Update(size_t maxBytesPerRead = 0xFF, double maxTimeToReadMS = 1)
			{
				// is file open?
				if (!m_filestream.is_open())
				{
					return false;
				}

				// did we finished reading the file already?
				if (m_filestream.eof())
				{
					return true;
				}

				timer::StopWatch sw;
				sw.Start();
				while (!m_filestream.eof())
				{
					// read chunk size data
					std::vector<char> buffer(maxBytesPerRead);
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

				// returns true if reached EOF already. false otherwise
				return m_filestream.eof();
			}

			long GetFileSizeInBytesLong()
			{
				// is file open?
				if (!m_filestream.is_open())
				{
					return -1;
				}

				// store current position. if tellg fails, it returns -1
				std::streampos currentPos = m_filestream.tellg();
				if (currentPos == std::streampos(-1))
				{
					return -1; // tellg failed
				}

				// clear any eof flags before seeking
				m_filestream.clear();

				// seek to end.
				m_filestream.seekg(0, std::ios::end);

				// if seekg failed, the stream state will be bad. restore to original position and return -1
				if (!m_filestream.good())
				{
					// attempt to clear bad state
					m_filestream.clear();

					// restore original position
					m_filestream.seekg(currentPos);

					// return -1 to indicate failure
					return -1;
				}


				std::streampos endPos = m_filestream.tellg();
				if (endPos == std::streampos(-1))
				{
					// attempt to clear bad state
					m_filestream.clear();

					// restore original position
					m_filestream.seekg(currentPos);

					// return -1 to indicate failure
					return -1;
				}

				// seek back to original position
				m_filestream.clear();
				m_filestream.seekg(currentPos);
				if (!m_filestream.good())
				{
					// attempt to clear bad state
					m_filestream.clear();

					// failed to restore position. return -1
					return -1;
				}


				// convert to long long for safe comparison
				long long size64 = static_cast<long long>(endPos);
				if (size64 < 0)
				{
					return -1;
				}

				// Note: call the member function through a pointer-to-function style to stop macro expansion to fix conflix with max macro on Windows
				if (static_cast<unsigned long long>(size64) > static_cast<unsigned long long>((std::numeric_limits<long>::max)()))
				{
					return -1; // would overflow long 
				}

				// return size. warning: this cast may lose data if file is larger than what long can hold
				return static_cast<long>(size64);
			}

			long GetNumberOfBytesReadLong()
			{
				// is file open?
				if (!m_filestream.is_open())
				{
					return -1;
				}

				// get current position. if tellg fails, it returns -1
				std::streampos currentPos = m_filestream.tellg();
				if (currentPos == std::streampos(-1))
				{
					return -1; // tellg failed
				}

				// convert to long long for safe comparison
				long long size64 = static_cast<long long>(currentPos);
				if (size64 < 0)
				{
					return -1;
				}

				// Note: call the member function through a pointer-to-function style to stop macro expansion to fix conflix with max macro on Windows
				if (static_cast<unsigned long long>(size64) > static_cast<unsigned long long>((std::numeric_limits<long>::max)()))
				{
					return -1; // would overflow long 
				}

				// return size. warning: this cast may lose data if file is larger than what long can hold
				return static_cast<long>(size64);
			}

			bool IsEndOfFile() const
			{
				return m_filestream.eof();
			}

			bool IsOpen() const
			{
				return m_filestream.is_open();
			}

			std::string GetFileName() const
			{
				return m_filename;
			}
		};

	}
};