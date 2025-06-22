#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include "ShaderClass.h"

struct Character {
    GLuint     TextureID;  // ID of the glyph's texture
    glm::ivec2 Size;       // Size of the glyph
    glm::ivec2 Bearing;    // Offset from baseline to left/top
    GLuint     Advance;    // Horizontal offset to advance to next glyph
};

class TextRenderer {
public:
    TextRenderer(const std::string& fontPath, int fontSize);
    ~TextRenderer() = default;
    float CalculateTextWidth(const std::string& text, float scale);
    const Character& GetCharacter(char c) const {
        return Characters.at(c);
    }
    void RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color);

private:
    std::map<char, Character> Characters;
    GLuint VAO, VBO;
};