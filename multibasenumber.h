#pragma once

#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;

class MultiBaseNumber
{
	vector<int> _bases;
	vector<int> _value;
public:
	MultiBaseNumber(const vector<int>& bases) : _bases(bases), _value(_bases.size())
	{
		fill(_value.begin(), _value.end(), 0);
	}
	vector<int>& value()
	{
		return _value;
	}
	// Increment current value
	// If maximum is achived, then return false
	bool inc()
	{
		bool ok = false;
		unsigned int pos;
		for (pos = 0; pos < _value.size(); pos++)
		{
			if (_value[pos] < _bases[pos] - 1)
			{
				ok = true;
				_value[pos]++;
				break;
			}
		}
		if (ok)
			fill(_value.begin(), _value.begin() + pos, 0);
		return ok;
	}
};
