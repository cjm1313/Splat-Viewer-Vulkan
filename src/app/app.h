#ifndef MYAPP_SPLAT
#define MYAPP_SPLAT

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdint.h>
#include <stdexcept>

constexpr uint32_t W_WIDTH = 1280;
constexpr uint32_t W_HEIGHT = 720;

class App {

  public:
    App();

    void run();

    ~App();

  private:
    GLFWwindow* window = nullptr;




};

#endif //MYAPP_SPLAT