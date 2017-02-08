#pragma once

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;

struct ArgParser
{
	ArgParser(int argc, char* argv[]) :
		minLen(0), maxLen(0), error(-1), algVer(1), printData(false)
	{
		if (argc < 5)
		{
			error = 1;
			return;
		}

		bool ok;
		
		// Min and max size of the data
		minLen = str2int(argv[1], &ok);
		if (!ok)
		{
			error = 2;
			return;
		}
		maxLen = str2int(argv[2], &ok);
		if (!ok)
		{
			error = 3;
			return;
		}
		if (!(maxLen && minLen))
		{
			error = 4;
			return;
		}
		if (maxLen < minLen)
			swap(minLen, maxLen);

		// Byte set
		auto ranges = split(argv[3], ":");
		for (auto range : ranges)
		{
			auto borders = split(range.c_str(), "-");
			int first = str2int(borders[0].c_str(), &ok);
			if (!ok)
			{
				error = 5;
				return;
			}
			int last = first;
			if (borders.size() > 1)
			{
				last = str2int(borders[borders.size() - 1].c_str(), &ok);
				if (!ok)
				{
					error = 6;
					return;
				}
			}
			if (last < first)
				swap(first, last);
			for (int i = first; i <= last; i++)
			{
				if (byteSet.end() == find(byteSet.begin(), byteSet.end(), (unsigned char)i))
					byteSet.push_back((unsigned char)i);
			}
		}

		// Hash (in hex format) to decrypt
		hash = hex2bytes(argv[4]);
		if (hash.size() != 16)
		{
			error = 7;
			return;
		}

		if (argc > 5)
		{
			// Algorith version
			algVer = str2int(argv[5], &ok);
			if (!ok)
			{
				error = 8;
				return;
			}
		}
		if (argc > 6)
		{
			// Algorith version
			printData = !!str2int(argv[6], &ok);
			if (!ok)
			{
				error = 9;
				return;
			}
		}

		error = 0;
	}

	int minLen, maxLen;
	vector<unsigned char> byteSet;
	vector<unsigned char> hash;
	int error;
	int algVer;
	bool printData;

private:
	static void release(void* p)
	{
		free(p);
	}
	static char* strclone(const char* str)
	{
		if (!str)
			return NULL;
		int len = strlen(str);
		char* clone = new char[len + 1];
		//char* clone = (char*)malloc(len + 1);
		memcpy(clone, str, len + 1);
		return clone;
	}
	static vector<string> split(const char* str, const char* delim)
	{
		vector<string> result;

		unique_ptr<char> dup(strclone(str));
		//auto dup = shared_ptr<char>(strclone(str), &release);
		char* word = strtok(dup.get(), delim);
		while (word != NULL)
		{
			result.push_back(word);
			word = strtok(NULL, delim);
		}

		return result;
	}
	static int str2int(const char* str, bool* ok = NULL)
	{
		char* end;
		int number = str ? (int)strtol(str, &end, 10) : 0;
		if (ok)
			*ok = str ? end == str+strlen(str) : false;
		return number;
	}
	static vector<unsigned char> hex2bytes(const char* str)
	{
		vector<unsigned char> bytes;
		if (str)
		{
			int len = strlen(str);
			if (!(len & 1))
			{
				char buf[3];
				buf[2] = 0;
				for (const char* hexByte = str; *hexByte;)
				{
					buf[0] = *hexByte++;
					buf[1] = *hexByte++;
					char* end;
					bytes.push_back((unsigned char)strtol(buf, &end, 16));
					if (end != buf + 2)
						return vector<unsigned char>();
				}
			}
		}
		return bytes;
	}
};

