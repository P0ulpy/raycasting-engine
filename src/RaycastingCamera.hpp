#pragma once

#include <raylib.h>
#include <raymath.h>

#include <imgui.h>

#include "RaycastingMath.hpp"

struct RaycastingCamera
{
    Vector2 position { 0 };
    float elevation  { 0 };
    float yaw        { 0 };
    float pitch      { 0 };

    float fov         { 60 };
    float fovVectical { 120 };

    float farPlaneDistance  = 1000.0f;
    float nearPlaneDistance = 100.f;

    RenderTexture2D renderTexture;

    uint32_t currentSector = 1;

    RaycastingCamera(Vector2 position = { 0 })
        : position(position)
        , renderTexture(LoadRenderTexture(GetScreenWidth(), GetScreenHeight()))
    {}

    RaycastingCamera(int renderTextureWidth, int renderTextureHeight, Vector2 position = { 0 })
        : position(position)
        , renderTexture(LoadRenderTexture(renderTextureWidth, renderTextureHeight))
    {}

    ~RaycastingCamera()
    {
        UnloadRenderTexture(renderTexture);
    }

    void UpdateRenderTextureSize(int width, int height)
    {
        UnloadRenderTexture(renderTexture);
        renderTexture = LoadRenderTexture(width, height);
    }

    Vector2 Forward()
    {
        return Vector2DirectionFromAngle(yaw);
    }

    Vector2 Right()
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
        
            ImGui::InputFloat2("position", (float*)(&position));
            ImGui::InputFloat("z", &elevation, 0.5);
            ImGui::SliderAngle("yaw", &yaw, -90, 90);
            ImGui::SliderAngle("pitch", &pitch, -180, 180);
            
            ImGui::SliderFloat("FOV", &fov, 20, 180);
            ImGui::SliderFloat("FOV Vetical", &fovVectical, 20, 180);
            
            ImGui::SliderFloat("Far plane Distance", &farPlaneDistance, 1, 5000);
            ImGui::SliderFloat("Near plane Distance", &nearPlaneDistance, 1, 500);

            ImGui::InputInt("Current Sector", (int*)(&currentSector));

        ImGui::End();
    }
};

inline float RayAngleforScreenXCam(int screenX, const RaycastingCamera& cam)
{
    float fovRate = cam.fov / cam.renderTexture.texture.width;
    float angle = -(cam.fov / 2);

    return angle + (fovRate * screenX);
}

inline void UpdateCamera(RaycastingCamera& cam, float deltaTime)
{
    Vector2 moveDirection { 0 };

    if(IsKeyDown(KEY_D))
    {
        moveDirection = Vector2Add(moveDirection, cam.Right());
    }
    if(IsKeyDown(KEY_A))
    {
        Vector2 left = Vector2Negate(cam.Right());
        moveDirection = Vector2Add(moveDirection, left);
    }
    if(IsKeyDown(KEY_W))
    {
        moveDirection = Vector2Add(moveDirection, cam.Forward());
    }
    if(IsKeyDown(KEY_S))
    {
        Vector2 backward = Vector2Negate(cam.Forward());
        moveDirection = Vector2Add(moveDirection, backward);
    }

    float moveSpeed = 100.f;

    moveDirection = Vector2Normalize(moveDirection);
    moveDirection = Vector2Scale(moveDirection, moveSpeed);
    moveDirection = Vector2Scale(moveDirection, deltaTime);

    cam.position = Vector2Add(cam.position, moveDirection);

    // Up / Down

    constexpr float UpDownSpeed = 500.f;

    if(IsKeyDown(KEY_SPACE))
    {
        cam.elevation += UpDownSpeed * deltaTime;
    }
    if(IsKeyDown(KEY_LEFT_SHIFT))
    {
        cam.elevation -= UpDownSpeed * deltaTime;
    }

    // Get the mouse delta (how much the mouse moved since the last frame)
    Vector2 mouseDelta = GetMouseDelta();

    // Scale the mouse movement by a sensitivity factor to control the speed of the rotation
    constexpr float sensitivity = 0.003f;
    float yawChange = mouseDelta.x * sensitivity;
    float pitchChange = mouseDelta.y * sensitivity;

    // Update the camera's yaw
    cam.yaw += yawChange;
    cam.pitch += pitchChange;

    // Clamp yaw in between 0 and 2 * PI
    if(cam.yaw > 2 * PI) cam.yaw -= 2 * PI; 
    if(cam.yaw < 0)      cam.yaw += 2 * PI;

    // Clamp pitch between -(PI / 2) and PI / 2
    if(cam.pitch > PI / 2)  cam.pitch = PI / 2;
    if(cam.pitch < -PI / 2) cam.pitch = -PI / 2;
    
}
