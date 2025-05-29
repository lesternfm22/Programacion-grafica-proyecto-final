////------- Ignore this ----------
//#include<filesystem>
//namespace fs = std::filesystem;
////------------------------------

#include"Model.h"

const unsigned int width = 1200;
const unsigned int height = 1200;

int main()
{
    // Initialize GLFW
    glfwInit();

    // Tell GLFW what version of OpenGL we are using 
    // In this case we are using OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Tell GLFW we are using the CORE profile
    // So that means we only have the modern functions
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
    GLFWwindow* window = glfwCreateWindow(width, height, "YoutubeOpenGL", NULL, NULL);
    // Error check if the window fails to create
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Introduce the window into the current context
    glfwMakeContextCurrent(window);

    //Load GLAD so it configures OpenGL
    gladLoadGL();
    // Specify the viewport of OpenGL in the Window
    // In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
    glViewport(0, 0, width, height);

    // Generates Shader objects
    Shader shaderProgram("default.vert", "default.frag");

    // Take care of all the light related things
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);


    // Creates camera object
    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 10.0f));

    // Load in model using simpler approach
   Model model("modelos/piso/scene.gltf", glm::vec3(2.0f)); // Escala normal
   Model casa("modelos/mus3/scene.gltf", glm::vec3(1.0f));  // Mitad de tamaño

    // Variables to create periodic event for FPS displaying
    double prevTime = 0.0;
    double crntTime = 0.0;
    double timeDiff;
    // Keeps track of the amount of frames in timeDiff
    unsigned int counter = 0;


    // Main while loop
    while (!glfwWindowShouldClose(window))
    {
        // Updates counter and times
        crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;

        if (timeDiff >= 1.0 / 30.0)
        {
            // Creates new title
            std::string FPS = std::to_string((1.0 / timeDiff) * counter);
            std::string ms = std::to_string((timeDiff / counter) * 1000);
            std::string newTitle = "YoutubeOpenGL - " + FPS + "FPS / " + ms + "ms";
            glfwSetWindowTitle(window, newTitle.c_str());

            // Resets times and counter
            prevTime = crntTime;
            counter = 0;
        }

        // Specify the color of the background
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        // Clean the back buffer and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handles camera inputs
        camera.Inputs(window);
        // Updates and exports the camera matrix to the Vertex Shader
        camera.updateMatrix(60.0f, 0.1f, 1000.0f);

        // Draw the normal model
        model.Draw(shaderProgram, camera);
        casa.Draw(shaderProgram, camera);


        // Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
        glDepthFunc(GL_LEQUAL);

        // Switch back to the normal depth function
        glDepthFunc(GL_LESS);

        // Swap the back buffer with the front buffer
        glfwSwapBuffers(window);
        // Take care of all GLFW events
        glfwPollEvents();
    }

    // Delete all the objects we've created
    shaderProgram.Delete();
    // Delete window before ending the program
    glfwDestroyWindow(window);
    // Terminate GLFW before ending the program
    glfwTerminate();
    return 0;
}