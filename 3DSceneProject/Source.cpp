// SNHU CS 330 3D Scene Project, Liam Daley
// This source code was assembled from provided materials, online resources,
// and blood, sweat and tears.

#define _USE_MATH_DEFINES

// C includes
#include <cmath>
#include <iostream>
#include <cstdlib>
// OpenGL includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// LearnOpenGL includes
#include <learnOpengl/camera.h>

// GLM includes (header only library!)
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>

// Image processing directives
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

using namespace std; // Standard namespace

/*Shader program macro to save some GLSL work*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    // Stores the window title
    const char* const WINDOW_TITLE = "Liam Daley 3D Scene - Module 6";

    // Default window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL handles for a mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;     // Handles for the vertex buffer object
        GLuint nVertices;    // Number of vertices of the mesh
    };

    enum class PrimitiveShape {
        CUBE,
        PYRAMID,
        PLANE,
        CYLINDER
    };

    enum class BasicTexture {
        NOTEX,
        FLATWHITE,
        BRICK,
        CONCRETE,
        DOOR,
        GLASS,
        ROAD,
        LEAF,
        BARK,
        METAL
    };

    // Stores a mesh and transform data
    struct GLObject
    {
        PrimitiveShape shape;

        BasicTexture texture;

        glm::vec2 uvScale;

        glm::mat4 scale;
        glm::mat4 rotation;
        glm::mat4 translation;

        GLMesh mesh;

        GLObject() {};

        GLObject(PrimitiveShape shape_, BasicTexture texture_, glm::vec2 uvScale_, glm::vec3 translation_, glm::vec3 rotation_, glm::vec3 scale_) {
            shape = shape_;
            texture = texture_;
            uvScale = uvScale_;
            scale = glm::scale(scale_);
            //rotation = glm::rotate((float)((2 * M_PI) / 360.0f), rotation_);
            //rotation = glm::rotate(0.f, rotation_);
            rotation = glm::eulerAngleXYZ(rotation_.x * (M_PI / 180),
                rotation_.y * (M_PI / 180),
                rotation_.z * (M_PI / 180));
            translation = glm::translate(translation_);
        }

        glm::mat4 GetModelMatrix() {
            return translation * rotation * scale;
        }
    };

    // Stores a reference to the main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Stores a handle to the shader program
    GLuint gProgramId;

    // Texture IDs
    GLuint gTextureIdNotex;
    GLuint gTextureIdWhite;
    GLuint gTextureIdBrick;
    GLuint gTextureIdConcrete;
    GLuint gTextureIdDoor;
    GLuint gTextureIdGlass;
    GLuint gTextureIdRoad;
    GLuint gTextureIdLeaf;
    GLuint gTextureIdBark;
    GLuint gTextureIdMetal;

    // Camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;
    bool gOrthoView = false;

    // Input
    bool orthoKeyPressed = false;

    // Timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Light parameters
    glm::vec3 gSkyLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gSkyLightPosition(10.f, 5.f, 10.f);
    float gSkyLightBrightness = 0.2f;

    glm::vec3 gBonusLightColor(0.f, 0.25f, 1.f);
    glm::vec3 gBonusLightPosition(0.f, 5.f, 0.f);
    float gBonusLightBrightness = 0.5f;

    /*
    GLObject(// Zero cube for reference
        PrimitiveShape::CUBE,
        BasicTexture::NOTEX,
        glm::vec2(1.0f, 1.0f),
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(45.f, 45.f, 0.f),
        glm::vec3(1.f, 1.f, 1.f)
    ),
    */

    // All scene objects are defined here
    GLObject sceneObjects[] = {
        GLObject(// Sidewalk cube
            PrimitiveShape::CUBE,
            BasicTexture::CONCRETE,
            glm::vec2(5.0f, 5.0f),
            glm::vec3(-5.f, -0.5f, -10.f),
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(20.f, 1.f, 20.f)
        ),
        GLObject(// Street plane
            PrimitiveShape::PLANE,
            BasicTexture::ROAD,
            glm::vec2(8.0f, 2.0f),
            glm::vec3(0.f, -0.5f, 10.f),
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(40.f, 1.f, 20.f)
        ),
        GLObject(// Building cube
            PrimitiveShape::CUBE,
            BasicTexture::BRICK,
            glm::vec2(2.0f, 2.0f),
            glm::vec3(-5.f, 5.f, -8.f),
            glm::vec3(90.f, 0.f, 0.f), // Rotated so brick pattern aligns
            glm::vec3(10.f, 10.f, 10.f)
        ),
        GLObject(// Front door 1 plane
            PrimitiveShape::PLANE,
            BasicTexture::DOOR,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(-3.f, 1.f, -2.9f),
            glm::vec3(90.f, 0.f, 0.f),
            glm::vec3(1.f, 1.f, 2.f)
        ),
        GLObject(// Front door 2 plane
            PrimitiveShape::PLANE,
            BasicTexture::DOOR,
            glm::vec2(-1.0f, 1.0f),
            glm::vec3(-1.5f, 1.f, -2.9f),
            glm::vec3(90.f, 0.f, 0.f),
            glm::vec3(1.f, 1.f, 2.f)
        ),
        GLObject(// Window plane
            PrimitiveShape::PLANE,
            BasicTexture::GLASS,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(-5.f, 1.5f, -2.9f),
            glm::vec3(90.f, 0.f, 0.f),
            glm::vec3(2.f, 1.f, 1.5f)
        ),
        GLObject(// Garbage can
            PrimitiveShape::CYLINDER,
            BasicTexture::NOTEX,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(0.f, 2.f, 0.f),
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(1.f, 1.f, 1.f)
        ),
        GLObject(// Car body cube
            PrimitiveShape::CUBE,
            BasicTexture::METAL,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(-7.f, 0.f, 0.f),
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(4.f, 1.5f, 2.f)
        ),
        GLObject(// Car top cube
            PrimitiveShape::CUBE,
            BasicTexture::GLASS,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(-6.5f, 1.25f, 0.f),
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(2.f, 1.f, 2.f)
        ),
        GLObject(// Tree trunk cube
            PrimitiveShape::CUBE,
            BasicTexture::BARK,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(3.f, 1.5f, -2.f),
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(1.f, 3.f, 1.f)
        ),
        GLObject(// First tree leaf triangle
            PrimitiveShape::PYRAMID,
            BasicTexture::LEAF,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(3.f, 4.f, -2.f),
            glm::vec3(0.f, 15.f, 0.f),
            glm::vec3(3.f, 3.f, 3.f)
        ),
        GLObject(// Second tree leaf triangle
            PrimitiveShape::PYRAMID,
            BasicTexture::LEAF,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(3.f, 5.f, -2.f),
            glm::vec3(0.f, -10.f, 0.f),
            glm::vec3(2.5f, 2.5f, 2.5f)
        ),
        GLObject(// Third tree leaf triangle
            PrimitiveShape::PYRAMID,
            BasicTexture::LEAF,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(3.f, 6.f, -2.f),
            glm::vec3(0.f, 0.f, 0.f),
            glm::vec3(2.f, 2.f, 2.f)
        ),
        GLObject(// Trash can cube
            PrimitiveShape::CUBE,
            BasicTexture::METAL,
            glm::vec2(1.0f, 1.0f),
            glm::vec3(-2.f, 0.5f, -0.5f),
            glm::vec3(0.f, -15.f, 0.f),
            glm::vec3(0.5f, 1.f, 0.5f)
        )
    };
}

// Forward definitions for all our custom functions
// because header files are for nerds
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateSceneObjects();
void UDestroySceneObjects();
void UCreateCubeMesh(GLMesh& mesh);
void UCreatePyramidMesh(GLMesh& mesh);
void UCreatePlaneMesh(GLMesh& mesh);
void UCreateCylinderMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void ULoadTextureSet();
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
float UGetBasicTexSpecIntensity(BasicTexture basicTex);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
    layout(location = 1) in vec3 normal; // VAP position 1 for normals
    layout(location = 2) in vec2 textureCoordinate;

    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate;

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
    }
);

/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor; // For outgoing cube color to the GPU

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec3 lightColor;
    uniform vec3 lightPosition;
    uniform vec3 light2Color;
    uniform vec3 light2Position;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;
    uniform float specularIntensity;

    vec3 CalcPointLight(vec3 nLightPos, vec3 nLightColor)
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Ambient lighting*/
        float ambientStrength = 0.25f; // Set ambient or global lighting strength
        vec3 ambient = ambientStrength * nLightColor; // Generate ambient light color

        //Calculate Diffuse lighting*/
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(nLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse = impact * nLightColor; // Generate diffuse light color

        //Calculate Specular lighting*/
        float highlightSize = 16.0f; // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
        //Calculate specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * nLightColor;

        return ambient + diffuse + specular;
    }

    void main()
    {
        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        vec3 phong = CalcPointLight(lightPosition, lightColor) * textureColor.xyz;
        phong += CalcPointLight(light2Position, light2Color) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
    }
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

// generate a unit circle on XY-plane
std::vector<GLfloat> getUnitCircleVertices(int sectorCount)
{
    const GLfloat PI = 3.1415926f;
    GLfloat sectorStep = 2 * PI / sectorCount;
    GLfloat sectorAngle;  // radian

    std::vector<GLfloat> unitCircleVertices;
    for (int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        unitCircleVertices.push_back(cos(sectorAngle)); // x
        unitCircleVertices.push_back(sin(sectorAngle)); // y
        unitCircleVertices.push_back(0);                // z
    }
    return unitCircleVertices;
}

// Main program loop
int main(int argc, char* argv[])
{
    // Attempt to initialize OpenGL
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create our scene objects and populate the sceneObjects array
    UCreateSceneObjects();

    // Create the shader program from source
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load textures
    ULoadTextureSet();

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureBase"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        UProcessInput(gWindow);

        // render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh and shader program memory
    UDestroySceneObjects();
    UDestroyTexture(gTextureIdBrick);
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Apple gets their own compiler directive, how cute
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    // Input callbacks
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    // Make sure GLEW is doing all right
    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Get the speed the camera wants to move at, we might need it
    // for our hacked vertical movement
    float velocity = gCamera.MovementSpeed * gDeltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    // Vertical movement is a local implementation of exactly what Camera.h
    // does in ProcessKeyboard, because there is no up/down option
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.Position -= gCamera.Up * velocity;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.Position += gCamera.Up * velocity;
    // This latch makes sure the camera state can't flip flop infinitely
    bool orthoKey = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (orthoKey && !orthoKeyPressed)
        gOrthoView = !gOrthoView;
    orthoKeyPressed = orthoKey;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // gCamera.ProcessMouseScroll(yoffset);
    // Capture the old camera speed, decrement it
    // Note that we negate yoffset instead of adding it directly
    // to get a more natural result
    float newCamSpeed = gCamera.MovementSpeed - yoffset;
    // then make sure we didn't set it below a minimum value
    newCamSpeed = newCamSpeed < 0.1f ? 0.1f : newCamSpeed;
    // ...or above a maximum
    newCamSpeed = newCamSpeed > 5.f ? 5.f : newCamSpeed;
    // and finally apply to the camera
    gCamera.MovementSpeed = newCamSpeed;
}

// glfw: handle mouse button events
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

// The money function, renders a single frame
void URender()
{
    // Enable z-depth so objects occlude properly
    glEnable(GL_DEPTH_TEST);

    // Clear the frame background and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Loop through the sceneObjects array
    for (GLObject& currentObject : sceneObjects) {
        // cout << "Rendering a shape with " << currentObject.mesh.nIndices << " indices" << endl;

        // Activate the VBOs contained within the mesh's VAO
        glBindVertexArray(currentObject.mesh.vao);

        // Set the shader to be used
        glUseProgram(gProgramId);

        glm::mat4 model = currentObject.GetModelMatrix();

        // Transforms the camera by move the camera
        glm::mat4 view = gCamera.GetViewMatrix();

        // Creates a perspective projection from the camera
        glm::mat4 projection;
        if (gOrthoView) {
            //projection = glm::ortho<float>(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT, -1.0f, 1.0f);
            float widthHalf = 2.f;
            float heightHalf = 2.f;
            projection = glm::ortho<float>(-widthHalf, widthHalf, -heightHalf, heightHalf, 0.1f, 100.0f);
        }
        else {
            projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        }

        // Get handles for our matrices
        GLint modelLoc = glGetUniformLocation(gProgramId, "model");
        GLint viewLoc = glGetUniformLocation(gProgramId, "view");
        GLint projLoc = glGetUniformLocation(gProgramId, "projection");

        // ... and pass the relevant data to the GPU
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Reference matrix uniforms from the Shader program for the light color, light position, and camera position
        GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
        GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPosition");
        GLint light2ColorLoc = glGetUniformLocation(gProgramId, "light2Color");
        GLint light2PositionLoc = glGetUniformLocation(gProgramId, "light2Position");
        GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
        GLint specIntensityLoc = glGetUniformLocation(gProgramId, "specularIntensity");

        // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
        glm::vec3 finalSkyLightColor = gSkyLightColor * gSkyLightBrightness;
        glm::vec3 finalBonusLightColor = gBonusLightColor * gBonusLightBrightness;
        glUniform3f(lightColorLoc, finalSkyLightColor.r, finalSkyLightColor.g, finalSkyLightColor.b);
        glUniform3f(lightPositionLoc, gSkyLightPosition.x, gSkyLightPosition.y, gSkyLightPosition.z);
        glUniform3f(light2ColorLoc, finalBonusLightColor.r, finalBonusLightColor.g, finalBonusLightColor.b);
        glUniform3f(light2PositionLoc, gBonusLightPosition.x, gBonusLightPosition.y, gBonusLightPosition.z);
        const glm::vec3 cameraPosition = gCamera.Position;
        glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
        glUniform1f(specIntensityLoc, UGetBasicTexSpecIntensity(currentObject.texture));

        GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
        glUniform2fv(UVScaleLoc, 1, glm::value_ptr(currentObject.uvScale));

        glActiveTexture(GL_TEXTURE0);

        // Resolve final texture Id
        GLuint texId = gTextureIdNotex;

        switch (currentObject.texture) {
        case BasicTexture::NOTEX:
            texId = gTextureIdNotex;
            break;
        case BasicTexture::FLATWHITE:
            texId = gTextureIdWhite;
            break;
        case BasicTexture::BRICK:
            texId = gTextureIdBrick;
            break;
        case BasicTexture::CONCRETE:
            texId = gTextureIdConcrete;
            break;
        case BasicTexture::DOOR:
            texId = gTextureIdDoor;
            break;
        case BasicTexture::GLASS:
            texId = gTextureIdGlass;
            break;
        case BasicTexture::ROAD:
            texId = gTextureIdRoad;
            break;
        case BasicTexture::LEAF:
            texId = gTextureIdLeaf;
            break;
        case BasicTexture::BARK:
            texId = gTextureIdBark;
            break;
        case BasicTexture::METAL:
            texId = gTextureIdMetal;
            break;
        }

        glBindTexture(GL_TEXTURE_2D, texId);

        // Draws the triangles
        if (currentObject.shape == PrimitiveShape::CYLINDER) {
            // Drawing cylinders doesn't work. DrawArrays-shaped peg in a DrawElements-shaped hole.
            // Instructor, if you read this, it would be excellent for future students to be
            // CLEARLY INFORMED that you should build your project around DrawElements, because
            // that will make adding more complex shapes infinitely more feasible.
            
            // glDrawElements(GL_TRIANGLES, (unsigned int)currentObject.mesh.nVertices, GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, currentObject.mesh.nVertices);
        }

        // Deactivate the Vertex Array Object
        glBindVertexArray(0);
    }

    // Flip the the back buffer with the front buffer every frame
    // to prevent screen tearing
    glfwSwapBuffers(gWindow);
}

// Creates and caches our scene objects
void UCreateSceneObjects()
{
    // Fill sceneObjects with juicy data
    for (GLObject& currentObject : sceneObjects) {
        switch (currentObject.shape) {
        case PrimitiveShape::CUBE:
            UCreateCubeMesh(currentObject.mesh);
            break;
        case PrimitiveShape::PYRAMID:
            UCreatePyramidMesh(currentObject.mesh);
            break;
        case PrimitiveShape::PLANE:
            UCreatePlaneMesh(currentObject.mesh);
            break;
        case PrimitiveShape::CYLINDER:
            UCreateCylinderMesh(currentObject.mesh);
            cout << "Cylinders are not currently supported!" << endl;   
            break;
        }
    }
}

// Frees the memory used by the meshes contained within each SceneObject
void UDestroySceneObjects() {
    // Loop through the sceneObjects array
    for (GLObject& currentObject : sceneObjects) {
        UDestroyMesh(currentObject.mesh);
    }
}

// Creates and caches all data required to draw a cube
void UCreateCubeMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions          //Normals
        // ------------------------------------------------------
        //Back Face          //Negative Z Normal  Texture Coords.
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        //Front Face         //Positive Z Normal
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        //Left Face          //Negative X Normal
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        //Right Face         //Positive X Normal
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        //Bottom Face        //Negative Y Normal
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        //Top Face           //Positive Y Normal
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

// Creates and caches all data required to draw a pyramid
void UCreatePyramidMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        //Positions            // Normals          //Texture Coordinates
        // Front face
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   0.0f, 0.0f, 1.0f,   0.5f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,

        // Right face
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   1.0f, 0.0f, 0.0f,   0.5f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,

        // Back face
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.f,   1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   0.0f, 0.0f, -1.f,   0.5f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.f,   0.0f, 0.0f,

        // Left face
        -0.5f, -0.5f, -0.5f,   -1.f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   -1.f, 0.0f, 0.0f,   0.5f, 1.0f,
        -0.5f, -0.5f,  0.5f,   -1.f, 0.0f, 0.0f,   0.0f, 0.0f,

        // Bottom face
        -0.5f, -0.5f, -0.5f,   0.0f, -1.f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.f, 0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.f, 0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.f, 0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.f, 0.0f,   0.0f, 1.0f,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

// Creates and caches all data required to draw a plane
void UCreatePlaneMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        // Positions           // Normals          //Texture Coordinates
        // Front face top tri
        -0.5f,  0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
         0.5f,  0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

        // Front face bottom tri
        -0.5f,  0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
         0.5f,  0.0f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.0f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

// Creates and caches all data required to draw a cylinder
void UCreateCylinderMesh(GLMesh& mesh)
{
    // Number of sides in the cylinder
    const int numSides = 16;

    const float radius = 0.5f;
    const float height = 1.0f;

    std::vector<GLfloat> finalData;

    // get unit circle vectors on XY-plane
    std::vector<GLfloat> unitVertices = getUnitCircleVertices(numSides);

    // put side vertices to arrays
    for (int i = 0; i < 2; ++i)
    {
        GLfloat h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
        GLfloat t = 1.0f - i;                              // vertical tex coord; 1 to 0

        for (int j = 0, k = 0; j <= numSides; ++j, k += 3)
        {
            GLfloat ux = unitVertices[k];
            GLfloat uy = unitVertices[k + 1];
            GLfloat uz = unitVertices[k + 2];

            // position vector
            finalData.push_back(ux * radius);             // vx
            finalData.push_back(uy * radius);             // vy
            finalData.push_back(h);                       // vz
            // normal vector
            finalData.push_back(ux);                       // nx
            finalData.push_back(uy);                       // ny
            finalData.push_back(uz);                       // nz
            // texture coordinate
            finalData.push_back((GLfloat)j / numSides);  // s
            finalData.push_back(t);                      // t
        }
    }

    // the starting index for the base/top surface
    //NOTE: it is used for generating indices later
    int baseCenterIndex = (int)(numSides * 2) / 3;
    int topCenterIndex = baseCenterIndex + numSides + 1; // include center vertex

    // put base and top vertices to arrays
    for (int i = 0; i < 2; ++i)
    {
        GLfloat h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
        GLfloat nz = -1 + i * 2;                           // z value of normal; -1 to 1

        // center point
        finalData.push_back(0);     finalData.push_back(0);     finalData.push_back(h);
        finalData.push_back(0);      finalData.push_back(0);      finalData.push_back(nz);
        finalData.push_back(0.5f); finalData.push_back(0.5f);

        for (int j = 0, k = 0; j < numSides; ++j, k += 3)
        {
            GLfloat ux = unitVertices[k];
            GLfloat uy = unitVertices[k + 1];

            // position vector
            finalData.push_back(ux * radius);             // vx
            finalData.push_back(uy * radius);             // vy
            finalData.push_back(h);                       // vz
            // normal vector
            finalData.push_back(0);                        // nx
            finalData.push_back(0);                        // ny
            finalData.push_back(nz);                       // nz
            // texture coordinate
            finalData.push_back(-ux * 0.5f + 0.5f);      // s
            finalData.push_back(-uy * 0.5f + 0.5f);      // t
        }
    }

    // Position and Color data
    const int vertsSize = (8 * ((numSides * 2) + 2)) * 2;
    if (vertsSize != finalData.size()) {
        cout << "BIG PROBLEM! vertsSize " << vertsSize << " finalData size " << finalData.size() << endl;
        return;
    }

    GLfloat verts[vertsSize];

    for (size_t i = 0; i < finalData.size(); i++) {
        verts[i] = finalData[i];
    }

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

// Free the memory used by our mesh VAO and VBOs
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, &mesh.vbo);
}

// Calls CreateTexture for each basic texture in the project
void ULoadTextureSet() {
    // Notex texture
    const char* texFilenameNotex = "./resources/notexture.png";
    if (!UCreateTexture(texFilenameNotex, gTextureIdNotex))
    {
        cout << "Failed to load texture " << texFilenameNotex << endl;
    }

    // Flatwhite texture
    const char* texFilenameWhite = "./resources/whitetexture.png";
    if (!UCreateTexture(texFilenameWhite, gTextureIdWhite))
    {
        cout << "Failed to load texture " << texFilenameWhite << endl;
    }

    // Brick texture
    const char* texFilenameBrick = "./resources/bricktexture.png";
    if (!UCreateTexture(texFilenameBrick, gTextureIdBrick))
    {
        cout << "Failed to load texture " << texFilenameBrick << endl;
    }

    // Concrete texture
    const char* texFilenameConcrete = "./resources/concretetexture.png";
    if (!UCreateTexture(texFilenameConcrete, gTextureIdConcrete))
    {
        cout << "Failed to load texture " << texFilenameConcrete << endl;
    }

    // Door texture
    const char* texFilenameDoor= "./resources/doortexture.png";
    if (!UCreateTexture(texFilenameDoor, gTextureIdDoor))
    {
        cout << "Failed to load texture " << texFilenameDoor << endl;
    }

    // Glass texture
    const char* texFilenameGlass = "./resources/glasstexture.png";
    if (!UCreateTexture(texFilenameGlass, gTextureIdGlass))
    {
        cout << "Failed to load texture " << texFilenameGlass << endl;
    }

    // Road texture
    const char* texFilenameRoad = "./resources/roadtexture.png";
    if (!UCreateTexture(texFilenameRoad, gTextureIdRoad))
    {
        cout << "Failed to load texture " << texFilenameRoad << endl;
    }

    // Leaf texture
    const char* texFilenameLeaf = "./resources/leaftexture.png";
    if (!UCreateTexture(texFilenameLeaf, gTextureIdLeaf))
    {
        cout << "Failed to load texture " << texFilenameLeaf << endl;
    }

    // Bark texture
    const char* texFilenameBark = "./resources/barktexture.png";
    if (!UCreateTexture(texFilenameBark, gTextureIdBark))
    {
        cout << "Failed to load texture " << texFilenameBark << endl;
    }

    // Metal texture
    const char* texFilenameMetal = "./resources/metaltexture.png";
    if (!UCreateTexture(texFilenameMetal, gTextureIdMetal))
    {
        cout << "Failed to load texture " << texFilenameMetal << endl;
    }
}

// Generate and load the texture
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

float UGetBasicTexSpecIntensity(BasicTexture basicTex) {
    switch (basicTex) {
    case BasicTexture::NOTEX:
        return 0.8f;
    case BasicTexture::FLATWHITE:
        return 0.8f;
    case BasicTexture::BRICK:
        return 0.2f;
    case BasicTexture::CONCRETE:
        return 0.2f;
    case BasicTexture::DOOR:
        return 1.f;
    case BasicTexture::GLASS:
        return 2.f;
    case BasicTexture::ROAD:
        return 0.2f;
    case BasicTexture::LEAF:
        return 0.2f;
    case BasicTexture::BARK:
        return 0.f;
    case BasicTexture::METAL:
        return 1.5f;
    }
}

// Creates Vertex and Fragment shaders and combines them into a shader program
// bound to the handle programId
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}

// Free the memory used by the shader program at handle programId
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}