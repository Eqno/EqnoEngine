#include <iostream>
#include <ostream>

#include "Engine/System/include/Application.h"

int main() {
	try {
		Application app("Application", "Games/Test/", "Index");
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
