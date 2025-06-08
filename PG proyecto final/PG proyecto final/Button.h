#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <string>

class Button {
public:
    Button(const glm::vec2& position, const glm::vec2& size,
        const std::string& text, std::function<void()> onClick);

    void Render(class TextRenderer& renderer, class Shader& shader);
    bool IsMouseOver(const glm::vec2& mousePos) const;
    void OnClick();

private:
    glm::vec2 position;
    glm::vec2 size;
    std::string text;
    std::function<void()> onClickCallback;
};