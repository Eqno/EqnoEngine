#include "Engine/RHI/Vulkan/include/vulkan.h"

#include <iostream>
#include <ostream>

#include "Engine/Model/include/model.h"

int main() {
	try {
		Model model;

		Vulkan app;
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
