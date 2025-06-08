#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include "ShaderClass.h"

struct Character {
    GLuint     TextureID;  // ID de la textura del glifo
    glm::ivec2 Size;       // Tamaño del glifo
    glm::ivec2 Bearing;    // Desplazamiento desde la línea base a la izquierda/arriba
    GLuint     Advance;    // Desplazamiento horizontal hasta la próxima posición
};

class TextRenderer {
public:
    TextRenderer(const std::string& fontPath, int fontSize);
    ~TextRenderer();

    void RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color);

private:
    std::map<char, Character> Characters;
    GLuint VAO, VBO;
};