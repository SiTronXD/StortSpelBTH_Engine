#include "pch.h"
#include "LightHandler.hpp"
#include "../application/Scene.hpp"

void LightHandler::updateLightBuffers(
    Scene* scene,
    ShaderInput& shaderInput,
    ShaderInput& animShaderInput,
    const UniformBufferID& allLightsInfoUB,
    const UniformBufferID& animAllLightsInfoUB,
    const StorageBufferID& lightBufferSB,
    const StorageBufferID& animLightBufferSB,
    const bool& hasAnimations)
{
    this->lightBuffer.clear();

    // Info about all lights in the shader
    AllLightsInfo lightsInfo{};

    // Loop through all ambient lights in scene
    auto ambientLightView = scene->getSceneReg().view<AmbientLight>(entt::exclude<Inactive>);
    ambientLightView.each([&](
        const AmbientLight& ambientLightComp)
        {
            // Create point light data
            LightBufferData lightData{};
            lightData.color = glm::vec4(ambientLightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.ambientLightsEndIndex++;
        }
    );

    // Loop through all directional lights in the scene
    lightsInfo.directionalLightsEndIndex = lightsInfo.ambientLightsEndIndex;
    auto directionalLightView = scene->getSceneReg().view<DirectionalLight>(entt::exclude<Inactive>);
    directionalLightView.each([&](
        const DirectionalLight& directionalLightComp)
        {
            // Create point light data
            LightBufferData lightData{};
            lightData.direction =
                glm::vec4(glm::normalize(directionalLightComp.direction), 1.0f);
            lightData.color =
                glm::vec4(directionalLightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.directionalLightsEndIndex++;
        }
    );

    // Loop through all point lights in scene
    lightsInfo.pointLightsEndIndex = lightsInfo.directionalLightsEndIndex;
    auto pointLightView = scene->getSceneReg().view<Transform, PointLight>(entt::exclude<Inactive>);
    pointLightView.each([&](
        Transform& transform,
        const PointLight& pointLightComp)
        {
            // Create point light data
            LightBufferData lightData{};
            lightData.position = glm::vec4(
                transform.position +
                transform.getRotationMatrix() * pointLightComp.positionOffset,
                1.0f);
            lightData.color = glm::vec4(pointLightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.pointLightsEndIndex++;
        }
    );

    // Loop through all spotlights in scene
    lightsInfo.spotlightsEndIndex = lightsInfo.pointLightsEndIndex;
    auto spotlightView = scene->getSceneReg().view<Transform, Spotlight>(entt::exclude<Inactive>);
    spotlightView.each([&](
        Transform& transform,
        const Spotlight& spotlightComp)
        {
            const glm::mat3 rotMat = transform.getRotationMatrix();

            // Create point light data
            LightBufferData lightData{};
            lightData.position = glm::vec4(
                transform.position +
                rotMat * spotlightComp.positionOffset,
                1.0f
            );
            lightData.direction = glm::vec4(
                glm::normalize(rotMat * spotlightComp.direction),
                std::cos(glm::radians(spotlightComp.angle * 0.5f))
            );
            lightData.color = glm::vec4(spotlightComp.color, 1.0f);

            // Add to list
            this->lightBuffer.push_back(lightData);

            // Increment end index
            lightsInfo.spotlightsEndIndex++;
        }
    );

    // Update storage buffer containing lights
    if (this->lightBuffer.size() > 0)
    {
        shaderInput.updateStorageBuffer(lightBufferSB, (void*)this->lightBuffer.data());
        if (hasAnimations)
        {
            animShaderInput.updateStorageBuffer(animLightBufferSB, (void*)this->lightBuffer.data());
        }
    }

    // Truncate indices to not overshoot max
    lightsInfo.ambientLightsEndIndex = std::min(
        lightsInfo.ambientLightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);
    lightsInfo.directionalLightsEndIndex = std::min(
        lightsInfo.directionalLightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);
    lightsInfo.pointLightsEndIndex = std::min(
        lightsInfo.pointLightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);
    lightsInfo.spotlightsEndIndex = std::min(
        lightsInfo.spotlightsEndIndex,
        LightHandler::MAX_NUM_LIGHTS);

#ifdef _CONSOLE
    if (this->lightBuffer.size() > LightHandler::MAX_NUM_LIGHTS)
    {
        Log::warning("The number of lights is larger than the maximum allowed number. Truncates " +
            std::to_string(this->lightBuffer.size()) + " lights to " +
            std::to_string(MAX_NUM_LIGHTS));
    }
#endif

    // Update all lights info buffer
    shaderInput.updateUniformBuffer(
        allLightsInfoUB,
        (void*)&lightsInfo
    );
    if (hasAnimations)
    {
        animShaderInput.updateUniformBuffer(
            animAllLightsInfoUB,
            (void*)&lightsInfo
        );
    }
}