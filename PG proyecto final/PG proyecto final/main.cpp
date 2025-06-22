#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <stb/stb_image.h>
#include <vector>
#include <string>
#include <limits>
#include "ShaderClass.h"
#include "Model.h"
#include "TextRenderer.h"
#include "Button.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "AudioManager.h"

// Structure for model information display
struct ModelInfo {
    Model* model;
    std::string title;
    std::string description;
    float triggerDistance;
};

// Global UI state variables
bool showHelp = false;
bool showHelpPage2 = false;
bool showCredits = false;
bool showModelInfoFlag = false;
std::string currentModelTitle;
std::string currentModelDescription;
std::string currentMusic = "";

// Rendering resources
TextRenderer* textRenderer;
TextRenderer* textRenderer2;
std::vector<Button> menuButtons;
glm::vec2 mousePos;
bool mousePressed = false;
bool menu = true;

// Camera movement boundaries
float radius = 17.5f;
glm::vec3 limit_min = glm::vec3(0.36923, 0.174271, 33.0354) - glm::vec3(radius, 0.174271, radius);
glm::vec3 limit_max = glm::vec3(0.36923, 0.174271, 33.0354) + glm::vec3(radius, radius / 2, radius);

// UI buttons
Button backButtonHelp(
    glm::vec2(20, 20),
    glm::vec2(300, 70),
    "BACK",
    [&]() { showHelp = false; }
);

Button backButtonCredits(
    glm::vec2(20, 20),
    glm::vec2(300, 70),
    "BACK",
    [&]() { showCredits = false; }
);

Button nextButtonHelp(
    glm::vec2(1600, 20),
    glm::vec2(300, 70),
    "NEXT",
    [&]() {
        showHelp = false;
        showHelpPage2 = true;
    }
);

Button prevButtonHelp(
    glm::vec2(20, 20),
    glm::vec2(300, 70),
    "PREVIOUS",
    [&]() {
        showHelpPage2 = false;
        showHelp = true;
    }
);

// Utility function to clamp values between min and max
float limits(float value, float minValue, float maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}

// Skybox vertex and index data
float skyboxVertices[] = 
{
    //   Coordinates
    -1.0f, -1.0f,  1.0f,//        7--------6
     1.0f, -1.0f,  1.0f,//       /|       /|
     1.0f, -1.0f, -1.0f,//      4--------5 |
    -1.0f, -1.0f, -1.0f,//      | |      | |
    -1.0f,  1.0f,  1.0f,//      | 3------|-2
     1.0f,  1.0f,  1.0f,//      |/       |/
     1.0f,  1.0f, -1.0f,//      0--------1
    -1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] = {
    // Right
    1, 2, 6,
    6, 5, 1,
    // Left
    0, 4, 7,
    7, 3, 0,
    // Top
    4, 5, 6,
    6, 7, 4,
    // Bottom
    0, 3, 2,
    2, 1, 0,
    // Front
    0, 1, 5,
    5, 4, 0,
    // Back
    2, 3, 7,
    7, 6, 2
};

// Mouse callback functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    mousePos = glm::vec2(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mousePressed = (action == GLFW_PRESS);

        if (mousePressed) {
            if (menu && !showHelp && !showHelpPage2 && !showCredits) {
                for (auto& btn : menuButtons) {
                    if (btn.IsMouseOver(mousePos)) {
                        btn.OnClick(mousePos);
                    }
                }
            }
            else if (showHelp) {
                if (backButtonHelp.IsMouseOver(mousePos)) {
                    backButtonHelp.OnClick(mousePos);
                }
                if (nextButtonHelp.IsMouseOver(mousePos)) {
                    nextButtonHelp.OnClick(mousePos);
                }
            }
            else if (showHelpPage2) {
                if (prevButtonHelp.IsMouseOver(mousePos)) {
                    prevButtonHelp.OnClick(mousePos);
                }
            }
            else if (showCredits) {
                if (backButtonCredits.IsMouseOver(mousePos)) {
                    backButtonCredits.OnClick(mousePos);
                }
            }
        }
    }
}

// Keyboard callback function
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (!menu && !showHelp && !showHelpPage2 && !showCredits) {
            menu = true;
        }
        else if (showHelp || showHelpPage2) {
            showHelp = false;
            showHelpPage2 = false;
        }
        else if (showCredits) {
            showCredits = false;
        }
    }
}

// Model information display functions
void showModelInfo(const std::string& title, const std::string& description) {
    showModelInfoFlag = true;
    currentModelTitle = title;
    currentModelDescription = description;
}

void hideModelInfo() {
    showModelInfoFlag = false;
}

void renderModelInfo(TextRenderer& textRenderer, Shader& textShader, int width, int height) {
    if (!showModelInfoFlag) return;

    // Set up rendering state for 2D overlay
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Panel dimensions and position
    float panelWidth = 1850.0f;
    float panelHeight = 200.0f;
    float margin = 20.0f;

    // Render semi-transparent background panel
    Shader panelShader("panel.vert", "panel.frag");
    panelShader.Activate();
    glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
    glUniformMatrix4fv(glGetUniformLocation(panelShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4f(glGetUniformLocation(panelShader.ID, "backgroundColor"), 0.1f, 0.1f, 0.1f, 0.9f);

    // Panel vertex data
    float panelX = margin;
    float panelY = height - panelHeight - margin;
    float vertices[] = {
        panelX, panelY + panelHeight, 0.0f, 0.0f, 1.0f,
        panelX, panelY, 0.0f, 0.0f, 0.0f,
        panelX + panelWidth, panelY, 0.0f, 1.0f, 0.0f,
        panelX, panelY + panelHeight, 0.0f, 0.0f, 1.0f,
        panelX + panelWidth, panelY, 0.0f, 1.0f, 0.0f,
        panelX + panelWidth, panelY + panelHeight, 0.0f, 1.0f, 1.0f
    };

    // Create and render panel
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // Render text on top of panel
    textShader.Activate();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    float scale = 0.8f;
    float yPos = height - panelHeight + margin;

    // Render title
    textRenderer.RenderText(textShader, currentModelTitle,
        margin + 10.0f, yPos + 10.0f,
        scale * 1.2f, glm::vec3(1.0f, 1.0f, 1.0f));

    // Render description
    yPos += 60.0f;
    textRenderer.RenderText(textShader, currentModelDescription,
        margin + 10.0f, yPos + 10.0f,
        scale * 0.7f, glm::vec3(0.9f, 0.9f, 0.9f));

    // Restore rendering state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

// Function to render a generic panel
void RenderPanel(Shader& panelShader, float x, float y, float width, float height,
    glm::vec4 color, glm::mat4& projection) {
    panelShader.Activate();
    glUniformMatrix4fv(glGetUniformLocation(panelShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4f(glGetUniformLocation(panelShader.ID, "backgroundColor"),
        color.r, color.g, color.b, color.a);

    // Panel vertex data
    float panelVertices[] = {
        x, y + height, 0.0f, 0.0f, 1.0f,
        x, y, 0.0f, 0.0f, 0.0f,
        x + width, y, 0.0f, 1.0f, 0.0f,
        x, y + height, 0.0f, 0.0f, 1.0f,
        x + width, y, 0.0f, 1.0f, 0.0f,
        x + width, y + height, 0.0f, 1.0f, 1.0f
    };

    // Create and render panel
    unsigned int panelVAO, panelVBO;
    glGenVertexArrays(1, &panelVAO);
    glGenBuffers(1, &panelVBO);
    glBindVertexArray(panelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, panelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(panelVertices), panelVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(panelVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up
    glDeleteVertexArrays(1, &panelVAO);
    glDeleteBuffers(1, &panelVBO);
}

// Music and sound configuration
std::vector<std::string> envSongs = {
    "Sound/env1.mp3",
    "Sound/env2.mp3",
    "Sound/env3.mp3"
};

// Function to randomly select a song from the list
std::string pickRandomSong(const std::vector<std::string>& songs) {
    if (songs.empty()) return "";
    int index = rand() % songs.size();
    return songs[index];
}

// Window dimensions
const unsigned int width = 1920;
const unsigned int height = 1080;

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Initialize random seed and audio manager
    srand(static_cast<unsigned int>(time(nullptr)));
    AudioManager Sound;
    Sound.setMusicVolume(1.0f);
    Sound.setEffectsVolume(0.0f);

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, "The Virtual Gallery", glfwGetPrimaryMonitor(), NULL);
    if (!window) {
        std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set up OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Initialize text renderers
    textRenderer = new TextRenderer("fonts/Caprasimo.ttf", 78);
    textRenderer2 = new TextRenderer("fonts/Moodcake.ttf", 78);

    // Set up callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    // Load shaders
    Shader menuShader("menu.vert", "menu.frag");
    Shader shaderProgram("default.vert", "default.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");
    Shader textShader("text.vert", "text.frag");

    // Set up menu buttons
    menuButtons.emplace_back(
        glm::vec2(width / 2 - 150, height / 2),
        glm::vec2(300, 60),
        "ENTER",
        [&]() { menu = false; }
    );

    menuButtons.emplace_back(
        glm::vec2(width / 2 - 150, height / 2 + 80),
        glm::vec2(300, 60),
        "HELP",
        [&]() { showHelp = true; }
    );

    menuButtons.emplace_back(
        glm::vec2(width / 2 - 150, height / 2 + 160),
        glm::vec2(300, 60),
        "CREDITS",
        [&]() { showCredits = true; }
    );

    menuButtons.emplace_back(
        glm::vec2(width / 2 - 150, height / 2 + 240),
        glm::vec2(300, 60),
        "EXIT",
        [window]() { glfwSetWindowShouldClose(window, true); }
    );

    // Set up quad geometry for 2D rendering
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    float vertices[] = {
        // Positions   // Texture Coords
        -1.0f,  1.0f,  0.0f, 1.0f,  // Top left
        -1.0f, -1.0f,  0.0f, 0.0f,  // Bottom left
         1.0f, -1.0f,  1.0f, 0.0f,  // Bottom right
        -1.0f,  1.0f,  0.0f, 1.0f,  // Top left
         1.0f, -1.0f,  1.0f, 0.0f,  // Bottom right
         1.0f,  1.0f,  1.0f, 1.0f   // Top right
    };

    // Light configuration
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    skyboxShader.Activate();
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

    // Set up quad VAO/VBO
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load menu background texture
    unsigned int menuTexture;
    glGenTextures(1, &menuTexture);
    glBindTexture(GL_TEXTURE_2D, menuTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    int imgWidth, imgHeight, channels;
    unsigned char* data = stbi_load("imagenes/pru.jpg", &imgWidth, &imgHeight, &channels, STBI_rgb_alpha);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        std::cerr << "ERROR: Failed to load menu texture (pru.jpg)" << std::endl;
        return -1;
    }

    // Create camera
    Camera camera(width, height, glm::vec3(4.0f, 2.0f, 60.0f));
    camera.setAudioManager(&Sound);

    stbi_set_flip_vertically_on_load(false); // Important for 3D models

    // Load all 3D models
    Model model("modelos/piso/scene.gltf", glm::vec3(2.0f), glm::vec3(0.0f, 13.0f, 0.0f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model pit("modelos/da vinci/mona_lisa/scene.gltf", glm::vec3(0.8f), glm::vec3(3.0f, 16.5f, -4.0f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul("modelos/miguel ang/david2/scene.gltf", glm::vec3(0.6f), glm::vec3(-0.7f, 31.0f, 5.4f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul2("modelos/miguel ang/pieta/scene.gltf", glm::vec3(1.3f), glm::vec3(13.0f, 18.0f, 0.8f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul3("modelos/miguel ang/moises/scene.gltf", glm::vec3(0.6f), glm::vec3(8.0f, 27.0f, -8.5f), glm::quat(0.0f, 0.0f, 1.0f, 0.0f));
    Model escul4("modelos/Botticelli/joven/scene.gltf", glm::vec3(2.1f), glm::vec3(0.5f, 4.35f, -1.3f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model room2("modelos/room2/scene.gltf", glm::vec3(1.3f), glm::vec3(0.0f, 20.0f, 0.5f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul5("modelos/fidias/atena/scene.gltf", glm::vec3(0.3f), glm::vec3(-11.0f, 5.0f, 18.9f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul6("modelos/policleto/dori/scene.gltf", glm::vec3(0.31f), glm::vec3(-20.0f, 58.0f, 28.5f), glm::quat(0.0f, 0.0f, 1.0f, 0.0f));
    Model escul7("modelos/praxi/afrodita/scene.gltf", glm::vec3(0.7f), glm::vec3(10.0f, -12.0f, -22.5f), glm::quat(0.0f, 0.0f, 0.0f, 1.0f));
    Model pit2("modelos/van gogh/noche_estrella/scene.gltf", glm::vec3(0.9f), glm::vec3(-3.0f, 16.2f, -4.0f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul8("modelos/torso/scene.gltf", glm::vec3(1.5f), glm::vec3(3.8f, 26.0f, 1.5f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model pilar("modelos/pilar/scene.gltf", glm::vec3(0.09f), glm::vec3(-16.0f, 28.0f, 0.9f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model pilar2("modelos/pilar/scene.gltf", glm::vec3(0.09f), glm::vec3(-16.0f, 42.0f, 0.9f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model pilar3("modelos/pilar/scene.gltf", glm::vec3(0.09f), glm::vec3(16.0f, 28.0f, 0.9f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model pilar4("modelos/pilar/scene.gltf", glm::vec3(0.09f), glm::vec3(16.0f, 42.0f, 0.9f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model vase("modelos/vase/rosa1/scene.gltf", glm::vec3(1.8f), glm::vec3(-3.0f, 10.5f, -1.6f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model vase2("modelos/vase/rosa2/scene.gltf", glm::vec3(0.7f), glm::vec3(-8.0f, 51.3f, -12.4f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model vase3("modelos/vase/rosa3/scene.gltf", glm::vec3(1.5f), glm::vec3(-15.5f, 26.4f, 4.1f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model vase4("modelos/vase/rosa4/scene.gltf", glm::vec3(1.5f), glm::vec3(3.7f, 22.0f, -2.3f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));

    // Set up skybox VAO, VBO, EBO
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Load skybox textures
    std::string facesCubemap[6] = {
        "skybox/px.png", "skybox/nx.png",
        "skybox/py.png", "skybox/ny.png",
        "skybox/pz.png", "skybox/nz.png"
    };

    unsigned int cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < 6; i++) {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Failed to load skybox texture: " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }

    while (!glfwWindowShouldClose(window)) {
        if (menu && !showHelp && !showHelpPage2 && !showCredits) {
            for (auto& btn : menuButtons) {
                btn.Update(mousePos);
            }
        }
        else if (showHelp) {
            backButtonHelp.Update(mousePos);
            nextButtonHelp.Update(mousePos);
        }
        else if (showHelpPage2) {
            prevButtonHelp.Update(mousePos);
        }
        else if (showCredits) {
            backButtonCredits.Update(mousePos);
        }
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        static bool inEnvironment = false;

        if (showHelp) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            menuShader.Activate();
            glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(menuShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            Shader panelShader("panel.vert", "panel.frag");
            float panelWidth = width * 0.8f;
            float panelHeight = height * 0.6f;
            float panelX = (width - panelWidth) / 2;
            float panelY = (height - panelHeight) / 2;

            RenderPanel(panelShader, panelX, panelY, panelWidth, panelHeight,
                glm::vec4(0.0f, 0.0f, 0.0f, 0.7f), projection);

            float currentY = height / 4;
            float baseFontSize = 78.0f;
            float lineSpacingFactor = 1.5f;

            float titleScale = 1.0f;
            std::string helpTitle = "ACERCA DE LA GALERIA";
            float helpTitleWidth = textRenderer->CalculateTextWidth(helpTitle, titleScale);
            textRenderer->RenderText(textShader, helpTitle,
                width / 2 - helpTitleWidth / 2,
                currentY,
                titleScale,
                glm::vec3(1.0f, 1.0f, 1.0f));

            currentY += (baseFontSize * titleScale) * lineSpacingFactor * 1.5f;

            float textScale = 0.40f;
            float leftMargin = width / 8;

            std::vector<std::string> descriptionLines = {
                "THE VIRTUAL GALLERY ES UN ESPACIO INMERSIVO DONDE EL ARTE COBRA VIDA",
                "EXPLORA REPLICAS DIGITALES DE OBRAS MAESTRAS DEL RENACIMIENTO",
                "ESCULTURAS ICONICAS Y PINTURAS LEGENDARIAS EN UN ENTORNO 3D INTERACTIVO",
                "CAMINA ENTRE LAS CREACIONES DE MIGUEL ANGEL, LEONARDO DA VINCI",
                "Y VAN GOGH COMO SI ESTUVIERAS EN UN MUSEO REAL, CON INFORMACION",
                "DETALLADA AL ALCANCE DE LA MANO"
            };

            for (const auto& line : descriptionLines) {
                textRenderer->RenderText(textShader, line,
                    leftMargin,
                    currentY,
                    textScale,
                    glm::vec3(1.0f, 1.0f, 1.0f));
                currentY += (baseFontSize * textScale) * lineSpacingFactor;
            }

            // Render buttons
            Shader buttonShader("button.vert", "button.frag");
            buttonShader.Activate();
            glUniformMatrix4fv(glGetUniformLocation(buttonShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            backButtonHelp.Render(*textRenderer, buttonShader);
            nextButtonHelp.Render(*textRenderer, buttonShader);

            textShader.Activate();
            glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            backButtonHelp.RenderTextOnly(*textRenderer, textShader, 0.5f);
            nextButtonHelp.RenderTextOnly(*textRenderer, textShader, 0.5f);
        }
        else if (showHelpPage2) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            menuShader.Activate();
            glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(menuShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            Shader panelShader("panel.vert", "panel.frag");
            float panelWidth = width * 0.8f;
            float panelHeight = height * 0.6f;
            float panelX = (width - panelWidth) / 2;
            float panelY = (height - panelHeight) / 2;

            RenderPanel(panelShader, panelX, panelY, panelWidth, panelHeight,
                glm::vec4(0.1f, 0.1f, 0.1f, 0.7f), projection);

            float currentY = height / 4;
            float baseFontSize = 78.0f;
            float lineSpacingFactor = 1.0f;

            float titleScale = 1.5f;
            std::string helpTitle = "COMO USAR";
            float helpTitleWidth = textRenderer->CalculateTextWidth(helpTitle, titleScale);
            textRenderer->RenderText(textShader, helpTitle,
                width / 2 - helpTitleWidth / 2,
                currentY,
                titleScale,
                glm::vec3(1.0f, 1.0f, 1.0f));

            currentY += (baseFontSize * titleScale) * lineSpacingFactor * 1.5f;
            float instructionScale = 0.45f;
            float leftMargin = width / 4;

            std::vector<std::string> controls = {
                "W: MOVER HACIA ADELANTE",
                "A: MOVER A LA IZQUIERDA",
                "S: MOVER HACIA ATRAS",
                "D: MOVER A LA DERECHA",
                "MOUSE: MIRAR ALREDEDOR",
                "ACERCATE A LOS MODELOS PARA VER SU INFORMACION.",
                "ESC: VOLVER AL MENU PRINCIPAL"
            };

            for (const auto& control : controls) {
                if (control == " ") {
                    currentY += (baseFontSize * instructionScale) * lineSpacingFactor * 1.5f;
                    continue;
                }
                textRenderer->RenderText(textShader, control,
                    leftMargin,
                    currentY,
                    instructionScale,
                    glm::vec3(1.0f, 1.0f, 1.0f));
                currentY += (baseFontSize * instructionScale) * lineSpacingFactor;
            }
            // Render buttons
            Shader buttonShader("button.vert", "button.frag");
            buttonShader.Activate();
            glUniformMatrix4fv(glGetUniformLocation(buttonShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            prevButtonHelp.Render(*textRenderer, buttonShader);

            textShader.Activate();
            glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            prevButtonHelp.RenderTextOnly(*textRenderer, textShader, 0.5f);
        }
        else if (showCredits) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            menuShader.Activate();
            glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(menuShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Renderizado 
            Shader buttonShader("button.vert", "button.frag");
            buttonShader.Activate();
            glUniformMatrix4fv(glGetUniformLocation(buttonShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            backButtonCredits.Render(*textRenderer, buttonShader);

            textShader.Activate();
            glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


            Shader panelShader("panel.vert", "panel.frag");
            float panelWidth = width * 0.9f;
            float panelHeight = height * 0.6f;
            float panelX = (width - panelWidth) / 2;
            float panelY = (height - panelHeight) / 2;

            RenderPanel(panelShader, panelX, panelY, panelWidth, panelHeight,
                glm::vec4(0.1f, 0.1f, 0.1f, 0.7f), projection);


            float currentY = height / 4;
            float baseFontSize = 128.0f;
            float lineSpacingFactor = 1.0f;

            float titleScale = 0.9f;
            std::string titleText = "UNIVERSIDAD NACIONAL DE INGENIERIA";
            float titleWidth = textRenderer->CalculateTextWidth(titleText, titleScale);
            textRenderer->RenderText(textShader, titleText,
                width / 2 - titleWidth / 2,
                currentY,
                titleScale,
                glm::vec3(1.0f, 1.0f, 1.0f));

            currentY += (baseFontSize * titleScale) * lineSpacingFactor * 1.2f;

            float subtitleScale = 0.8f;
            std::string devByText = "DESARROLLADO POR:";
            float devByWidth = textRenderer->CalculateTextWidth(devByText, subtitleScale);
            textRenderer->RenderText(textShader, devByText,
                width / 2 - devByWidth / 2,
                currentY,
                subtitleScale,
                glm::vec3(1.0f, 1.0f, 1.0f));

            currentY += (baseFontSize * subtitleScale) * lineSpacingFactor;

            float nameScale = 0.7f;
            std::vector<std::string> names = {
                "ALANIZ HERRERA ROGER ANTONIO 2023-0625U",
                "BRAN RAMOS NAZARETH DE LOS ANGELES 2023-0863U",
                "FLORES MENDOZA LESTER NAHUM 2023-0632U"
            };

            for (const auto& name : names) {
                float nameWidth = textRenderer->CalculateTextWidth(name, nameScale);
                textRenderer->RenderText(textShader, name,
                    width / 2 - nameWidth / 2,
                    currentY,
                    nameScale,
                    glm::vec3(1.0f, 1.0f, 1.0f));
                currentY += (baseFontSize * nameScale) * lineSpacingFactor;
            }
            backButtonCredits.RenderTextOnly(*textRenderer, textShader, 0.5f);

        }
        else if (menu) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            inEnvironment = false;
            camera.Position = glm::vec3(0.0f, 2.0f, 60.0f);

            if (currentMusic != "Sound/MenuSound.mp3") {
                Sound.playBackgroundMusic("Sound/MenuSound.mp3", 1.0f);
                currentMusic = "Sound/MenuSound.mp3";
            }

            // Render background image
            menuShader.Activate();
            glm::mat4 menuProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(menuShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(menuProjection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Botones del menu
            Shader buttonShader("button.vert", "button.frag");
            buttonShader.Activate();
            glm::mat4 buttonProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(buttonShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(buttonProjection));

            for (auto& btn : menuButtons) {
                btn.Render(*textRenderer, buttonShader);
            }

            // Render text for title and buttons
            textShader.Activate();
            glm::mat4 textProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

            // Titulo
            float titleScale = 1.8f;
            std::string title = "THE VIRTUAL GALLERY";
            float titleWidth = textRenderer->CalculateTextWidth(title, titleScale);
            textRenderer->RenderText(textShader, title,
                width / 2 - titleWidth / 2,
                height / 5,
                titleScale,
                glm::vec3(0.0f, 0.0f, 0.0f));

            float buttonTextScale = 1.8f;
            for (auto& btn : menuButtons) {
                btn.RenderTextOnly(*textRenderer, textShader, buttonTextScale);
            }
        }
        else {
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);

            if (!inEnvironment) {
                std::string randomSong = pickRandomSong(envSongs);
                Sound.playBackgroundMusic(randomSong, 1.0f);
                currentMusic = "ENVIRONMENT";
                inEnvironment = true;
            }
            else {
                // Si la música terminó, reproducimos otra
                if (!Sound.getBackgroundMusic() || Sound.getBackgroundMusic()->isFinished()) {
                    std::string randomSong = pickRandomSong(envSongs);
                    Sound.playBackgroundMusic(randomSong, 1.0f);
                }
            }

            // Manejo de la tecla ESC para volver al menú principal
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !menu) {
                menu = true;
                inEnvironment = false;
                // Opcional: detener la música actual
                if (Sound.getBackgroundMusic()) {
                    Sound.getBackgroundMusic()->stop();
                }
            }

            // Render de la escena 3D
            camera.Inputs(window);
            camera.updateMatrix(60.0f, 0.1f, 100.0f);

            // Aplicar límites a la cámara
            camera.Position.x = limits(camera.Position.x, limit_min.x, limit_max.x);
            camera.Position.y = limits(camera.Position.y, limit_min.y, limit_max.y);
            camera.Position.z = limits(camera.Position.z, limit_min.z, limit_max.z);

            // Colisiones (ajusta los valores según tus modelos y necesidades)
            camera.AddCollider(glm::vec3(0.3201, 3.97696, 32.9264), 3.0f, "David", "Escultura de Miguel angel (1501-1504). Marmol blanco de 5.17 metros.");
            camera.AddCollider(glm::vec3(-13.9183, 2.28625, 18.5213), 3.0f, "La Piedad", "Escultura de Miguel angel (1498-1499). Marmol, Basilica de San Pedro.");
            camera.AddCollider(glm::vec3(16.4938, 3.65387, 16.5488), 2.0f, "Atenea Partenos", "Escultura de Fidias (siglo V a.C.). Replica moderna.");
            camera.AddCollider(glm::vec3(14.8127, 3.80944, 49.5978), 2.0f, "Doriforo", "Escultura de Policleto (450-440 a.C.). Copia romana en marmol.");
            camera.AddCollider(glm::vec3(8.51457, 2.15766, 49.942), 2.0f, "Afrodita de Cnido", "Escultura de Praxoteles (siglo IV a.C.). Copia romana.");
            camera.AddCollider(glm::vec3(-8.46455, 1.67231, 50.2564), 2.0f, "Moises", "Escultura de Miguel angel (1513-1515). Marmol, tumba del Papa Julio II.");
            camera.AddCollider(glm::vec3(-16.9746, 2.62017, 45.6913), 2.0f, "Torso de Belvedere", "Escultura helenestica (siglo I a.C.). Marmol, Museos Vaticanos.");
            camera.AddCollider(glm::vec3(-17.1125, 2.5, 25.5523), 1.0f);
            camera.AddCollider(glm::vec3(-17.1308, 2.5, 38.5671), 1.0f);
            camera.AddCollider(glm::vec3(17.507, 2.5, 25.7276), 1.0f);
            camera.AddCollider(glm::vec3(17.1492, 2.5, 38.6279), 1.0f);

            // Render Skybox
            glDepthFunc(GL_LEQUAL);
            skyboxShader.Activate();

            glm::mat4 view = glm::mat4(glm::mat3(glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up)));
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

            glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glDepthFunc(GL_LESS);

            // Render modelos 3D
            shaderProgram.Activate();
            camera.Matrix(shaderProgram, "camMatrix");

            model.Draw(shaderProgram, camera);
            pit.Draw(shaderProgram, camera);
            escul.Draw(shaderProgram, camera);
            escul2.Draw(shaderProgram, camera);
            escul3.Draw(shaderProgram, camera);
            escul4.Draw(shaderProgram, camera);
            room2.Draw(shaderProgram, camera);
            escul5.Draw(shaderProgram, camera);
            escul6.Draw(shaderProgram, camera);
            escul7.Draw(shaderProgram, camera);
            pit2.Draw(shaderProgram, camera);
            escul8.Draw(shaderProgram, camera);
            pilar.Draw(shaderProgram, camera);
            pilar2.Draw(shaderProgram, camera);
            pilar3.Draw(shaderProgram, camera);
            pilar4.Draw(shaderProgram, camera);
            vase.Draw(shaderProgram, camera);
            vase2.Draw(shaderProgram, camera);
            vase3.Draw(shaderProgram, camera);
            vase4.Draw(shaderProgram, camera);
            // Renderizar informacion del modelo
            renderModelInfo(*textRenderer2, textShader, width, height);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up resources
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &menuTexture);
    glDeleteTextures(1, &cubemapTexture);
    delete textRenderer;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}