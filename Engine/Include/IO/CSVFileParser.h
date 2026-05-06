#pragma once
#include <IO/ASyncFileReader.h>
#include <Utilities/CSVParser.h>
#include <Containers/Table.h>
#include <Utilities/Utilities.h>

namespace engine
{
	namespace io
	{
#pragma region // CSV file parser
		class CSVFileParser
		{
		private:
			AsyncFileReader m_fileReader;
			engine::utilities::parser::CSVParser m_csvParser;

		public:
			CSVFileParser()
			{
				// chain our events where CSV parser listens to file reader when it extract chunk of data from file
				m_fileReader.ProcessChunkEvent += engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
				m_fileReader.EndOfFileFoundEvent += engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
			}

			~CSVFileParser()
			{
				m_fileReader.ProcessChunkEvent -= engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseChunk);
				m_fileReader.EndOfFileFoundEvent -= engine::event::Handler(&m_csvParser, &engine::utilities::parser::CSVParser::ParseRemaining);
			}

			bool ReadImmediate(const std::string& filename, engine::container::Table<std::string>& table)
			{
				// chain CSV table to CSV parser to acquire row of data from CSV Parser when it parse chunk of data and extracts rows of CSV data
				m_csvParser.ParseRowEvent += engine::event::Handler(&table, &engine::container::Table<std::string>::AddRow);
				m_csvParser.ParseRemainingEvent += engine::event::Handler(&table, &engine::container::Table<std::string>::AddRange);

				// RAII: this ensures Table object unsubscribes to CSVParser once this function goes out of scope
				engine::utilities::OnOutOfScope cleanup([&]
					{
						// unsubscribe to CSV parser 
						m_csvParser.ParseRowEvent -= engine::event::Handler(&table, &engine::container::Table<std::string>::AddRow);
						m_csvParser.ParseRemainingEvent -= engine::event::Handler(&table, &engine::container::Table<std::string>::AddRange);
					});

				// make sure to close file first if there was a previous one
				m_fileReader.Close();

				// open the file
				m_fileReader.Open(filename);

				// read the contents of the file
				bool result = m_fileReader.SyncReadAll();

				// close the file
				m_fileReader.Close();

				// return result
				return result;
			}
		};
#pragma endregion
	}
}