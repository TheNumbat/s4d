
#include <iostream>
#include "platform/platform.h"

int main(int argc, char** argv) {

	Platform eng;
	App app(eng);
	eng.loop(app);
	return 0;
}
