#include <iomanip>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "header files/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <iostream>

#include "header files/camera.h"
#include "header files/texture.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
constexpr GLuint WIDTH = 1280;
constexpr GLuint HEIGHT = 720;
GLfloat FOV = 45.0f;

double deltaTime = 0.0f;
double lastFrame = 0.0f;
double currentFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 1.0f, 0.0f));

float lastScroll = 0;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

glm::vec3 lightPos = glm::vec3(6.0f, 1.0f, 6.0f);
glm::vec3 lightDir = glm::vec3(0.0f, -1.0f, 0.0f);

enum lightTypes {
    DIRECTIONAL_LIGHT = 0,
    POINT_LIGHT = 1,
};

float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
};

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "All hail Orbo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(1);
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    Shader shader("C:/Users/oocon/CLionProjects/Engine3d/resources/shaders/vertex.glsl", "C:/Users/oocon/CLionProjects/Engine3d/resources/shaders/fragment.glsl"); // you can name your shader files however you like
    Shader lightShader("C:/Users/oocon/CLionProjects/Engine3d/resources/shaders/light_vertex.glsl", "C:/Users/oocon/CLionProjects/Engine3d/resources/shaders/light_fragment.glsl");

    unsigned int VBO, VAO;

    //generates the vertex arrays and buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    //binds the vertex array, so it is the one that is currently active
    glBindVertexArray(VAO);

    //binds the buffer so we can store data in it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //let me break it down for you Mark.
    //the size is the number of elements in the attribute. e.g. a vec3 would have 3 and a vec2 would have 2
    //the stride is the total number of elements multiplied by the float size in bytes. e.g. a vec2 and vec3 would have a combined size of 5
    // the pointer is the offset for each attribute from the previous one
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //texture coordinates attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //normals attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // You can unbind the VAO afterward so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyway so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    Texture diffuseTex("resources/textures/floor_tiles/floor_tiles_06_diff_2k.png", true);
    Texture specularTex("resources/textures/floor_tiles/floor_tiles_06_spec_2k.png", true);
    Texture emissionMap("resources/textures/taper.png", true);

    Texture lowTex("resources/textures/Loooooowwww.png", true);

    //makes sure that the shader is currently being used
    shader.use();

    //tells the shader that the diffuse texture will be passed in texture slot 0
    shader.uploadInt("material.diffuse", 0);
    shader.uploadInt("material.specular", 1);

    float delay = 0;
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        processInput(window);
        camera.update(deltaTime);
        glClearColor(0.0f, 0.61f, 0.81f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        shader.use();

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.zoom), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);

        glm::mat4 view;
        view = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.cameraFront, camera.cameraUp);

        shader.uploadUniformMatrix4f("projection", projection);
        shader.uploadUniformMatrix4f("view", view);

        float mixFactor = 1.0f;
        int shininess = 32;

        shader.uploadInt("lightType", POINT_LIGHT);

        shader.uploadUniformVector3f("light.lightPos", lightPos);
        shader.uploadUniformVector3f("light.ambient", glm::vec3(0.15f));
        shader.uploadUniformVector3f("light.diffuse",  glm::vec3(0.6f));
        shader.uploadUniformVector3f("light.specular",  glm::vec3(1.0f));
        shader.setFloat("light.constant", 0.7f);
        shader.setFloat("light.linear",0.09f );
        shader.setFloat("light.quadratic", 0.032f);


        // shader.uploadUniformVector3f("directionLight.direction", lightDir);
        // shader.uploadUniformVector3f("directionLight.ambient", glm::vec3(0.15f));
        // shader.uploadUniformVector3f("directionLight.diffuse",  glm::vec3(0.6f));
        // shader.uploadUniformVector3f("directionLight.specular",  glm::vec3(1.0f));

        shader.setFloat("material.emissionStrength", 0.0f);
        shader.uploadInt("material.specularRoughness", shininess);
        shader.setFloat("mixFactor", mixFactor);

        //activates texture slot 0 and sets the texture to be the diffuse
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTex.getId());
        //specular texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularTex.getId());
        //emissive texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, emissionMap.getId());

        shader.uploadUniformVector3f("viewPos", camera.cameraPosition);


        for (unsigned int i = 0; i < 13; i++) {
            for (unsigned int j = 0; j < 13; j++) {

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i, -1, j));

                shader.uploadUniformMatrix4f("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }


        glm::mat4 cube = glm::mat4(1.0f);
        glm::vec3 cubePos = glm::vec3(3.0f, 0.0f, 3.0f);
        cube = glm::translate(cube, cubePos);

        shader.uploadUniformMatrix4f("model", cube);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        lightShader.use();
        lightShader.uploadUniformMatrix4f("projection", projection);
        lightShader.uploadUniformMatrix4f("view", view);
        glm::mat4 light = glm::mat4(1.0f);

        glm::vec3 pivotPos = glm::vec3(6.0f, 1.0f, 6.0f);
        const float radius = 2.0f;
        float dX = pivotPos.x + (sin(glfwGetTime()) * radius);
        float dY = pivotPos.y + sin(glfwGetTime()) * radius / 4.0f;
        float dZ = pivotPos.z + (cos(glfwGetTime()) * radius);

        lightPos.x = dX;
        lightPos.y = dY;
        lightPos.z = dZ;

        light = glm::translate(light, lightPos);
        light = glm::scale(light, glm::vec3(0.35f));

        lightShader.uploadUniformMatrix4f("light", light);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrame = glfwGetTime();
        deltaTime = lastFrame - currentFrame;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.processKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.processKeyboard(JUMP, deltaTime);
    }
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    std::cout << "Frame buffer size: " << width << ", " << height << std::endl;
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float mouseDx = (float)xpos - lastX;
    float mouseDy = (float)lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(mouseDx, mouseDy);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}
