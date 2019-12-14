
#include <misc/flags.h>
#include <iostream>

#include "platform.h"

int main(int argc, char** argv) {

	Platform eng;
	App app(eng);
	eng.loop(app);
	return 0;
}
