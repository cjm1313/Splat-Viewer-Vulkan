#ifndef MYAPP_SPLAT
#define MYAPP_SPLAT

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdint.h>

#include <vulkan/vulkan_raii.hpp>


class App {

  public:
    App();

    void run();

    ~App();

  private:
    uint32_t W_WIDTH = 1280;
    uint32_t W_HEIGHT = 720;
    GLFWwindow* window = nullptr;

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;



    void initVulkan();

    void createInstance();





};

#endif //MYAPP_SPLAT