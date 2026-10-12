#include "VulkanBindGroup.h"

#include <algorithm>
#include <deque>
#include <stdexcept>
#include <vector>

#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"


VulkanBindGroupLayout::VulkanBindGroupLayout(VulkanDevice& device, const BindGroupLayoutDesc& desc)
    : m_device(device), m_desc(desc) {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    bindings.reserve(desc.entries.size());
    for (const BindGroupLayoutEntry& entry : desc.entries) {
        bindings.emplace_back(entry.binding, vkutil::toVk(entry.type), entry.count, vkutil::toVk(entry.visibility));
    }
    m_layout = m_device.native().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings));
}

VulkanBindGroupLayout::~VulkanBindGroupLayout() {
    // Only referenced while creating pipeline layouts and descriptor sets, never by the GPU.
    m_device.native().destroyDescriptorSetLayout(m_layout);
}

void VulkanBindGroupLayout::setDebugName(const std::string& name) {
    m_device.setObjectName(m_layout, name);
}


namespace {

VulkanBindGroupLayout& requireLayout(IBindGroupLayout* layout) {
    if (layout == nullptr)
        throw std::runtime_error("VulkanBindGroup: layout is required");
    return *static_cast<VulkanBindGroupLayout*>(layout);
}

}

VulkanBindGroup::VulkanBindGroup(VulkanDevice& device, const BindGroupDesc& desc)
    : m_device(device), m_layout(requireLayout(desc.layout)) {
    vk::DescriptorSetLayout set_layout = m_layout.native();
    m_set = m_device.native().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_device.descriptorPool(), set_layout)).front();

    const auto& layout_entries = m_layout.desc().entries;

    // Deques keep element addresses stable while the writes point into them.
    std::deque<vk::DescriptorBufferInfo> buffer_infos;
    std::deque<vk::DescriptorImageInfo> image_infos;
    std::vector<vk::WriteDescriptorSet> writes;

    for (const BindGroupEntry& entry : desc.entries) {
        auto layout_entry = std::find_if(layout_entries.begin(), layout_entries.end(), [&](const BindGroupLayoutEntry& e) {
            return e.binding == entry.binding;
        });
        if (layout_entry == layout_entries.end())
            throw std::runtime_error("VulkanBindGroup: binding not present in layout");

        vk::WriteDescriptorSet write(m_set, entry.binding, 0, 1, vkutil::toVk(layout_entry->type));

        switch (layout_entry->type) {
        case BindingType::UniformBuffer:
        case BindingType::StorageBuffer: {
            auto* buffer = static_cast<VulkanBuffer*>(entry.buffer);
            buffer_infos.emplace_back(buffer->native(), entry.offset, entry.range == 0 ? VK_WHOLE_SIZE : entry.range);
            write.setPBufferInfo(&buffer_infos.back());
            break;
        }
        case BindingType::SampledTexture:
        case BindingType::StorageTexture:
        case BindingType::CombinedTextureSampler:
        case BindingType::Sampler: {
            vk::DescriptorImageInfo info;
            if (entry.sampler)
                info.sampler = static_cast<VulkanSampler*>(entry.sampler)->native();
            if (entry.texture) {
                info.imageView = static_cast<VulkanTexture*>(entry.texture)->view();
                info.imageLayout = layout_entry->type == BindingType::StorageTexture
                    ? vk::ImageLayout::eGeneral
                    : vk::ImageLayout::eShaderReadOnlyOptimal;
            }
            image_infos.push_back(info);
            write.setPImageInfo(&image_infos.back());
            break;
        }
        }
        writes.push_back(write);
    }

    m_device.native().updateDescriptorSets(writes, {});
}

VulkanBindGroup::~VulkanBindGroup() {
    m_device.deferDestroy([device = m_device.native(), pool = m_device.descriptorPool(), set = m_set] {
        device.freeDescriptorSets(pool, set);
    });
}

void VulkanBindGroup::setDebugName(const std::string& name) {
    m_device.setObjectName(m_set, name);
}
