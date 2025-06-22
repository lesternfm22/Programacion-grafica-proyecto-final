#include "Camera.h"
#include <glm/gtx/norm.hpp> 
#include <vector>
#include "AudioManager.h"

extern void showModelInfo(const std::string& title, const std::string& description);
extern void hideModelInfo();

// Structure representing a collidable object in the scene
struct CollisionObject {
    glm::vec3 position;      // World position of the object
    float radius;            // Collision radius/sphere
    std::string collisionTitle;  // Title to display on collision
    std::string collisionText;   // Description to display on collision

    CollisionObject(glm::vec3 pos, float rad,
        std::string title, std::string text)
        : position(pos), radius(rad),
        collisionTitle(title), collisionText(text) {
    }
};

std::vector<CollisionObject> obstacles;  // Collection of all collidable objects
float collisionRadius = 0.98f;           // Player's collision radius

// Constructor - initializes camera with screen dimensions and starting position
Camera::Camera(int width, int height, glm::vec3 position)
{
    Camera::width = width;
    Camera::height = height;
    Position = position;
}

// Adds a new collidable object to the scene
void Camera::AddCollider(glm::vec3 position, float radius,
    std::string collisionTitle,
    std::string collisionText)
{
    obstacles.push_back(CollisionObject(position, radius, collisionTitle, collisionText));
}

// Checks if camera would collide with any object at given position
bool Camera::IsColliding(glm::vec3 newPosition) {
    const float EPSILON = 0.001f;  // Small buffer to prevent clipping
    bool collisionDetected = false;

    for (const auto& obstacle : obstacles) {
        glm::vec3 diff = newPosition - obstacle.position;
        float distanceSq = glm::dot(diff, diff);  // Squared distance for performance
        float minDistance = obstacle.radius + collisionRadius + EPSILON;
        float minDistanceSq = minDistance * minDistance;

        if (distanceSq < minDistanceSq) {
            if (!obstacle.collisionTitle.empty() || !obstacle.collisionText.empty()) {
                showModelInfo(obstacle.collisionTitle, obstacle.collisionText);
            }
            collisionDetected = true;
            break; // Only show info for first detected collision
        }
    }

    if (!collisionDetected) {
        hideModelInfo();
    }

    return collisionDetected;
}

// Adjusts camera position to prevent passing through collidable objects
glm::vec3 Camera::ResolveCollision(glm::vec3 desiredPos) {
    glm::vec3 adjustedPos = desiredPos;

    for (const auto& obstacle : obstacles) {
        glm::vec3 dirToCam = adjustedPos - obstacle.position;
        float distance = glm::length(dirToCam);
        float minDistance = obstacle.radius + collisionRadius;

        if (distance < minDistance) {
            // Normalize direction and push camera outside collision sphere
            glm::vec3 pushDir = glm::normalize(dirToCam);
            adjustedPos = obstacle.position + pushDir * minDistance;
        }
    }

    return adjustedPos;
}

// Updates camera view and projection matrices
void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
    // Initialize matrices
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // Create view matrix looking from camera position along orientation
    view = glm::lookAt(Position, Position + Orientation, Up);
    // Create perspective projection matrix
    projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

    // Combine into final camera matrix
    cameraMatrix = projection * view;
}

// Exports camera matrix to shader
void Camera::Matrix(Shader& shader, const char* uniform)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

// Handles keyboard and mouse input for camera movement
void Camera::Inputs(GLFWwindow* window)
{
    float fixedHeight = 2.5f;  // Lock camera at this height (first-person perspective)
    glm::vec3 desiredPosition = Position;

    // Calculate movement vectors (ignore Y component for ground movement)
    glm::vec3 forward = Orientation;
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    glm::vec3 right = glm::normalize(glm::cross(forward, Up));

    bool moved = false;

    // Handle WASD movement keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        desiredPosition += speed * forward;
        moved = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        desiredPosition -= speed * right;
        moved = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        desiredPosition -= speed * forward;
        moved = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        desiredPosition += speed * right;
        moved = true;
    }
    else {
        moved = false;
    }

    // Handle footstep sound effects
    if (audioManager) {
        if (moved && !isMoving) {
            audioManager->playFootstepLoop();
            isMoving = true;
        }
        else if (!moved && isMoving) {
            audioManager->stopFootstepLoop();
            isMoving = false;
        }
    }

    // Handle collision detection and resolution
    if (IsColliding(desiredPosition)) {
        Position = ResolveCollision(desiredPosition);
    }
    else {
        Position = desiredPosition;
    }

    Position.y = fixedHeight;  // Maintain fixed height

    // Sprint functionality with shift key
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 0.12f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        speed = 0.06f;
    }

    // Mouse look control (activated with left mouse button)
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (firstClick)
        {
            glfwSetCursorPos(window, (width / 2), (height / 2));
            firstClick = false;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Calculate rotation amounts based on mouse movement
        float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
        float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

        // Vertical rotation (with limits to prevent over-rotation)
        glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));
        if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
        {
            Orientation = newOrientation;
        }

        // Horizontal rotation (always allowed)
        Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

        glfwSetCursorPos(window, (width / 2), (height / 2));
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
}