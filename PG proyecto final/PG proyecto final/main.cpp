#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include<stb/stb_image.h>
#include "ShaderClass.h" 
#include "Model.h"
#include "TextRenderer.h"
#include "Button.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

TextRenderer* textRenderer;
std::vector<Button> menuButtons;
glm::vec2 mousePos;
bool mousePressed = false;
bool menu = true;
float radio = 17.5f;
glm::vec3 limit_min = glm::vec3(0.36923, 0.174271, 33.0354) - glm::vec3(radio, 0.174271, radio);
glm::vec3 limit_max = glm::vec3(0.36923, 0.174271, 33.0354) + glm::vec3(radio, radio / 2, radio);
float limits(float value, float minValue, float maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}

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

unsigned int skyboxIndices[] =
{
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


//  FUNCION
// Función de callback para el mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    mousePos = glm::vec2(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mousePressed = (action == GLFW_PRESS);

        if (menu && mousePressed) {
            for (auto& button : menuButtons) {
                if (button.IsMouseOver(mousePos)) {
                    button.OnClick();
                }
            }
        }
    }
}



const unsigned int width = 1920;
const unsigned int height = 1080;


int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, "The Virtual Gallery", NULL, NULL);
    if (!window) {
        std::cerr << "ERROR al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Cargar funciones OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR al inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    textRenderer = new TextRenderer("fonts/OpenSans.ttf", 48);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Load shaders
    Shader menuShader("menu.vert", "menu.frag");
    Shader shaderProgram("default.vert", "default.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");
    Shader textShader("text.vert", "text.frag");
    // Create cube for menu
    float vertices[] = {
        // Posiciones   // Coord. Textura (asegúrate que estén en [0,1])
       -1.0f,  1.0f,  0.0f, 1.0f,  // Superior izquierda
       -1.0f, -1.0f,  0.0f, 0.0f,  // Inferior izquierda
        1.0f, -1.0f,  1.0f, 0.0f,  // Inferior derecha

       -1.0f,  1.0f,  0.0f, 1.0f,  // Superior izquierda
        1.0f, -1.0f,  1.0f, 0.0f,  // Inferior derecha
        1.0f,  1.0f,  1.0f, 1.0f   // Superior derecha

    };

    // Light configuration
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    skyboxShader.Activate();
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

    // Crear botones del menú
    menuButtons.emplace_back(
        glm::vec2(width / 2 - 100, height / 2),
        glm::vec2(200, 50),
        "Entrar",
        [&]() { menu = false; } // Captura por referencia
    );
    menuButtons.emplace_back(
        glm::vec2(width / 2 - 100, height / 2 + 70),
        glm::vec2(200, 50),
        "Salir",
        [window]() { glfwSetWindowShouldClose(window, true); }
    );

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Posiciones
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TexCoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load menu texture
    unsigned int menuTexture;
    glGenTextures(1, &menuTexture);
    glBindTexture(GL_TEXTURE_2D, menuTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load imagen
    stbi_set_flip_vertically_on_load(true);
    int imgWidth, imgHeight, channels;
    unsigned char* data = stbi_load("imagenes/menu.png", &imgWidth, &imgHeight, &channels, STBI_rgb_alpha);
    if (data) {
        std::cout << "Textura cargada correctamente. Dimensiones: " << imgWidth << "x" << imgHeight << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        std::cerr << "ERROR: No se pudo cargar 'imagenes/menu.png'. Ruta incorrecta o archivo dañado." << std::endl;
        return -1; // stop the program
    }

    // Enable depth test for 3D scene
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Create camera
    Camera camera(width, height, glm::vec3(0.0f, 2.0f, 20.0f));

    stbi_set_flip_vertically_on_load(false); // Important for 3D models

    // Load models
    Model model("modelos/piso/scene.gltf", glm::vec3(2.0f), glm::vec3(0.0f, 13.0f, 0.0f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f)); // Escala normal
    Model pit("modelos/da vinci/mona_lisa/scene.gltf", glm::vec3(0.8f), glm::vec3(3.0f, 16.5f, -4.0f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul("modelos/miguel ang/david2/scene.gltf", glm::vec3(0.6f), glm::vec3(-0.7f, 31.0f, 5.4f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model escul2("modelos/miguel ang/pieta/scene.gltf", glm::vec3(1.3f), glm::vec3(13.0f, 18.0f, 0.8f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));

    Model escul3("modelos/miguel ang/moises/scene.gltf", glm::vec3(0.6f), glm::vec3(8.0f, 27.0f, -8.5f), glm::quat(0.0f, 0.0f, 1.0f, 0.0f));

    Model escul4("modelos/Botticelli/joven/scene.gltf", glm::vec3(2.1f), glm::vec3(0.5f, 4.35f, -1.3f), glm::quat(0.0f, 1.0f, 0.0f, 0.0f));
    Model room("modelos/room/scene.gltf", glm::vec3(1.2f), glm::vec3(-1.0f, 3.5f, -4.0f), glm::quat(0.0f, 0.0f, 0.0f, 1.0f));
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


    // FPS variables
   // double prevTime = 0.0;
   // double crntTime = 0.0;
   // double timeDiff;
   // unsigned int counter = 0;

    // Skybox VAO, VBO, EBO
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

    // Skybox textures
    std::string facesCubemap[6] =
    {
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
    //  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < 6; i++)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load skybox texture: " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }




    // --- BUCLE PRINCIPAL ---
    while (!glfwWindowShouldClose(window)) {


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (menu) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Render background image
            menuShader.Activate();
            glm::mat4 menuProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(menuShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(menuProjection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Render text and buttons
            textShader.Activate();
            glm::mat4 textProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
            glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

            textRenderer->RenderText(textShader, "THE VIRTUAL GALLERY", width / 2 - 250, height / 4, 1.0f, glm::vec3(1.0f));
            for (auto& button : menuButtons) {
                button.Render(*textRenderer, textShader);
            }
        }
        else {
            // Configuración para renderizado 3D
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);

            // render de escena 3D
            camera.Inputs(window);
            camera.updateMatrix(60.0f, 0.1f, 100.0f);
            camera.Position.x = limits(camera.Position.x, limit_min.x, limit_max.x);
            camera.Position.y = limits(camera.Position.y, limit_min.y, limit_max.y);
            camera.Position.z = limits(camera.Position.z, limit_min.z, limit_max.z);
            camera.AddCollider(glm::vec3(0.3201, 3.97696, 32.9264), 4.5f);
            camera.AddCollider(glm::vec3(-13.9183, 2.28625, 18.5213), 4.5f);
            camera.AddCollider(glm::vec3(16.4938, 3.65387, 16.5488), 2.5f);
            camera.AddCollider(glm::vec3(14.8127, 3.80944, 49.5978), 2.5f);
            camera.AddCollider(glm::vec3(8.51457, 2.15766, 49.942), 2.5f);
            camera.AddCollider(glm::vec3(-8.46455, 1.67231, 50.2564), 2.5f);
            camera.AddCollider(glm::vec3(-16.9746, 2.62017, 45.6913), 2.5f);
            // render skybox
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

            // render modelos 3D
            shaderProgram.Activate();
            /*std::cout << "Position: (" <<
                camera.Position.x << ", " <<
                camera.Position.y << ", " <<
                camera.Position.z << ")\n";*/
            camera.Matrix(shaderProgram, "camMatrix");

            model.Draw(shaderProgram, camera);
            pit.Draw(shaderProgram, camera);
            escul.Draw(shaderProgram, camera);
            escul2.Draw(shaderProgram, camera);
            escul3.Draw(shaderProgram, camera);
            escul4.Draw(shaderProgram, camera);
            //room.Draw(shaderProgram, camera);
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
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // clean
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &menuTexture);
    glDeleteTextures(1, &cubemapTexture);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
