#include <Engine/System/include/Application.h>

#include <iostream>
#include <ostream>

int main() {
  try {
    std::shared_ptr<Application> app =
        BaseObject::CreateImmediately<Application>("Games/Test/", "Index");
    app->RunApplication();
    app->Destroy();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
