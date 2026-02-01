#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <Core/Event.h>

namespace engine
{
	namespace utilities
	{
		namespace parser
		{
			//	design consideration
			//	-	purpose
			//		-	a utility tool that translates a raw character stream into structured rows of string tokens 
			//			stored in array(vector) assuming CSV format with ',' as delimiter. 
			//		-	It emits each parsed row via an event callback for downstream consumers.
			//	-	features
			//		-	stateless parsing of chunks: parser does not own the source stream, it only consumes provided data chunks in bytes.
			//		-	partial line handling: incomplete rows at chunk boundaries are buffered until the next chunk arrives.
			//		-	event-driven: rows are dispatched immediately via ParseRowEvent, allowing subscribers to process data incrementally.
			//		-	format assumption: delimiter is ',' and newline separates rows. No support for quoted fields or escaped 
			//			delimiters (basic CSV only).
			//		-	handling remaining data: ParseRemainingEvent allows processing of any leftover partial row when no more data is expected.
			//	-	highlights
			//		-	incremental parsing: supports streaming input (e.g. network, async file reads). 
			//		-	deferred completion: handles partial rows gracefully across chunk boundaries. 
			//		-	decoupled output: uses event::Event<const std::vector<std::string>&> so multiple subscribers can react to parsed 
			//			rows without tight coupling.
			//	-	Limitations: 
			//		-	no CSV quoting support: fields containing commas or newlines inside quotes are not handled. 
			//		-	assumes UTF-8 or ASCII input; no encoding detection. 
			//		-	does not trim whitespace around tokens. 
			//		-	designed for simplicity and speed, not full RFC 4180 compliance. 
			//		-	Intended as a utility parser; higher-level classes (e.g. StringTable, TileGridLoader) consume its output for domain-specific use.
			//	-	usage:
			//		-	create a CSVParser instance.
			//		-	subscribe to ParseRowEvent with a handler that consumes each parsed row.
			//		-	feed data chunks (from file, network, or memory) into ParseChunk().
			//		-	each complete row is emitted as std::vector<std::string>.
			//		-	can call ParseRemaining() when no more data to parse to flush out any remaining partial row.
			class CSVParser
			{
			private:
				std::string m_partial;

				// this method will parse the data from line. since we expect data from CSV file, we assume delimiter is a ','
				// in this parser, we take whatever data between delimiter as the data and store it as std::string
				std::vector<std::string> ParseLine(const std::string& line)
				{
					std::vector<std::string> tokens;
					std::string token;
					std::istringstream ss(line);
					// std::getline reads from the stream ss into token, stopping whenever it encounters the delimiter ','
					// Each time it succeeds, the extracted substring is appended to the tokens vector
					while (std::getline(ss, token, ','))
					{
						tokens.push_back(token);
					}
					return tokens;
				}

			public:
				void ParseChunk(const char* data, size_t len)
				{
					// append leftover + new chunk
					std::string chunk(data, len);
					std::string full = m_partial + chunk;
					m_partial.clear();

					// this is one long series of characters in bytes.
					std::istringstream stream(full);
					std::string line;

					// getline extracts a series of characters from beginning up to the first occurrence of EOL and store it in line
					while (std::getline(stream, line))
					{
						// if seeking in stream reach EOF and the end of the stream is not EOL, then then there is no full row in the stream.
						// so we save it as partial meaning "partial row" since it is not complete yet. we will process it along with next chunk 
						if (stream.eof() && !full.empty() && full.back() != '\n')
						{
							// incomplete line → save for next chunk
							m_partial = line;
						}
						// but if we did find a full row (there is EOL) somewhere in the chunk, we set it as row
						else
						{
							ParseRowEvent(ParseLine(line));
						}
					}
				}

				// can be called when there is no more data to parse to flush out any remaining partial row
				void ParseRemaining()
				{
					if (!m_partial.empty())
					{
						ParseRemainingEvent(ParseLine(m_partial));
						m_partial.clear();
					}
				}

				event::Event<const std::vector<std::string>&> ParseRowEvent;
				event::Event<const std::vector<std::string>&> ParseRemainingEvent;
			};
		};
	};
};


