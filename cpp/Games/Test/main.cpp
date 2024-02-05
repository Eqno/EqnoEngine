#include <Engine/System/include/Application.h>

#include <iostream>
#include <ostream>

int main() {
  try {
    Application* app =
        BaseObject::CreateImmediately<Application>("Games/Test/", "Index");
    app->RunApplication();
    app->DestroyImmediately();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
