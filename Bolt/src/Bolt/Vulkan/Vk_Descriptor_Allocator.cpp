#include "Vk_Descriptor_Allocator.h"
#include "Vk_Assist.h"
#include "../Core/Assert.h"

void Bolt::Vk_Descriptor_Allocator::Init(VkDevice device, u32 frames_in_flight)
{
	ASSERT(m_device == VK_NULL_HANDLE, "Descriptor Allocator already Initialized!");
	ASSERT(device, "Device is a nullptr!");

	m_device = device;

	Init_Material_Pools();
	Init_Scene_Pools(frames_in_flight);
}


void Bolt::Vk_Descriptor_Allocator::Dest()
{
	ASSERT(m_device, "Descriptor Allocator NOT Initialized!");

	for (auto& descriptor : m_descriptors)
		descriptor.Dest();
}

void Bolt::Vk_Descriptor_Allocator::Allocate(Descriptor_Types type, VkDescriptorSet& output_set)
{
	ASSERT(output_set == VK_NULL_HANDLE, "Descriptor set already allocated!");
	ASSERT(u32(type) < u32(Descriptor_Types::COUNT), "Invalid Descriptor type!");
	m_descriptors[u32(type)].Allocate(output_set);
}

VkDescriptorSetLayout Bolt::Vk_Descriptor_Allocator::Get_Layout(Descriptor_Types type)
{
	ASSERT(u32(type) < u32(Descriptor_Types::COUNT), "Invalid Descriptor type!");
	return m_descriptors[u32(type)].Get_Layout();
}

void Bolt::Vk_Descriptor_Allocator::Init_Material_Pools()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayoutBinding& sampler_binding = bindings.emplace_back();
	sampler_binding.binding = 0;
	sampler_binding.descriptorCount = 1;
	sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_binding.pImmutableSamplers = nullptr;
	sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding& material_ub_binding = bindings.emplace_back();
	material_ub_binding.binding = 1;
	material_ub_binding.descriptorCount = 1;
	material_ub_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	material_ub_binding.pImmutableSamplers = nullptr;
	material_ub_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorPoolSize> pool_sizes;
	VkDescriptorPoolSize& pz_sampler = pool_sizes.emplace_back();
	pz_sampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pz_sampler.descriptorCount = s_bucket_size;

	VkDescriptorPoolSize& pz_material_ub = pool_sizes.emplace_back();
	pz_material_ub.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pz_material_ub.descriptorCount = s_bucket_size;

	m_descriptors[u32(Descriptor_Types::Material)].Init(m_device, s_bucket_size, bindings, pool_sizes);
}

void Bolt::Vk_Descriptor_Allocator::Init_Scene_Pools(u32 frames_in_flight)
{
	const u32 count = frames_in_flight * 1;

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayoutBinding& layout_binding = bindings.emplace_back();
	layout_binding.binding = 0;
	layout_binding.descriptorCount = 1;
	layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorPoolSize> pool_sizes;
	VkDescriptorPoolSize& ps = pool_sizes.emplace_back();
	ps.descriptorCount = count;
	ps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	m_descriptors[u32(Descriptor_Types::Scene)].Init(m_device, count, bindings, pool_sizes);
}

void Bolt::Descriptor_Pool_Bucket::Init(VkDevice device, u32 bucket_size, std::vector<VkDescriptorSetLayoutBinding>& layout_bindings, std::vector<VkDescriptorPoolSize>& pool_sizes)
{
	ASSERT(m_device == VK_NULL_HANDLE, "Descriptor_Pool_Bucket already initialized!");

	m_device = device;
	m_bucket_size = bucket_size;
	m_pool_sizes = pool_sizes;

	Vk_Init::Create_Descriptor_Set_Layout(m_device, layout_bindings, m_layout);
	
	Create_Pool();
}

void Bolt::Descriptor_Pool_Bucket::Dest()
{
	ASSERT(m_device != VK_NULL_HANDLE, "Descriptor_Pool_Bucket NOT initialized!");

	vkDeviceWaitIdle(m_device);

	for (VkDescriptorPool pool : m_pools)
		vkDestroyDescriptorPool(m_device, pool, nullptr);

	vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
}

void Bolt::Descriptor_Pool_Bucket::Allocate(VkDescriptorSet& output_set)
{
	ASSERT(m_device, "Descriptor Allocator NOT Initialized!");

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;

	alloc_info.pSetLayouts = &m_layout;
	alloc_info.descriptorPool = m_pools.back();
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

	alloc_info.descriptorPool = Create_Pool();

	if (vkAllocateDescriptorSets(m_device, &alloc_info, &output_set) != VK_SUCCESS)
		ERROR("Descriptor set allocation failed!");
}

VkDescriptorPool& Bolt::Descriptor_Pool_Bucket::Create_Pool()
{
	Vk_Init::Create_Descriptor_Pool(m_device, m_layout, m_pool_sizes, m_bucket_size, m_pools.emplace_back());
	return m_pools.back();
}