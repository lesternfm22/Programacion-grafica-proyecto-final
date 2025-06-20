#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include "ShaderClass.h"

struct Character {
    GLuint     TextureID;  // ID de la textura del glifo
    glm::ivec2 Size;       // Tama�o del glifo
    glm::ivec2 Bearing;    // Desplazamiento desde la l�nea base a la izquierda/arriba
    GLuint     Advance;    // Desplazamiento horizontal hasta la pr�xima posici�n
};

class TextRenderer {
public:
    TextRenderer(const std::string& fontPath, int fontSize);
    ~TextRenderer() = default;
   // float GetTextHeight(const std::string& text, float scale);
    float CalculateTextWidth(const std::string& text, float scale);
    const Character& GetCharacter(char c) const {
        return Characters.at(c);
    }
    void RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color);

private:
    std::map<char, Character> Characters;
    GLuint VAO, VBO;
};