#include "app/app.h"


App::App(){
    if (!glfwInit()){
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // not using opengl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // no window resizing


    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "Splat Viewer", nullptr, nullptr);


    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
        glfwTerminate();
        
    }
}


void App::run(){

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); 
    }

}



App::~App(){
    glfwDestroyWindow(window);
    glfwTerminate();
}