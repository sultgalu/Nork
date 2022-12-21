#pragma once

#include "Vulkan/CommandBuffer.h"
#include "Vulkan/SwapChain.h"
#include "Core/InputState.h"
#include "Vulkan/Shaderc.h"
#include "Vulkan/Image.h"
#include "LoadUtils.h"

namespace Nork::Renderer {
    using namespace Vulkan;

    inline static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    inline static constexpr uint32_t MAX_IMG_ARR_SIZE = 10;
    inline static constexpr uint32_t DRAW_BATCH_COUNT = 5;
    inline static constexpr uint32_t INSTANCE_COUNT = 2;
    inline static constexpr uint32_t DRAW_COUNT = DRAW_BATCH_COUNT * INSTANCE_COUNT;
    inline static constexpr uint32_t INVALID_FRAME_IDX = UINT_MAX;


    struct SSBO
    {
        glm::mat4 model;
    };
    struct Vertex
    {
        using Self = Vertex;
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


    class State
    {
    public:
        State()
        {
            commandPool = std::make_shared<CommandPool>();
            createVertexBuffer();
            createIndexBuffer();
            vk::MemoryPropertyFlags hostVisibleFlags;
            {
                using enum vk::MemoryPropertyFlagBits;
                hostVisibleFlags = eHostVisible | eHostCoherent | eDeviceLocal;
            }
            drawCommandsBuffer = std::make_shared<Buffer>(sizeof(VkDrawIndexedIndirectCommand) * MAX_FRAMES_IN_FLIGHT * DRAW_BATCH_COUNT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                hostVisibleFlags, true);
            uboStride = PhysicalDevice::Instance().physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
            if (uboStride < sizeof(uint32_t)) // never, but maybe i'll change uint32_t for a larger type
                uboStride = sizeof(uint32_t);
            uniformBuffer = std::make_shared<Buffer>(uboStride * MAX_FRAMES_IN_FLIGHT * DRAW_COUNT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                hostVisibleFlags, true);
            storageBuffer = std::make_shared<Buffer>(sizeof(SSBO) * MAX_FRAMES_IN_FLIGHT * DRAW_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                hostVisibleFlags, true);
        }
        void createIndexBuffer()
        {
            uint32_t stride = sizeof(indices[0]) * indices.size();
            VkDeviceSize bufferSize = stride * drawRepeat;

            using enum vk::MemoryPropertyFlagBits;
            Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, eHostVisible | eHostCoherent, true);
            for (size_t i = 0; i < drawRepeat; i++)
            {
                memcpy((char*)stagingBuffer.Ptr() + stride * i, indices.data(), stride);
            }
            indexBuffer = std::make_shared<Buffer>(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                vk::MemoryPropertyFlagBits::eDeviceLocal);

            copyBuffer(stagingBuffer, *indexBuffer, bufferSize);
        }
        void createVertexBuffer()
        {
            VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            using enum vk::MemoryPropertyFlagBits;
            Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, eHostVisible | eHostCoherent);

            memcpy(stagingBuffer.memory->Map(), vertices.data(), (size_t)bufferSize);
            vertexBuffer = std::make_shared<Buffer>(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                vk::MemoryPropertyFlagBits::eDeviceLocal);

            copyBuffer(stagingBuffer, *vertexBuffer, bufferSize);
        }
        void copyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, VkDeviceSize size)
        {
            auto cmdBuf = CommandBuilder(*commandPool)
                .BeginCommands()
                .CopyBuffer(srcBuffer, dstBuffer, size)
                .EndCommands();

            vk::SubmitInfo submitInfo;
            submitInfo.commandBufferCount = 1;
            vk::CommandBuffer cmdbuf = cmdBuf.handle;
            submitInfo.pCommandBuffers = &cmdbuf;

            Device::Instance().graphicsQueue.submit(submitInfo);
            Device::Instance().graphicsQueue.waitIdle();

            commandPool->FreeCommandBuffer(cmdBuf);
        }

        ~State()
        {

        }
        void updateUniformBuffer(uint32_t currentFrame)
        {
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            //glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            static auto offs = 1.0f;
            static auto pos = glm::vec3(0, 0.5, -1);
            if (Input::Instance().IsDown(Key::Up)) pos.y += 0.1f;
            if (Input::Instance().IsDown(Key::Down)) pos.y -= 0.1f;
            if (Input::Instance().IsDown(Key::Right)) pos.x += 0.1f;
            if (Input::Instance().IsDown(Key::Left)) pos.x -= 0.1f;
            if (Input::Instance().IsDown(Key::W)) pos.z += 0.1f;
            if (Input::Instance().IsDown(Key::S)) pos.z -= 0.1f;
            if (Input::Instance().IsDown(Key::D)) offs *= 0.9f;
            if (Input::Instance().IsDown(Key::A)) offs *= 1.1f;
            auto camPos = glm::translate(glm::identity<glm::mat4>(), pos);
            // model *= glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.1, 0.1, 0.1));
            glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), SwapChain::Instance().Width() / (float)SwapChain::Instance().Height(), 0.1f, 1000.0f);
            //proj[1][1] *= -1;
            vp = proj * view * camPos;
            auto models = &((glm::mat4*)storageBuffer->Ptr())[currentFrame * DRAW_COUNT];
            auto modelIdxs = &((char*)uniformBuffer->Ptr())[currentFrame * DRAW_COUNT * uboStride];
            for (size_t i = 0; i < DRAW_COUNT; i++)
            {
                *(uint32_t*)(&modelIdxs[i * uboStride]) = currentFrame * DRAW_COUNT + i;
                models[i] = glm::translate(glm::identity<glm::mat4>(), glm::vec3(objOffset * i, 0, 0));
            }

            auto drawCmds = (VkDrawIndexedIndirectCommand*)drawCommandsBuffer->Ptr() + DRAW_BATCH_COUNT * currentFrame;
            for (size_t i = 0; i < DRAW_BATCH_COUNT; i++)
            {
                auto& drawCmd = drawCmds[i];
                drawCmd.indexCount = indices.size();
                drawCmd.instanceCount = INSTANCE_COUNT;
                drawCmd.firstIndex = 0;
                drawCmd.vertexOffset = 0;
                drawCmd.firstInstance = i * INSTANCE_COUNT;
            }
        }
    public:
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
        const std::vector<float> quad = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
        };
        const std::vector<uint16_t> indices = {
         0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
        };
        uint32_t drawRepeat = 1;
        float objOffset = 1.0f;
        std::shared_ptr<Buffer> vertexBuffer;
        std::shared_ptr<Buffer> indexBuffer;

        glm::mat4 vp;
        uint32_t uboStride;
        std::shared_ptr<Buffer> uniformBuffer;
        std::shared_ptr<Buffer> storageBuffer; // 268 MB is MAX for DeviceLocal HostVisible Coherent
        std::shared_ptr<Buffer> drawCommandsBuffer;

        std::shared_ptr<CommandPool> commandPool; // dup
    };
	class RenderPass_
	{
	public:
        RenderPass_()
        {
            commandPool = std::make_shared<CommandPool>();
        }
        std::vector<uint32_t> LoadShader(const fs::path& srcFile, std::vector<std::array<std::string, 2>> macros = {})
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

        auto data = Shaderc::Compile(src, type, macros);
        // save with hash
        data.resize(data.size() + 2);
        *((size_t*)&data.data()[data.size() - 2]) = srcHash;
        FileUtils::WriteBinary(data, binFile.string());
        // remove hash
        data.resize(data.size() - 2);
        return data;
    }
        std::unordered_map<VkDescriptorType, uint32_t> DescriptorCounts(const std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts)
        {
            std::unordered_map<VkDescriptorType, uint32_t> result;
            for (auto& layout : layouts)
                for (auto& binding : layout->bindings)
                    result[binding.descriptorType] += binding.descriptorCount;
            return result;
        }
        std::shared_ptr<Vulkan::Image> createTextureImage(const std::string& path)
        {
            auto image = Renderer::LoadUtils::LoadImage(path, true);
            auto imgSize = image.data.size();

            using enum vk::MemoryPropertyFlagBits;
            Buffer stagingBuffer(imgSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, eHostVisible | eHostCoherent);
            memcpy(stagingBuffer.memory->Map(), image.data.data(), imgSize);

            auto texImg = std::make_shared<Vulkan::Image>(ImageCreateInfo(image.width, image.height, Format::rgba8Unorm,
                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled), eDeviceLocal);

            transitionImageLayout(*texImg.operator*(), (VkFormat)texImg->Format(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer, *texImg, image.width, image.height);
            transitionImageLayout(**texImg, (VkFormat)texImg->Format(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            return texImg;
        }
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
        {
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

            auto cmdBuf = CommandBuilder(*commandPool)
                .BeginCommands()
                .PipelineBarrier(sourceStage, destinationStage, std::vector<VkImageMemoryBarrier>{ barrier })
                .EndCommands();

            endSingleTimeCommands(cmdBuf);
        }
        void copyBufferToImage(const Buffer& buffer, const Vulkan::Image& image, uint32_t width, uint32_t height)
        {
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

            auto cmdBuf = CommandBuilder(*commandPool)
                .BeginCommands()
                .CopyBufferToImage(buffer, image, region)
                .EndCommands();

            endSingleTimeCommands(cmdBuf);
        }
        void endSingleTimeCommands(CommandBuffer cmdBuffer)
        {
            vk::SubmitInfo submitInfo;
            submitInfo.commandBufferCount = 1;
            vk::CommandBuffer cmdbuf = cmdBuffer.handle;
            submitInfo.pCommandBuffers = &cmdbuf;

            Device::Instance().graphicsQueue.submit(submitInfo);
            Device::Instance().graphicsQueue.waitIdle();

            commandPool->FreeCommandBuffer(cmdBuffer);
        }

        virtual void recordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t currentFrame, State& state) = 0;
public:
        std::shared_ptr<CommandPool> commandPool; // dup
	};
    class DeferredPass: public RenderPass_
    {
    public:
        DeferredPass(State& state)
        {
            createRenderPass();

            textureSampler = std::make_shared<Sampler>();

            auto w = SwapChain::Instance().Width();
            auto h = SwapChain::Instance().Height();
            using enum vk::ImageUsageFlagBits;
            using Image = Vulkan::Image;
            auto depthImage_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::depth32, eDepthStencilAttachment),
                vk::MemoryPropertyFlagBits::eDeviceLocal);
            depthImage = std::make_shared<ImageView>(ImageViewCreateInfo(depthImage_, vk::ImageAspectFlagBits::eDepth), textureSampler);
            auto fbColor_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eInputAttachment | eTransferSrc | eSampled),
                vk::MemoryPropertyFlagBits::eDeviceLocal);
            fbColor = std::make_shared<ImageView>(ImageViewCreateInfo(fbColor_, vk::ImageAspectFlagBits::eColor), textureSampler);

            auto gPos_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eInputAttachment),
                vk::MemoryPropertyFlagBits::eDeviceLocal);
            gPos = std::make_shared<ImageView>(ImageViewCreateInfo(gPos_, vk::ImageAspectFlagBits::eColor), textureSampler);
            auto gCol_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eInputAttachment),
                vk::MemoryPropertyFlagBits::eDeviceLocal);
            gCol = std::make_shared<ImageView>(ImageViewCreateInfo(gCol_, vk::ImageAspectFlagBits::eColor), textureSampler);
            fb = std::make_shared<Framebuffer>(w, h, *renderPass, std::vector<std::shared_ptr<ImageView>>{ gPos, gCol, fbColor, depthImage });
            // Imgui
            // auto uiImg = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eTransferSrc),
            //     vk::MemoryPropertyFlagBits::eDeviceLocal);
            // auto uiImgView = std::make_shared<ImageView>(ImageViewCreateInfo(uiImg, vk::ImageAspectFlagBits::eColor), textureSampler);
            // fbUI = std::make_shared<Framebuffer>(w, h, *renderPassUI, std::vector<std::shared_ptr<ImageView>>{ uiImgView });

            descriptorSetLayoutGPass = DescriptorSetLayout::Builder()
                .Binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT)
                .Binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .Binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_IMG_ARR_SIZE, true)
                .Build();
            descriptorSetLayoutLPass = DescriptorSetLayout::Builder()
                .Binding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Binding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build();
            descriptorSetLayoutPP = DescriptorSetLayout::Builder()
                .Binding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build();

            //descriptorPool = std::make_shared<DescriptorPool>(*descriptorSetLayoutGPass);
            descriptorPool = std::make_shared<DescriptorPool>(DescriptorCounts({
                descriptorSetLayoutGPass, descriptorSetLayoutLPass, descriptorSetLayoutPP
                }), 3);
            descriptorSet = std::make_shared<DescriptorSet>(*descriptorPool, *descriptorSetLayoutGPass, MAX_IMG_ARR_SIZE);
            descriptorSetLPass = std::make_shared<DescriptorSet>(*descriptorPool, *descriptorSetLayoutLPass);
            descriptorSetPP = std::make_shared<DescriptorSet>(*descriptorPool, *descriptorSetLayoutPP);
            createGraphicsPipeline();

            textureImage = createTextureImage("texture.png");
            textureImage2 = createTextureImage("clown.png");
            textureView = std::make_shared<ImageView>(ImageViewCreateInfo(textureImage, vk::ImageAspectFlagBits::eColor), textureSampler);
            textureView2 = std::make_shared<ImageView>(ImageViewCreateInfo(textureImage2, vk::ImageAspectFlagBits::eColor), textureSampler);

            auto writer = descriptorSet->Writer()
                .Buffer(0, *state.uniformBuffer, 0, sizeof(uint32_t), true)
                .Buffer(1, *state.storageBuffer, 0, state.storageBuffer->memory->allocInfo.allocationSize);
            for (size_t i = 0; i < MAX_IMG_ARR_SIZE; i++)
            {
                writer.Image(2, i % 3 ? *textureView2 : *textureView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i);
            }
            writer.Write();
            descriptorSetLPass->Writer()
                .Image(0, *gPos, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
                .Image(1, *gCol, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
                .Write();
            descriptorSetPP->Writer()
                .Image(0, *fbColor, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
                .Write();
        }
        void createGraphicsPipeline()
        {
            ShaderModule vertShaderModule(LoadShader("Source/Shaders/shader.vert", { {"MAX_IMG_ARR_SIZE", std::to_string(MAX_IMG_ARR_SIZE)} }), VK_SHADER_STAGE_VERTEX_BIT);
            ShaderModule fragShaderModule(LoadShader("Source/Shaders/shader.frag"), VK_SHADER_STAGE_FRAGMENT_BIT);
            VkPushConstantRange p{};
            p.size = sizeof(glm::mat4);
            p.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            PipelineInfo info = PipelineInfo()
                .AddShader(vertShaderModule)
                .AddShader(fragShaderModule)
                .VertexInput<Vertex>()
                .InputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                .Rasterization(false) // TRUE
                .Multisampling()
                .ColorBlend(2)
                .Layout(descriptorSetLayoutGPass->handle, { p })
                .DepthStencil(true, true, VK_COMPARE_OP_LESS);
            pipelineGPass = std::make_shared<Pipeline>(info, *renderPass, 0);

            ShaderModule vertShaderModule3(LoadShader("Source/Shaders/pp.vert"), VK_SHADER_STAGE_VERTEX_BIT);
            ShaderModule fragShaderModule3(LoadShader("Source/Shaders/lPass.frag"), VK_SHADER_STAGE_FRAGMENT_BIT);
            PipelineInfo info3 = PipelineInfo()
                .AddShader(vertShaderModule3)
                .AddShader(fragShaderModule3)
                .VertexInputHardCoded()
                .InputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                .Rasterization(false)
                .Multisampling()
                .ColorBlend(1)
                .Layout(descriptorSetLayoutLPass->handle)
                .DepthStencil(false);
            pipelineLPass = std::make_shared<Pipeline>(info3, *renderPass, 1);

            ShaderModule vertShaderModule2(LoadShader("Source/Shaders/pp.vert"), VK_SHADER_STAGE_VERTEX_BIT);
            ShaderModule fragShaderModule2(LoadShader("Source/Shaders/pp.frag"), VK_SHADER_STAGE_FRAGMENT_BIT);
            PipelineInfo info2 = PipelineInfo()
                .AddShader(vertShaderModule2)
                .AddShader(fragShaderModule2)
                .VertexInputHardCoded()
                .InputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                .Rasterization(false)
                .Multisampling()
                .ColorBlend(1)
                .Layout(descriptorSetLayoutPP->handle)
                .DepthStencil(false);
            pipelinePP = std::make_shared<Pipeline>(info2, *renderPass, 2);
        }
        void createRenderPass()
        {
            uint32_t gPosAtt = 0, gColAtt = 1, lPassAtt = 2, depthAtt = 3;
            RenderPass::Config config(4, 3);
            config.Attachment(gPosAtt, AttachmentDescription::ColorInternalUse((VkFormat)Format::rgba8Unorm).FinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
            config.Attachment(gColAtt, AttachmentDescription::ColorInternalUse((VkFormat)Format::rgba8Unorm).FinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
            config.Attachment(lPassAtt, AttachmentDescription::ColorForLaterCopy((VkFormat)Format::rgba8Unorm).FinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
            config.Attachment(depthAtt, AttachmentDescription::CleanDepth());

            auto gPass = SubPass(0)
                .ColorAttachment(gPosAtt)
                .ColorAttachment(gColAtt)
                .DepthAttachment(depthAtt);
            auto lPass = SubPass(1)
                .InputAttachment(gPosAtt, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                .InputAttachment(gColAtt, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                .ColorAttachment(lPassAtt);
            auto ppPass = SubPass(2)
                .ColorAttachment(lPassAtt, VK_IMAGE_LAYOUT_GENERAL)
                .InputAttachment(lPassAtt, VK_IMAGE_LAYOUT_GENERAL);

            config.AddSubPass(gPass)
                .AddSubPass(lPass)
                .AddSubPass(ppPass);

            config.DependencyExternalSrc(gPass, SubPassDependency()
                .SrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
                .DstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
                .DstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT));
            config.Dependency(gPass, lPass, SubPassDependency()
                .SrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                .SrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                .DstStageMask(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
                .DstAccessMask(VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)
                .Flags_ByRegion());
            config.Dependency(lPass, ppPass, SubPassDependency()
                .SrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
                .SrcAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                .DstStageMask(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
                .DstAccessMask(VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)
                .Flags_ByRegion());

            renderPass = std::make_shared<RenderPass>(config);
        }
        void recordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t currentFrame, State& state) override
        {
            auto builder = CommandBuilder(commandBuffer)
                .BeginRenderPass(*fb, *renderPass)
                .Viewport().Scissor()

                .BindPipeline(*pipelineGPass)
                .PushConstants(*pipelineGPass, VK_SHADER_STAGE_VERTEX_BIT, &state.vp, sizeof(glm::mat4))
                .BindVB(*state.vertexBuffer)
                .BindIB(*state.indexBuffer, VK_INDEX_TYPE_UINT16)
                .BindDescriptorSet(pipelineGPass->layoutHandle, *descriptorSet, { currentFrame * state.uboStride * DRAW_COUNT })
                //.DrawIndexed(indices.size(), 1'000'000)
                .DrawIndexedIndirect(*state.drawCommandsBuffer, DRAW_BATCH_COUNT, sizeof(VkDrawIndexedIndirectCommand) * currentFrame * DRAW_BATCH_COUNT)

                .NextSubPass()
                .BindPipeline(*pipelineLPass)
                .BindDescriptorSet(pipelineLPass->layoutHandle, *descriptorSetLPass)
                .DrawQuad()

                .NextSubPass()
                .BindPipeline(*pipelinePP)
                .BindDescriptorSet(pipelinePP->layoutHandle, *descriptorSetPP)
                .DrawQuad()
                .EndRenderPass();
        }
    public:

        std::shared_ptr<RenderPass> renderPass;

        std::shared_ptr<ImageView> depthImage;
        std::shared_ptr<ImageView> gPos;
        std::shared_ptr<ImageView> gCol;
        std::shared_ptr<ImageView> fbColor;
        std::shared_ptr<Framebuffer> fb;

        VkDescriptorPool imguiPool;
        std::shared_ptr<DescriptorPool> descriptorPool;

        std::shared_ptr<DescriptorSetLayout> descriptorSetLayoutGPass;
        std::shared_ptr<DescriptorSet> descriptorSet;
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayoutPP;
        std::shared_ptr<DescriptorSet> descriptorSetPP;
        std::shared_ptr<DescriptorSetLayout> descriptorSetLayoutLPass;
        std::shared_ptr<DescriptorSet> descriptorSetLPass;

        std::shared_ptr<Pipeline> pipelineGPass;
        std::shared_ptr<Pipeline> pipelineLPass;
        std::shared_ptr<Pipeline> pipelinePP;

        std::shared_ptr<Sampler> textureSampler;
        std::shared_ptr<Vulkan::Image> textureImage;
        std::shared_ptr<Vulkan::Image> textureImage2;
        std::shared_ptr<ImageView> textureView;
        std::shared_ptr<ImageView> textureView2;
    };
    class EditorPass : public RenderPass_
    {
    public:     
        EditorPass()
        {
        }
        void recordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t currentFrame, State& state) override
        {
            auto builder = commandBuffer.CommandBuilder();
            Editor::Editor::Get().RenderPassUI(builder);

            auto& uiImg = *Editor::Editor::Get().fbUI->attachments[0]->Image();
            builder.PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                { // already converted to eTransferSrcOptimal by renderpass (attachment config)
                    ImageMemoryBarrier(*uiImg, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eTransferSrcOptimal,
                        vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eTransferRead),
                    ImageMemoryBarrier(SwapChain::Instance().images[imageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                        vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite),
                })
                .CopyImage(*uiImg, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, SwapChain::Instance().images[imageIndex],
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, { SwapChain::Instance().Width(), SwapChain::Instance().Height(), 1 })
                .PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    {
                        //fbColor->Barrier(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        //    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT),
                        ImageMemoryBarrier(SwapChain::Instance().images[imageIndex], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
                            vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eNone),
                    });
        }

    public:
    };
	class RenderLoop
	{
	public:
        RenderLoop()
        {
            createSyncObjects();
            commandPool = std::make_shared<CommandPool>();
            commandBuffers = commandPool->CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
            renderPasses.push_back(std::make_shared<DeferredPass>(state));
            renderPasses.push_back(std::make_shared<EditorPass>());
        }
        ~RenderLoop()
        {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(*Device::Instance(), renderFinishedSemaphores[i], nullptr);
                vkDestroySemaphore(*Device::Instance(), imageAvailableSemaphores[i], nullptr);
                vkDestroyFence(*Device::Instance(), inFlightFences[i], nullptr);
            }
        }
        uint32_t BeginFrame()
        {
            static Timer sumT;
            static float elapsed = 0;

            Timer t;
            vkWaitForFences(*Device::Instance(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
            vkResetFences(*Device::Instance(), 1, &inFlightFences[currentFrame]);
            elapsed += t.Elapsed();

            vkResetCommandBuffer(commandBuffers[currentFrame].handle, 0);
            state.updateUniformBuffer(currentFrame);

            auto result = SwapChain::Instance().acquireNextImage(UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE);
            if (sumT.ElapsedSeconds() >= 1.0f)
            {
                Logger::Info("\t", elapsed, " ms (", 100 * elapsed / sumT.Elapsed(), "%)");
                sumT.Restart();
                elapsed = 0;
            }

            if (result.first == vk::Result::eErrorOutOfDateKHR)
            {
                //swapChain->recreateSwapChain();
                return INVALID_FRAME_IDX;
            }
            else if (result.first != vk::Result::eSuccess && result.first != vk::Result::eSuboptimalKHR)
            {
                throw std::runtime_error("failed to acquire swap chain image!");
            }
            return result.second;
        }
        void EndFrame(uint32_t imgIdx)
        {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers[currentFrame].handle;

            VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            vkQueueSubmit(*Device::Instance().graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) == VkSuccess();

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = { *SwapChain::Instance() };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imgIdx;

            presentInfo.pResults = nullptr; // Optional

            auto res = Device::Instance().graphicsQueue.presentKHR(presentInfo);
            if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || framebufferResized)
            {
                framebufferResized = false;
                //swapChain->recreateSwapChain();
            }
            else if (res != vk::Result::eSuccess)
            {
                throw std::runtime_error("failed to present swap chain image!");
            }
            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
        void DrawFrame()
        {
            auto imgIdx = BeginFrame();
            if (imgIdx == INVALID_FRAME_IDX)
                return;
            commandBuffers[currentFrame].CommandBuilder().BeginCommands();
            for (auto& pass : renderPasses)
            {
                pass->recordCommandBuffer(commandBuffers[currentFrame], imgIdx, currentFrame, state);
                //recordCommandBuffer(commandBuffers[currentFrame], imgIdx);
            }
            commandBuffers[currentFrame].CommandBuilder().EndCommands();
            EndFrame(imgIdx);
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
                vkCreateSemaphore(*Device::Instance(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VkSuccess();
                vkCreateSemaphore(*Device::Instance(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VkSuccess();
                vkCreateFence(*Device::Instance(), &fenceInfo, nullptr, &inFlightFences[i]) == VkSuccess();
            }
        }
	public:
        State state;
        std::vector<std::shared_ptr<RenderPass_>> renderPasses;
        std::vector<CommandBuffer> commandBuffers;
        std::shared_ptr<CommandPool> commandPool;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        uint32_t currentFrame = 0;
        bool framebufferResized = false;
	};

	class Frame
	{
	public:

	public:
	};
}