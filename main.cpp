#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {

    glfwInit();

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Setup", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
    }

    glfwMakeContextCurrent(window);

    gladLoadGL();

    glViewport(0, 0, 800, 600);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glfwSwapBuffers(window);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0.61f, 0.31f, 1.0f);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}