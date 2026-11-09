
#include "app/myWindow.h"

#include <stdexcept>
#include <cstdlib>
#include <iostream>

MyWindow::MyWindow(){
if (!glfwInit()){
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // not using opengl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


    window = glfwCreateWindow(1280, 720, "Splat Viewer", nullptr, nullptr); // default window size


    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
        glfwTerminate();
        
    }

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

MyWindow::MyWindow(uint32_t width, uint32_t height){
if (!glfwInit()){
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // not using opengl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


    window = glfwCreateWindow(width, height, "Splat Viewer", nullptr, nullptr); // default window size


    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
        glfwTerminate();
        
    }

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

MyWindow::~MyWindow(){
    glfwDestroyWindow(window);
    glfwTerminate();
}


int MyWindow::wantsToClose(){
  return glfwWindowShouldClose(window);
}

void MyWindow::pollEvents(){
    glfwPollEvents();
}

void MyWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height){
		auto myWindow                = reinterpret_cast<MyWindow *>(glfwGetWindowUserPointer(window));
		myWindow->framebufferResized = true;
}

vk::raii::SurfaceKHR MyWindow::createSurface(const vk::raii::Instance& instance){
    VkSurfaceKHR       _surface;
    
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
        throw std::runtime_error("failed to create window surface!");
    }
    
    return vk::raii::SurfaceKHR(instance, _surface);
}

const char ** MyWindow::getInstanceExtensions(uint32_t &glfwExtensionCount) const{
	return glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
}

void MyWindow::getFramebufferSize(int *width, int *height){
    glfwGetFramebufferSize(window, width, height);
}

void MyWindow::waitEvents(){
    glfwWaitEvents();
}

const bool MyWindow::getFramebufferResized() const {
    return framebufferResized;
}
void MyWindow::setFramebufferResized(bool value){
    framebufferResized = value;
}