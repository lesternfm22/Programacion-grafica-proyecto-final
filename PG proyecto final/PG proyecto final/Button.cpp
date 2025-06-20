#include "Button.h"
#include "TextRenderer.h"
#include "ShaderClass.h"
#include <iostream>

Button::Button(const glm::vec2& position, const glm::vec2& size,
    const std::string& text, std::function<void()> onClick)
    : position(position), size(size), text(text), onClickCallback(onClick) {
}

void Button::Update(const glm::vec2& mousePos) {
    bool wasHovered = isHovered;
    isHovered = IsMouseOver(mousePos);

    // Animación de click
    if (isClicked) {
        clickAnimTime += 0.1f;
        if (clickAnimTime >= 1.0f) {
            isClicked = false;
            clickAnimTime = 0.0f;
        }
    }
}

void Button::RenderTextOnly(TextRenderer& renderer, Shader& textShader, float scale) {
    float textWidth = 0.0f;
    for (const char& c : text) {
        Character ch = renderer.GetCharacter(c);
        textWidth += (ch.Advance >> 6) * scale;
    }
    float textX = position.x + (size.x - text.length() * 25) / 2.0f;
    float textY = position.y + size.y / 5.0f;
    renderer.RenderText(textShader, text, textX, textY, 0.5f, glm::vec3(0.0f));
}

void Button::Render(TextRenderer& renderer, Shader& shader) {
    // Calcular color basado en estado
    glm::vec3 buttonColor;
    if (isClicked) {
        buttonColor = glm::vec3(0.2f, 0.5f, 0.8f); // Azul cuando se hace click
    }
    else if (isHovered) {
        buttonColor = glm::vec3(0.3f, 0.3f, 0.3f); // Gris cuando el mouse está encima
    }
    else {
        buttonColor = glm::vec3(1.0f, 1.0f, 1.0f); //blanco normsl
    }

    // Renderizar fondo del botón (un rectángulo)
    float vertices[] = {
        position.x, position.y, 0.0f,
        position.x, position.y + size.y, 0.0f,
        position.x + size.x, position.y + size.y, 0.0f,

        position.x, position.y, 0.0f,
        position.x + size.x, position.y + size.y, 0.0f,
        position.x + size.x, position.y, 0.0f
    };


    // Configurar y dibujar el fondo
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    shader.Activate();
    glUniform3f(glGetUniformLocation(shader.ID, "buttonColor"),
        buttonColor.r, buttonColor.g, buttonColor.b);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Limpiar
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);


}

bool Button::IsMouseOver(const glm::vec2& mousePos) const {
    return mousePos.x >= position.x &&
        mousePos.x <= position.x + size.x &&
        mousePos.y >= position.y &&
        mousePos.y <= position.y + size.y;
}

void Button::OnClick(const glm::vec2& mousePos) {  // Añade el parámetro
    if (onClickCallback && IsMouseOver(mousePos)) {
        isClicked = true;
        onClickCallback();
    }

}

