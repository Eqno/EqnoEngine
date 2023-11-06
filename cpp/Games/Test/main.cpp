#include "Engine/RHI/Vulkan/include/vulkan.h"

#include <iostream>
#include <ostream>

int main() {
	try {
		Vulkan app(VulkanConfig::GAMES_PATH + "Test/");
		app.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
