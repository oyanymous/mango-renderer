#include "VulkanDevice.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

#include "VulkanBindGroup.h"
#include "VulkanBuffer.h"
#include "VulkanCommandQueue.h"
#include "VulkanFence.h"
#include "VulkanPipeline.h"
#include "VulkanShaderModule.h"
#include "VulkanSwapchain.h"
#include "VulkanTexture.h"


namespace {

constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*) {
    const char* level = severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ? "error" : "warning";
    std::cerr << "[vulkan " << level << "] " << data->pMessage << '\n';
    return VK_FALSE;
}

bool hasExtension(const std::vector<vk::ExtensionProperties>& extensions, const char* name) {
    return std::any_of(extensions.begin(), extensions.end(), [name](const vk::ExtensionProperties& e) {
        return std::strcmp(e.extensionName, name) == 0;
    });
}

bool hasLayer(const char* name) {
    const auto layers = vk::enumerateInstanceLayerProperties();
    return std::any_of(layers.begin(), layers.end(), [name](const vk::LayerProperties& l) {
        return std::strcmp(l.layerName, name) == 0;
    });
}

}


void DeletionQueue::push(uint64_t timeline_value, DestroyFn fn) {
    m_entries.push_back({timeline_value, std::move(fn)});
}

void DeletionQueue::flush(uint64_t completed_value) {
    // Entries are pushed with non-decreasing timeline values.
    while (!m_entries.empty() && m_entries.front().timeline_value <= completed_value) {
        DestroyFn fn = std::move(m_entries.front().fn);
        m_entries.pop_front();
        fn();
    }
}

void DeletionQueue::flushAll() {
    flush(UINT64_MAX);
}


VulkanDevice::VulkanDevice(const DeviceDesc& desc) {
    // Loads vulkan-1.dll / libvulkan and resolves vkGetInstanceProcAddr.
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    createInstance(desc);

    vk::SurfaceKHR probe_surface;
    if (desc.surface.window || desc.surface.layer)
        probe_surface = vkutil::createSurface(m_instance, desc.surface);
    pickPhysicalDevice(probe_surface);
    if (probe_surface)
        m_instance.destroySurfaceKHR(probe_surface);

    createLogicalDevice();
    createAllocator();
    createDescriptorPool();
    fillCapabilities();

    m_graphics_queue = std::make_unique<VulkanCommandQueue>(*this, m_graphics_family);
}

VulkanDevice::~VulkanDevice() {
    waitIdle();
    m_graphics_queue.reset();

    m_device.destroyDescriptorPool(m_descriptor_pool);
    vmaDestroyAllocator(m_allocator);
    m_device.destroy();

    if (m_debug_messenger)
        m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger);
    m_instance.destroy();
}

void VulkanDevice::createInstance(const DeviceDesc& desc) {
    if (vk::enumerateInstanceVersion() < VK_API_VERSION_1_3)
        throw std::runtime_error("VulkanDevice: the Vulkan loader does not support Vulkan 1.3");

    const auto available = vk::enumerateInstanceExtensionProperties();

    std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME, vkutil::platformSurfaceExtension()};
    vk::InstanceCreateFlags flags;
    if (hasExtension(available, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        // Needed to see MoltenVK, which is not a fully conformant implementation.
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    }

    std::vector<const char*> layers;
    if (desc.enable_validation) {
        if (hasLayer(kValidationLayer))
            layers.push_back(kValidationLayer);
        else
            std::cerr << "[vulkan] validation requested but " << kValidationLayer
                      << " is not installed (install the Vulkan SDK)\n";

        if (hasExtension(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            m_debug_utils = true;
        }
    }

    vk::ApplicationInfo app_info(desc.application_name.c_str(), 1, "Mango", 1, VK_API_VERSION_1_3);
    m_instance = vk::createInstance(vk::InstanceCreateInfo(flags, &app_info, layers, extensions));
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

    if (m_debug_utils) {
        using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
        vk::DebugUtilsMessengerCreateInfoEXT messenger_info(
            {},
            Severity::eWarning | Severity::eError,
            Type::eGeneral | Type::eValidation | Type::ePerformance,
            reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(&debugCallback));
        m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(messenger_info);
    }
}

void VulkanDevice::pickPhysicalDevice(vk::SurfaceKHR probe_surface) {
    int best_score = -1;

    for (vk::PhysicalDevice device : m_instance.enumeratePhysicalDevices()) {
        const vk::PhysicalDeviceProperties properties = device.getProperties();
        if (properties.apiVersion < VK_API_VERSION_1_3)
            continue;

        auto features = device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features,
                                            vk::PhysicalDeviceVulkan13Features>();
        const auto& features12 = features.get<vk::PhysicalDeviceVulkan12Features>();
        const auto& features13 = features.get<vk::PhysicalDeviceVulkan13Features>();
        if (!features12.timelineSemaphore || !features13.dynamicRendering || !features13.synchronization2)
            continue;

        if (!hasExtension(device.enumerateDeviceExtensionProperties(), VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            continue;

        // A single queue family that does graphics, compute and (if we have a surface) present.
        const auto families = device.getQueueFamilyProperties();
        std::optional<uint32_t> family;
        for (uint32_t i = 0; i < families.size(); ++i) {
            const auto required = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;
            if ((families[i].queueFlags & required) != required)
                continue;
            if (probe_surface && !device.getSurfaceSupportKHR(i, probe_surface))
                continue;
            family = i;
            break;
        }
        if (!family)
            continue;

        int score = 1;
        if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            score = 3;
        else if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
            score = 2;

        if (score > best_score) {
            best_score = score;
            m_physical_device = device;
            m_graphics_family = *family;
        }
    }

    if (!m_physical_device)
        throw std::runtime_error("VulkanDevice: no GPU supports Vulkan 1.3 with dynamic rendering and presentation");
}

void VulkanDevice::createLogicalDevice() {
    std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    // MoltenVK requires the portability subset extension to be enabled when it is exposed.
    if (hasExtension(m_physical_device.enumerateDeviceExtensionProperties(), "VK_KHR_portability_subset")) {
        extensions.push_back("VK_KHR_portability_subset");
    }

    const float priority = 1.0f;
    vk::DeviceQueueCreateInfo queue_info({}, m_graphics_family, 1, &priority);

    vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features> chain;
    chain.get<vk::DeviceCreateInfo>().setQueueCreateInfos(queue_info).setPEnabledExtensionNames(extensions);
    chain.get<vk::PhysicalDeviceVulkan12Features>().setTimelineSemaphore(true);
    chain.get<vk::PhysicalDeviceVulkan13Features>().setDynamicRendering(true).setSynchronization2(true);

    m_device = m_physical_device.createDevice(chain.get<vk::DeviceCreateInfo>());
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);
}

void VulkanDevice::createAllocator() {
    VmaVulkanFunctions functions = {};
    functions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo info = {};
    info.vulkanApiVersion = VK_API_VERSION_1_3;
    info.physicalDevice = m_physical_device;
    info.device = m_device;
    info.instance = m_instance;
    info.pVulkanFunctions = &functions;

    if (vmaCreateAllocator(&info, &m_allocator) != VK_SUCCESS)
        throw std::runtime_error("VulkanDevice: vmaCreateAllocator failed");
}

void VulkanDevice::createDescriptorPool() {
    constexpr uint32_t kDescriptorsPerType = 1024;
    const std::vector<vk::DescriptorPoolSize> sizes = {
        {vk::DescriptorType::eUniformBuffer, kDescriptorsPerType},
        {vk::DescriptorType::eStorageBuffer, kDescriptorsPerType},
        {vk::DescriptorType::eSampledImage, kDescriptorsPerType},
        {vk::DescriptorType::eStorageImage, kDescriptorsPerType},
        {vk::DescriptorType::eSampler, kDescriptorsPerType},
        {vk::DescriptorType::eCombinedImageSampler, kDescriptorsPerType},
    };
    m_descriptor_pool = m_device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, kDescriptorsPerType, sizes));
}

void VulkanDevice::fillCapabilities() {
    const vk::PhysicalDeviceProperties properties = m_physical_device.getProperties();
    const vk::PhysicalDeviceLimits& limits = properties.limits;

    m_capabilities.device_name = properties.deviceName.data();
    m_capabilities.supports_compute = true;
    m_capabilities.supports_push_constants = true;
    m_capabilities.max_texture_size = limits.maxImageDimension2D;
    m_capabilities.max_bind_groups = limits.maxBoundDescriptorSets;
    m_capabilities.max_color_attachments = limits.maxColorAttachments;
    m_capabilities.uniform_buffer_alignment = static_cast<uint32_t>(limits.minUniformBufferOffsetAlignment);
    m_capabilities.clip_depth = ClipSpaceDepth::ZeroToOne;
    m_capabilities.framebuffer_origin_top_left = true;
}

ICommandQueue& VulkanDevice::graphicsQueue() {
    return *m_graphics_queue;
}

std::unique_ptr<ISwapchain> VulkanDevice::createSwapchain(const SwapchainDesc& desc) {
    return std::make_unique<VulkanSwapchain>(*this, desc);
}

std::unique_ptr<IBuffer> VulkanDevice::createBuffer(const BufferDesc& desc, const void* initial_data) {
    return std::make_unique<VulkanBuffer>(*this, desc, initial_data);
}

std::unique_ptr<ITexture> VulkanDevice::createTexture(const TextureDesc& desc) {
    return std::make_unique<VulkanTexture>(*this, desc);
}

std::unique_ptr<ISampler> VulkanDevice::createSampler(const SamplerDesc& desc) {
    return std::make_unique<VulkanSampler>(*this, desc);
}

std::unique_ptr<IShaderModule> VulkanDevice::createShaderModule(const ShaderModuleDesc& desc) {
    return std::make_unique<VulkanShaderModule>(*this, desc);
}

std::unique_ptr<IBindGroupLayout> VulkanDevice::createBindGroupLayout(const BindGroupLayoutDesc& desc) {
    return std::make_unique<VulkanBindGroupLayout>(*this, desc);
}

std::unique_ptr<IBindGroup> VulkanDevice::createBindGroup(const BindGroupDesc& desc) {
    return std::make_unique<VulkanBindGroup>(*this, desc);
}

std::unique_ptr<IPipelineLayout> VulkanDevice::createPipelineLayout(const PipelineLayoutDesc& desc) {
    return std::make_unique<VulkanPipelineLayout>(*this, desc);
}

std::unique_ptr<IGraphicsPipeline> VulkanDevice::createGraphicsPipeline(const GraphicsPipelineDesc& desc) {
    return std::make_unique<VulkanGraphicsPipeline>(*this, desc);
}

std::unique_ptr<IComputePipeline> VulkanDevice::createComputePipeline(const ComputePipelineDesc& desc) {
    return std::make_unique<VulkanComputePipeline>(*this, desc);
}

std::unique_ptr<IFence> VulkanDevice::createFence(bool signaled) {
    return std::make_unique<VulkanFence>(*this, signaled);
}

void VulkanDevice::waitIdle() {
    m_device.waitIdle();
    m_deletion_queue.flushAll();
}

void VulkanDevice::deferDestroy(DestroyFn fn) {
    // +1 also protects resources recorded into a command buffer that has not been submitted yet.
    m_deletion_queue.push(m_graphics_queue->lastSubmittedValue() + 1, std::move(fn));
}

void VulkanDevice::collectGarbage() {
    m_deletion_queue.flush(m_graphics_queue->completedValue());
}

void VulkanDevice::immediateSubmit(const std::function<void(vk::CommandBuffer)>& record) {
    vk::CommandBufferAllocateInfo info(m_graphics_queue->commandPool(), vk::CommandBufferLevel::ePrimary, 1);
    vk::CommandBuffer command_buffer = m_device.allocateCommandBuffers(info).front();

    command_buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    record(command_buffer);
    command_buffer.end();

    m_graphics_queue->submitAndWait(command_buffer);
    m_device.freeCommandBuffers(m_graphics_queue->commandPool(), command_buffer);
}

void VulkanDevice::setObjectName(vk::ObjectType type, uint64_t handle, const std::string& name) {
    if (!m_debug_utils)
        return;
    m_device.setDebugUtilsObjectNameEXT(vk::DebugUtilsObjectNameInfoEXT(type, handle, name.c_str()));
}
