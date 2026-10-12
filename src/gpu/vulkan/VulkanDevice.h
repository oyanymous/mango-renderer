#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>

#include "IGraphicsDevice.h"
#include "vulkan_helpers.h"

class VulkanCommandQueue;


// Destroys Vulkan objects only once the GPU work that may still reference them has finished.
// Each entry is tagged with the queue timeline value after which it is safe to destroy.
class DeletionQueue {
public:
    using DestroyFn = std::function<void()>;

    void push(uint64_t timeline_value, DestroyFn fn);
    // Runs every entry whose timeline value is <= completed_value.
    void flush(uint64_t completed_value);
    void flushAll();
private:
    struct Entry {
        uint64_t timeline_value;
        DestroyFn fn;
    };
    std::deque<Entry> m_entries;
};


class VulkanDevice final : public IGraphicsDevice {
public:
    using DestroyFn = DeletionQueue::DestroyFn;

    explicit VulkanDevice(const DeviceDesc& desc);
    ~VulkanDevice() override;

    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    [[nodiscard]] GraphicsAPI api() const override { return GraphicsAPI::Vulkan; }
    [[nodiscard]] const DeviceCapabilities& capabilities() const override { return m_capabilities; }
    ICommandQueue& graphicsQueue() override;

    std::unique_ptr<ISwapchain> createSwapchain(const SwapchainDesc& desc) override;
    std::unique_ptr<IBuffer> createBuffer(const BufferDesc& desc, const void* initial_data) override;
    std::unique_ptr<ITexture> createTexture(const TextureDesc& desc) override;
    std::unique_ptr<ISampler> createSampler(const SamplerDesc& desc) override;
    std::unique_ptr<IShaderModule> createShaderModule(const ShaderModuleDesc& desc) override;
    std::unique_ptr<IBindGroupLayout> createBindGroupLayout(const BindGroupLayoutDesc& desc) override;
    std::unique_ptr<IBindGroup> createBindGroup(const BindGroupDesc& desc) override;
    std::unique_ptr<IPipelineLayout> createPipelineLayout(const PipelineLayoutDesc& desc) override;
    std::unique_ptr<IGraphicsPipeline> createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    std::unique_ptr<IComputePipeline> createComputePipeline(const ComputePipelineDesc& desc) override;
    std::unique_ptr<IFence> createFence(bool signaled) override;

    void waitIdle() override;

    [[nodiscard]] vk::Instance instance() const { return m_instance; }
    [[nodiscard]] vk::PhysicalDevice physicalDevice() const { return m_physical_device; }
    [[nodiscard]] vk::Device native() const { return m_device; }
    [[nodiscard]] VmaAllocator allocator() const { return m_allocator; }
    [[nodiscard]] vk::DescriptorPool descriptorPool() const { return m_descriptor_pool; }
    [[nodiscard]] VulkanCommandQueue& queue() { return *m_graphics_queue; }

    // Schedules `fn` to run once all work submitted so far (and the next submit) has completed.
    void deferDestroy(DestroyFn fn);
    // Destroys everything in the deletion queue the GPU is done with. Called after each submit.
    void collectGarbage();

    // Records commands with `record`, submits them and blocks until they complete.
    void immediateSubmit(const std::function<void(vk::CommandBuffer)>& record);

    [[nodiscard]] bool debugUtilsEnabled() const { return m_debug_utils; }

    template <typename Handle>
    void setObjectName(Handle handle, const std::string& name) {
        setObjectName(Handle::objectType, (uint64_t) static_cast<typename Handle::CType>(handle), name);
    }
private:
    void createInstance(const DeviceDesc& desc);
    void pickPhysicalDevice(vk::SurfaceKHR probe_surface);
    void createLogicalDevice();
    void createAllocator();
    void createDescriptorPool();
    void fillCapabilities();
    void setObjectName(vk::ObjectType type, uint64_t handle, const std::string& name);

    vk::Instance m_instance;
    vk::DebugUtilsMessengerEXT m_debug_messenger;
    vk::PhysicalDevice m_physical_device;
    vk::Device m_device;
    VmaAllocator m_allocator = nullptr;
    vk::DescriptorPool m_descriptor_pool;
    std::unique_ptr<VulkanCommandQueue> m_graphics_queue;
    DeletionQueue m_deletion_queue;
    DeviceCapabilities m_capabilities;

    uint32_t m_graphics_family = 0;
    bool m_debug_utils = false;
};
