#include "application.hpp"       // for Application
#include "engine_exceptions.hpp" // for EngineError
#include <exception>             // for exception
#include <ios>                   // for ios_base
#include <print>                 // for println

int main() try {
  engine::Application app;
  app.run();
} catch (const engine::exceptions::EngineError &e) {
  std::println("[ENGINE ERROR]\t {}", e.what());
  return 1;
} catch (const std::ios_base::failure &) {
  std::println("[IOS EXCEPTION]\t Check your binaries paths!");
} catch (const std::exception &e) {
  std::println("[UNHANDLED EXCEPTION]\t {}", e.what());
  return 1;
} catch (...) {
  std::println("[UNKNOWN ERROR]");
  return 1;
}
