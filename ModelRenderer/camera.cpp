#include "camera.h"

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch -= yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    if (Zoom >= 1.0f && Zoom <= 45.0f)
        Zoom -= yoffset;
    if (Zoom <= 1.0f)
        Zoom = 1.0f;
    if (Zoom >= 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = -cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = -sin(glm::radians(Pitch));
    front.z = -sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(-Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    r = glm::rotate(r, glm::radians(-Pitch), glm::vec3(0.0f, 0.0f, -1.0f));
    r = glm::rotate(r, glm::radians(-Bank), glm::vec3(1.0f, 0.0f, 0.0f));
    Up = glm::normalize(glm::vec3(r * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
    Front = glm::normalize(front);
    // Also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, Up));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    //Up    = glm::normalize(glm::cross(Right, Front));
}

void Camera::SetRandomPosition(float dist)
{
    Yaw = (float)(rand() % 360 - 180);
    Pitch = -(float)(rand() % 45);
    Bank = (float)(rand() % 30 - 15);
    Position.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position.y = sin(glm::radians(Pitch));
    Position.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position = glm::normalize(Position);
    Position = Position * dist;
    //std::cout << Position.x << " " << Position.y << " " << Position.z << std::endl;
    updateCameraVectors();
}

void Camera::SetPosition(float yaw, float pitch)
{
    Yaw = yaw;
    Pitch = pitch;
    Position.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position.y = sin(glm::radians(Pitch));
    Position.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position = glm::normalize(Position);
    updateCameraVectors();
}

void Camera::SetPosition(float yaw, float pitch, float bank)
{
    Yaw = yaw;
    Pitch = pitch;
    Bank = bank;
    Position.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position.y = sin(glm::radians(Pitch));
    Position.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position = glm::normalize(Position);
    updateCameraVectors();
}

void Camera::SetPositionDist(float yaw, float pitch, float dist)
{
    Yaw = yaw;
    Pitch = pitch;
    Position.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position.y = sin(glm::radians(Pitch));
    Position.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position = glm::normalize(Position);
    Position = Position * dist;
    //std::cout << Position.x << " " << Position.y << " " << Position.z << std::endl;
    updateCameraVectors();
}

void Camera::SetPositionDist(float yaw, float pitch, float bank, float dist)
{
    Yaw = yaw;
    Pitch = pitch;
    Bank = bank;
    Position.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position.y = sin(glm::radians(Pitch));
    Position.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Position = glm::normalize(Position);
    Position = Position * dist;
    //std::cout << Position.x << " " << Position.y << " " << Position.z << std::endl;
    updateCameraVectors();
}