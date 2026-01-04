#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

namespace test
{
	// Write an N x M CSV file filled with 1s.
	// Returns true on success, false on failure.
	bool WriteCsvOnes(const std::string& path, std::size_t rows, std::size_t cols, char delimiter = ',') {
		if (cols == 0) return false; // no columns to write

		std::ofstream out(path.c_str());
		if (!out.is_open()) return false;

		// Build one row string like "1,1,1,1\n" (no trailing delimiter before newline)
		std::string row;
		// Reserve approximate size to avoid repeated reallocations
		row.reserve(cols * 2);

		for (std::size_t c = 0; c < cols; ++c) {
			row += '1';
			if (c + 1 < cols) row += delimiter;
		}
		row += '\n';

		// Write the row 'rows' times
		for (std::size_t r = 0; r < rows; ++r) {
			out.write(row.data(), static_cast<std::streamsize>(row.size()));
			if (!out) return false; // write error
		}

		out.close();
		return out.good();
	}

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

		void ProcessChunk(const char* data, size_t len) 
		{ 
			// streaming state-machine parser: accumulate chars into m_currentCell, 
			// // push on delimiter/newline, handle CRLF, quoted fields if needed. 
		
			// put temporary delay to simulate processing time

			timer::StopWatch sw;

			sw.Start();
			while (sw.Peek<timer::milliseconds>() < 10.0f)
			{
				// busy wait
			}
			sw.Stop();
		}

		bool Update(size_t maxBytesPerRead = 0xFF)
		{
			// did we finished reading the file already?
			if (m_filestream.eof())
			{
				return true;
			}

			// read chunk size data
			std::vector<char> buffer(maxBytesPerRead);
			m_filestream.read(buffer.data(), buffer.size());
			
			std::streamsize n = m_filestream.gcount(); 
			if (n > 0) 
			{ 
				ProcessChunk(buffer.data(), static_cast<size_t>(n)); 
			}

			// returns true if reached EOF already. false otherwise
			return m_filestream.eof();
		}
	};

	class TestFileReader
	{
	private:

	public:
		TestFileReader()
		{
			bool ok = WriteCsvOnes("big.csv", 5000, 5000);
			if (!ok) 
			{
				std::cerr << "Failed to write CSV file\n";
			}

		}
	};
}