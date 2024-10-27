#pragma once

#include <raylib.h>
#include <raymath.h>
#include <imgui.h>

#include "Renderer/RaycastingMath.hpp"

struct RaycastingCamera
{
    // World relative 
    SectorID currentSectorId = 1;

    // Transform
    Vector2 position { 0 };
    float elevation  { 0 };
    float yaw        { 0 };
    float pitch      { 0 };

    // Camera options
    float fov         { 60 };
    float fovVectical { 120 };
    float farPlaneDistance  = 900.0f;
    float nearPlaneDistance = 100.f;
    
    // Rendrer options
    size_t maxRenderItr { 25 };

    // Player controller options
    float moveSpeed = 100.f;
    float zAxisMoveSpeed = 500.f;
    float mouseSensitivity = 0.2f;

    RaycastingCamera(Vector2 position = { 0 })
        : position(position)
    {}

    Vector2 Forward() const
    {
        return Vector2DirectionFromAngle(yaw);
    }

    Vector2 Right() const
    {
        Vector2 forward = Forward();
        return {
            -forward.y, forward.x
        };
    }

    void LookAt(float x, float y) { LookAt({x, y}); }
    void LookAt(const Vector2& positionLook)
    {
        Vector2 direction = Vector2Subtract(positionLook, position);
        direction = Vector2Normalize(direction);

        yaw = Vector2DirectionToAngle(direction);
    }

    void DrawGUI()
    {
        ImGui::Begin("Camera");
        
            ImGui::InputFloat2("position", (float*)&position);
            ImGui::InputFloat("z", &elevation, 0.5);
            ImGui::SliderAngle("yaw", &yaw, 0, 360);
            ImGui::SliderAngle("pitch", &pitch, -90, 90);
            
            ImGui::SliderFloat("FOV", &fov, 20, 180);
            ImGui::SliderFloat("FOV Vertical", &fovVectical, 20, 180);
            ImGui::SliderFloat("Far plane Distance", &farPlaneDistance, 1, 5000);
            ImGui::SliderFloat("Near plane Distance", &nearPlaneDistance, 1, 500);

            ImGui::SliderFloat("Move Speed", &moveSpeed, 0, 1000);
            ImGui::SliderFloat("Z axis Move Speed", &zAxisMoveSpeed, 0, 5000);
            ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0, 2);

            ImGui::InputInt("Max render itr", (int*)&maxRenderItr);
            ImGui::InputInt("Current Sector", (int*)&currentSectorId);

        ImGui::End();
    }

    void Update(float deltaTime)
    {
        Vector2 moveDirection { 0 };

        if(IsKeyDown(KEY_D))
        {
            moveDirection = Vector2Add(moveDirection, Right());
        }
        if(IsKeyDown(KEY_A))
        {
            Vector2 left = Vector2Negate(Right());
            moveDirection = Vector2Add(moveDirection, left);
        }
        if(IsKeyDown(KEY_W))
        {
            moveDirection = Vector2Add(moveDirection, Forward());
        }
        if(IsKeyDown(KEY_S))
        {
            Vector2 backward = Vector2Negate(Forward());
            moveDirection = Vector2Add(moveDirection, backward);
        }

        moveDirection = Vector2Normalize(moveDirection);
        moveDirection = Vector2Scale(moveDirection, moveSpeed);
        moveDirection = Vector2Scale(moveDirection, deltaTime);

        position = Vector2Add(position, moveDirection);

        // Up / Down

        if(IsKeyDown(KEY_SPACE))
        {
            elevation += zAxisMoveSpeed * deltaTime;
        }
        if(IsKeyDown(KEY_LEFT_SHIFT))
        {
            elevation -= zAxisMoveSpeed * deltaTime;
        }

        // Yaw / Pitch
        Vector2 mouseDelta = GetMouseDelta();

        yaw +=  (mouseDelta.x * mouseSensitivity) * deltaTime;
        pitch += (mouseDelta.y * mouseSensitivity) * deltaTime;

        // Clamp yaw in between 0 and 2 * PI
        if(yaw > 2 * PI) yaw -= 2 * PI;
        if(yaw < 0)      yaw += 2 * PI;

        // Clamp pitch between -(PI / 6) and PI / 6 (to limit the distortion effect)
        if(pitch > PI / 6)  pitch = PI / 6;
        if(pitch < -PI / 6) pitch = -PI / 6;

        SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    }
};

inline float RayAngleForScreenXCam(int screenX, const RaycastingCamera& cam, uint32_t RenderTargetWidth)
{
    float fovRate = cam.fov / RenderTargetWidth;
    float angle = -(cam.fov / 2);

    return angle + (fovRate * screenX);
}
