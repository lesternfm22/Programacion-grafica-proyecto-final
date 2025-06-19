#include"Camera.h"
#include <glm/gtx/norm.hpp> 
#include <vector>
#include "AudioManager.h"


struct CollisionObject {
    glm::vec3 position;
    float radius;

    CollisionObject(glm::vec3 pos, float rad) : position(pos), radius(rad) {}
};

std::vector<CollisionObject> obstacles;
float collisionRadius = 0.98f;

Camera::Camera(int width, int height, glm::vec3 position)
{
    Camera::width = width;
    Camera::height = height;
    Position = position;
}

void Camera::AddCollider(glm::vec3 position, float radius) {
    obstacles.push_back(CollisionObject(position, radius));
}

bool Camera::IsColliding(glm::vec3 newPosition) {
    const float EPSILON = 0.001f;

    for (const auto& obstacle : obstacles) {
        glm::vec3 diff = newPosition - obstacle.position;
        float distanceSq = glm::dot(diff, diff);
        float minDistance = obstacle.radius + collisionRadius + EPSILON;
        float minDistanceSq = minDistance * minDistance;

        if (distanceSq < minDistanceSq) {
            return true;
        }
    }
    return false;
}

// Si hay colisión, ajusta la posición para que no atraviese el objeto
glm::vec3 Camera::ResolveCollision(glm::vec3 desiredPos) {
    glm::vec3 adjustedPos = desiredPos;

    for (const auto& obstacle : obstacles) {
        glm::vec3 dirToCam = adjustedPos - obstacle.position;
        float distance = glm::length(dirToCam);
        float minDistance = obstacle.radius + collisionRadius;

        if (distance < minDistance) {
            // Normalizamos la dirección y empujamos la cámara afuera
            glm::vec3 pushDir = glm::normalize(dirToCam);
            adjustedPos = obstacle.position + pushDir * minDistance;
        }
    }

    return adjustedPos;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
    // Initializes matrices since otherwise they will be the null matrix
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // Makes camera look in the right direction from the right position
    view = glm::lookAt(Position, Position + Orientation, Up);
    // Adds perspective to the scene
    projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

    // Sets new camera matrix
    cameraMatrix = projection * view;
}

void Camera::Matrix(Shader& shader, const char* uniform)
{
    // Exports camera matrix
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}


void Camera::Inputs(GLFWwindow* window)
{
    float fixedHeight = 2.5f;

    glm::vec3 desiredPosition = Position;


    glm::vec3 forward = Orientation;
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    glm::vec3 right = glm::normalize(glm::cross(forward, Up));
   
    bool moved = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        desiredPosition += speed * forward;
        moved = true;
    }else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        desiredPosition -= speed * right;
        moved = true;
    }else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        desiredPosition -= speed * forward;
        moved = true;
    }else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        desiredPosition += speed * right;
        moved = true;
    }else{
        moved = false;
    }

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

    if (IsColliding(desiredPosition)) {
        Position = ResolveCollision(desiredPosition);
    }
    else {
        Position = desiredPosition;
    }


    Position.y = fixedHeight;

    // Sprint
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 0.12f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        speed = 0.06f;
    }

    // Mouse look
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

        float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
        float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

        // Rotación vertical (limitada)
        glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));
        if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
        {
            Orientation = newOrientation;
        }

        // Rotación horizontal (siempre permitida)
        Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

        glfwSetCursorPos(window, (width / 2), (height / 2));
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
}