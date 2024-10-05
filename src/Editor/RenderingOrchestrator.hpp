#pragma once

#include <imgui.h>
#include <vector>

#include "RaycastingCameraViewport.hpp"

class RenderingOrchestrator
{
public:
    RenderingOrchestrator(RaycastingCameraViewport& cameraViewport)
        : cameraViewport(cameraViewport)
    {}
    
    void Render(World &world, RaycastingCamera &cam)
    {
        auto& renderTexture = cameraViewport.GetRenderTexture();

        if(play)
        {
            InitilizeFrame(world, cam);

            if(rasterizer.IsRenderIterationRemains())
            {
                OneRenderItr(true);
            }

            while(rasterizer.IsRenderIterationRemains())
            {
                OneRenderItr(false);
            }
        }
        else if(stepByStepNewFrame)
        {
            stepByStepNewFrame = false;
            InitilizeFrame(world, cam);
            OneRenderItr(true);
        }
    }

    void InitilizeFrame(World &world, RaycastingCamera &cam)
    {
        auto& renderTexture = cameraViewport.GetRenderTexture();
        rasterizer.Reset(renderTexture.texture.width, renderTexture.texture.height, world, cam);

        if(rasterizingItrsTextures.size() > 0)
        {
            int width = rasterizingItrsTextures[0].texture.width;
            int height = rasterizingItrsTextures[0].texture.height;

            if(renderTexture.texture.width != width || renderTexture.texture.height != height)
            {
                for(RenderTexture2D& texture : rasterizingItrsTextures)
                {
                    UnloadRenderTexture(texture);
                    texture = { 0 };
                }
            }
        }

        if(rasterizingItrsTextures.size() != cam.maxRenderItr)
        {
            rasterizingItrsTextures.resize(cam.maxRenderItr, { 0 });
        }
    }

    void OneRenderItr(bool firstItr)
    {
        if(rasterizer.IsRenderIterationRemains())
        {
           auto& renderTexture = cameraViewport.GetRenderTexture();
     
            BeginTextureMode(renderTexture);

                if(firstItr)
                {
                    ClearBackground(MY_BLACK);
                }

                rasterizer.RenderIteration();
            EndTextureMode();

            auto& ctx = rasterizer.GetContext();
            assert(rasterizingItrsTextures.size() >= ctx.currentRenderItr);

            auto& texture = rasterizingItrsTextures.at(ctx.currentRenderItr - 1);

            if(texture.id == 0)
            {
                texture = LoadRenderTexture(renderTexture.texture.width, renderTexture.texture.height);
            }

            BeginTextureMode(texture);
                DrawTexture(renderTexture.texture, 0, 0, WHITE);
            EndTextureMode();
        }
        else
        {
            stepByStepNewFrame = true;
        }
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
                stepByStepNewFrame = true;
            }
            ImGui::SameLine();
            if(ImGui::Button("> Step") && !play)
            {
                OneRenderItr(stepByStepNewFrame);
            }
            ImGui::SameLine();
            if(ImGui::Button(">> Step") && !play)
            {

            }

            {
                auto& ctx = rasterizer.GetContext();
            
                for(size_t i = 0; auto& texture : rasterizingItrsTextures)
                {
                    if(texture.id != 0 && i < ctx.currentRenderItr)
                    {
                        ImGui::Text("Iteration - %d", i);
                        rlImGuiImageRenderTextureFitWidth(&texture);
                    }

                    ++i;
                }
            }

        ImGui::End();
    }

private:    
    RaycastingCameraViewport& cameraViewport;

    // Step By step
    bool play = true;
    bool stepByStepNewFrame = false;

    WorldRasterizer rasterizer;

    std::vector<RenderTexture> rasterizingItrsTextures;
};

