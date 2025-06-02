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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void OrbitLight(glm::mat4 &light, glm::vec3 &lightPos, const glm::vec3 &pivotPos, float radius, const int offsetMultiplier);
void uploadPointLightUniforms(Shader &shader, int index, glm::vec3 &lightPos, float ambientStrength, glm::vec3 lightColour,
    float constant, float linear, float quadratic);
void createSpotLight(Shader &shader, const int index, glm::vec3 &lightPos, glm::vec3 &lightDirection, float ambientStrength, glm::vec3 lightColour,
    float cutOffAngle, float outerCutOffAngle, float constant, float linear, float quadratic);

void uploadDirectionLightUniforms(Shader &shader, const glm::vec3 &direction, float ambientStrength, glm::vec3 lightColour);
GLuint loadCubemapTextures(std::vector<std::string> faces);

#define log(x) std::cout << x << std::endl
#define constexpr GLuint WIDTH = 1920;
constexpr GLuint HEIGHT = 1080;

typedef glm::vec3 Vector3f;
typedef glm::mat4 Matrix4f;

double deltaTime = 0.0f;
double lastFrame = 0.0f;
double currentFrame = 0.0f;

float lastScroll = 0;
float lastX = static_cast<float>(WIDTH) / 2.0f;
float lastY = static_cast<float>(HEIGHT) / 2.0f;
bool firstMouse = true;

Vector3f lightPos = glm::vec3(-2.0f, 1.0f, 0.0f);
Vector3f lightPos2 = glm::vec3(3.0f, 1.0f, 3.0f);

Vector3f modelItemPos = Vector3f(0.0f, 0.0f, 1.0f);
glm::vec3 spotLightPos = glm::vec3(2.0f, 2.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);

Vector3f spotLightCol = Vector3f(0.5f);
Vector3f spotLightCol2 = Vector3f(0.5f);

glm::vec3 lightDir = glm::vec3(0.0f, -0.7f, 0.5f);

glm::vec3 cacheCameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
Camera camera(cacheCameraPos);

int main()
{
    // glfw: initialize and configure
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "All hail Orbo", glfwGetPrimaryMonitor(), nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
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

    std::string cacheText;
    ifstream readCacheCamPosFile("orbo.txt");
    while (getline(readCacheCamPosFile, cacheText)) {
        cout << cacheText << endl;
    }

    // cacheCameraPos.x = std::stof(cacheText.substr(0, cacheText.find_first_of(',')).c_str());
    // cacheCameraPos.y = std::stof(cacheText.substr(cacheText.find_first_of(',') + 1, 14).c_str());
    // cacheCameraPos.z = std::stof(cacheText.substr(14, 21).c_str());

    // camera.cameraPosition = cacheCameraPos;
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);

    // build and compile our shader program
    Shader shader("resources/shaders/vertex.glsl", "resources/shaders/fragment.glsl"); // you can name your shader files however you like
    Shader screenShader("resources/shaders/postProcessVertex.glsl", "resources/shaders/postProcessFragment.glsl");

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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    //makes sure that the shader is currently being used
    shader.use();

    Model orboModel("resources/models/orbo/Orbo_Obj.obj");
    // Model floorTiles("resources/models/floor Tiles/tiles.obj");
    Model trebModel("resources/models/treb/Trebushay.obj");
    Model vecModel("resources/models/vec/Vector_001.obj", false);
    Model pcModel("resources/models/pc/pc.obj");
    Model ballModel("resources/models/ball/ball.obj", false);
    Model terrainModel("resources/models/terrain/Terrain.obj", false);

    Transform terrainTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f));
    Entity terrainEntity(terrainTransform, terrainModel);

    Transform ballTransform(glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f));
    Entity ballEntity(ballTransform, ballModel);

    Transform pcTransform(glm::vec3(3.0f, 0.0f, 3.0f), glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.5f));
    Entity pcEntity(pcTransform, pcModel);

    Transform npcOrboTransform(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.75f));
    Entity npcOrboEntity(npcOrboTransform, orboModel);

    Transform vecTransform(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    Entity vecEntity(vecTransform, vecModel);

    Transform trebTransform(glm::vec3(-2.0f, 1.75f, 9.5f), glm::vec3(0.0f, 0.01f, 0.0f), glm::vec3(0.1f));
    Entity trebEntity(trebTransform, trebModel);

    Transform orbotransform(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    Entity orboEntity(orbotransform, orboModel);

    // Transform floortransform(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    // Entity floorEntity(floortransform, floorTiles);

    log(camera.cameraPosition.x << camera.cameraPosition.y << camera.cameraPosition.z);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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

    // std::vector<std::string> cubeMapTexturePaths = {"resources/models/orbo/cinema.jpg",
    //                                             "resources/models/orbo/diffuseTex2.png",
    //                                             "resources/models/orbo/diffuseTex2.png",
    //                                             "resources/models/floor Tiles/diffuseTex.png",
    //                                             "resources/models/floor Tiles/plainTex.png",
    //                                             "resources/models/floor Tiles/specularTex.png"};


    glEnable(GL_CULL_FACE);

    float ballVelocity = 2.0f;
    enum heldItem {

        NONE,
        ORBO,
        VECTOR
    };

    bool pressed = false;

    std::cout << "started" << std::endl;
    bool in_hand = false;
    heldItem item = NONE;
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        processInput(window);

        camera.update(static_cast<float>(deltaTime));

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.11f, 0.21f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(camera.cameraPosition, camera.cameraPosition + camera.cameraFront, camera.cameraUp);
        shader.uploadUniformMatrix4f("projection", projection);
        shader.uploadUniformMatrix4f("view", view);
        shader.uploadUniformVector3f("viewPos", camera.cameraFront);

        uploadDirectionLightUniforms(shader, glm::vec3(0.0f, -1.0f, 0.2f), 0.3f, glm::vec3(1.0f, 1.0f, 1.0f));

        uploadPointLightUniforms(shader, 0, lightPos, 0.05f, glm::vec3(0.2f), 1.0f, 0.09f, 0.032f);

        createSpotLight(shader, 0, spotLightPos, spotLightDirection, 0.05f, spotLightCol,
            cos(glm::radians(35.0f)), cos(glm::radians(40.0f)), 1.0f, 0.09f, 0.032f);

        ballEntity.draw(shader);

        orboEntity.draw(shader);
        if (orboEntity.transform.scale.z <= 1.5f) {
            orboEntity.transform.scale.z += deltaTime * 0.5f;
        }else if (orboEntity.transform.scale.y <= 1.5f) {
            orboEntity.transform.scale.y += deltaTime * 0.5f;
        }else if (orboEntity.transform.scale.x <= 1.5f) {
            orboEntity.transform.scale.x += deltaTime * 0.5f;
        }else if (orboEntity.transform.position.x <= 5.5f) {
            orboEntity.transform.position.x += deltaTime * 0.5f;
        } else {
            orboEntity.transform.rotation.x += deltaTime * 0.5f;
            orboEntity.transform.rotation.y += deltaTime * 0.5f;
            orboEntity.transform.rotation.z += deltaTime * 0.5f;
        }



        //big orbo
        if (in_hand && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            in_hand = false;
            item = NONE;
            std::cout << "hand" << std::endl;
        }
        if (glm::length(vecEntity.transform.position - camera.cameraPosition) < 2
            && glm::dot(camera.cameraFront, glm::normalize(vecEntity.transform.position - camera.cameraPosition)) > 0.9f
            && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            in_hand = true;
            item = VECTOR;
        }

        if (glm::length(npcOrboEntity.transform.position - camera.cameraPosition) < 2
            && glm::dot(camera.cameraFront, glm::normalize(npcOrboEntity.transform.position - camera.cameraPosition)) > 0.85f
            && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            in_hand = true;
            item = ORBO;
        }

        //hey orbo...

        if (item == ORBO) {
            npcOrboEntity.transform.position = camera.cameraPosition + camera.cameraFront - glm::vec3(0.0f, 0.6f, 0.0f);
        } else if (item == VECTOR) {
            vecEntity.transform.position = camera.cameraPosition + camera.cameraFront;
            npcOrboEntity.transform.rotation.y += glfwGetTime() * 0.001f;

        } else {
            item = NONE;
            npcOrboEntity.transform.rotation.y += deltaTime;

        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            in_hand = false;
            item = NONE;
        }

        std::cout << in_hand << std::endl;


        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
            in_hand = false;
        }
        // std::cout << "dot: -> " << glm::dot(camera.cameraFront, glm::normalize(vecEntity.transform.position - camera.cameraPosition)) << std::endl;

        vecEntity.draw(shader);
        // floorEntity.draw(shader);
        trebEntity.draw(shader);
        pcEntity.draw(shader);

        npcOrboEntity.draw(shader);
        terrainEntity.draw(shader);

        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer colour texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear colour to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        screenShader.setFloat("time", (float)glfwGetTime());
        screenShader.setFloat("distance", glm::distance(npcOrboEntity.transform.position, camera.cameraPosition));
        log(1/glm::distance(npcOrboEntity.transform.position, camera.cameraPosition));
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, texture);	// use the colour attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrame = glfwGetTime();
        deltaTime = lastFrame - currentFrame;
    }

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
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        std::cout << "printing" << std::endl;
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
void OrbitLight(glm::mat4 &light, glm::vec3 &lightPos, const glm::vec3 &pivotPos, const float radius, const int offsetMultiplier) {

    float dX = pivotPos.x + offsetMultiplier * ((sin(glfwGetTime()) * radius));
    float dY = pivotPos.y + offsetMultiplier * (sin(glfwGetTime()) * radius / 8.0f);
    float dZ = pivotPos.z + offsetMultiplier * (cos(glfwGetTime()) * radius);

    lightPos.x = dX;
    lightPos.y = dY;
    lightPos.z = dZ;

    light = glm::translate(light, lightPos);
    light = glm::scale(light, glm::vec3(0.35f));
}
void createSpotLight(Shader &shader, const int index, glm::vec3 &lightPos, glm::vec3 &lightDirection, float ambientStrength, glm::vec3 lightColour,
    float cutOffAngle, float outerCutOffAngle, float constant, float linear, float quadratic) {

    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].parentLight.position", lightPos);
    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].direction", lightDirection);
    shader.setFloat("spotlights[" + std::to_string(index) + "].parentLight.ambientStrength", ambientStrength);
    shader.uploadUniformVector3f("spotlights[" + std::to_string(index) + "].parentLight.colour", lightColour);
    shader.setFloat("spotlights[" + std::to_string(index) + "].cutOffAngle", cutOffAngle);
    shader.setFloat("spotlights[" + std::to_string(index) + "].outerCutOffAngle", outerCutOffAngle);
    shader.setFloat("spotlights[" + std::to_string(index) + "].attenuation.constant", constant);
    shader.setFloat("spotlights[" + std::to_string(index) + "].attenuation.linear", linear);
    shader.setFloat("spotlights[" + std::to_string(index) + "].attenuation.quadratic", quadratic);
}
void uploadPointLightUniforms(Shader &shader, const int index, Vector3f &lightPos, float ambientStrength, Vector3f lightColour,
    float constant, float linear, float quadratic) {

    shader.uploadUniformVector3f("pointLights[" + std::to_string(index) + "].parentLight.position", lightPos);
    shader.uploadUniformVector3f("pointLights[" + std::to_string(index) + "].parentLight.colour", lightColour);
    shader.setFloat("pointLights[" + std::to_string(index) + "].parentLight.ambientStrength", ambientStrength);
    shader.setFloat("pointLights[" + std::to_string(index) + "].attenuation.constant", constant);
    shader.setFloat("pointLights[" + std::to_string(index) + "].attenuation.linear", linear);
    shader.setFloat("pointLights[" + std::to_string(index) + "].attenuation.quadratic", quadratic);
}
void uploadDirectionLightUniforms(Shader &shader, const glm::vec3 &direction, float ambientStrength, glm::vec3 lightColour) {

    shader.uploadUniformVector3f("directionLight.direction", direction);
    shader.uploadUniformVector3f("directionLight.parentLight.colour", lightColour);
    shader.setFloat("directionLight.parentLight.ambientStrength", ambientStrength);

}
GLuint loadCubemapTextures(std::vector<std::string> faces) {

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
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