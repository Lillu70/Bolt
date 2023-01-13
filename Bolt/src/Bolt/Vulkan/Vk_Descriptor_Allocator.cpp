#include "Vk_Descriptor_Allocator.h"
#include "Vk_Assist.h"
#include "../Core/Assert.h"

void Bolt::Vk_Descriptor_Allocator::Init(VkDevice device, uint32_t frames_in_flight)
{
	ASSERT(m_device == VK_NULL_HANDLE, "Descriptor Allocator already Initialized!");
	ASSERT(device, "Device is a nullptr!");

	m_device = device;

	Create_Scene_Descriptor_Layout();
	Create_Scene_Descriptor_Pool(frames_in_flight);

	Create_Material_Descriptor_Layout();

	Create_Material_Descriptor_Pool();
}


void Bolt::Vk_Descriptor_Allocator::Dest()
{
	ASSERT(m_device, "Descriptor Allocator NOT Initialized!");

	vkDeviceWaitIdle(m_device);

	for (VkDescriptorPool pool : m_material_description_pools)
		vkDestroyDescriptorPool(m_device, pool, nullptr);

	vkDestroyDescriptorSetLayout(m_device, m_material_description_layout, nullptr);

	vkDestroyDescriptorPool(m_device, m_scene_ub_pool, nullptr);
	vkDestroyDescriptorSetLayout(m_device, m_scene_ub_layout, nullptr);
}


void Bolt::Vk_Descriptor_Allocator::Allocate(VkDescriptorSet& output_set)
{
	ASSERT(m_device, "Descriptor Allocator NOT Initialized!");

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;

	alloc_info.pSetLayouts = &m_material_description_layout;
	alloc_info.descriptorPool = m_material_description_pools.back();
	alloc_info.descriptorSetCount = 1;

	switch (vkAllocateDescriptorSets(m_device, &alloc_info, &output_set))
	{
	case VK_SUCCESS:
		return;

	case VK_ERROR_FRAGMENTED_POOL:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		//reallocate pool
		break;

	default:
		ERROR("Descriptor set allocation failed!");
	}

	Create_Material_Descriptor_Pool();
	alloc_info.descriptorPool = m_material_description_pools.back();

	if (vkAllocateDescriptorSets(m_device, &alloc_info, &output_set) != VK_SUCCESS)
		ERROR("Descriptor set allocation failed!");
}


void Bolt::Vk_Descriptor_Allocator::Allocate(Frame_Data& output_frame)
{
	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;

	alloc_info.pSetLayouts = &m_scene_ub_layout;
	alloc_info.descriptorPool = m_scene_ub_pool;
	alloc_info.descriptorSetCount = 1;

	if (vkAllocateDescriptorSets(m_device, &alloc_info, &output_frame.camera_descriptor) != VK_SUCCESS)
		ERROR("Descriptor set allocation failed!");
}


VkDescriptorSetLayout Bolt::Vk_Descriptor_Allocator::Get_Material_Descriptor_Layout()
{
	return m_material_description_layout;
}


VkDescriptorSetLayout Bolt::Vk_Descriptor_Allocator::Get_Scene_Descriptor_Layout()
{
	return m_scene_ub_layout;
}


void Bolt::Vk_Descriptor_Allocator::Create_Material_Descriptor_Pool()
{
	constexpr uint32_t descriptors_in_pool = 10;

	VkDescriptorPoolSize pz_sampler;
	pz_sampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pz_sampler.descriptorCount = descriptors_in_pool;

	VkDescriptorPoolSize pz_material_ub;
	pz_material_ub.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pz_material_ub.descriptorCount = descriptors_in_pool;

	std::vector<VkDescriptorPoolSize> pool_sizes{ pz_sampler, pz_material_ub };

	Vk_Init::Create_Descriptor_Pool(m_device, m_material_description_layout, pool_sizes, descriptors_in_pool, m_material_description_pools.emplace_back());
}


void Bolt::Vk_Descriptor_Allocator::Create_Material_Descriptor_Layout()
{
	VkDescriptorSetLayoutBinding sampler_binding{};
	sampler_binding.binding = 0;
	sampler_binding.descriptorCount = 1;
	sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_binding.pImmutableSamplers = nullptr;
	sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding material_ub_binding{};
	material_ub_binding.binding = 1;
	material_ub_binding.descriptorCount = 1;
	material_ub_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	material_ub_binding.pImmutableSamplers = nullptr;
	material_ub_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	Vk_Init::Create_Descriptor_Set_Layout(m_device, std::vector<VkDescriptorSetLayoutBinding>{ sampler_binding, material_ub_binding }, m_material_description_layout);
}


void Bolt::Vk_Descriptor_Allocator::Create_Scene_Descriptor_Layout()
{
	VkDescriptorSetLayoutBinding layout_binding;
	layout_binding.binding = 0;
	layout_binding.descriptorCount = 1;
	layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	Vk_Init::Create_Descriptor_Set_Layout(m_device, std::vector<VkDescriptorSetLayoutBinding>{ layout_binding }, m_scene_ub_layout);
}


void Bolt::Vk_Descriptor_Allocator::Create_Scene_Descriptor_Pool(uint32_t frames_in_flight)
{
	VkDescriptorPoolSize ps;
	ps.descriptorCount = frames_in_flight;
	ps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	Vk_Init::Create_Descriptor_Pool(m_device, m_scene_ub_layout, std::vector<VkDescriptorPoolSize>{ ps }, frames_in_flight, m_scene_ub_pool);
}
