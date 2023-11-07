#include <iostream>
#include <ostream>

#include "Engine/RHI/Vulkan/include/vulkan.h"
#include "Engine/System/include/Application.h"

int main() {
	try {
		Application app("Games/Test");
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
