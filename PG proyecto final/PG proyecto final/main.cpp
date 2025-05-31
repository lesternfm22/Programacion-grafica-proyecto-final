#include "Model.h"
#include <filesystem>
#include <iostream>

const unsigned int width = 1200;
const unsigned int height = 1200;

// Variables globales para el estado del menú
bool menu = true;

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

// Función para procesar inputs
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        menu = false;
}

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, "The virtual Gallery", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport
    glViewport(0, 0, width, height);

    // Load shaders
    Shader shaderProgram("default.vert", "default.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");
    Shader menuShader("menu.vert", "menu.frag");

    // Check if menu shaders loaded correctly
    if (!menuShader.CompiledSuccessfully())
    {
        std::cerr << "Error al cargar los shaders del menú!" << std::endl;
        return -1;
    }

    // Light configuration
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    skyboxShader.Activate();
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

    // Configure menu quad
    

    // Datos del quad (asegúrate que sean exactamente estos)
    float vertices[] = {
        // Posiciones   // Coord. Textura (asegúrate que estén en [0,1])
        -1.0f,  1.0f,  0.0f, 1.0f,  // Superior izquierda
        -1.0f, -1.0f,  0.0f, 0.0f,  // Inferior izquierda
         1.0f, -1.0f,  1.0f, 0.0f,  // Inferior derecha

        -1.0f,  1.0f,  0.0f, 1.0f,  // Superior izquierda
         1.0f, -1.0f,  1.0f, 0.0f,  // Inferior derecha
         1.0f,  1.0f,  1.0f, 1.0f   // Superior derecha
    };

    GLuint quadVAO, quadVBO;// 1. Generar y bindear VAO/VBO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    // 2. Bindear y configurar buffers
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. Configurar atributos (VERIFICAR ESTOS PARÁMETROS)
    glEnableVertexAttribArray(0); // Posición
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // Coordenadas de textura
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // 4. Verificar
    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    std::cout << "Atributos configurados (debería ser 0 y 1): ";
    for (int i = 0; i < maxAttribs; ++i) {
        GLint enabled;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        if (enabled) std::cout << i << " ";
    }
    std::cout << std::endl;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 5. Verificar configuración
    GLint bufferSize;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    std::cout << "VBO size correcto: " << bufferSize << " bytes" << std::endl;

    // Load menu texture
    // Cargar textura PNG del menú
    // Load menu texture - Versión mejorada
    unsigned int menuTexture;
    glGenTextures(1, &menuTexture);
    glBindTexture(GL_TEXTURE_2D, menuTexture);



    // Configuración de textura más robusta
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Cargar imagen con manejo mejorado de errores
    // Cargar textura con verificación exhaustiva
    std::string texturePath = "imagenes/menu.png";
    std::cout << "Intentando cargar: " << std::filesystem::absolute(texturePath).string() << std::endl;

    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!data) {
        std::cerr << "FALLO CRÍTICO: " << stbi_failure_reason() << std::endl;

        // Crear textura de emergencia (franjas rojas/blancas)
        const int texSize = 64;
        unsigned char debugTex[texSize * texSize * 4];
        for (int i = 0; i < texSize * texSize * 4; i++) {
            bool stripe = ((i / 4 % texSize) / 16) % 2 == 0;
            debugTex[i] = (i % 4 == 3) ? 255 : (stripe ? 255 : (i % 4 == 0 ? 255 : 0));
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, debugTex);
        std::cerr << "Usando textura de debug (franjas rojas/blancas)" << std::endl;
    }
    else {
        std::cout << "Textura cargada (" << width << "x" << height << ")" << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Enable depth test for 3D scene
    glEnable(GL_DEPTH_TEST);
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
    double prevTime = 0.0;
    double crntTime = 0.0;
    double timeDiff;
    unsigned int counter = 0;

    // Skybox VAO, VBO, EBO
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
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
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Calculate FPS
        /*crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;
        if (timeDiff >= 1.0 / 30.0)
        {
            std::string FPS = std::to_string((1.0 / timeDiff) * counter);
            std::string ms = std::to_string((timeDiff / counter) * 1000);
            std::string newTitle = "Aplicación 3D - " + FPS + "FPS / " + ms + "ms";
            glfwSetWindowTitle(window, newTitle.c_str());
            prevTime = crntTime;
            counter = 0;
        }*/

        if (menu) {
            // 1. Limpiar buffer con color de debug (ahora morado para confirmar que se ejecuta)
            glClearColor(0.5f, 0.0f, 0.5f, 1.0f);  // Morado = debug
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            // 2. Activar shader y verificar
            menuShader.Activate();
            std::cout << "Shader activado. ID: " << menuShader.ID << std::endl;

            // 3. Bind de textura con verificación EXTRA
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            GLint boundTexture;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
            std::cout << "Textura bindeada: " << boundTexture << " (debería ser: " << menuTexture << ")" << std::endl;

            // 4. Dibujar con verificación EXTRA del VAO
            glBindVertexArray(quadVAO);
            GLint vaoBound;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBound);
            std::cout << "VAO bindeado: " << vaoBound << " (debería ser: " << quadVAO << ")" << std::endl;

            // 5. Verificar atributos de vértice
            GLint enabledAttribs;
            glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &enabledAttribs);
            std::cout << "Atributos activos: ";
            for (GLint i = 0; i < enabledAttribs; ++i) {
                GLint enabled;
                glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
                if (enabled) std::cout << i << " ";
            }
            std::cout << std::endl;

            glDrawArrays(GL_TRIANGLES, 0, 6);

            // 6. Verificación de errores POST-dibujo
            bool hasError = false;
            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cerr << "ERROR OpenGL POST-draw: " << err << std::endl;
                hasError = true;
            }

            if (!hasError) {
                std::cout << "Dibujado completado sin errores OpenGL" << std::endl;
            }
        }
        else
        {
            // Render 3D scene
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            camera.Inputs(window);
            camera.updateMatrix(60.0f, 0.1f, 1000.0f);

            model.Draw(shaderProgram, camera);
            casa.Draw(shaderProgram, camera);
            sta.Draw(shaderProgram, camera);

            // Skybox
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
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    shaderProgram.Delete();
    skyboxShader.Delete();
    menuShader.Delete();
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteTextures(1, &menuTexture);
    glDeleteTextures(1, &cubemapTexture);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}