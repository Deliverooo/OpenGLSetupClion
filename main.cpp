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
#include "header files/entity.h"
#include "header files/model.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

void OrbitLight(glm::mat4 &light, glm::vec3 &lightPos, const glm::vec3 &pivotPos, float radius, int offsetMultiplier);

void uploadPointLightUniforms(Shader &shader, int index, glm::vec3 &lightPos, float ambientStrength, glm::vec3 lightColour,
    float constant, float linear, float quadratic);

void createSpotLight(const Shader &shader, int index, glm::vec3 &lightPos, glm::vec3 &lightDirection, float ambientStrength, glm::vec3 lightColour,
    float cutOffAngle, float outerCutOffAngle, float constant, float linear, float quadratic);

void uploadDirectionLightUniforms(Shader &shader, const glm::vec3 &direction, float ambientStrength, glm::vec3 lightColour);

void uploadLightUniforms(const Shader &shader, int index, const glm::vec3 &lightPos, glm::vec3 lightColour,
    float linear, float quadratic);

GLuint loadCubemapTextures(const std::vector<std::string> &faces);

#define log(x) std::cout << x << std::endl

GLsizei WIDTH = 1920;
GLsizei HEIGHT = 1080;
float ASPECT_RATIO = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);

typedef glm::vec3 Vector3f;
typedef glm::mat4 Matrix4f;

double deltaTime = 0.0;
double lastFrame = 0.0;
double currentFrame = 0.0;

float lastScroll = 0;
double lastX = (WIDTH) / 2.0;
double lastY = (HEIGHT) / 2.0;
bool firstMouse = true;

bool ui_mode = false;

Vector3f lightPos = glm::vec3(5.0f, 1.0f, 5.0f);
Vector3f lightPos2 = glm::vec3(3.0f, 1.0f, 3.0f);

glm::vec3 cacheCameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
Camera camera(cacheCameraPos);

enum heldItem {
    NONE,
    ORBO,
    VECTOR,
    BALL
};

bool in_hand = false;
heldItem item = NONE;

int main()
{
    // glfw: initialize and configure
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "All hail Orbo", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //enables v-sync
    glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);

    // build and compile our shader program
    Shader shader("resources/shaders/vertex.glsl", "resources/shaders/fragment.glsl"); // you can name your shader files however you like
    Shader shader_001("resources/shaders/vertex_001.glsl", "resources/shaders/fragment_001.glsl");
    Shader screenShader("resources/shaders/postProcessVertex.glsl", "resources/shaders/postProcessFragment.glsl");
    Shader skyboxShader("resources/shaders/skyboxVertex.glsl", "resources/shaders/skyboxFragment.glsl");

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));

    //makes sure that the shader is currently being used
    shader_001.use();

    Model orboModel("resources/models/orbo/Orbo_Obj.obj");
    Model floorTiles("resources/models/floor Tiles/tiles.obj");
    Model trebModel("resources/models/treb/Trebushay.obj");
    Model vecModel("resources/models/vec/Vector_001.obj");
    Model pcModel("resources/models/pc/pc.obj");
    Model ballModel("resources/models/ball/bally.obj");
    Model terrainModel("resources/models/terrain/Terrain.obj");

    Transform terrainTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f));
    Transform ballTransform(glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.25f));
    Transform pcTransform(glm::vec3(3.0f, 0.0f, 3.0f), glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.5f));
    Transform npcOrboTransform(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.75f));
    Transform vecTransform(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    Transform trebTransform(glm::vec3(-2.0f, 1.75f, 9.5f), glm::vec3(0.0f, 0.01f, 0.0f), glm::vec3(0.1f));
    Transform orbotransform(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    Transform floortransform(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log("failed to create framebuffer");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<std::string> cubeMapTexturePaths = {
                                                    "resources/skybox/back.jpg",
                                                "resources/skybox/left.jpg",
                                                "resources/skybox/top.jpg",
                                                "resources/skybox/bottom.jpg",
                                                "resources/skybox/front.jpg",
                                                "resources/skybox/back.jpg"
    };

    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(nullptr));

    GLuint cubeMapTexture = loadCubemapTextures(cubeMapTexturePaths);

    glEnable(GL_CULL_FACE);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        processInput(window);

        camera.update(static_cast<float>(deltaTime));
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), ASPECT_RATIO, 0.1f, 100.0f);
        glm::mat4 skyView = glm::mat4(glm::mat3(camera.getViewMatrix()));
        glm::mat4 view = camera.getViewMatrix();

        // glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.11f, 0.21f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glDepthMask(GL_FALSE);
        skyboxShader.use();
        skyboxShader.uploadUniformMatrix4f("projection", projection);
        skyboxShader.uploadUniformMatrix4f("view", skyView);
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);

        shader_001.use();
        shader_001.uploadUniformMatrix4f("projection", projection);
        shader_001.uploadUniformMatrix4f("view", view);
        shader_001.uploadUniformVector3f("viewPos", camera.cameraFront);

        uploadLightUniforms(shader_001, 0, lightPos, glm::vec3(0.7f), 0.09f, 0.032f);
        uploadLightUniforms(shader_001, 1, lightPos2, glm::vec3(0.7f), 0.09f, 0.032f);

        if (in_hand && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            in_hand = false;
            item = NONE;
        }
        if (glm::length(vecTransform.position - camera.cameraPosition) < 2
            && glm::dot(camera.cameraFront, glm::normalize(vecTransform.position - camera.cameraPosition)) > 0.9f
            && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            in_hand = true;
            item = VECTOR;
        }

        if (glm::length(npcOrboTransform.position - camera.cameraPosition) < 2
            && glm::dot(camera.cameraFront, glm::normalize(npcOrboTransform.position - camera.cameraPosition)) > 0.85f
            && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            in_hand = true;
            item = ORBO;
        }

        if (glm::length(ballTransform.position - camera.cameraPosition) < 2
            && glm::dot(camera.cameraFront, glm::normalize(ballTransform.position - camera.cameraPosition)) > 0.85f
            && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            in_hand = true;
            item = BALL;
        }

        //hey orbo...
        if (item == ORBO) {
            npcOrboTransform.position = camera.cameraPosition + camera.cameraFront - glm::vec3(0.0f, 0.6f, 0.0f);
        } else if (item == VECTOR) {
            vecTransform.position = camera.cameraPosition + camera.cameraFront;

        } else if (item == BALL) {
            ballTransform.position = camera.cameraPosition + camera.cameraFront;
        } else {
            item = NONE;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            in_hand = false;
            item = NONE;
        }

        if (item != ORBO) {
            npcOrboTransform.rotation.y += static_cast<float>(deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
            in_hand = false;
        }

        glm::mat4 orboModelMat = glm::mat4(1.0f);
        orboModelMat = glm::translate(orboModelMat, orbotransform.position);
        orboModelMat = glm::scale(orboModelMat, orbotransform.scale);
        orboModelMat = glm::rotate(orboModelMat, 1.0f, orbotransform.rotation);
        orboModelMat = glm::rotate(orboModelMat, orbotransform.rotation.x, orbotransform.rotation);
        orboModelMat = glm::rotate(orboModelMat, orbotransform.rotation.y, orbotransform.rotation);
        orboModelMat = glm::rotate(orboModelMat, orbotransform.rotation.z, orbotransform.rotation);
        shader_001.uploadUniformFloat("shininess", 16);
        shader_001.uploadUniformMatrix4f("model", orboModelMat);
        orboModel.draw(shader_001);

        glm::mat4 ballModelMat = glm::mat4(1.0f);
        ballModelMat = glm::translate(ballModelMat, ballTransform.position);
        ballModelMat = glm::scale(ballModelMat, ballTransform.scale);
        ballModelMat = glm::rotate(ballModelMat, 1.0f, ballTransform.rotation);
        shader_001.uploadUniformFloat("shininess", 16);
        shader_001.uploadUniformMatrix4f("model", ballModelMat);
        ballModel.draw(shader_001);

        glm::mat4 vecModelMat = glm::mat4(1.0f);
        vecModelMat = glm::translate(vecModelMat, vecTransform.position);
        vecModelMat = glm::scale(vecModelMat, vecTransform.scale);
        vecModelMat = glm::rotate(vecModelMat, 1.0f, vecTransform.rotation);
        shader_001.uploadUniformFloat("shininess", 4);
        shader_001.uploadUniformMatrix4f("model", vecModelMat);
        vecModel.draw(shader_001);

        glm::mat4 floorModelMat = glm::mat4(1.0f);
        floorModelMat = glm::translate(floorModelMat, floortransform.position);
        floorModelMat = glm::scale(floorModelMat, floortransform.scale);
        floorModelMat = glm::rotate(floorModelMat, 1.0f, floortransform.rotation);
        shader_001.uploadUniformFloat("shininess", 32);
        shader_001.uploadUniformMatrix4f("model", floorModelMat);
        floorTiles.draw(shader_001);

        glm::mat4 trebModelMat = glm::mat4(1.0f);
        trebModelMat = glm::translate(trebModelMat, trebTransform.position);
        trebModelMat = glm::scale(trebModelMat, trebTransform.scale);
        trebModelMat = glm::rotate(trebModelMat, 1.0f, trebTransform.rotation);
        shader_001.uploadUniformFloat("shininess", 4);
        shader_001.uploadUniformMatrix4f("model", trebModelMat);
        trebModel.draw(shader_001);

        glm::mat4 pcModelMat = glm::mat4(1.0f);
        pcModelMat = glm::translate(pcModelMat, pcTransform.position);
        pcModelMat = glm::scale(pcModelMat, pcTransform.scale);
        pcModelMat = glm::rotate(pcModelMat, 1.0f, pcTransform.rotation);
        shader_001.uploadUniformFloat("shininess", 32);
        shader_001.uploadUniformMatrix4f("model", pcModelMat);
        pcModel.draw(shader_001);

        glm::mat4 npcOrboModelMat = glm::mat4(1.0f);
        npcOrboModelMat = glm::translate(npcOrboModelMat, npcOrboTransform.position);
        npcOrboModelMat = glm::scale(npcOrboModelMat, npcOrboTransform.scale);
        npcOrboModelMat = glm::rotate(npcOrboModelMat, npcOrboTransform.rotation.x, npcOrboTransform.rotation);
        npcOrboModelMat = glm::rotate(npcOrboModelMat, npcOrboTransform.rotation.y, npcOrboTransform.rotation);
        npcOrboModelMat = glm::rotate(npcOrboModelMat, npcOrboTransform.rotation.z, npcOrboTransform.rotation);
        shader_001.uploadUniformFloat("shininess", 64);
        shader_001.uploadUniformMatrix4f("model", npcOrboModelMat);
        orboModel.draw(shader_001);

        glm::mat4 terrainModelMat = glm::mat4(1.0f);
        terrainModelMat = glm::translate(terrainModelMat, terrainTransform.position);
        terrainModelMat = glm::scale(terrainModelMat, terrainTransform.scale);
        terrainModelMat = glm::rotate(terrainModelMat, 1.0f, terrainTransform.rotation);
        shader_001.uploadUniformFloat("shininess", 4);
        shader_001.uploadUniformMatrix4f("model", terrainModelMat);
        terrainModel.draw(shader_001);
        //
        // // now bind back to default framebuffer and draw a quad plane with the attached framebuffer colour texture
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // // clear all relevant buffers
        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear colour to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        // glClear(GL_COLOR_BUFFER_BIT);
        //
        // screenShader.use();
        // screenShader.uploadUniformFloat("time", (float)glfwGetTime());
        // screenShader.uploadUniformFloat("distance", glm::distance(npcOrboTransform.position, camera.cameraPosition));
        // glBindVertexArray(quadVAO);
        // glBindTexture(GL_TEXTURE_2D, texture);	// use the colour attachment texture as the texture of the quad plane
        // glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui::Begin("Hello ImGui!");
        ImGui::Text("This is text!");
        ImGui::SliderFloat("Trebushay scale X", &trebTransform.scale.x, 0.0f, 1.0f);
        ImGui::SliderFloat("Trebushay scale Y", &trebTransform.scale.y, 0.0f, 1.0f);
        ImGui::SliderFloat("Trebushay scale Z", &trebTransform.scale.z, 0.0f, 1.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrame = glfwGetTime();
        deltaTime = lastFrame - currentFrame;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    ofstream cachePosFile("orbo.txt");
    cachePosFile << std::to_string(camera.cameraPosition.x) + " , " << std::to_string(camera.cameraPosition.y) + " , " << std::to_string(camera.cameraPosition.z);

    glfwDestroyWindow(window);
    glfwTerminate();
    glDeleteFramebuffers(1, &fbo);
    return 0;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (!ui_mode) {
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
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_I && action == GLFW_PRESS && ui_mode == false) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        ui_mode = true;
        log("ui mode");

    } else if (key == GLFW_KEY_I && action == GLFW_PRESS && ui_mode == true) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ui_mode = false;
        log("not ui mode");
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
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
    double mouseDx = xpos - lastX;
    double mouseDy = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (!ui_mode) {
        camera.processMouseMovement(mouseDx, mouseDy);
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}
void OrbitLight(glm::mat4 &light, glm::vec3 &lightPos, const glm::vec3 &pivotPos, const float radius, const float offsetMultiplier) {

    const float dX = pivotPos.x + offsetMultiplier * static_cast<float>((sin(glfwGetTime()) * radius));
    const float dY = pivotPos.y + offsetMultiplier * static_cast<float>(sin(glfwGetTime()) * radius / 8.0f);
    const float dZ = pivotPos.z + offsetMultiplier * static_cast<float>(cos(glfwGetTime()) * radius);

    lightPos.x = dX;
    lightPos.y = dY;
    lightPos.z = dZ;

    light = glm::translate(light, lightPos);
    light = glm::scale(light, glm::vec3(0.35f));
}
void createSpotLight(const Shader &shader, const int index, const glm::vec3 &lightPos, const glm::vec3 &lightDirection,
    const float ambientStrength, const glm::vec3 lightColour, const float cutOffAngle, const float outerCutOffAngle,
    const float linear, const float quadratic) {

    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].parentLight.position", lightPos);
    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].direction", lightDirection);
    shader.uploadUniformFloat("spotlights[" + std::to_string(index) + "].parentLight.ambientStrength", ambientStrength);
    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].parentLight.colour", lightColour);
    shader.uploadUniformFloat("spotlights[" + std::to_string(index) + "].cutOffAngle", cutOffAngle);
    shader.uploadUniformFloat("spotlights[" + std::to_string(index) + "].outerCutOffAngle", outerCutOffAngle);
    shader.uploadUniformFloat("spotlights[" + std::to_string(index) + "].attenuation.linear", linear);
    shader.uploadUniformFloat("spotlights[" + std::to_string(index) + "].attenuation.quadratic", quadratic);
}
void uploadPointLightUniforms(const Shader &shader, const int index, const Vector3f &lightPos, const float ambientStrength,
    const Vector3f lightColour, const float linear, const float quadratic) {

    shader.uploadUniformVector3f("pointLights[" + std::to_string(index) + "].parentLight.position", lightPos);
    shader.uploadUniformVector3f("pointLights[" + std::to_string(index) + "].parentLight.colour", lightColour);
    shader.uploadUniformFloat("pointLights[" + std::to_string(index) + "].parentLight.ambientStrength", ambientStrength);
    shader.uploadUniformFloat("pointLights[" + std::to_string(index) + "].attenuation.linear", linear);
    shader.uploadUniformFloat("pointLights[" + std::to_string(index) + "].attenuation.quadratic", quadratic);
}
void uploadLightUniforms(const Shader &shader, const int index, const glm::vec3 &lightPos, const glm::vec3 lightColour,
    const float linear, const float quadratic) {

    shader.uploadUniformVector3f("lights[" + std::to_string(index) + "].position", lightPos);
    shader.uploadUniformVector3f("lights[" + std::to_string(index) + "].colour", lightColour);
    shader.uploadUniformFloat("lights[" + std::to_string(index) + "].linear", linear);
    shader.uploadUniformFloat("lights[" + std::to_string(index) + "].quadratic", quadratic);
}

void uploadDirectionLightUniforms(const Shader &shader, const glm::vec3 &direction, const float ambientStrength,
    const glm::vec3 lightColour) {

    shader.uploadUniformVector3f("directionLight.direction", direction);
    shader.uploadUniformVector3f("directionLight.parentLight.colour", lightColour);
    shader.uploadUniformFloat("directionLight.parentLight.ambientStrength", ambientStrength);

}
GLuint loadCubemapTextures(const std::vector<std::string> &faces) {

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++) {
        if (unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0)) {

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}