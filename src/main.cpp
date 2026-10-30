#include <GLFW/glfw3.h>

#include <iostream>
#include <stdint.h>

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 720;


int main(void){

    if (!glfwInit()){
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // not using opengl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // no window resizing


    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Splat Viewer", nullptr, nullptr);


    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); 
    }

    // free resources
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}