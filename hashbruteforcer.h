#pragma once

#include <thread>
#include <chrono>
#include <mutex>
#include <memory>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;
using namespace chrono;

#include "md5.h"

//thread_local int _threadNumber;

// Hash function
vector<unsigned char> hashFunc(vector<unsigned char>& data)
{
	unsigned char result[16];

	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, &data[0], data.size());
	MD5_Final(result, &ctx);

	return vector<unsigned char>(result, result + sizeof(result));
}

#include "multibasenumber.h"
#include "indexincrementer.h"

class HashBruteForser
{
	unsigned int _threadCount;         // Thread count to run (based on core count in system)
	vector<unsigned char> _hash;       // Hash to crack
	vector<unsigned char> _byteSet;    // Byte set of data (all possible bytes in data)
	int _minLen;                       // Min data length
	int _maxLen;                       // Max data length
	list<shared_ptr<thread>> _threads; // Thread object pointers
	bool _exit;                        // Exit flag for threads
	mutex _mutexExit;                  // Mutex for exit flag
	list<int> _threadNumbers;          // Thread numbers (just 1, 2, 3, ...)
	mutex _mutexThreadNumbers;         // Mutex for thread numbers
	string _dataPrintable;             // Found data printable presentation
	mutex _mutexDataPrintable;         // Mutex for printable data
	bool _printAllData;                // Flag indicating if threads should print each data that is checked
	milliseconds _startTime;           // Start time in milliseconds

public:
	HashBruteForser(vector<unsigned char> hash, vector<unsigned char> byteSet, int minLen, int maxLen, bool printAllData)
		: _hash(hash), _byteSet(byteSet), _minLen(minLen), _maxLen(maxLen), _printAllData(printAllData)
	{
		_threadCount = thread::hardware_concurrency();
	}
	~HashBruteForser()
	{
		// Tell all threads to exit
		setExit(true);

		// Wait for all threads exit
		for (auto thread : _threads)
			thread->join();
	}

	void start(vector<unsigned char> hash)
	{
		setExit(false);

		_startTime = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

		// Split byte set into parts which count is core count
		// These parts will be used for first byte brute-forcing
		if (_threadCount > _byteSet.size())
			_threadCount = _byteSet.size();
		vector<vector<unsigned char>> firsts;
		for (unsigned int i = 0; i < _byteSet.size(); i++)
		{
			if (firsts.size() < _threadCount)
				firsts.push_back(vector<unsigned char>());
			firsts[i % _threadCount].push_back(_byteSet[i]);
		}

		for (unsigned int i = 0; i < _threadCount; i++)
		{
			// Table of all possible bytes for each byte position in data
			vector<vector<unsigned char>> byteTable;

			// for first byte
			byteTable.push_back(firsts[i]);

			// for all other byte positions
			for (int j = 1; j < _maxLen; j++)
				byteTable.push_back(_byteSet);

			int threadNumber = i + 1;

			// Start thread
			_threads.push_back(shared_ptr<thread>(new thread([threadNumber, byteTable, this]() { bruteForce(threadNumber, byteTable); })));

			// Save thread number (1, 2, 3, ...) in a list
			lock_guard<mutex> lock(_mutexThreadNumbers);
			_threadNumbers.push_back(threadNumber);
		}
	}

private:
	bool isExit()
	{
		lock_guard<mutex> lock(_mutexExit);
		return _exit;
	}

	void setExit(bool need)
	{
		lock_guard<mutex> lock(_mutexExit);
		_exit = need;
	}

	// Thread completion handler
	void threadCompleted(int threadNumber, bool found)
	{
		printf("Thread %d is completed (%s)\n", threadNumber, found ? "found" : "not found");

		// Flag indicating if all thread are completed (current thread is the last working)
		bool allCompleted;

		// Remove curent thread number from the list
		{
			lock_guard<mutex> lock(_mutexThreadNumbers);
			_threadNumbers.remove(threadNumber);
			allCompleted = _threadNumbers.empty();
		}

		// If this thread is the last working
		if (allCompleted)
		{
			milliseconds curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

			// Print results
			{
				printf("Search is completed in %d ms.\n", (int)(curTime.count() - _startTime.count()));

				lock_guard<mutex> lock(_mutexDataPrintable);

				if (_dataPrintable.length())
					printf("Data is \"%s\"\n", _dataPrintable.c_str());
				else
					puts("Data was not found");
			}
		}
	}

	// Main function
	void bruteForce(int threadNumber, vector<vector<unsigned char>> byteTable)
	{
		printf("Thread %d is working...\n", threadNumber);

		// Data found flag
		bool found = false;

		// Go through all data lengths
		for (int len = _minLen; len <= _maxLen; len++)
		{
			// Build byte value counts (bases) for all byte positions
			vector<int> bases(len);
			for (unsigned int i = 0; i < bases.size(); i++)
				bases[i] = byteTable[i].size();

			// Create multi base number for enumerating indexes of bytes in all positions
			MultiBaseNumber indexes(bases);

			do
			{
				// Build data (based on current index set)
				vector<unsigned char> data(len);
				for (unsigned int i = 0; i < bases.size(); i++)
					data[i] = byteTable[i][indexes.value()[i]];

				// Get data hash
				vector<unsigned char> dataHash = hashFunc(data);

				// Append zero termination for further printing
				data.push_back(0);

				// Print current data if needed
				if (_printAllData)
					puts((char*)&data[0]);

				// Compare hashes
				if (!memcmp(&dataHash[0], &_hash[0], _hash.size()))
				{
					// Data is found. Tell all threads to exit.
					setExit(found=true);

					lock_guard<mutex> lock(_mutexDataPrintable);
					_dataPrintable = (char*)&data[0];
				}

			} while (!isExit() && indexes.inc()); // Go through all indexes
		}
		
		// Call thread completion handler
		threadCompleted(threadNumber, found);
	}
};


class HashBruteForser2
{
	unsigned int _threadCount;          // Thread count to run (based on core count in system)
	vector<unsigned char> _hash;        // Hash to crack
	vector<unsigned char> _byteSet;     // Byte set of data (all possible bytes in data)
	list<shared_ptr<thread>> _threads;  // Thread object pointers
	bool _exit;                         // Exit flag for threads
	mutex _mutexExit;                   // Mutex for exit flag
	list<int> _threadNumbers;           // Thread numbers (just 1, 2, 3, ...)
	mutex _mutexThreadNumbers;          // Mutex for thread numbers
	string _dataPrintable;              // Found data printable presentation
	mutex _mutexDataPrintable;          // Mutex for printable data
	bool _printAllData;                 // Flag indicating if threads should print each data that is checked
	milliseconds _startTime;            // Start time in milliseconds
	IndexIncrementer _indexIncrementer; // Byte indexes

public:
	HashBruteForser2(vector<unsigned char> hash, vector<unsigned char> byteSet, int minLen, int maxLen, bool printAllData)
		: _hash(hash), _byteSet(byteSet), _printAllData(printAllData), _indexIncrementer(byteSet.size(), minLen, maxLen)
	{
		_threadCount = thread::hardware_concurrency();
	}
	~HashBruteForser2()
	{
		// Tell all threads to exit
		setExit(true);

		// Wait for all threads exit
		for (auto thread : _threads)
			thread->join();
	}

	void start(vector<unsigned char> hash)
	{
		setExit(false);

		_startTime = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

		for (unsigned int i = 0; i < _threadCount; i++)
		{
			int threadNumber = i + 1;

			// Start thread
			_threads.push_back(shared_ptr<thread>(new thread([threadNumber, this]() { bruteForce(threadNumber); })));

			// Save thread number (1, 2, 3, ...) in a list
			lock_guard<mutex> lock(_mutexThreadNumbers);
			_threadNumbers.push_back(threadNumber);
		}
	}

private:
	bool isExit()
	{
		lock_guard<mutex> lock(_mutexExit);
		return _exit;
	}

	void setExit(bool need)
	{
		lock_guard<mutex> lock(_mutexExit);
		_exit = need;
	}

	// Thread completion handler
	void threadCompleted(int threadNumber, bool found)
	{
		printf("Thread %d is completed (%s)\n", threadNumber, found ? "found" : "not found");

		// Flag indicating if all thread are completed (current thread is the last working)
		bool allCompleted;

		// Remove curent thread number from the list
		{
			lock_guard<mutex> lock(_mutexThreadNumbers);
			_threadNumbers.remove(threadNumber);
			allCompleted = _threadNumbers.empty();
		}

		// If this thread is the last working
		if (allCompleted)
		{
			milliseconds curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

			// Print results
			{
				printf("Search is completed in %d ms.\n", (int)(curTime.count() - _startTime.count()));

				lock_guard<mutex> lock(_mutexDataPrintable);

				if (_dataPrintable.length())
					printf("Data is \"%s\"\n", _dataPrintable.c_str());
				else
					puts("Data was not found");
			}
		}
	}

	// Main function
	void bruteForce(int threadNumber)
	{
		printf("Thread %d is working...\n", threadNumber);
		this_thread::sleep_for(chrono::milliseconds(10));

		// Data found flag
		bool found = false;

		vector<int> indexes;
		double progress;

		while (!isExit() && _indexIncrementer.inc(indexes,&progress))
		{
			// Build data (based on current index set)
			vector<unsigned char> data(indexes.size());
			for (unsigned int i = 0; i < data.size(); i++)
				data[i] = _byteSet[indexes[i]];

			// Get data hash
			vector<unsigned char> dataHash = hashFunc(data);

			//printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%lf%%", progress);

			// Append zero termination for further printing
			data.push_back(0);

			// Print current data if needed
			if (_printAllData)
				printf("%d: %s (%lf%%)\n", threadNumber, (char*)&data[0], progress);

			// Compare hashes
			if (!memcmp(&dataHash[0], &_hash[0], _hash.size()))
			{
				// Data is found. Tell all threads to exit.
				setExit(found = true);

				lock_guard<mutex> lock(_mutexDataPrintable);
				_dataPrintable = (char*)&data[0];
			}
		}

		// Call thread completion handler
		threadCompleted(threadNumber, found);
	}
	string intVecToStr(const vector<int>& v)
	{
		char buf[256] = "";
		for (auto d : v)
			sprintf(buf+strlen(buf), "%d.", d);
		return string(buf);
	}
};

