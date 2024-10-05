#pragma once

#include <imgui.h>
#include <vector>

#include "RaycastingCameraViewport.hpp"
#include "Utils/DrawingHelper.hpp"

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
            AllRenderItr(world, cam);
        }
        else
        {
            switch(invokeEvent)
            {
                case StepInto: OneRenderItr(world, cam); break;
                case StepOver: AllRenderItr(world, cam); break;
            }

            invokeEvent = None;
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

    void AllRenderItr(World &world, RaycastingCamera &cam)
    {
        do
        {
            OneRenderItr(world, cam);
        } while(rasterizer.IsRenderIterationRemains());
    }

    void OneRenderItr(World &world, RaycastingCamera &cam)
    {
        if(!rasterizer.IsRenderIterationRemains())
        {
            InitilizeFrame(world, cam);
        }

        assert(rasterizer.IsRenderIterationRemains());

        auto& renderTexture = cameraViewport.GetRenderTexture();
        auto& ctx = rasterizer.GetContext();
    
        BeginTextureMode(renderTexture);

            if(ctx.currentRenderItr == 0)
            {
                ClearBackground(MY_BLACK);
            }

            rasterizer.RenderIteration();
            
        EndTextureMode();

        assert(rasterizingItrsTextures.size() >= ctx.currentRenderItr);
        assert(ctx.currentRenderItr > 0);

        auto& texture = rasterizingItrsTextures.at(ctx.currentRenderItr - 1);

        if(texture.id == 0)
        {
            texture = LoadRenderTexture(renderTexture.texture.width, renderTexture.texture.height);
        }

        BeginTextureMode(texture);
            DrawTextureFlippedY(renderTexture.texture, 0, 0, WHITE);
        EndTextureMode();
    }

    void DrawGUI()
    {
        ImGui::Begin("Rendering");

            constexpr const char* LabelPlay = "Play";
            constexpr const char* LabelPause = "Pause";

            if(ImGui::Button(!play ? LabelPlay : LabelPause))
            {
                play = !play;
            } 
            ImGui::SameLine();
            if(ImGui::Button("> Step") && !play)
            {
                invokeEvent = StepInto;
            }
            ImGui::SameLine();
            if(ImGui::Button(">> Step") && !play)
            {
                invokeEvent = StepOver;
            }

            // Render Iterations UI
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
    WorldRasterizer rasterizer;

    bool play = true;
    enum InvokeEvent { None, StepInto, StepOver };
    InvokeEvent invokeEvent { None };
    std::vector<RenderTexture> rasterizingItrsTextures;
};

