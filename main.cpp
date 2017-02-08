#include <iostream>

#include "argparser.h"
#include "hashbruteforcer.h"

int main(int argc, char* argv[])
{
	//IndexIncrementer abn(16, 2, 4);

	//printf("%d combinations\n", (int)abn.maxCount());

	////vector<int> value;
	////while (abn.inc(value))
	////	IndexIncrementer::print(value);

	//return 0;

	ArgParser ap(argc, argv);
	if (ap.error)
	{
		printf("Invalid argument format (%d). Correct format is:\n", ap.error);
		puts("ExeFile MinLength MaxLength ByteSet HexHash");
		puts("Byte set should has the following format: from-to:from-to:...:from-to");
		return 1;
	}

	HashBruteForser hbf(ap.hash, ap.byteSet, ap.minLen, ap.maxLen, ap.printData);
	HashBruteForser2 hbf2(ap.hash, ap.byteSet, ap.minLen, ap.maxLen, ap.printData);

	if (ap.algVer == 1)
	{
		// Start cracking
		hbf.start(ap.hash);
	}
	else
	{
		// Start cracking
		hbf2.start(ap.hash);
	}

	// Give threads a little time to print initial messages first
	this_thread::sleep_for(chrono::milliseconds(10));

	// Then print this
	puts("Press ENTER to exit");
	string line;
	getline(cin, line);

	return 0;
}


