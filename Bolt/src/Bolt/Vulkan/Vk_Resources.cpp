#include "Vk_Resources.h"
#include "../Core/Assert.h"

#include <tiny_obj_loader.h>
#include <stb_image.h>


std::pair<std::vector<Bolt::Vertex>, std::vector<uint32_t>> Bolt::Mesh::Load_From_OBJ(const char* file_path)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, file_path);

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.m_position = {
                attributes.vertices[3 * index.vertex_index + 0],
                attributes.vertices[3 * index.vertex_index + 1],
                attributes.vertices[3 * index.vertex_index + 2]
            };

            vertex.m_normal = {
                attributes.normals[3 * index.normal_index + 0],
                attributes.normals[3 * index.normal_index + 1],
                attributes.normals[3 * index.normal_index + 2]
            };

            vertex.m_uv = {
                attributes.texcoords[2 * index.texcoord_index + 0],
                1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
            };

            uint32_t index;
            auto find = unique_vertices.find(vertex);
            if (find == unique_vertices.end())
            {
                index = static_cast<uint32_t>(vertices.size());
                unique_vertices[vertex] = index;
                vertices.push_back(vertex);
            }
            else
                index = find->second;

            indices.push_back(index);
        }
    }


    return std::make_pair(vertices, indices);;
}


std::vector<char> Bolt::Shader::Read_Byte_Code(const std::string& file_name)
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
