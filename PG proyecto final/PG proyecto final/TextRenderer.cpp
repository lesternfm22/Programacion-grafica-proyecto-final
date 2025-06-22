#include "TextRenderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H

// Constructor - initializes the text renderer with a specific font and size
TextRenderer::TextRenderer(const std::string& fontPath, int fontSize) {
    // Initialize FreeType library
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cerr << "ERROR::FREETYPE: Could not initialize FreeType Library" << std::endl;

    // Load font face from file
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;

    // Set font pixel size (width of 0 means auto-calculate based on height)
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // Disable byte-alignment restriction (since we're using 1-byte grayscale)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 ASCII characters
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph with rendering
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load glyph " << c << std::endl;
            continue;
        }

        // Generate texture for the character
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,  // Store only red component (grayscale)
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store character data for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)  // Advance in 1/64 pixels
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    // Clean up FreeType resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Configure VAO/VBO for text rendering quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Allocate memory for 6 vertices (2 triangles) with 4 floats each (x,y,s,t)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Calculates the total width of a text string when rendered
float TextRenderer::CalculateTextWidth(const std::string& text, float scale) {
    float width = 0.0f;
    for (const char& c : text) {
        Character ch = Characters[c];
        // Advance is in 1/64 pixels, so right-shift by 6 to get pixels (64=2^6)
        width += (ch.Advance >> 6) * scale;
    }
    return width;
}

// Renders text string at specified position with given color and scale
void TextRenderer::RenderText(Shader& shader, const std::string& text, float x, float y, float scale, glm::vec3 color) {
    // Activate shader and set text color
    shader.Activate();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Render each character in the string
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        // Calculate position and size of character quad
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;  // Adjust for bearing

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // Update VBO for each character (2 triangles forming a quad)
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },  // Top-left
            { xpos,     ypos,       0.0f, 1.0f },  // Bottom-left
            { xpos + w, ypos,       1.0f, 1.0f },  // Bottom-right

            { xpos,     ypos + h,   0.0f, 0.0f },  // Top-left
            { xpos + w, ypos,       1.0f, 1.0f },  // Bottom-right
            { xpos + w, ypos + h,   1.0f, 0.0f }   // Top-right
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance cursor position for next character
        // Bitshift by 6 to get value in pixels (2^6 = 64)
        x += (ch.Advance >> 6) * scale;
    }
    // Clean up
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}