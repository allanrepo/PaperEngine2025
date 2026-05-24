#include <vector>
#include <algorithm>
#include <iostream>


// ([)]

bool ValidateBrackets(const std::string& data)
{
	std::vector<int> opened;
	for (char c : data)
	{
		if (c == '{')
		{
			opened.push_back(0);
		}

		if (c == '(')
		{
			opened.push_back(1);
		}

		if (c == '[')
		{
			opened.push_back(2);
		}

		if (c == '}')
		{
			// top open must be parenthesis
			if (opened.empty()) return false;
			if (opened.back() != 0) return false;
			opened.pop_back();
		}

		if (c == ')')
		{
			// top open must be bracket
			if (opened.empty()) return false;
			if (opened.back() != 1) return false;
			opened.pop_back();
		}

		if (c == ']')
		{
			// top open must be enclosure
			if (opened.empty()) return false;
			if (opened.back() != 2) return false;
			opened.pop_back();
		}
	}
	if (opened.size()) return false;

	return true;
}

std::vector<std::pair<int, int>> MergeIntervals(std::vector<std::pair<int, int>>& intervals)
{
	std::vector<std::pair<int, int>> result;

	if (intervals.empty())
	{
		return result;
	}

	// sort first based on first value ascending order
	std::sort(intervals.begin(), intervals.end(), [](std::pair<int, int>& a, std::pair<int, int>& b) 
		{
			return a.first < b.first;
		});



	std::pair<int, int> curr = intervals.front();
	for (std::pair<int, int>& pair : intervals)
	{
		if (curr.second >= pair.first)
		{
			curr.second = pair.second > curr.second? pair.second : curr.second;
		}
		else
		{
			result.push_back(curr);
			curr = pair;
		}
	}

	result.push_back(curr);


	return result;
}

class LRU
{
private:
	int m_max;
public:
	LRU(int max): m_max(max)
	{

	}

	void Put(int key, int val)
	{
		// if cache size < max just add this

		// if cache size = max, decide what to remove

		// check frequency counter which key has least usage. remove that.

		// add new key/val into our cache

		// add new key/val 

	}

	int Get(int key)
	{

	}
};

int FindDiff(std::vector<int> data, int min, int max)
{
	if (min < 0 || max >= data.size() || min > max)
	{
		return -1;
	}

	int highest = data[min];
	int lowest = data[min];
	for (int i = min; i <= max; i++)
	{
		if (data[i] > highest) highest = data[i];
		if (data[i] < lowest) lowest = data[i];
	}
	return highest - lowest;
}

int LongestStabilizedSegment(std::vector<int> data, int k)
{
	// iterate through each index in array
	// remember indices min and max, starting at min = 0, max = 0
	// set initial max at 0
	// 

	if (data.empty()) return 0;
	int min = 0, max = 0;
	int best = 0;
	while (max < data.size())
	{
		int diff = FindDiff(data, min, max);

		if (diff == -1) break;

		if (diff <= k)
		{
			if (max - min + 1> best) best = max - min + 1;
			max++;
		}
		else
		{
			min++;
			if (min > max) max++;
		}
	}
	return best;
}

int LongestSubArrayEqualElements(std::vector<int>& data)
{
	int min = 0;
	int max = 0;
	int best = 0;
	if (data.empty()) return best;

	while (max < data.size())
	{
		if (data[min] == data[max])
		{
			if (best < max - min + 1) best = max - min + 1;
			max++;
		} 
		else
		{
			min = max;
		}
	}
	return best;
}

int main()
{
	if(false)
	{
		std::string data = "(({[}]))";
		std::cout << data << " = " << (ValidateBrackets(data)?"good": "bad") << std::endl;
	}

	if (false)
	{
		std::vector<std::pair<int, int>> intervals;
		intervals.push_back({ 2,6 });
		intervals.push_back({ 15,18 });
		intervals.push_back({ 1,3 });
		intervals.push_back({ 8,10 });
		intervals = MergeIntervals(intervals);
	}

	if (true)
	{
		std::vector<int> data = {1,1,1, 1,1, 2,2,2,3};

		int best = LongestSubArrayEqualElements(data);

		std::cout << best << std::endl;

	}

	return 0;
}