#pragma once
#include <Utilities/Logger.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


namespace utilities::fileio
{
    class CSVFile 
    {
    public:
        explicit CSVFile(
            const std::string& filename, 
            char delimiter = ',',
            const std::string& commentMarker = "//"
        )
            : m_filename(filename), m_delimiter(delimiter), m_commentMarker(commentMarker) 
        {
        }

        bool read() 
        {
            m_data.clear();

            std::ifstream file(m_filename);

            if (!file.is_open()) 
            {
                LOG("Could not open file: " << m_filename << " as CSV file.");
                return false;
            }

            std::string line;
            while (std::getline(file, line))
            {
                // Trim leading whitespace
                size_t firstChar = line.find_first_not_of(" \t");
                if (firstChar == std::string::npos) continue; // skip empty line
                std::string trimmed = line.substr(firstChar);

                // Skip comment lines
                if (!m_commentMarker.empty() && trimmed.compare(0, m_commentMarker.size(), m_commentMarker) == 0) 
                {
                    continue;
                }

                std::vector<std::string> row;
                std::stringstream ss(line);
                std::string cell;

                while (std::getline(ss, cell, m_delimiter)) 
                {
                    row.push_back(cell);
                }

                m_data.push_back(std::move(row));
            }
			return true;
        }

		template<typename T>
        T GetValue(int row, int col) const
        {
            if (row < 0 || row >= static_cast<int>(m_data.size()) ||
                col < 0 || col >= static_cast<int>(m_data[row].size())) 
            {
                throw std::out_of_range("Index out of range");
            }
            std::istringstream iss(m_data[row][col]);
            T value;
            iss >> value;
            if (!iss.fail()) 
            {
                return value;
            }
            else
            {
                // handle special case if type bool and string value is "false" or "true"
                if constexpr (std::is_same_v<T, bool>)
                {
                    std::istringstream issIsBool(m_data[row][col]);
                    bool booleanIsBool;
                    issIsBool >> std::boolalpha >> booleanIsBool;
                    if (!issIsBool.fail())
                    {
                        return booleanIsBool;
                    }
                }
            }
            // if you reached this point, data conversion failed somewhere 
            throw std::runtime_error("Failed to convert value: " + m_data[row][col]);
        }

        size_t GetRowCount() const 
        {
            return m_data.size();
		}

        size_t GetColCount(size_t row = 0) const 
        {
            if (row >= m_data.size()) 
            {
                throw std::out_of_range("Row index out of range");
            }
            return m_data[row].size();
		}

     private:
        std::string m_filename;
        char m_delimiter;
        std::string m_commentMarker;
        std::vector<std::vector<std::string>> m_data;

    };

}