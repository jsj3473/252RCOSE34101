/*
	Author: Jonghyeok Park
	E-mail: jonghyeok_park@korea.ac.kr

	[WARNING] DO NOT MODIFY THIS FILE
*/
#include <iostream>
#include <stdlib.h>

#include "system.h"

#ifdef LOCK
#include "lock.h"
#elif defined(OCC)
#include "occ.h"
#endif

int main(int argc, char** argv) {

	if (argc != 2) {
		std::cout << "Usage. <executable> <filepath>\n";
		return 0;
	}

	std::string filename = argv[1];
	parse_trx_schedule(filename);

#ifdef LOCK
	lock lock;
	lock.run();
#elif defined(OCC)
	occ occ;
	occ.run();
#endif
	return 0;
}
