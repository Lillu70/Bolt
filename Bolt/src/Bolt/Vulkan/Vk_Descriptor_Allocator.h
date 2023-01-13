#pragma once
#include "Vk_Internal_Components.h"

namespace Bolt
{
	class Vk_Descriptor_Allocator final
	{
	public:
		void Init(VkDevice device, uint32_t frames_in_flight);
		void Dest();

		void Allocate(VkDescriptorSet& output_set);
		void Allocate(Frame_Data& output_frame);
		VkDescriptorSetLayout Get_Material_Descriptor_Layout();
		VkDescriptorSetLayout Get_Scene_Descriptor_Layout();

	private:
		void Create_Material_Descriptor_Pool();

		void Create_Material_Descriptor_Layout();
		void Create_Scene_Descriptor_Layout();
		void Create_Scene_Descriptor_Pool(uint32_t frames_in_flight);

	private:
		std::vector<VkDescriptorPool> m_material_description_pools;
		VkDescriptorSetLayout m_material_description_layout;
		VkDescriptorSetLayout m_scene_ub_layout;
		VkDescriptorPool m_scene_ub_pool;
		VkDevice m_device;
	};
}


