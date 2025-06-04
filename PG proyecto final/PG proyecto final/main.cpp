#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include<stb/stb_image.h>
#include "ShaderClass.h" 
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


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
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, bool& menu) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        menu = false; // Salir del menú
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Cargar funciones OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR al inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Load shaders
    Shader menuShader("menu.vert", "menu.frag");
    Shader shaderProgram("default.vert", "default.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");

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
        return -1; // Detener el programa si falla.
    }

    // Enable depth test for 3D scene
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Create camera
    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 10.0f));

    // Load models
    Model model("modelos/piso/scene.gltf", glm::vec3(2.0f));
    Model casa("modelos/mus3/scene.gltf", glm::vec3(1.0f));
    Model sta("modelos/sta/scene.gltf", glm::vec3(0.5f));


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


    // Bandera de control
    bool menu = true;

    // --- BUCLE PRINCIPAL ---
    while (!glfwWindowShouldClose(window)) {
        processInput(window, menu);

        // Limpiar buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (menu) {
            // Configuración específica para renderizado 2D
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Renderizar menú
            menuShader.Activate();

            // Configurar matriz de proyección ortográfica para 2D
            glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
            glUniformMatrix4fv(glGetUniformLocation(menuShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glUniform1i(glGetUniformLocation(menuShader.ID, "menuTexture"), 0);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

        }
        else {
            // Configuración para renderizado 3D
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDisable(GL_BLEND);

            // Renderizado de escena 3D
            camera.Inputs(window);
            camera.updateMatrix(60.0f, 0.1f, 100.0f);

            // Renderizar skybox
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

            // Renderizar modelos 3D
            shaderProgram.Activate();
            camera.Matrix(shaderProgram, "camMatrix");
            model.Draw(shaderProgram, camera);
            casa.Draw(shaderProgram, camera);
            sta.Draw(shaderProgram, camera);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // Limpieza
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &menuTexture);
    glDeleteTextures(1, &cubemapTexture);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
