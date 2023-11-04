#include <vulkan.h>
#include <iostream>
#include <ostream>

int main() {
	try {
		Vulkan app;
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
