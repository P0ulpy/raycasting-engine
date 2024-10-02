#pragma once

#include <imgui.h>

#include "RaycastingCameraViewport.hpp"

class RenderingOrchestrator
{
public:
    RenderingOrchestrator(RaycastingCameraViewport& cameraViewport)
        : cameraViewport(cameraViewport)
    {}
    
    void Render(World &world, RaycastingCamera &cam)
    {
        if(play)
        {
            RasterizeWorldInTexture(cameraViewport.GetRenderTexture(), world, cam);
        }
        else
        {
            RenderStepByStep();
        }
    }
    void RenderStepByStep()
    {
        /// ...
    }

    void DrawGUI()
    {
        ImGui::Begin("Rendering");

            if(ImGui::Button("Play"))
            {
                play = true;
            } 
            ImGui::SameLine();
            if(ImGui::Button("Pause"))
            {
                play = false;
            }
            ImGui::SameLine();
            if(ImGui::Button(">> Step") && !play)
            {
                
            }

            // ***
            // Display renderStack step by step
            // ***

        ImGui::End();
    }

private:    
    bool play = true;

    RaycastingCameraViewport& cameraViewport;
};

