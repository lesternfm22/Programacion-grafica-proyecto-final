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
    void OnClick(const glm::vec2& mousePos);
    void Update(const glm::vec2& mousePos);
    void RenderTextOnly(TextRenderer& renderer, Shader& textShader, float scale = 0.8f);
    // New state methods
    bool IsHovered() const { return isHovered; }
    bool IsClicked() const { return isClicked; }

private:
    glm::vec2 position;
    glm::vec2 size;
    std::string text;
    std::function<void()> onClickCallback;

    // New states
    bool isHovered = false;
    bool isClicked = false;
    float clickAnimTime = 0.0f;
};