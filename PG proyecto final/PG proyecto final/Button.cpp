#include "Button.h"
#include "TextRenderer.h"
#include "ShaderClass.h"

Button::Button(const glm::vec2& position, const glm::vec2& size,
    const std::string& text, std::function<void()> onClick)
    : position(position), size(size), text(text), onClickCallback(onClick) {
}

void Button::Render(TextRenderer& renderer, Shader& shader) {
    // Renderizar texto centrado en el botón
    float textX = position.x + (size.x - text.length() * 20) / 2.0f;
    float textY = position.y + (size.y - 20) / 2.0f;
    renderer.RenderText(shader, text, textX, textY, 0.5f, glm::vec3(1.0f));
}

bool Button::IsMouseOver(const glm::vec2& mousePos) const {
    return mousePos.x >= position.x &&
        mousePos.x <= position.x + size.x &&
        mousePos.y >= position.y &&
        mousePos.y <= position.y + size.y;
}

void Button::OnClick() {
    if (onClickCallback) {
        onClickCallback();
    }
}