#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <ctime>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float BANK        =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    float Bank;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
        //return glm::lookAt(Position, glm::vec3(0.0f, 0.0f, 0.0f), Up);
    }

    glm::mat4 GetRUDMatrix()
    {
        glm::mat4 RUD;
        RUD[0][0] = Right.x; RUD[0][1] = Up.x; RUD[0][2] = Position.x; RUD[0][3] = 0.0f;
        RUD[1][0] = Right.y; RUD[1][1] = Up.y; RUD[1][2] = Position.y; RUD[1][3] = 0.0f;
        RUD[2][0] = Right.z; RUD[2][1] = Up.z; RUD[2][2] = Position.z; RUD[2][3] = 0.0f;
        RUD[3][0] = 0.0f; RUD[3][1] = 0.0f; RUD[3][2] = 0.0f; RUD[3][3] = 1.0f;

        std::cout << RUD[0][0] << " " << RUD[0][1] << " " << RUD[0][2] << " " << RUD[0][3] << std::endl;
        std::cout << RUD[1][0] << " " << RUD[1][1] << " " << RUD[1][2] << " " << RUD[1][3] << std::endl;
        std::cout << RUD[2][0] << " " << RUD[2][1] << " " << RUD[2][2] << " " << RUD[2][3] << std::endl;
        std::cout << RUD[3][0] << " " << RUD[3][1] << " " << RUD[3][2] << " " << RUD[3][3] << std::endl;
        return RUD;
    }

    glm::vec3 getPosition() const
    {
        return Position;
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);

    // for random sample rendering
    void SetRandomPosition(float dist);
    void SetPosition(float yaw, float pitch);
    void SetPosition(float yaw, float pitch, float bank);
    void SetPositionDist(float yaw, float pitch, float dist);
    void SetPositionDist(float yaw, float pitch, float bank, float dist);

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};
#endif