#include "pch.h"
#include "Core/CameraController.h"
#include "Utils/Logger.h"
#include "Utils/Timer.h"
#include "Components/Common.h"
#include "Editor/Editor.h"
#include "Core/NorkWindow.h"
#include "Core/Engine.h"
#include "App/Application.h"
#include "Shaderc.h"
#include "Modules/Renderer/LoadUtils.h"
#include "VulkanContext.h"
#include "Window.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "Sampler.h"
#include "Pipeline.h"
#include "DescriptorSet.h"

using namespace Nork;

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    static const std::array<VkVertexInputAttributeDescription, 3>& getAttributeDescriptions()
    {
        static auto val = getAttributeDescriptions_();
        return val;
    }
    static const VkVertexInputBindingDescription& getBindingDescription()
    {
        static auto val = getBindingDescription_();
        return val;
    }
private:
    static const VkVertexInputBindingDescription getBindingDescription_()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static const std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions_()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

class HelloTriangleApplication
{
public:
    void run()
    {
        mainLoop();
    }

    HelloTriangleApplication()
    {
        initVulkan();
    }
    ~HelloTriangleApplication()
    {
        vkDestroyRenderPass(device.device, swapChain.renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device.device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device.device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device.device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device.device, commandPool, nullptr);
    }
private:

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0; // TODO
        barrier.dstAccessMask = 0; // TODO
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer);
    }
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(commandBuffer);
    }
    void createTextureImage()
    {
        auto image = Renderer::LoadUtils::LoadImage("texture.jpg", true);
        auto imgSize = image.data.size();

        Buffer stagingBuffer(imgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        memcpy(stagingBuffer.memory->Map(0, imgSize), image.data.data(), imgSize);

        textureImage = std::make_shared<Image>(image.width, image.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
            textureSampler);

        transitionImageLayout(textureImage->handle, textureImage->imageInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        textureImage->layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyBufferToImage(stagingBuffer.handle, textureImage->handle, image.width, image.height);
        transitionImageLayout(textureImage->handle, textureImage->imageInfo.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        textureImage->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    void createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        memcpy(stagingBuffer.memory->Map(0, bufferSize), indices.data(), (size_t)bufferSize);
        indexBuffer = std::make_shared<Buffer>(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copyBuffer(stagingBuffer.handle, indexBuffer->handle, bufferSize);
    }
    void createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        memcpy(stagingBuffer.memory->Map(0, bufferSize), vertices.data(), (size_t)bufferSize);
        vertexBuffer = std::make_shared<Buffer>(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copyBuffer(stagingBuffer.handle, vertexBuffer->handle, bufferSize);
    }
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }
    void createDepthResources()
    {
        depthImage = std::make_shared<Image>(swapChain.swapChainExtent.width, swapChain.swapChainExtent.height, VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    std::vector<uint32_t> LoadShader(const fs::path& srcFile)
    {
        fs::path binFile = fs::path(srcFile).replace_extension(srcFile.extension().string() + "_bin");
        auto src = FileUtils::ReadAsString(srcFile.string());
        size_t srcHash = std::hash<std::string>()(src);
        if (fs::exists(binFile))
        { // check if hash is still the same, if so return saved binary
            auto data = FileUtils::ReadBinary<uint32_t>(binFile.string());
            size_t hash = *((size_t*)&data.data()[data.size() - 2]);
            if (srcHash == hash)
            {
                data.resize(data.size() - 2);
                return data;
            }
        }

        using namespace Renderer;
        ShaderType type = ShaderType::None;
        if (srcFile.extension() == ".vert")
            type = ShaderType::Vertex;
        else if (srcFile.extension() == ".frag")
            type = ShaderType::Fragment;
        else
            std::unreachable();

        auto data = Shaderc::Compile(src, type);
        // save with hash
        data.resize(data.size() + 2);
        *((size_t*)&data.data()[data.size() - 2]) = srcHash;
        FileUtils::WriteBinary(data, binFile.string());
        // remove hash
        data.resize(data.size() - 2);
        return data;
    }

    void createGraphicsPipeline()
    {
        ShaderModule vertShaderModule(LoadShader("Source/Shaders/shader.vert"), VK_SHADER_STAGE_VERTEX_BIT);
        ShaderModule fragShaderModule(LoadShader("Source/Shaders/shader.frag"), VK_SHADER_STAGE_FRAGMENT_BIT);

        PipelineInfo info = PipelineInfo()
            .AddShader(vertShaderModule)
            .AddShader(fragShaderModule)
            .VertexInput<Vertex>()
            .InputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .Rasterization()
            .Multisampling()
            .ColorBlend()
            .Layout(descriptorSetLayout->handle)
            .DepthStencil(true, true, VK_COMPARE_OP_LESS);
        pipeline = std::make_shared<Pipeline>(info, swapChain.renderPass);
    }
    
    void initVulkan()
    {
        createDepthResources();

        descriptorSetLayout = DescriptorSetLayout::Builder()
            .Binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .Binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
        descriptorPool = std::make_shared<DescriptorPool>(*descriptorSetLayout);
        descriptorSet = std::make_shared<DescriptorSet>(*descriptorPool, *descriptorSetLayout);
        createGraphicsPipeline();

        createCommandPool();

        textureSampler = std::make_shared<Sampler>();

        createVertexBuffer();
        createIndexBuffer();

        createTextureImage();
        uniformBuffer = std::make_shared<Buffer>(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
        descriptorSet->Writer()
            .Buffer(0, *uniformBuffer, 0, sizeof(glm::mat4))
            .Image(1, *textureImage)
            .Write();

        createCommandBuffer();
        createSyncObjects();
    }

    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }
    void endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.graphicsQueue);

        vkFreeCommandBuffers(device.device, commandPool, 1, &commandBuffer);
    }
    
    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // create as already signaled (first wait wont deadlock)

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VkSuccess();
            vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VkSuccess();
            vkCreateFence(device.device, &fenceInfo, nullptr, &inFlightFences[i]) == VkSuccess();
        }
    }
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional
        vkBeginCommandBuffer(commandBuffer, &beginInfo) == VkSuccess();

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain.renderPass;
        renderPassInfo.framebuffer = swapChain.swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChain.swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); 
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = swapChain.swapChainExtent.width;
        viewport.height = swapChain.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChain.swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { vertexBuffer->handle };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->handle, 0, VK_INDEX_TYPE_UINT16);
        
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layoutHandle, 0, 1, &descriptorSet->handle, 0, nullptr);
        
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        
        vkCmdEndRenderPass(commandBuffer);
        vkEndCommandBuffer(commandBuffer) == VkSuccess();
    }
    void createCommandBuffer()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
        vkAllocateCommandBuffers(device.device, &allocInfo, commandBuffers.data()) == VkSuccess();
    }
    void createCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = device.graphicsQueueFamily;

        vkCreateCommandPool(device.device, &poolInfo, nullptr, &commandPool) == VkSuccess();
    }
   
    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChain.swapChainExtent.width / (float)swapChain.swapChainExtent.height, 0.1f, 10.0f);
        proj[1][1] *= -1;
        glm::mat4 vp = proj * view * model;
        memcpy(uniformBuffer->Ptr(), &vp, sizeof(vp));
    }
    void mainLoop()
    {
        uint32_t frames = 0;
        Timer timer;
        while (!glfwWindowShouldClose(window.glfwWindow))
        {
            glfwPollEvents();
            drawFrame();
            frames++;
            auto seconds = timer.ElapsedSeconds();
            if (seconds > 1.0f)
            {
                auto fps = frames / seconds;
                Logger::Info(fps, " fps");
                timer = Timer();
                frames = 0;
            }
        }
        vkDeviceWaitIdle(device.device);
    }
    void drawFrame()
    {
        vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device.device, swapChain.swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            swapChain.recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        vkResetFences(device.device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);

        updateUniformBuffer(currentFrame);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) == VkSuccess();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain.swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        presentInfo.pResults = nullptr; // Optional
        
        result = vkQueuePresentKHR(device.presentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            framebufferResized = false;
            swapChain.recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
private:
    VulkanWindow window = VulkanWindow(800, 600);
    VulkanContext ctx = VulkanContext(window);
    Device device = Device(ctx);
    SwapChain swapChain = SwapChain(ctx, window, device);

    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::shared_ptr<DescriptorPool > descriptorPool;
    std::shared_ptr<DescriptorSet> descriptorSet;

    std::shared_ptr<Pipeline> pipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    std::shared_ptr<Image> depthImage;

    const int MAX_FRAMES_IN_FLIGHT = 2;
    
    bool framebufferResized = false;

    const std::vector<Vertex> vertices = {
     {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };
    const std::vector<uint16_t> indices = {
     0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
    };
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;

    std::shared_ptr<Buffer> uniformBuffer;

    
    std::shared_ptr<Image> textureImage;
    std::shared_ptr<Sampler> textureSampler;
};

int main()
{
    Logger::PushStream(std::cout);

    try
    {
        HelloTriangleApplication app;
        app.run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}