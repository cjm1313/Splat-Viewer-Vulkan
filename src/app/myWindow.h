#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

class MyWindow{
    public:
        MyWindow();
        MyWindow(uint32_t width, uint32_t height);
        ~MyWindow();

        int wantsToClose();
        void pollEvents();

        vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance) const;

        const char** getInstanceExtensions(uint32_t &glfwExtensionCount) const;

        const bool getFramebufferResized() const;
        
        void setFramebufferResized(bool value);

        void getFramebufferSize(int *width, int *height);

        void waitEvents();

    private:

    GLFWwindow* window = nullptr;
    bool framebufferResized = false;
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
};



#endif //MYWIDNOW_H