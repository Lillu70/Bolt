#pragma once
#include "Vk_Internal_Components.h"
#include "../Core/Bolt_Types.h"

namespace Bolt
{
	class Descriptor_Pool_Bucket
	{
	public:
		void Init(VkDevice device, u32 bucket_size, std::vector<VkDescriptorSetLayoutBinding>& layout_bindings, std::vector<VkDescriptorPoolSize>& pool_sizes);
		void Dest();

		void Allocate(VkDescriptorSet& output_set);
		
		VkDescriptorSetLayout Get_Layout() { return m_layout; }
	
	private:
		VkDescriptorPool& Create_Pool();

	private:
		std::vector<VkDescriptorPoolSize> m_pool_sizes;
		std::vector<VkDescriptorPool> m_pools;
		VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		u32 m_bucket_size = 0;
	};

	enum class Descriptor_Types
	{
		Material,
		Scene,
		COUNT,
	};

	class Vk_Descriptor_Allocator final
	{
	public:
		void Init(VkDevice device, u32 frames_in_flight);
		void Dest();

		void Allocate(Descriptor_Types type, VkDescriptorSet& output_set);
		
		VkDescriptorSetLayout Get_Layout(Descriptor_Types type);

	private:
		void Init_Material_Pools();
		void Init_Scene_Pools(u32 frames_in_flight);

	private:
		std::array<Descriptor_Pool_Bucket, u32(Descriptor_Types::COUNT)> m_descriptors;
		VkDevice m_device;
		
		static constexpr u32 s_bucket_size = 10;
	};	
}


