#pragma once

#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;

class IndexIncrementer
{
	int          _base;
	unsigned int _minLen;
	unsigned int _maxLen;
	vector<int>  _value;
	long long    _count, _maxCount;
	mutex        _mutex;
public:
	IndexIncrementer(int base, unsigned int minLen, unsigned int maxLen) : _base(base), _minLen(minLen), _maxLen(maxLen), _count(0)
	{
		if (_maxLen < _minLen)
			swap(_minLen, _maxLen);
		_maxCount = combinationNumber();
	}
	long long combinationNumber()
	{
		long long allCount = 0;
		for (unsigned int i = _minLen; i <= _maxLen; i++)
		{
			long long baseCount = 1;
			for (unsigned int j = 0; j < i; j++)
				baseCount *= _base;
			allCount += baseCount;
		}
		return allCount;
	}
	// Increment current value
	// If maximum is achived, then return false
	bool inc(vector<int>& value, double* progress=NULL)
	{
		// MinLen==0 is not allowed
		if (!_minLen)
			return false;

		bool ok = false;
		lock_guard<mutex> lock(_mutex);

		if (_value.size()) // not first call
		{
			unsigned int pos;
			for (pos = 0; pos < _value.size(); pos++)
			{
				if (_value[pos] < _base - 1)
				{
					_value[pos]++;
					ok = true;
					break;
				}
			}
			if (ok)
			{
				fill(_value.begin(), _value.begin() + pos, 0);
			}
			else if (_value.size() < _maxLen)
			{
				_value.resize(_value.size() + 1);
				fill(_value.begin(), _value.end(), 0);
				ok = true;
			}
		}
		else // first call
		{
			_value.resize(_minLen, 0);
			ok = true;
		}

		value = _value;
		_count++;

		if (progress)
		{
			*progress = double(_count) / _maxCount * 100;
		}

		return ok;
	}
	static void print(const vector<int>& value)
	{
		for (auto digit : value)
			printf("%d.", digit);
		puts("");
	}
	void print()
	{
		lock_guard<mutex> lock(_mutex);
		print(_value);
	}
};

