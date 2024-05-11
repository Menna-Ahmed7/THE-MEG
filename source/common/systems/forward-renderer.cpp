#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"

namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // TODO: (Req 10) Pick the correct pipeline state to draw the sky
            //  Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            //  We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            //////////////////////////
            skyPipelineState.depthTesting.enabled = true;
            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.depthTesting.function = GL_LEQUAL; // mshfahmo
            skyPipelineState.faceCulling.culledFace = GL_FRONT; // mshfahmo
            //////////////////////////

            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            // TODO: (Req 11) Create a framebuffer
            ////////////////////////////
            glGenFramebuffers(1, &postprocessFrameBuffer);
            ///////////////////////////
            // TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            //  Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            //  The depth format can be (Depth component with 24 bits).
            /////////////////////
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);
            ////////////////////
            // TODO: (Req 11) Unbind the framebuffer just to be safe
            //////////////////////
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            //////////////////////

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram *postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // testttttttttttt
            if (config.contains("addedTex"))
            {
                addedTex = texture_utils::loadImage(config.value<std::string>("addedTex", ""));
            }

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy()
    {
        // Delete all objects related to the sky
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World *world, bool grey_effect)
    {
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        for (auto entity : world->getEntities())
        {
            // If we hadn't found a camera yet, we look for a camera in this entity
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer)
            {
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if (command.material->transparent)
                {
                    transparentCommands.push_back(command);
                }
                else
                {
                    // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
            // if light component store it
            // this computes this first auto lightComp = entity->getComponent<LightComponent>()
            // then check if lightComp isn't null store it
            if (auto lightComp = entity->getComponent<LightComponent>(); lightComp)
            {
                lightSources.push_back(lightComp);
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;
        // TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        //  HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        /////////////////////////////////////////////////////////////////////////////////////
        auto owner = camera->getOwner();
        auto tfMatrix = owner->getLocalToWorldMatrix();
        glm::vec3 eye = tfMatrix * glm::vec4(0, 0, 0, 1);
        glm::vec3 center = tfMatrix * glm::vec4(0, 0, -1, 1);
        glm::vec3 cameraForward = glm::normalize(center - eye);
        /////////////////////////////////////////////////////////////////////////////////////
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  {
                      // TODO: (Req 9) Finish this function
                      //  HINT: the following return should return true "first" should be drawn before "second".
                      ///////////////////////////////////////////////////////
                      float dotProductFirst = glm::dot(cameraForward, first.center);
                      float dotProductSecond = glm::dot(cameraForward, second.center);
                      return dotProductFirst > dotProductSecond;
                      ///////////////////////////////////////////////////////
                  });

        // TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP
        ///////////////
        glm::mat4 P = camera->getProjectionMatrix(windowSize);
        glm::mat4 V = camera->getViewMatrix();
        glm::mat4 VP = P * V;
        ///////////////
        // TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        /////////////////
        glViewport(0, 0, windowSize.x, windowSize.y);
        /////////////////
        // TODO: (Req 9) Set the clear color to black and the clear depth to 1
        /////////////////////
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0);
        ////////////////////
        // TODO: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        ///////////////////
        glColorMask(true, true, true, true);
        glDepthMask(true);
        ///////////////////

        // If there is a postprocess material, bind the framebuffer
        if (postprocessMaterial)
        {
            // TODO: (Req 11) bind the framebuffer
            //////////////////////
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
            /////////////////////
        }

        // TODO: (Req 9) Clear the color and depth buffers
        /////////////////////
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /////////////////////

        // TODO: (Req 9) Draw all the opaque commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        ////////////////////////
        glm::mat4 MVP;
        for (const auto &command : opaqueCommands)
        {
            // Set up the material for rendering
            command.material->setup();

            // Set the transform uniform to the MVP matrix
            MVP = VP * command.localToWorld;
            // if the material of the object is lighted
            if (auto light_material = dynamic_cast<LitMaterial *>(command.material); light_material)
            {

                light_material->shader->set("VP", VP);
                light_material->shader->set("M", command.localToWorld);
                light_material->shader->set("eye", eye);
                light_material->shader->set("M_IT", glm::transpose(glm::inverse(command.localToWorld)));
                light_material->shader->set("light_count", (int)lightSources.size());

                for (int i = 0; i < (int)lightSources.size(); i++)
                {
                    if (lightSources[i]->lightType >= 0)
                    {
                        // calculate position and direction of the light source based on the object
                        glm::vec3 position = lightSources[i]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
                        glm::vec3 direction = lightSources[i]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, -1, 0, 0);

                        light_material->shader->set("lights[" + std::to_string(i) + "].direction", direction);
                        light_material->shader->set("lights[" + std::to_string(i) + "].color", lightSources[i]->color);
                        light_material->shader->set("lights[" + std::to_string(i) + "].type", lightSources[i]->lightType);
                        light_material->shader->set("lights[" + std::to_string(i) + "].position", position);
                        light_material->shader->set("lights[" + std::to_string(i) + "].diffuse", lightSources[i]->diffuse);
                        light_material->shader->set("lights[" + std::to_string(i) + "].specular", lightSources[i]->specular);
                        light_material->shader->set("lights[" + std::to_string(i) + "].attenuation", lightSources[i]->attenuation);
                        light_material->shader->set("lights[" + std::to_string(i) + "].cone_angles", lightSources[i]->cone_angles);
                    }
                }
            }
            else
            {
                command.material->shader->set("transform", MVP);
            }

            // Draw the mesh associated with the render command
            command.mesh->draw();
        }
        ////////////////////////

        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            // TODO: (Req 10) setup the sky material
            ///////////////////////
            this->skyMaterial->setup();
            //////////////////////
            // TODO: (Req 10) Get the camera position
            ////////////
            glm::vec3 cameraPosition = eye;
            ///////////
            // TODO: (Req 10) Create a model matrix for the sy such that it always follows the camera (sky sphere center = camera position)
            /////////////////////
            glm::mat4 modelMatrixSky = glm::translate(glm::mat4(1.0f), cameraPosition);
            /////////////////////
            // TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            //  We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f, // x        x
                0.0f, 1.0f, 0.0f, 0.0f, // y        y
                0.0f, 0.0f, 0.0f, 0.0f, // z   =    w
                0.0f, 0.0f, 1.0f, 1.0f  // w       w - z
            );
            // TODO: (Req 10) set the "transform" uniform
            /////////////////////////////////
            glm::mat4 skyTransform = alwaysBehindTransform * VP * modelMatrixSky; // *beltrteb*
            this->skyMaterial->shader->set("transform", skyTransform);
            /////////////////////////////////
            // TODO: (Req 10) draw the sky sphere
            ///////////////////////////
            this->skySphere->draw();
            ///////////////////////////
        }
        // TODO: (Req 9) Draw all the transparent commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        ////////////////////////////////
        for (auto trCommand : transparentCommands)
        {
            trCommand.material->setup();
            // same concept as opaqueCommands loop
            trCommand.material->shader->set("transform", VP * trCommand.localToWorld);
            trCommand.mesh->draw();
        }

        ////////////////////////////////

        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // TODO: (Req 11) Return to the default framebuffer
            //////////////
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            //////////////

            // TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            ///////////////////////////////////
            postprocessMaterial->setup();
            glBindVertexArray(this->postProcessVertexArray);

            if (addedTex)
                cout << "yaraaaab" << endl;

            // Test
            glActiveTexture(GL_TEXTURE1);
            addedTex->bind();
            postprocessMaterial->sampler->bind(1);
            postprocessMaterial->shader->set("additional_sampler", 1);
            /////////////
            ShaderProgram *postprocessShader = new ShaderProgram();
            // attach the vertex shader
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);

            // attach the fragment shader based on the effect type (fish eye or blur or power up or radial blur or vignette)
            if (grey_effect)
                postprocessShader->attach("assets/shaders/postprocess/grayscale.frag", GL_FRAGMENT_SHADER);
            else
                postprocessShader->attach("assets/shaders/postprocess/water.frag", GL_FRAGMENT_SHADER);

            // link the shader program
            postprocessShader->link();
            postprocessMaterial->shader = postprocessShader;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            //////////
            glDrawArrays(GL_TRIANGLES, 0, 3);
            /////////////////////////////////
            // if   there is a light material apply it
            if (lightMaterial)
            {
                lightMaterial->setup();
            }
        }
    }

}