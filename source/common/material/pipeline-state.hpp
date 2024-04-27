#pragma once

#include <glad/gl.h>
#include <glm/vec4.hpp>
#include <json/json.hpp>

namespace our
{
    // There are some options in the render pipeline that we cannot control via shaders
    // such as blending, depth testing and so on
    // Since each material could require different options (e.g. transparent materials usually use blending),
    // we will encapsulate all these options into a single structure that will also be responsible for configuring OpenGL's pipeline
    struct PipelineState
    {
        // This set of pipeline options specifies whether face culling will be used or not and how it will be configured
        struct
        {
            bool enabled = false;
            GLenum culledFace = GL_BACK;
            GLenum frontFace = GL_CCW;
        } faceCulling;

        // This set of pipeline options specifies whether depth testing will be used or not and how it will be configured
        struct
        {
            bool enabled = false;
            GLenum function = GL_LEQUAL;
        } depthTesting;

        // This set of pipeline options specifies whether blending will be used or not and how it will be configured
        struct
        {
            bool enabled = false;
            GLenum equation = GL_FUNC_ADD;
            GLenum sourceFactor = GL_SRC_ALPHA;
            GLenum destinationFactor = GL_ONE_MINUS_SRC_ALPHA;
            glm::vec4 constantColor = {0, 0, 0, 0};
        } blending;

        // These options specify the color and depth mask which can be used to
        // prevent the rendering/clearing from modifying certain channels of certain targets in the framebuffer
        // The color mask determines which color components (red, green, blue, and alpha) are writable for each pixel during rendering operations.
        glm::bvec4 colorMask = {true, true, true, true}; // To know how to use it, check glColorMask
        // This variable is of type bool and represents whether depth values calculated during rasterization are allowed to modify the contents of the depth buffer.
        bool depthMask = true; // To know how to use it, check glDepthMask
                               // for transparent --> depthMask -> false as we don't want to change
        // the depth buffer which hold the depth(z-coordinate) of objects

        // This function should set the OpenGL options to the values specified by this structure
        // For example, if faceCulling.enabled is true, you should call glEnable(GL_CULL_FACE), otherwise, you should call glDisable(GL_CULL_FACE)
        void setup() const
        {
            // TODO: (Req 4) Write this function
            // When you enable front-face culling (glCullFace(GL_FRONT)), OpenGL will discard polygons that are facing towards the viewer,
            // i.e., those whose vertices are ordered in a counterclockwise direction when viewed from the camera.
            if (faceCulling.enabled)
            {
                glEnable(GL_CULL_FACE);
                // CCw means the front face is the face whose points are CCW and the
                // culled face is the face at the back where its points are CW
                glFrontFace(faceCulling.frontFace);
                glCullFace(faceCulling.culledFace);
            }
            else
                glDisable(GL_CULL_FACE);
            if (depthTesting.enabled)
            {
                glEnable(GL_DEPTH_TEST);
                // GL_LEQUAL
                // Draw if the incoming depth value is less than or equal to the stored depth value
                glDepthFunc(depthTesting.function);
            }
            else
                glDisable(GL_DEPTH_TEST);
            if (blending.enabled)
            {
                glEnable(GL_BLEND);
                // This means that the source color is multiplied by its alpha value,
                // and the destination color is multiplied by one minus the source alpha value. The resulting colors are then combined using addition.
                glBlendFunc(blending.sourceFactor, blending.destinationFactor);
                glBlendEquation(blending.equation);
                glBlendColor(blending.constantColor[0], blending.constantColor[1], blending.constantColor[2], blending.constantColor[3]);
            }
            else
                glDisable(GL_BLEND);
            glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
            glDepthMask(depthMask ? GL_TRUE : GL_FALSE);
        }

        // Given a json object, this function deserializes a PipelineState structure
        void deserialize(const nlohmann::json &data);
    };

}