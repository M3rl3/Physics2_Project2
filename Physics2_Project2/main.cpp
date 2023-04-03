#include "OpenGL.h"
#include "cMeshInfo.h"
#include "sCamera.h"
#include "DrawMesh.h"
#include "DrawBoundingBox.h"

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iPhysicsFactory.h>
#include <PhysicsFactory.h>

#include <iPhysicsWorld.h>
#include <iShape.h>
#include <SphereShape.h>
#include <BoxShape.h>
#include <PlaneShape.h>
#include <CylinderShape.h>

#include "PlyFileLoader/PlyFileLoader.h"
#include "cShaderManager/cShaderManager.h"
#include "cVAOManager/cVAOManager.h"
#include "cLightManager/cLightManager.h"
#include "cBasicTextureManager/cBasicTextureManager.h"
#include "cFBO/cFBO.h"
#include "cMazeMaker_W2023/cMazeMaker_W2023.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <chrono>

#include <stdlib.h>
#include <stdio.h>

GLFWwindow* window;
GLint mvp_location = 0;
GLuint shaderID = 0;

PlyFileLoader* plyLoader;
cVAOManager* VAOMan;
cBasicTextureManager* TextureMan;
cLightManager* LightMan;
cFBO* FrameBuffer;
cMazeMaker_W2023* mazeMaker;

physics::iPhysicsWorld* physicsWorld;
physics::iPhysicsFactory* physicsFactory;

sModelDrawInfo player_obj;

cMeshInfo* full_screen_quad;
cMeshInfo* skybox_sphere_mesh;
cMeshInfo* player_mesh;
cMeshInfo* cube;

cMeshInfo* bulb_mesh;
cLight* pointLight;

unsigned int readIndex = 0;
int object_index = 0;
int throwableIndex = 0;
int copyIndex = throwableIndex;
int elapsed_frames = 0;

bool enableMouse = true;
bool useFBO = false;
bool mouseClick = false;

std::vector <std::string> meshFiles;
std::vector <cMeshInfo*> meshArray;
std::vector <cMeshInfo*> throwables;
std::vector <cMeshInfo*> cylinders;
std::vector <cMeshInfo*> temp;

void ReadFromFile();
void LoadTextures();
void ManageLights();
float RandomFloat(float a, float b);
bool RandomizePositions(cMeshInfo* mesh);
void LoadPlyFilesIntoVAO(void);
void RenderToFBO(GLFWwindow* window, sCamera* camera,
    GLuint eyeLocationLocation, GLuint viewLocation, GLuint projectionLocation,
    GLuint modelLocaction, GLuint modelInverseLocation);

void CreateLightBulb();
void CreateFlatPlane(float size, glm::vec4 color);
void CreatePlayerBall(glm::vec3 position, float size);
void CreateMoon();
void CreateSkyBoxSphere();
void LoadTextures();
void CreateBall(std::string modelName, glm::vec3 position, glm::vec4 color, float mass);
void CreateWall(std::string modelName, glm::vec3 position, glm::vec3 rotation, glm::vec3 normal, glm::vec3 size, glm::vec4 color, float mass);
void CreateCube(std::string modelName, glm::vec3 position, glm::vec4 color, float mass);
void CreateCylinder(std::string modelName, glm::vec3 position, glm::vec4 color, float mass);
void CreateCylinders();
void CreateStaticBoxes();
void CreateWalls();

void HardReset();
void Reset();

const glm::vec3 origin = glm::vec3(0);
const glm::mat4 matIdentity = glm::mat4(1.0f);
const glm::vec3 upVector = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 roundingError = glm::vec3(10.f, 0.f, 0.f);

bool pressed = false;

glm::vec3 direction = glm::vec3(0.f);

// attenuation on all lights
glm::vec4 constLightAtten = glm::vec4(1.0f);

enum eEditMode
{
    MOVING_CAMERA,
    MOVING_LIGHT,
    MOVING_SELECTED_OBJECT,
    TAKE_CONTROL
};

// glm::vec3 cameraEye = glm::vec3(-280.0, 140.0, -700.0);
// glm::vec3 cameraTarget = glm::vec3(0.f, 0.f, 0.f);

sCamera* camera;

glm::vec3 initCamera;

eEditMode theEditMode = TAKE_CONTROL;

float yaw = 0.f;
float pitch = 0.f;
float fov = 45.f;

// mouse state
bool firstMouse = true;
float lastX = 800.f / 2.f;
float lastY = 600.f / 2.f;

float beginTime = 0.f;
float currentTime = 0.f;
float timeDiff = 0.f;
int frameCount = 0;

// Pre-existing light, independent of the scene lighting
float ambientLight = 1.f;

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // Create new spheres
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        CreateBall("ball", 
            glm::vec3(RandomFloat(-100, 100), RandomFloat(20, 50), RandomFloat(-100, 100)),   // position
            glm::vec4(RandomFloat(0, 1), RandomFloat(0, 1), RandomFloat(0, 1), 1),            // color
            RandomFloat(1, 3));                                                               // size

        pressed = true;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
        pressed = false;
    }

    // Reset the scene
    if (key == GLFW_KEY_DELETE && action == GLFW_PRESS) {
        Reset();
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        HardReset();
    }

    // Change the sphere under the user's control
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS) {
        if (throwables.size() > 0) {
            if (throwableIndex != throwables.size() - 1) {
                throwableIndex++;
            }
        }
    }
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS) {
        if (throwables.size() > 0) {
            if (throwableIndex != 0) {
                throwableIndex--;
            }
        }
    }

    // enable/disable mouse look
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        enableMouse = !enableMouse;
    }

    // Camera input
    if (key == GLFW_KEY_LEFT)     // Left
    {
        camera->StrafeLeft();
    }
    if (key == GLFW_KEY_RIGHT)     // Right
    {
        camera->StrafeRight();
    }
    if (key == GLFW_KEY_UP)     // Forward
    {
        camera->MoveForward();
    }
    if (key == GLFW_KEY_DOWN)     // Backwards
    {
        camera->MoveBackward();
    }
    if (key == GLFW_KEY_PAGE_UP)     // Up
    {
        camera->MoveUp();
    }
    if (key == GLFW_KEY_PAGE_DOWN)     // Down
    {
        camera->MoveDown();
    }

    // user input to determine the direction of the force applied
    // Forward = Where the camera is facing
    // Back = -Forward
    // Left/Right = Cross product of forward and up
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        direction = -camera->forward;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        direction = glm::vec3(0.f);
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        direction = camera->forward;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        direction = glm::vec3(0.f);
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        glm::vec3 crossProduct = glm::cross(camera->forward, camera->up);
        direction = crossProduct;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        direction = glm::vec3(0.f);
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        glm::vec3 crossProduct = glm::cross(camera->forward, camera->up);
        direction = -crossProduct;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        direction = glm::vec3(0.f);
    }
}

static void MouseCallBack(GLFWwindow* window, double xposition, double yposition) {

    if (firstMouse) {
        lastX = xposition;
        lastY = yposition;
        firstMouse = false;
    }

    float xoffset = xposition - lastX;
    float yoffset = lastY - yposition;  // reversed since y coordinates go from bottom to up
    lastX = xposition;
    lastY = yposition;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // prevent perspective from getting flipped by capping it
    if (pitch > 89.f) {
        pitch = 89.f;
    }
    if (pitch < -89.f) {
        pitch = -89.f;
    }

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    if (enableMouse) {
        camera->target = glm::normalize(front);
    }
}

static void ScrollCallBack(GLFWwindow* window, double xoffset, double yoffset) {
    if (fov >= 1.f && fov <= 45.f) {
        fov -= yoffset;
    }
    if (fov <= 1.f) {
        fov = 1.f;
    }
    if (fov >= 45.f) {
        fov = 45.f;
    }
}

void Initialize() {

    if (!glfwInit()) {
        std::cerr << "GLFW init failed." << std::endl;
        glfwTerminate();
        return;
    }

    const char* glsl_version = "#version 420";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* currentMonitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(currentMonitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // windowed mode
    window = glfwCreateWindow(1366, 768, "Man", NULL, NULL);

    // fullscreen support based on current monitor
    // window = glfwCreateWindow(mode->width, mode->height, "Man", currentMonitor, NULL);
    
    if (!window) {
        std::cerr << "Window creation failed." << std::endl;
        glfwTerminate();
        return;
    }

    glfwSetWindowAspectRatio(window, 16, 9);

    // keyboard callback
    glfwSetKeyCallback(window, KeyCallback);

    // mouse and scroll callback
    glfwSetCursorPosCallback(window, MouseCallBack);
    glfwSetScrollCallback(window, ScrollCallBack);

    // capture mouse input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetErrorCallback(ErrorCallback);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress))) {
        std::cerr << "Error: unable to obtain pocess address." << std::endl;
        return;
    }
    glfwSwapInterval(1); //vsync

    // Init camera object
    camera = new sCamera();

    // Init camera values
    camera->position = glm::vec3(160, 60, -160);
    camera->target = glm::vec3(-160.f, 0.f, 160.f);
}

void Render() {

    // Had to use the class type here
    // Init physics factory
    physicsFactory = new physics::PhysicsFactory();

    // Init physics world
    physicsWorld = physicsFactory->CreateWorld();

    // Set Gravity
    physicsWorld->SetGravity(glm::vec3(0.f, -9.8f, 0.f));

    // FBO
    std::cout << "\nCreating FBO..." << std::endl;
    FrameBuffer = new cFBO();
    useFBO = true;

    int screenWidth = 0;
    int screenHeight = 0;

    // Get window size
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    // Create the frame buffer based on window size
    std::string fboErrorString = "";
    if (!FrameBuffer->init(screenWidth, screenHeight, fboErrorString)) {
        std::cout << "FBO creation error: " << fboErrorString << std::endl;
    }
    else {
        std::cout << "FBO created." << std::endl;
    }

    // Shader Manager
    std::cout << "\nCompiling shaders..." << std::endl;
    cShaderManager* shadyMan = new cShaderManager();

    cShader vertexShader;
    cShader geometryShader;
    cShader fragmentShader;

    vertexShader.fileName = "../assets/shaders/vertexShader.glsl";
    geometryShader.fileName = "../assets/shaders/geometryShader_passthrough.glsl";
    fragmentShader.fileName = "../assets/shaders/fragmentShader.glsl";

    if (!shadyMan->createProgramFromFile("ShadyProgram", vertexShader, geometryShader, fragmentShader)) {
        std::cout << "Error: Shader program failed to compile." << std::endl;
        std::cout << shadyMan->getLastError();
        return;
    }
    else {
        std::cout << "Shaders compiled." << std::endl;
    }

    initCamera = camera->position;

    shaderID = shadyMan->getIDFromFriendlyName("ShadyProgram");
    glUseProgram(shaderID);

    // Load asset paths from external file
    ReadFromFile();

    // Load the ply model
    plyLoader = new PlyFileLoader();

    // VAO Manager
    VAOMan = new cVAOManager();

    // Scene
    std::cout << "\nLoading assets..." << std::endl;

    // Load all ply files
    LoadPlyFilesIntoVAO();

    // Lights
    LightMan = new cLightManager();

    // Pre-existing light, independent of the scene lighting
    ambientLight = 0.5f;
    LightMan->SetAmbientLightAmount(ambientLight);

    // Attenuation on all lights in the scene
    constLightAtten = glm::vec4(0.1f, 2.5e-5f, 2.5e-5f, 1.0f);

    // The actual light
    pointLight = LightMan->AddLight(glm::vec4(0.f, 0.f, 0.f, 1.f));
    pointLight->diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);
    pointLight->specular = glm::vec4(1.f, 1.f, 1.f, 1.f);
    pointLight->direction = glm::vec4(1.f, 1.f, 1.f, 1.f);
    pointLight->atten = constLightAtten;
    pointLight->param1 = glm::vec4(0.f, 50.f, 50.f, 1.f);
    pointLight->param2 = glm::vec4(1.f, 0.f, 0.f, 1.f);

    // The model to be drawn where the light exists
    bulb_mesh = new cMeshInfo();
    bulb_mesh->meshName = "bulb";
    bulb_mesh->position = glm::vec3(0.f, 125.f, 0.f);
    bulb_mesh->friendlyName = "bulb";
    meshArray.push_back(bulb_mesh);

    // The quad that the scene is drawn over
    full_screen_quad = new cMeshInfo();
    full_screen_quad->meshName = "fullScreenQuad";
    full_screen_quad->friendlyName = "fullScreenQuad";
    full_screen_quad->doNotLight = false;

    skybox_sphere_mesh = new cMeshInfo();
    skybox_sphere_mesh->meshName = "skybox_sphere";
    skybox_sphere_mesh->friendlyName = "skybox_sphere";
    skybox_sphere_mesh->isSkyBoxMesh = true;

    glm::vec4 terrainColor = glm::vec4(0.25f, 0.25f, 0.25f, 1.f);

    // Scene
    CreateMoon();

    // The floor
    CreateFlatPlane(1.f, terrainColor);

    // Bowling pin cylinders
    CreateCylinders();

    // Stacked boxes
    CreateStaticBoxes();

    // Arena walls
    CreateWalls();
    
    // all textures loaded here
    LoadTextures();
}

void Update() {

    // Cull back facing triangles
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Depth test
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // MVP
    glm::mat4x4 model;

    GLint modelLocaction = glGetUniformLocation(shaderID, "Model");
    GLint viewLocation = glGetUniformLocation(shaderID, "View");
    GLint projectionLocation = glGetUniformLocation(shaderID, "Projection");
    GLint modelInverseLocation = glGetUniformLocation(shaderID, "ModelInverse");
    
    // Lighting
    // ManageLights();
    LightMan->LoadLightUniformLocations(shaderID);
    LightMan->CopyLightInformationToShader(shaderID);
    
    pointLight->position = glm::vec4(bulb_mesh->position, 1.f);
    pointLight->atten = constLightAtten;

    float ratio;
    int width, height;

    // Bind the Frame Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer->ID);

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float)height;
    glViewport(0, 0, width, height);

    FrameBuffer->clearBuffers(true, true);

    // mouse support
    if (enableMouse) {
        camera->view = glm::lookAt(camera->position, camera->position + camera->target, upVector);
        camera->projection = glm::perspective(glm::radians(fov), ratio, 0.1f, 10000.f);
    }
    else {
        camera->view = glm::lookAt(camera->position, camera->target, upVector);
        camera->projection = glm::perspective(0.6f, ratio, 0.1f, 10000.f);
    }

    // Calculate the forward direction to determine direction of the force
    // The forward direction is stored in the third column of the matrix
    camera->forward = glm::normalize(glm::vec3(camera->view[0][2], 
                                               camera->view[1][2],
                                               camera->view[2][2]));

    GLint eyeLocationLocation = glGetUniformLocation(shaderID, "eyeLocation");
    glUniform4f(eyeLocationLocation, camera->position.x, camera->position.y, camera->position.z, 1.f);

    model = glm::mat4(1.f);

    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera->view));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera->projection));

    // non-physics objects
    for (int i = 0; i < meshArray.size(); i++) {

        cMeshInfo* currentMesh = meshArray[i];

        // Draw all the meshes pushed onto the vector
        DrawMesh(currentMesh,      // theMesh
            model,                 // Model Matrix
            shaderID,              // Compiled Shader ID
            TextureMan,            // Instance of the Texture Manager
            VAOMan,                // Instance of the VAO Manager
            camera,                // Instance of the struct Camera
            modelLocaction,        // UL for model matrix
            modelInverseLocation); // UL for transpose of model matrix

        // Physics update step
        physicsWorld->TimeStep(0.06f);
    }

    // Check if there are any spheres to be drawn at all
    if (throwables.size() != 0) {
        for (int i = 0; i < throwables.size(); i++) {

            // All the spheres that were instantiated
            cMeshInfo* currentMesh = throwables[i];

            // The sphere currently under the user's control
            cMeshInfo* controllableBall = throwables[throwableIndex];

            // HACK
            // Make a copy of the previous color
            // So we can change back to the original color
            if (currentMesh->doOnce) {
                currentMesh->colour = currentMesh->RGBAColour;
                currentMesh->doOnce = false;
            }

            // HACK
            // Have a force applied to the sphere that was last created
            // Otherwise the sphere does not react to any forces applied henceforth
            // For some reason ¯\_(ツ)_/¯
            if (pressed) {
                int currentBallIndex = throwables.size() - 1;
                physics::iRigidBody* rigidBody = dynamic_cast<physics::iRigidBody*>(throwables[currentBallIndex]->collisionBody);

                // Have it go in a random direction
                int random = RandomFloat(0, 3);

                if (random == 0) {
                    roundingError = glm::vec3(10, 0, 0);
                }
                if (random == 1) {
                    roundingError = glm::vec3(-10, 0, 0);
                }
                if (random == 2) {
                    roundingError = glm::vec3(0, 0, 10);
                }
                if (random == 3) {
                    roundingError = glm::vec3(0, 0, -10);
                }

                // Sort of a rounding error
                rigidBody->ApplyForce(roundingError);
            }

            // Convert the sphere's colliding body to a rigid body
            physics::iRigidBody* rigidBody = dynamic_cast<physics::iRigidBody*>(throwables[throwableIndex]->collisionBody);

            // Check if the rigid body actually exists
            if (rigidBody != nullptr) {

                // Multilpier
                float speed = 25.f;

                // Don't need any forces applied upwards/downwards
                direction.y = 0;

                // Set the forces from user inputs
                rigidBody->ApplyForce(direction * speed);
                rigidBody->ApplyTorque(direction * speed);
                
                // Set the color of the specific sphere instance to bright yellow
                // To indicate that its under the user's control
                controllableBall->RGBAColour = glm::vec4(10, 10, 0, 1);
            }

            // Check if the sphere the user was controlling has changed or no
            if (throwableIndex != copyIndex) {

                // Give the original color back to it
                if ((throwableIndex - 1) >= 0) {
                    throwables[throwableIndex - 1]->RGBAColour = throwables[throwableIndex - 1]->colour;
                }

                if ((throwableIndex + 1) <= (throwables.size() - 1)) {
                    throwables[throwableIndex + 1]->RGBAColour = throwables[throwableIndex + 1]->colour;
                }

                // update the copy
                copyIndex = throwableIndex;
            }

            // HACK
            // If the sphere is scaled by 2 or more units
            // Move the model matrix silghtly up to prevent
            // the model from clipping under the floor
            if (currentMesh->scale.x > 2) {
                model += glm::translate(glm::mat4x4(1.f), glm::vec3(0.f, 1.5f, 0.f));
            }
            if (currentMesh->scale.x > 1) {
                model += glm::translate(glm::mat4x4(1.f), glm::vec3(0.f, 0.5f, 0.f));
            }

            // Draw all the meshes pushed onto the vector
            DrawMesh(currentMesh,      // theMesh
                model,                 // Model Matrix
                shaderID,              // Compiled Shader ID
                TextureMan,            // Instance of the Texture Manager
                VAOMan,                // Instance of the VAO Manager
                camera,                // Instance of the struct Camera
                modelLocaction,        // UL for model matrix
                modelInverseLocation); // UL for transpose of model matrix
        }
    }

    // Draw the cylinders
    if (cylinders.size() != 0) {
        for (int i = 0; i < cylinders.size(); i++) {

            cMeshInfo* currentMesh = cylinders[i];

            // Draw all the meshes pushed onto the vector
            DrawMesh(currentMesh,      // theMesh
                model,                 // Model Matrix
                shaderID,              // Compiled Shader ID
                TextureMan,            // Instance of the Texture Manager
                VAOMan,                // Instance of the VAO Manager
                camera,                // Instance of the struct Camera
                modelLocaction,        // UL for model matrix
                modelInverseLocation); // UL for transpose of model matrix
        }
    }

    // Draw the skybox
    DrawMesh(skybox_sphere_mesh, matIdentity, shaderID, 
        TextureMan, VAOMan, camera, modelLocaction, modelInverseLocation);

    // Redirect output to an offscreen quad
    if (useFBO)
    {
        RenderToFBO(window, camera,
            eyeLocationLocation, viewLocation, projectionLocation,
            modelLocaction, modelInverseLocation);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    currentTime = glfwGetTime();
    timeDiff = currentTime - beginTime;
    frameCount++;

    // Set the window title to display relevent information
    if (timeDiff >= 1.f / 30.f) {
        std::string frameRate = std::to_string((1.f / timeDiff) * frameCount);
        std::string frameTime = std::to_string((timeDiff / frameCount) * 1000);

        std::stringstream ss;
        
        ss << " Camera: " << "(" << camera->position.x << ", " << camera->position.y << ", " << camera->position.z << ")"
            << "    Current Sphere: " << throwableIndex
            << "    Total Spheres: " << throwables.size();

        glfwSetWindowTitle(window, ss.str().c_str());

        beginTime = currentTime;
        frameCount = 0;
    }
}

// Gracefully close everything down
void Shutdown() {

    glfwDestroyWindow(window);
    glfwTerminate();

    window = nullptr;
    delete window;

    exit(EXIT_SUCCESS);
}

void ReadFromFile() {

    std::ifstream readFile("readFile.txt");
    std::string input0;

    while (readFile >> input0) {
        meshFiles.push_back(input0);
        readIndex++;
    }  
}

// All lights managed here
void ManageLights() {
    
    GLint PositionLocation = glGetUniformLocation(shaderID, "sLightsArray[0].position");
    GLint DiffuseLocation = glGetUniformLocation(shaderID, "sLightsArray[0].diffuse");
    GLint SpecularLocation = glGetUniformLocation(shaderID, "sLightsArray[0].specular");
    GLint AttenLocation = glGetUniformLocation(shaderID, "sLightsArray[0].atten");
    GLint DirectionLocation = glGetUniformLocation(shaderID, "sLightsArray[0].direction");
    GLint Param1Location = glGetUniformLocation(shaderID, "sLightsArray[0].param1");
    GLint Param2Location = glGetUniformLocation(shaderID, "sLightsArray[0].param2");

    //glm::vec3 lightPosition0 = meshArray[1]->position;
    glm::vec3 lightPosition0 = meshArray[0]->position;
    glUniform4f(PositionLocation, lightPosition0.x, lightPosition0.y, lightPosition0.z, 1.0f);
    //glUniform4f(PositionLocation, 0.f, 0.f, 0.f, 1.0f);
    glUniform4f(DiffuseLocation, 1.f, 1.f, 1.f, 1.f);
    glUniform4f(SpecularLocation, 1.f, 1.f, 1.f, 1.f);
    glUniform4f(AttenLocation, 0.1f, 0.5f, 0.1f, 1.f);
    glUniform4f(DirectionLocation, 1.f, 1.f, 1.f, 1.f);
    glUniform4f(Param1Location, 0.f, 0.f, 0.f, 1.f); //x = Light Type
    glUniform4f(Param2Location, 1.f, 0.f, 0.f, 1.f); //x = Light on/off
}

void LoadTextures() {

    std::cout << "\nLoading textures...";

    std::string errorString = "";
    TextureMan = new cBasicTextureManager();

    TextureMan->SetBasePath("../assets/textures");

    // skybox/cubemap textures
    std::string skyboxName = "NightSky";
    if (TextureMan->CreateCubeTextureFromBMPFiles(skyboxName,
        "SpaceBox_right1_posX.bmp",
        "SpaceBox_left2_negX.bmp",
        "SpaceBox_top3_posY.bmp",
        "SpaceBox_bottom4_negY.bmp",
        "SpaceBox_front5_posZ.bmp",
        "SpaceBox_back6_negZ.bmp",
        true, errorString))
    {
        std::cout << "\nLoaded skybox textures: " << skyboxName << std::endl;
    }
    else
    {
        std::cout << "\nError: failed to load skybox because " << errorString;
    }

    // Basic texture2D
    if (TextureMan->Create2DTextureFromBMPFile("moon_texture.bmp"))
    {
        std::cout << "Loaded moon texture." << std::endl;
    }
    else
    {
        std::cout << "Error: failed to load moon texture.";
    }

    // Crosshair texture
    if (TextureMan->Create2DTextureFromBMPFile("crosshair.bmp"))
    {
        std::cout << "Loaded crosshair texture." << std::endl;
    }
    else
    {
        std::cout << "Error: failed to load crosshair texture.";
    }
}

float RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

// Render to a Frame Buffer Object
void RenderToFBO(GLFWwindow* window, sCamera* camera,
                 GLuint eyeLocationLocation, GLuint viewLocation, GLuint projectionLocation,
                 GLuint modelLocaction, GLuint modelInverseLocation)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 exCameraEye = camera->position;
    glm::vec3 exCameraLookAt = camera->target;

    camera->position = glm::vec3(0.0f, 0.0f, -6.0f);
    camera->target = glm::vec3(0.0f, 0.0f, 0.0f);

    float ratio;
    int width, height;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float)height;

    GLint FBOsizeLocation = glGetUniformLocation(shaderID, "FBO_size");
    GLint screenSizeLocation = glGetUniformLocation(shaderID, "screen_size");

    glUniform2f(FBOsizeLocation, (GLfloat)FrameBuffer->width, (GLfloat)FrameBuffer->height);
    glUniform2f(screenSizeLocation, (GLfloat)width, (GLfloat)height);

    camera->projection = glm::perspective(0.6f, ratio, 0.1f, 100.f);

    glViewport(0, 0, width, height);

    camera->view = glm::lookAt(camera->position, camera->target, upVector);

    // Set eyelocation again
    glUniform4f(eyeLocationLocation, camera->position.x, camera->position.y, camera->position.z, 1.f);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera->view));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera->projection));

    // Drawing
    GLint bIsFullScreenQuadLocation = glGetUniformLocation(shaderID, "bIsFullScreenQuad");
    glUniform1f(bIsFullScreenQuadLocation, (GLfloat)GL_TRUE);

    // Texture on the quad
    GLuint texture21Unit = 21;			// Picked 21 because it's not being used
    glActiveTexture(texture21Unit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984
    glBindTexture(GL_TEXTURE_2D, FrameBuffer->colourTexture_0_ID);

    GLint FBO_TextureLocation = glGetUniformLocation(shaderID, "FBO_Texture");
    glUniform1i(FBO_TextureLocation, texture21Unit);

    // Crosshair Texture
    GLuint crosshairTextureID = TextureMan->getTextureIDFromName("crosshair.bmp");

    GLuint texture15Unit = 15;
    glActiveTexture(texture15Unit + GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, crosshairTextureID);

    GLint crosshairTextureLocation = glGetUniformLocation(shaderID, "crosshair_texture");
    glUniform1i(crosshairTextureLocation, texture15Unit);

    full_screen_quad->SetUniformScale(10.f);
    full_screen_quad->isVisible = true;

    // Draw the quad
    DrawMesh(full_screen_quad,
        matIdentity,
        shaderID,
        TextureMan,
        VAOMan,
        camera,
        modelLocaction,
        modelInverseLocation);

    camera->position = exCameraEye;
    camera->target = exCameraLookAt;

    glUniform1f(bIsFullScreenQuadLocation, (GLfloat)GL_FALSE);
}

bool RandomizePositions(cMeshInfo* mesh) {

    int i = 0;
    float x, y, z, w;

    x = RandomFloat(-500, 500);
    y = mesh->position.y;
    z = RandomFloat(-200, 200);

    mesh->position = glm::vec3(x, y, z);
    
    return true;
}

// Models loaded here
void LoadPlyFilesIntoVAO(void)
{
    sModelDrawInfo bulb;
    plyLoader->LoadModel(meshFiles[0], bulb);
    if (!VAOMan->LoadModelIntoVAO("bulb", bulb, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }

    sModelDrawInfo flat_plain_obj;
    plyLoader->LoadModel(meshFiles[9], flat_plain_obj);
    if (!VAOMan->LoadModelIntoVAO("flat_plain", flat_plain_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }

    plyLoader->LoadModel(meshFiles[7], player_obj);
    if (!VAOMan->LoadModelIntoVAO("player", player_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }

    sModelDrawInfo moon_obj;
    plyLoader->LoadModel(meshFiles[5], moon_obj);
    if (!VAOMan->LoadModelIntoVAO("moon", moon_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }

    sModelDrawInfo wall_cube;
    plyLoader->LoadModel(meshFiles[2], wall_cube);
    if (!VAOMan->LoadModelIntoVAO("wall_cube", wall_cube, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    } 
    
    sModelDrawInfo cylinder;
    plyLoader->LoadModel(meshFiles[11], cylinder);
    if (!VAOMan->LoadModelIntoVAO("cylinder", cylinder, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }

    // 2-sided full screen quad aligned to x-y axis
    sModelDrawInfo fullScreenQuad;
    plyLoader->LoadModel(meshFiles[10], fullScreenQuad);
    if (!VAOMan->LoadModelIntoVAO("fullScreenQuad", fullScreenQuad, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }

    // skybox sphere with inverted normals
    sModelDrawInfo skybox_sphere_obj;
    plyLoader->LoadModel(meshFiles[6], skybox_sphere_obj);
    if (!VAOMan->LoadModelIntoVAO("skybox_sphere", skybox_sphere_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }
}

// The floor of the scene
void CreateFlatPlane(float size, glm::vec4 color) {

    cMeshInfo* plain_mesh = new cMeshInfo();

    plain_mesh->meshName = "flat_plain";
    plain_mesh->friendlyName = "flat_plain";

    plain_mesh->position = glm::vec3(0.f);
    plain_mesh->scale = glm::vec3(size);
    plain_mesh->rotation = glm::quat(glm::vec3(0.f, 0.f, 0.f));

    plain_mesh->useRGBAColour = true;
    plain_mesh->RGBAColour = color;
    plain_mesh->useRGBAColour = true;
    plain_mesh->hasTexture = false;

    physics::iShape* planeShape = new physics::PlaneShape(1.0f, glm::vec3(0.f, 1.f, 0.f));
    physics::RigidBodyDesc description;
    description.isStatic = true;
    description.mass = 0.f;
    description.position = plain_mesh->position;
    description.rotation = plain_mesh->rotation;
    description.linearVelocity = glm::vec3(0.f);

    plain_mesh->collisionBody = physicsFactory->CreateRigidBody(description, planeShape);
    physicsWorld->AddBody(plain_mesh->collisionBody);

    meshArray.push_back(plain_mesh);
}

// Create a player object
void CreatePlayerBall(glm::vec3 position, float size) {

    player_mesh = new cMeshInfo();
    player_mesh->meshName = "player";
    player_mesh->friendlyName = "player";

    player_mesh->position = position;
    player_mesh->SetUniformScale(size);

    player_mesh->hasTexture = true;
    player_mesh->RGBAColour = glm::vec4(10.f, 1.f, 1.f, 1.f);
    player_mesh->useRGBAColour = false;
    player_mesh->drawBBox = false;
    player_mesh->textures[0] = "basketball_sph.bmp";
    player_mesh->textureRatios[0] = 1.f;
    player_mesh->CopyVertices(player_obj);

    physics::iShape* ballShape = new physics::SphereShape(1.0f);
    physics::RigidBodyDesc description;
    description.isStatic = false;
    description.mass = size;
    description.position = player_mesh->position;
    description.linearVelocity = glm::vec3(0.f);

    player_mesh->collisionBody = physicsFactory->CreateRigidBody(description, ballShape);
    physicsWorld->AddBody(player_mesh->collisionBody);

    meshArray.push_back(player_mesh);
}

// Instantiate the spheres on user input
void CreateBall(std::string modelName, glm::vec3 position, glm::vec4 color, float mass) {

    cMeshInfo* ball = new cMeshInfo();
    ball->meshName = "player";
    ball->friendlyName = modelName;

    ball->position = position;
    ball->SetUniformScale(mass);
    ball->RGBAColour = color;
    ball->useRGBAColour = true;

    physics::iShape* ballShape = new physics::SphereShape(1.0f);
    physics::RigidBodyDesc description;
    description.isStatic = false;
    description.mass = mass * 0.5f;
    description.position = position;
    description.linearVelocity = glm::vec3(0.f);

    ball->collisionBody = physicsFactory->CreateRigidBody(description, ballShape);
    physicsWorld->AddBody(ball->collisionBody);

    throwables.push_back(ball);
}

// Individual cylinder parameters
void CreateCylinder(std::string modelName, glm::vec3 position, glm::vec4 color, float mass) {

    cMeshInfo* cylinder = new cMeshInfo();
    cylinder->meshName = "cylinder";
    cylinder->friendlyName = modelName;

    cylinder->position = position;
    cylinder->SetUniformScale(mass);
    cylinder->RGBAColour = color;
    cylinder->useRGBAColour = true;

    physics::iShape* cylinderShape = new physics::CylinderShape(glm::vec3(1.f));
    physics::RigidBodyDesc description;
    description.isStatic = false;
    description.mass = mass * 0.5f;
    description.position = position;
    description.linearVelocity = glm::vec3(0.f);

    cylinder->collisionBody = physicsFactory->CreateRigidBody(description, cylinderShape);
    physicsWorld->AddBody(cylinder->collisionBody);

    cylinders.push_back(cylinder);
}

// All the cylinders are loaded here
void CreateCylinders() {

    float cylinderSize = 1.f;

    CreateCylinder("cylinder0", glm::vec3(10, 2, 0), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder1", glm::vec3(15, 2, 0), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder2", glm::vec3(20, 2, 0), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder3", glm::vec3(25, 2, 0), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder4", glm::vec3(12.5, 2, 5), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder5", glm::vec3(17.5, 2, 5), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder6", glm::vec3(22.5, 2, 5), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder7", glm::vec3(15, 2, 10), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder8", glm::vec3(20, 2, 10), glm::vec4(1), cylinderSize);
    CreateCylinder("cylinder9", glm::vec3(17.5, 2, 15), glm::vec4(1), cylinderSize);
}

void CreateStaticBoxes() {
    float cubeSize = 2.f;

    CreateCube("cube0", glm::vec3(50, 2, 0), glm::vec4(1), cubeSize);
    CreateCube("cube1", glm::vec3(50, 8, 0), glm::vec4(1), cubeSize);
    CreateCube("cube2", glm::vec3(50, 13, 0), glm::vec4(1), cubeSize);

    CreateCube("cube3", glm::vec3(-50, 2, 0), glm::vec4(1), cubeSize);
    CreateCube("cube4", glm::vec3(-50, 8, 0), glm::vec4(1), cubeSize);
    CreateCube("cube5", glm::vec3(-50, 13, 0), glm::vec4(1), cubeSize);
    
    CreateCube("cube6", glm::vec3(50, 2, -50), glm::vec4(1), cubeSize);
    CreateCube("cube7", glm::vec3(50, 8, -50), glm::vec4(1), cubeSize);
    CreateCube("cube8", glm::vec3(50, 13, -50), glm::vec4(1), cubeSize);
    
    CreateCube("cube9", glm::vec3(-50, 2, 50), glm::vec4(1), cubeSize);
    CreateCube("cube10", glm::vec3(-50, 8, 50), glm::vec4(1), cubeSize);
    CreateCube("cube11", glm::vec3(-50, 13, 50), glm::vec4(1), cubeSize);
}

void CreateWalls() {
    // arena walls created here
    CreateWall("wall0", glm::vec3(0, 6, 100), glm::vec3(67.55f, 0.f, 0.f), glm::vec3(0, 0, 0), glm::vec3(1, 1, 0.06), glm::vec4(0.25f, 0.25f, 0.25f, 1.f), 1.f);
    CreateWall("wall1", glm::vec3(0, 6, -100), glm::vec3(-67.55f, 0.f, 0.f), glm::vec3(0, 0, 0), glm::vec3(1, 1, 0.06), glm::vec4(0.25f, 0.25f, 0.25f, 1.f), 1.f);
    CreateWall("wall2", glm::vec3(100, 6, 0), glm::vec3(0.f, 0.f, -67.55f), glm::vec3(0, 0, 0), glm::vec3(0.06, 1, 1), glm::vec4(0.25f, 0.25f, 0.25f, 1.f), 1.f);
    CreateWall("wall3", glm::vec3(-100, 6, 0), glm::vec3(0.f, 0.f, 67.55f), glm::vec3(0, 0, 0), glm::vec3(0.06, 1, 1), glm::vec4(0.25f, 0.25f, 0.25f, 1.f), 1.f);
}

void CreateCube(std::string modelName, glm::vec3 position, glm::vec4 color, float mass) {

    cube = new cMeshInfo();
    cube->meshName = "wall_cube";
    cube->friendlyName = modelName;

    cube->position = position;
    cube->SetUniformScale(mass);
    cube->RGBAColour = color;
    cube->useRGBAColour = true;

    physics::iShape* cubeShape = new physics::BoxShape(glm::vec3(1.f));
    physics::RigidBodyDesc description;
    description.isStatic = false;
    description.mass = mass * 0.5f;
    description.position = position;
    description.linearVelocity = glm::vec3(0.f);

    cube->collisionBody = physicsFactory->CreateRigidBody(description, cubeShape);
    physicsWorld->AddBody(cube->collisionBody);

    cylinders.push_back(cube);
}

void CreateWall(std::string modelName, glm::vec3 position, glm::vec3 rotation, glm::vec3 normal, glm::vec3 size, glm::vec4 color, float mass) {

    cMeshInfo* wall = new cMeshInfo();
    wall->meshName = "flat_plain";
    wall->friendlyName = modelName;

    wall->position = position;
    wall->AdjustRoationAngleFromEuler(rotation);
    wall->scale = size;
    wall->RGBAColour = color;
    wall->useRGBAColour = true;
    wall->doNotLight = false;

    /*physics::iShape* planeShape = new physics::PlaneShape(0.0f, normal);
    physics::RigidBodyDesc description;
    description.isStatic = true;
    description.mass = 0;
    description.position = position;
    description.rotation = wall->rotation;
    description.linearVelocity = glm::vec3(0.f);

    wall->collisionBody = physicsFactory->CreateRigidBody(description, planeShape);
    physicsWorld->AddBody(wall->collisionBody);*/

    meshArray.push_back(wall);
}


void CreateMoon() {

    cMeshInfo* moon_mesh = new cMeshInfo();
    moon_mesh->meshName = "moon";
    moon_mesh->friendlyName = "moon";

    moon_mesh->position = glm::vec3(-180.f, 370.f, 400.f);
    moon_mesh->AdjustRoationAngleFromEuler(glm::vec3(0.f));
    moon_mesh->SetUniformScale(20.f);

    moon_mesh->useRGBAColour = false;
    moon_mesh->hasTexture = true;
    moon_mesh->textures[0] = "moon_texture.bmp";
    moon_mesh->textureRatios[0] = 1.0f;
    moon_mesh->doNotLight = true;
    meshArray.push_back(moon_mesh);
}

void CreateLightBulb() {

    bulb_mesh = new cMeshInfo();
    bulb_mesh->meshName = "bulb";
    bulb_mesh->friendlyName = "bulb";
    bulb_mesh->position = glm::vec3(-220.f, 30.f, 0.f);
    bulb_mesh->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1, 0, 0));
    bulb_mesh->SetUniformScale(0.1f);
    meshArray.push_back(bulb_mesh);
}

void CreateSkyBoxSphere() {

    skybox_sphere_mesh = new cMeshInfo();
    skybox_sphere_mesh->meshName = "skybox_sphere";
    skybox_sphere_mesh->friendlyName = "skybox_sphere";
    skybox_sphere_mesh->isSkyBoxMesh = true;
    meshArray.push_back(skybox_sphere_mesh);
}

void HardReset() {

    if (cylinders.size() != 0) {
        for (int i = 0; i < meshArray.size(); i++) {
            delete cylinders[i];
        }
        cylinders.clear();

        CreateCylinders();
        CreateStaticBoxes();
    }

    if (throwables.size() != 0) {
        for (int i = 0; i < throwables.size(); i++) {
            delete throwables[i];
        }
        throwables.clear();
    }
}

void Reset() {

    if (throwables.size() != 0) {
        for (int i = 0; i < throwables.size(); i++) {
            delete throwables[i];
        }
        throwables.clear();
    }
}

int main(int argc, char** argv) {

    Initialize();
    Render();

    while (!glfwWindowShouldClose(window)) {
        Update();
    }

    Shutdown();

    return 0;
}