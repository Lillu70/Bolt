#include "Vk_Resources.h"
#include "../Core/Assert.h"

#include <stb_image.h>
#include <fstream>


std::vector<char> Bolt::Raw_Shader::Read_Byte_Code(const std::string& file_name)
{
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        ERROR("failed to open file!");

    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}


Bolt::Image_Data Bolt::Texture::Load_Image(const char* file_path)
{
    Image_Data payload;
    payload.pixels = stbi_load(file_path, &payload.width, &payload.height, &payload.channels, STBI_rgb_alpha);
    return payload;
}

void Bolt::Image_Data::Free()
{
    if (pixels)
        stbi_image_free(pixels);
}



void Bolt::Scene_Descriptor::Update(u32 index)
{
	Scene_Uniform_Buffer_Object scene_data(camera_data, enviroment_data);

	Vk_Util::Write_To_Buffer(device, pfd[index].buffer, sizeof(scene_data), &scene_data);

	VkDescriptorBufferInfo buffer_info;
	buffer_info.buffer = pfd[index].buffer.handle;
	buffer_info.offset = 0;
	buffer_info.range = sizeof(Scene_Uniform_Buffer_Object);

	std::array<VkWriteDescriptorSet, 1> writes{};

	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = pfd[index].descriptor_set;
	writes[0].dstBinding = 0;
	writes[0].dstArrayElement = 0;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].descriptorCount = 1;
	writes[0].pBufferInfo = &buffer_info;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void Bolt::Scene_Descriptor::Write_Default_Values_Into_All_Descriptors()
{
	for (u32 i = 0; i < pfd.size(); i++)
		Update(i);
}
