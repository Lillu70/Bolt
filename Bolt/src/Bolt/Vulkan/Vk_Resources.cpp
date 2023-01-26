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
